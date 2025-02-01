#include "../segment_Graph.h"

#define RANGE 0.02f
int NUM_ADJUSTMENT = 300;
#define BALANCE_ADJUSTMENT 30000
#define BALANCE_RANGE 0.05f


char* is_ICN;
uint16_t* num_ICN;
uint16_t* to_degree;

int cur_step;
int target;

timeval st, ed;
float time_memset = 0.0f, time_refine = 0.0f, time_balance = 0.0f;

// inline void move_Global(csr mat, int from, int to, int n, int step) {
//     sg_index[n] = to;
//     sg_size[from]--;
//     sg_size[to]++;
//     num_ICN[from]--;
//     for (int e = mat.ptr[n]; e < mat.ptr[n + 1]; ++e) {
//         if (sg_index[mat.idx[e]] != to) {
//             num_ICN[to]++;
//             break;
//         }
//     }
// #pragma unroll
//     for (int e = mat.ptr[n]; e < mat.ptr[n + 1]; ++e) {
//         if (mat.idx[e] == n) continue;
//         to_degree[(mat.idx[e] << step) + from]--;
//         to_degree[(mat.idx[e] << step) + to]++;
//     }
// }

inline void move_Node(csr mat, int from, int to, int n) {
    sg_index[n] = to;
    sg_size[from]--;
    sg_size[to]++;
    num_ICN[from]--;
    is_ICN[n] = 0;
    if (to_degree[(n << cur_step) + to] < mat.nnz[n]) {
        num_ICN[to]++;
        is_ICN[n] = 1;
    }
    // for (int e = mat.ptr[n]; e < mat.ptr[n + 1]; ++e) {
    //     if (sg_index[mat.idx[e]] != to) {
    //         num_ICN[to]++;
    //         is_ICN[n] = 1;
    //         break;
    //     }
    // }
    for (int e = mat.ptr[n]; e < mat.ptr[n + 1]; ++e) {
        int dst = mat.idx[e];
        if (dst == n) continue;

        if (sg_index[dst] == from && !is_ICN[dst]) {
            is_ICN[dst] = 1;
            num_ICN[from]++;
        }

        if (sg_index[dst] == to && is_ICN[dst]) {
            int found = 0;
            for (int te = mat.ptr[dst]; te < mat.ptr[dst + 1]; ++te) {
                if (sg_index[mat.idx[te]] != to) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                is_ICN[dst] = 0;
                num_ICN[to]--;
            }
        }

        to_degree[(dst << cur_step) + from]--;
        to_degree[(dst << cur_step) + to]++;
    }
}

void ICN_Adjustment(csr mat, int step) {
    cur_step = step;
    target = 1 << step;
    // cur_Time(&st);
    // memset(to_degree, 0, mat.nr << step + 0);
    // cur_Time(&ed);
    // time_memset += elapsed_Time(st, ed);
    if (step > 1) {
        cur_Time(&st);
        memset(to_degree, 0, mat.nr << step);
        cur_Time(&ed);
        time_memset += elapsed_Time(st, ed);
    }

    for (int n = 0; n < mat.nr; ++n) {
        int from = sg_index[n];
        for (int e = mat.ptr[n]; e < mat.ptr[n + 1]; ++e) {
            if (mat.idx[e] == n) continue;
            to_degree[(n << step) + sg_index[mat.idx[e]]]++;
        }
    }

    memset(num_ICN, 0, target << 1);
    memset(is_ICN, 0, mat.nr + 1);
    for (int n = 0; n < mat.nr; ++n) {
        for (int e = mat.ptr[n]; e < mat.ptr[n + 1]; ++e) {
            if (sg_index[n] != sg_index[mat.idx[e]] && mat.idx[e] != n) {
                num_ICN[sg_index[n]]++;
                is_ICN[n] = 1;
                break;
            }
        }
    }

    int avg = mat.nr / target;
    int high = ceil((1 + RANGE) * avg);
    int low = floor((1 - RANGE) * avg);
    // int low = avg;

    cur_Time(&st);
    int i;
    for (i = 0; i < NUM_ADJUSTMENT; ++i) {
        int converged = 1;
        for (int n = 0; n < mat.nr; ++n) {
            int from = sg_index[n];
            if (!is_ICN[n] || sg_size[from] < low) continue;

            int to = -1;
            int second_to = -1;
            int third_to = -1;
            for (int e = mat.ptr[n]; e < mat.ptr[n + 1]; ++e) {
                int temp_to = sg_index[mat.idx[e]];
                if (sg_size[temp_to] > high) continue;  // Check with is_ICN

                if (to_degree[(n << step) + temp_to] > to_degree[(n << step) + from] && num_ICN[from] > num_ICN[temp_to] + 1) {
                    to = temp_to;
                    break;
                }
                else if (second_to == -1 && to_degree[(n << step) + temp_to] == to_degree[(n << step) + from] && num_ICN[from] > num_ICN[temp_to] + 1) {
                    second_to = temp_to;
                    // break;
                }
                else if (second_to == -1 && third_to == -1 && to_degree[(n << step) + temp_to] > to_degree[(n << step) + from]) {
                    third_to = temp_to;
                }
            }
            if (to != -1) move_Node(mat, from, to, n);
            else if (second_to != -1) move_Node(mat, from, second_to, n);
            else if (third_to != -1) move_Node(mat, from, third_to, n);
            // if (to != -1 || second_to != -1) converged = 0;
            if (to != -1 || second_to != -1 || third_to != -1) converged = 0;
        }
        if (converged) break;
    }
    cur_Time(&ed);
    time_refine += elapsed_Time(st, ed);
    // printf("Target: %d. Converged after %d th iteration\n", target, i);
}


