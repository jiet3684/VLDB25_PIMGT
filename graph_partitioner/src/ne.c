#include "../segment_Graph.h"

int segment_ne(csr mat, int num_DPUs) {
	// printf("row: %d, col: %d, edge: %d\n\n", mat.nr, mat.nc, mat.ne);
    int target = num_DPUs;
    int avg_node_per_SG = mat.nr / target;
    pairInfo* hub_nodes = make_SortedPair(&mat);
    int* queue = malloc(sizeof(int) * mat.nr);
    char* visited = calloc(mat.nr, sizeof(char));
    sg_index = malloc(mat.nr * sizeof(int));
    memset(sg_index, 0xff, mat.nr * sizeof(int));
    pairInfo* degree = malloc(mat.nr << 3);
    int* mapping = malloc(mat.nr << 2);
    sg_size = calloc(target, sizeof(int));
    int* visited_nodes = malloc(mat.nr << 2);

    timeval st, ed;

    float range = 0.01;
    // int low = floor((1 - 5 * range) * avg_node_per_SG);
    // int high = ceil((1 + range) * avg_node_per_SG);
    int low = avg_node_per_SG + 1;
    int high = avg_node_per_SG;
    int num_large = mat.nr - ((mat.nr / target) * target);
    if (!num_large) low--;

    int num_sg = 0;
    cur_Time(&st);
    while (num_sg < target) {
        int hub_offset = 0;
        for (int i = 0; i < mat.nr; ++i) {
            int starting_node = hub_nodes[hub_offset++].index;
            if (sg_index[starting_node] != 0xffffffff) continue;
            int size = 0;
            memset(mapping, 0xff, mat.nr << 2);
            size = push(degree, mapping, size, starting_node, 1);

            while (size > 0) {
                pairInfo cur_node = pop(degree, mapping, size - 1);
                size--;
                if (visited[cur_node.index]) continue;
                visited[cur_node.index] = 1;
                // visited_nodes[sg_size[num_sg]] = cur_node.index;
                sg_size[num_sg]++;
                sg_index[cur_node.index] = num_sg;
                if (sg_size[num_sg] >= low) break;

                for (int e = mat.ptr[cur_node.index]; e < mat.ptr[cur_node.index + 1]; ++e) {
                    int dst = mat.idx[e];
                    if (mapping[dst] >= 0 && !visited[dst] ) {
                        degree[mapping[dst]].nnz++;
                        heapify(degree, mapping, mapping[dst]);
                    }
                    else if (mapping[dst] == -1 && !visited[dst] && sg_index[dst] == 0xffffffff){
                        size = push(degree, mapping, size, dst, 1);
                    }
                }
            }
            if (sg_size[num_sg] >= low) {
                num_sg++;
                if (num_sg == num_large) low--;
            }
            // if (sg_size[num_sg] < low) {
            //     for (int j = 0; j < sg_size[num_sg]; ++j) {
            //         visited[visited_nodes[j]] = 0;
            //     }
            //     sg_size[num_sg] = 0;
            // }
            // else {
            //     for (int j = 0; j < sg_size[num_sg]; ++j) {
            //         sg_index[visited_nodes[j]] = num_sg;
            //     }
            //     num_sg++;
            //     if (num_sg == target) break;
            // }
        }
        // printf("#SG is %d\n", num_sg);
    }

    // int total = 0;
    // for (int i = 0; i < num_sg; ++i) {
    //     total += sg_size[i];
    // }
    // printf("In-SG: %d, Not-in-SG: %d\n", total, mat.nr - total);


    // printf("\nSecond Phase\n");
    // pairInfo* connected_SG = malloc(target << 3);
    // for (int i = 0; i < mat.nr; ++i) {
    //     int starting_node = hub_nodes[i].index;
    //     if (sg_index[starting_node] != 0xffffffff) continue;

    //     int max_space = 0x7fffffff;
    //     for (int sg = 0; sg < target; ++sg) if (max_space > sg_size[sg]) max_space = sg_size[sg];
    //     max_space = high - max_space;

    //     int top = 0, bottom = 0;
    //     queue[top++] = starting_node;
    //     visited[starting_node] = 1;
    //     memset(connected_SG, 0, target << 3);
    //     int flag_con = 0;

    //     while (top > bottom) {
    //         int cur_node = queue[bottom++];
    //         for (int e = mat.ptr[cur_node]; e < mat.ptr[cur_node + 1]; ++e) {
    //             int dst = mat.idx[e];
    //             if (!visited[dst]) {
    //                 queue[top++] = dst;
    //                 visited[dst] = 1;
    //             }
    //             else if (sg_index[dst] != 0xffffffff) {
    //                 connected_SG[sg_index[dst]].index = sg_index[dst];
    //                 connected_SG[sg_index[dst]].nnz++;
    //                 // flag_con = 1;
    //                 if (connected_SG[sg_index[dst]].nnz == 1) flag_con++;
    //             }
    //             if (top >= max_space) {
    //                 bottom = top;
    //                 break;
    //             }
    //         }
    //     }
    //     // printf("top is %d, offset is %d\n", top, flag_con);
    //     int min_size = 0x7fffffff;
    //     int min_index = -1;
    //     if (flag_con) {
    //         sort(connected_SG, 0, target - 1);
    //         for (int j = 0; j < flag_con; ++j) {
    //             if (sg_size[connected_SG[j].index] + top <= high) {
    //                 min_index = connected_SG[j].index;
    //                 break;
    //             }
    //         }
    //     }
    //     if (!flag_con || min_index == -1) {
    //         min_size = 0x7fffffff;
    //         min_index = -1;
    //         for (int j = target - 1; j >= 0; --j) {
    //             if (sg_size[j] < min_size) {
    //                 min_size = sg_size[j];
    //                 min_index = j;
    //             }
    //         }
    //         // for (int j = target - 1; j >= 0; --j) {
    //         //     if (sg_size[j] + top <= avg_node_per_SG) {
    //         //         min_index = j;
    //         //         break;
    //         //     }
    //         // }
    //     }
    //     sg_size[min_index] += top;
    //     for (int j = 0; j < top; ++j) {
    //         sg_index[queue[j]] = min_index;
    //     }
    // }

    int total = 0;
    for (int i = 0; i < target; ++i) {
        // printf("%d: %d\n", i, sg_size[i]);
        total += sg_size[i];
    }
    int* check = calloc(target, 4);
    for (int i = 0; i < mat.nr; ++i) {
        check[sg_index[i]]++;
    }
    for (int i = 0; i < target; ++i) {
        if (check[i] != sg_size[i]) printf("\t%d: %d %d\n", i, check[i], sg_size[i]);
    }
    cur_Time(&ed);
    printf("Time for Neighbor Expansion (NE) : %f sec\n\n", elapsed_Time(st, ed));
#ifdef debug
    fprintf(fp, "%.4f,", elapsed_Time(st, ed));
#endif
    // printf("In-SG: %d, Not-in-SG: %d\n\n", total, mat.nr - total);
    free(hub_nodes);
    free(queue);
    free(visited);
    // free(connected_SG);
    free(degree);
    free(mapping);
    free(visited_nodes);

    return target;
}
