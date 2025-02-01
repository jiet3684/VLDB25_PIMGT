#include "../segment_Graph.h"

int* sg_index;
int* sg_size;
csr mat;
csr inc_mat;
int* deg;
FILE* fp;
FILE* perf;
int* num_ICNs;
char* flag_ICN;

int segment_base(csr* mat, int target);

int main(int argc, char** argv) {
	struct timeval st, ed;
	clock_t start, end;
    char input[64];
	if (argc < 2) {
		puts(" - Usage -\nBinary_name Input_graph #Subgraphs Refinement_mode");
		puts("(Option) #Subgraphs: Power of 2. Default = 2048.\n(Option) Refinement_mode: 0(HDV) or 1(HDV+R) or 2(Default, HDV+R+B).");
		puts("Example: ./bin/divide ../../dataset/mario002.mtx 2 2048");
		puts("Or, just simply run the check_ICN.sh script :) (Recommended).");
		exit(0);
	}
    strcpy(input, argv[1]);
    strcat(input, ".csr");
    if (access(input, F_OK) < 0) {
        printf("Input CSR File does not exist.\n");

        return 0;
    }

    fp = fopen(input, "rb");
    int row, col, edge;
    fread(&row, sizeof(int), 1, fp);
    fread(&col, sizeof(int), 1, fp);
    fread(&edge, sizeof(int), 1, fp);
    mat.nr = row;
    mat.nc = col;
    mat.ne = edge;
    mat.nnz = (int*)malloc((mat.nr + 1) << 2);
    mat.ptr = (int*)malloc((mat.nr + 1) << 2);
    mat.idx = (int*)malloc(mat.ne << 2);
    fread(mat.nnz, sizeof(int), mat.nr, fp);
    fread(mat.ptr, sizeof(int), mat.nr + 1, fp);
    fread(mat.idx, sizeof(int), mat.ne, fp);

	inc_mat.nr = row;
	inc_mat.nc = col;
    inc_mat.nnz = (int*)malloc((inc_mat.nr + 1) << 2);
    inc_mat.ptr = (int*)malloc((inc_mat.nr + 1) << 2);
    fread(inc_mat.nnz, sizeof(int), inc_mat.nr, fp);
    fread(inc_mat.ptr, sizeof(int), inc_mat.nr + 1, fp);
	inc_mat.ne = inc_mat.ptr[inc_mat.nr];
    inc_mat.idx = (int*)malloc(inc_mat.ne << 2);
    fread(inc_mat.idx, sizeof(int), inc_mat.ne, fp);
    fclose(fp);

	printf("Input Graph: %s\nRow: %d, Col: %d, Edge: %d\n\n", argv[1], mat.nr, mat.nc, mat.ne);
	int target = 2048;
	if (argc >= 3) target = atoi(argv[2]);
	mat.target = target;

    sg_index = malloc(mat.nr << 2);
    sg_size = malloc(target << 2);
    num_ICNs = calloc(target, 4);
	flag_ICN = calloc(mat.nr, 1);
	int total_ICN;

	// For storing the partitioned subgraphs into a file.
	int write_to_file = 0;
	
	

#if METIS
	exec_METIS(argv[1], target, mat);
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "metis");
#elif BPART
	exec_BPart(argv[1], target, mat);
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "bpart");
#elif FENNEL
	exec_Fennel(argv[1], target, mat);
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "fennel");
#elif TOPOX
	exec_TopoX(argv[1], target, mat);
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "topox");
#elif NE
	exec_NE(argv[1], target, mat);
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "ne");
#elif HEP
	exec_HEP(argv[1], target, mat);
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "hep");
#elif FSM
	exec_FSM(argv[1], target, mat);
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "fsm");
#elif HDV
	refine mode = BALANCE_ADJUSTMENT;
	if (argc == 4) mode = atoi(argv[3]);
	half_Division(inc_mat, target, mode);
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "hdv");
#elif DEBUG
	fp = fopen("./quality.csv", "at");
	strcpy(input, argv[1]);
	char* ptr;
	ptr = strstr(input, "dataset");
	ptr = strtok(ptr, "/");
	ptr = strtok(NULL, ".");
	perf = fopen("./performance.csv", "at");
	fprintf(perf, "%s,", ptr);
	fclose(perf);

	
	exec_METIS(argv[1], target, mat);
	puts("METIS.");
	fprintf(fp, "%s,METIS,", ptr);
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "metis");

	exec_BPart(argv[1], target, mat);
	puts("BPart.");
	fprintf(fp, ",BPart,");
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "bpart");

	exec_Fennel(argv[1], target, mat);
	puts("Fennel.");
	fprintf(fp, ",Fennel,");
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "fennel");

	exec_NE(argv[1], target, mat);
	puts("NE.");
	fprintf(fp, ",NE,");
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "ne");

	exec_TopoX(argv[1], target, mat);
	puts("TopoX.");
	fprintf(fp, ",TopoX,");
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "topox");

	exec_HEP(argv[1], target, mat);
	puts("HEP.");
	fprintf(fp, ",HEP,");
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "hep");

	exec_FSM(argv[1], target, mat);
	puts("FSM.");
	fprintf(fp, ",FSM,");
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "fsm");

	puts("HDV.");
	fprintf(fp, ",HDV,");
	target = half_Division(inc_mat, target, NO_ADJUSTMENT);
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "hdv");

	puts("HDV+A.");
	fprintf(fp, ",HDV+A,");
	target = half_Division(inc_mat, target, ICN_ADJUSTMENT);
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "hdv+a");

	puts("HDV+A+B.");
	fprintf(fp, ",HDV+A+B,");
	target = half_Division(inc_mat, target, BALANCE_ADJUSTMENT);
	total_ICN = find_ICN(mat, num_ICNs);
	check_balance(mat, total_ICN, num_ICNs);
	if (write_to_file) write_to_storage(mat, target, argv[1], "hdv+a+b");
	puts("=========================================================================================================\n");

	fclose(fp);
#endif

	
	
	free(mat.nnz);
	free(mat.ptr);
	free(mat.idx);
	free(inc_mat.nnz);
	free(inc_mat.ptr);
	free(inc_mat.idx);
	free(sg_index);
	free(sg_size);
	free(deg);
	free(num_ICNs);
	free(flag_ICN);
}

int segment_base(csr* mat, int target) {
	int up = (mat->nr / target) + 1;
	sg_index = malloc(mat->nr * sizeof(int));
	memset(sg_index, 0xff, mat->nr * sizeof(int));
	sg_size = calloc(target, sizeof(int));
	srand(time(NULL));

	int sg = 0;
	for (int i = 0; i < mat->nr; ++i) {
		sg_index[i] = sg;
		sg_size[sg]++;
		sg = sg == target - 1 ? 0 : sg + 1;
	}
	return target;
}