// Functions for ICN Balancing
int check_with_criteria_zero(csr mat, int from, int n) {
    for (int e = mat.ptr[n]; e < mat.ptr[n + 1]; ++e) {
        int dst = mat.idx[e];
        if (sg_index[dst] == from && !is_ICN[dst]) return 0;
    }
    return 1;
}

int check_with_criteria_one(csr mat, int from, int n) {
    int count = 0;
    for (int e = mat.ptr[n]; e < mat.ptr[n + 1]; ++e) {
        int dst = mat.idx[e];
        if (sg_index[dst] == from && !is_ICN[dst]) {
            count++;
            if (count == 2) return 0;
        }
    }
    return 1;
}

int check_for_balancing(csr mat, int from, int n) {
    int count = 0;
    for (int e = mat.ptr[n]; e < mat.ptr[n + 1]; ++e) {
        int dst = mat.idx[e];
        if (sg_index[dst] == from && !is_ICN[dst]) {
            count++;
            if (count == 2) break;
        }
    }
    return count;
}

void ICN_Balancing(csr mat) {
    // memset(to_degree, 0, mat.nr * target << 1); // Can be deleted.
    // for (int n = 0; n < mat.nr; ++n) {
    //     int from = sg_index[n];
    //     int flag_ICN = 0;
    //     for (int e = mat.ptr[n]; e < mat.ptr[n + 1]; ++e) {
    //         to_degree[n * target + sg_index[mat.idx[e]]]++;
    //     }
    // }

    // memset(num_ICN, 0, target << 2);
    // int total_ICN = 0;
    // for (int i = 0; i < mat.nr; ++i) {
    //     for (int e = mat.ptr[i]; e < mat.ptr[i + 1]; ++e) {
    //         int dst = mat.idx[e];
    //         if (sg_index[i] != sg_index[dst]) {
    //             num_ICN[sg_index[i]]++;
    //             total_ICN++;
    //             break;
    //         }
    //     }
    // }


    cur_Time(&st);
    int i;
    for (i = 0; i < BALANCE_ADJUSTMENT; ++i) {
        int avg = mat.nr / target;
        int high = ceil((1 + BALANCE_RANGE) * avg);
        int low = floor((1 - RANGE) * avg);

        int max_ICN = 0;
        int from = -1;
        for (int sg = 0; sg < target; ++sg) {
            if (max_ICN < num_ICN[sg]) {
                max_ICN = num_ICN[sg];
                from = sg;
            }
        }

        int first_max = 0x80000001; // INTEGER MINIMUM VALUE
        int first_ICN = 0;
        int first_to = -1;
        int second_max = 0x80000001; // INTEGER MINIMUM VALUE
        int second_ICN = 0;
        int second_to = -1;

        for (int n = 0; n < mat.nr; ++n) {
            if (sg_index[n] != from || !is_ICN[n]) continue;
            
            for (int e = mat.ptr[n]; e < mat.ptr[n + 1]; ++e) {
                int dst = mat.idx[e];
                int temp_to = sg_index[dst];

                if (sg_size[temp_to] > high || num_ICN[from] <= num_ICN[temp_to] + 1) continue;
                int num_nonICN = check_for_balancing(mat, from, n);
                int ICE_decre = to_degree[(n << cur_step) + temp_to] - to_degree[(n << cur_step) + from];
                
                if (first_max < ICE_decre && num_nonICN == 0) {
                    first_max = ICE_decre;
                    first_ICN = n;
                    first_to = temp_to;
                }
                else if (second_max < ICE_decre && num_nonICN == 1) {
                    second_max = ICE_decre;
                    second_ICN = n;
                    second_to = temp_to;
                }
            }
        }

        if (first_to != -1) move_Node(mat, from, first_to, first_ICN);
        else if (second_to != -1) move_Node(mat, from, second_to, second_ICN);
        else break;
    }
    cur_Time(&ed);
    time_balance += elapsed_Time(st, ed);

    // printf("Balancing: Converged after %d th iteration\n", i);
}

void prepare_Adjustment(csr mat, int total_numSG) {
    target = total_numSG;
    is_ICN = malloc(mat.nr + 1);
    num_ICN = malloc(target << 2);
    to_degree = malloc((uint64_t)mat.nr * target << 1);
    memset(to_degree, 0, (uint64_t)mat.nr * target << 1);

    time_memset = time_refine = time_balance = 0;
}

void finish_Adjustment() {
    free(is_ICN);
    free(num_ICN);
    free(to_degree);
    // printf(" - Time Log -\nICN Adjustment:\t%.2f sec\nICN Balancing:\t%.2f sec\n", time_memset + time_refine, time_balance);
    printf(" - Time Log -\nICN Adjustment: %.2f\nICN Balancing: %.2f\n", time_refine, time_balance);
}
