#include "../segment_Graph.h"
#include <omp.h>

int half_Division(csr mat, int target, refine ref) {
    int total_step = log2(target);
    pairInfo* hub_nodes = make_SortedPair(&mat);
    memset(sg_index, 0, mat.nr << 2);
    pairInfo* degree = malloc(mat.nr << 3);
    int* mapping = malloc(mat.nr << 2);
    int* cur_nodes = malloc(mat.nr << 2);
    char* visited = malloc(mat.nr);
    sg_size[0] = mat.nr;
    float range = 0.01f;
    timeval st_total, ed_total;
    timeval st_inn, ed_inn;
    float time_total, time_hdv = 0;

    timeval st_tmp, ed_tmp;
    float t_heap = 0.0f;
    
    if (ref > NO_ADJUSTMENT) prepare_Adjustment(mat, target);

    cur_Time(&st_total);
    for (int step = 0; step < total_step; ++step) {
        cur_Time(&st_inn);
        memset(visited, 0, mat.nr);
        memset(mapping, 0xff, mat.nr << 2);

        for (int sg = (1 << step) - 1; sg >= 0; --sg) {
            int target_size = sg_size[sg] >> 1;
            int low = floor((1 - range) * target_size);
            int high = ceil((1 + range) * target_size);
            int cur_size = 0;
            for (int i = 0; i < mat.nr; ++i) {
                if (sg_index[hub_nodes[i].index] == sg) {
                    cur_nodes[cur_size++] = hub_nodes[i].index;
                }
            }
            assert(cur_size == sg_size[sg]);

            int flag = 0;
            int temp_size[2] = {0, 0};
            while (temp_size[0] < target_size && temp_size[1] < target_size) {
                int cur_sg = (sg << 1) + flag;
                int hub_node;
            // cur_Time(&st_tmp);
                for (int i = 0; i < cur_size; ++i) {
                    if (!visited[cur_nodes[i]]) {
                        hub_node = cur_nodes[i];
                        break;
                    }
                }
                // for (int i = 0; i < mat.nr; ++i) {
                //     if (sg_index[hub_nodes[i].index] == sg && !visited[hub_nodes[i].index]) {
                //         hub_node = hub_nodes[i].index;
                //         break;
                //     }
                // }
            // cur_Time(&ed_tmp);
            // t_heap += elapsed_Time(st_tmp, ed_tmp);

                int size = 0;
                size = push(degree, mapping, size, hub_node, 1);

                int count = 0;
                while (size > 0) {
                    pairInfo cur_node = pop(degree, mapping, size - 1);
                    size--; count++;
                    if (visited[cur_node.index] || sg_index[cur_node.index] != sg) continue;
                    sg_index[cur_node.index] = cur_sg;
                    visited[cur_node.index] = 1;
                    temp_size[flag]++;
                    if (temp_size[flag] >= target_size) {
                        break;
                    }
                    for (int e = mat.ptr[cur_node.index]; e < mat.ptr[cur_node.index + 1]; ++e) {
                        int dst = mat.idx[e];
                        if (sg_index[dst] == sg && !visited[dst]) {
                            if (mapping[dst] >= 0) {
                                degree[mapping[dst]].nnz++;
                                heapify(degree, mapping, mapping[dst]);
                            }
                            else if (mapping[dst] == -1) {
                                size = push(degree, mapping, size, dst, 1);
                            }
                        }
                    }
                }
                flag = 1 - flag;
            }
            if (temp_size[flag] < target_size) {
                for (int i = 0; i < cur_size; ++i) {
                    int node = cur_nodes[i];
                    if (!visited[node]) {
                        sg_index[node] = (sg << 1) + flag;
                        visited[node] = 1;
                        temp_size[flag]++;
                    }
                }
                // for (int i = 0; i < mat.nr; ++i) {
                //     if (sg_index[i] == sg && !visited[i]) {
                //         sg_index[i] = (sg << 1) + flag;
                //         visited[i] = 1;
                //         temp_size[flag]++;
                //     }
                // }
            }
            sg_size[(sg << 1)] = temp_size[0];
            sg_size[(sg << 1) + 1] = temp_size[1];

            // printf("\tDivide Subgraph %d: %d, %d\n", sg, sg_size[sg << 1], sg_size[(sg << 1) + 1]);
        }
        cur_Time(&ed_inn);
        time_hdv += elapsed_Time(st_inn, ed_inn);
        if (ref >= ICN_ADJUSTMENT) ICN_Adjustment(mat, step + 1);
    }
    if (ref == BALANCE_ADJUSTMENT) ICN_Balancing(mat);
    cur_Time(&ed_total);
    time_total = elapsed_Time(st_total, ed_total);

    if (ref > NO_ADJUSTMENT) finish_Adjustment();
    printf("Half-DiVision:\t%.2f sec\n", time_hdv);
    printf("Total Time:\t%.2f sec\n\n", time_total);
    // printf("Heapify Time:\t%.2f sec\n\n", t_heap);

#ifdef DEBUG
    perf = fopen("./performance.csv", "at");
    fprintf(perf, "%.4f,", time_total);
    fclose(perf);
#endif

    free(hub_nodes);
    free(degree);
    free(mapping);
    free(visited);
    free(cur_nodes);
    return target;
}
