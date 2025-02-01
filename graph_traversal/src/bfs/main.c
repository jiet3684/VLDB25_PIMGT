#include "../../header/host.h"
#include "../../header/common.h"

void initialize(int, char **);

char name[64];
csr mat;
int num_SGs;
int num_DPUs;
int* sg_Sizes;
sgInfo* SGs;
int* ICNs;
int* mapping_intra;
// int* mapping_global;
int* num_ICEs;
int* ICEs;
char* is_ICN;
argument* args;

int main(int argc, char **argv) {
    // Clear cache
	// pid_t pid = fork();
	// if (pid == 0) execlp("sh", "sh", "-c", "echo 3 > /proc/sys/vm/drop_caches", NULL);
	// else wait(NULL);

    // Initialize and read input
    initialize(argc, argv);
    read_CSR(name);

    timespec st, ed;
    float time_dpu_alloc, time_analyze, time_h2d, time_kernel, time_host, time_d2h, time_spf, time_finalize;

    // Allocate DPUs
    dpu_set set, dpu;
    uint32_t i;
    cur_Time(&st);
    DPU_ASSERT(dpu_alloc(num_DPUs, "backend=simulator", &set));
    DPU_ASSERT(dpu_load(set, "./bin/bfs_kernel", NULL));
    cur_Time(&ed);
    time_dpu_alloc = elapsed_Time(st, ed);

    // Analyze the structure of ICNs
    mapping_ICNs();

    uint16_t max_node, max_icn;
    uint32_t max_edge;
    argument* args = malloc(num_SGs * sizeof(argument));
    set_Argument(args, &max_node, &max_icn, &max_edge);


    // Calculate 8-byte aligned sizes of input for parallel transfer
    uint32_t size_node_aligned = MUL4_ALIGN8(max_node + 1);
    uint32_t size_edge_aligned = MUL4_ALIGN8(max_edge);
    uint32_t size_icn_aligned = MUL4_ALIGN8(max_icn);
    uint32_t size_flag_aligned = ALIGN8(max_node);

    uint32_t size_output_nnz = MUL2_ALIGN8(max_icn + 1);
    uint32_t size_output_ptr = MUL2_ALIGN8(max_icn + 1);
	uint32_t size_output_idx = max_icn * MUL2_ALIGN8(max_icn);
    uint32_t dpu_base_addr = size_output_nnz + size_output_ptr + (size_output_idx << 1);


    // Host-to-Device Data Transfer
    cur_Time(&st);
    DPU_FOREACH(set, dpu, i) {
        DPU_ASSERT(dpu_prepare_xfer(dpu, args + i));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "arg", 0, sizeof(argument), DPU_XFER_DEFAULT));

    DPU_FOREACH(set, dpu, i) {
        DPU_ASSERT(dpu_prepare_xfer(dpu, SGs[i].ptr));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, DPU_MRAM_HEAP_POINTER_NAME, dpu_base_addr, size_node_aligned, DPU_XFER_DEFAULT));
    dpu_base_addr += size_node_aligned;

    DPU_FOREACH(set, dpu, i) {
        DPU_ASSERT(dpu_prepare_xfer(dpu, SGs[i].idx));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, DPU_MRAM_HEAP_POINTER_NAME, dpu_base_addr, size_edge_aligned, DPU_XFER_DEFAULT));
    dpu_base_addr += size_edge_aligned;

    DPU_FOREACH(set, dpu, i) {
        DPU_ASSERT(dpu_prepare_xfer(dpu, ICNs + sg_Sizes[i]));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, DPU_MRAM_HEAP_POINTER_NAME, dpu_base_addr, size_icn_aligned, DPU_XFER_DEFAULT));
    dpu_base_addr += size_icn_aligned;

    DPU_FOREACH(set, dpu, i) {
        DPU_ASSERT(dpu_prepare_xfer(dpu, is_ICN + sg_Sizes[i]));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, DPU_MRAM_HEAP_POINTER_NAME, dpu_base_addr, size_flag_aligned, DPU_XFER_DEFAULT));
    dpu_base_addr += size_flag_aligned;

    cur_Time(&ed);
    time_h2d = elapsed_Time(st, ed);


    // Kernel Execution
    cur_Time(&st);
    DPU_ASSERT(dpu_launch(set, DPU_ASYNCHRONOUS));
    // cur_Time(&ed);
    // time_kernel = elapsed_Time(st, ed);
    // cur_Time(&st);
    analyze_ICEs();
    DPU_ASSERT(dpu_sync(set));
    cur_Time(&ed);
    // time_host = elapsed_Time(st, ed);
    time_kernel = elapsed_Time(st, ed);
    
	uint16_t* result_ptr = malloc(size_output_ptr * num_SGs);
	uint16_t* result_nnz = malloc(size_output_nnz * num_SGs);
    cur_Time(&st);
    DPU_FOREACH(set, dpu, i) {
        DPU_ASSERT(dpu_prepare_xfer(dpu, result_ptr + i * ALIGN4(max_icn + 1)));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_FROM_DPU, DPU_MRAM_HEAP_POINTER_NAME, 0, size_output_ptr, DPU_XFER_DEFAULT));

    DPU_FOREACH(set, dpu, i) {
        DPU_ASSERT(dpu_prepare_xfer(dpu, result_nnz + i * ALIGN4(max_icn + 1)));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_FROM_DPU, DPU_MRAM_HEAP_POINTER_NAME, size_output_ptr, size_output_nnz, DPU_XFER_DEFAULT));

    uint32_t upperBound = 0;
    for (int sg = 0; sg < num_SGs; ++sg) {
        upperBound = upperBound < result_ptr[sg * ALIGN4(max_icn + 1) + SGs[sg].num_ICN] ? result_ptr[sg * ALIGN4(max_icn + 1) + SGs[sg].num_ICN] : upperBound;
    }
    uint16_t* result_idx = malloc(num_SGs * (upperBound << 1));

    dpu_base_addr = size_output_nnz + size_output_ptr + size_output_idx;
    DPU_FOREACH(set, dpu, i) {
        DPU_ASSERT(dpu_prepare_xfer(dpu, result_idx + i * upperBound));
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_FROM_DPU, DPU_MRAM_HEAP_POINTER_NAME, dpu_base_addr, upperBound << 1, DPU_XFER_ASYNC));

    prepare_final_BFS();
    DPU_ASSERT(dpu_sync(set));
    cur_Time(&ed);
    time_d2h = elapsed_Time(st, ed);

	cur_Time(&st);
	int* length = final_BFS(result_nnz, result_ptr, result_idx, max_icn, upperBound);
	cur_Time(&ed);
	time_spf = elapsed_Time(st, ed);


    puts("\n  -- Execution Time Log");
    printf("\tAllocate %d DPUs:\t%.2f ms\n", num_DPUs, time_dpu_alloc);
    // printf("\tAnalyze ICN Structure:\t%.2f ms\n", time_analyze);
    printf("\tPIM transfer + Execute:\t%.2f ms\n", time_h2d + time_kernel + time_d2h);
    printf("\t- H-to-D Transfer:\t%.2f ms\n", time_h2d);
    printf("\t- SG + ICE Analysis:\t%.2f ms\n", time_kernel);
    printf("\t- D-to-H Transfer:\t%.2f ms\n", time_d2h);
    printf("\tFinalize Traversal:\t%.2f ms\n", time_spf);
    printf("\tTotal Execution:\t%.2f ms\n\n", time_h2d + time_kernel + time_d2h + time_spf);

    DPU_ASSERT(dpu_free(set));
    free(mat.nnz);
    free(mat.ptr);
    free(mat.idx);
    free(mat.val);
    free(mat.sg_info);
    free(sg_Sizes);
    free(SGs);
    free(is_ICN);
	free(args);
    free(result_ptr);
    free(result_idx);
    free(ICEs);
    free(num_ICEs);
    // free(length);
}

void initialize(int argc, char **argv) {
    if (argc == 2) {
        strcpy(name, argv[1]);
        num_SGs = 256;
        num_DPUs = 256;
    }
    else if (argc == 1) {
        strcpy(name, "dataset/mario001.mtx");
        num_SGs = 256;
        num_DPUs = 256;
    }
    else if (argc == 3) {
        strcpy(name, argv[1]);
        num_SGs = atoi(argv[2]);
        num_DPUs = num_SGs;
    }
    else if (argc == 4) {
        strcpy(name, argv[1]);
        num_SGs = atoi(argv[2]);
        num_DPUs = atoi(argv[3]);
    }
    else {
        puts("Usage:\n1) ./bin/spf input_data\n");
        exit(0);
    }
}

