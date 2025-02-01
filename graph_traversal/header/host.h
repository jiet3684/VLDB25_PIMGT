#ifndef __STDINT_H
    #define __STDINT_H
    #include <stdint.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <omp.h>
#include <sys/time.h>
#include <time.h>

#include <dpu.h>
#include <dpu_log.h>

#define NUM_THREAD 32

typedef struct timespec timespec;
inline void cur_Time(timespec* t) {
    clock_gettime(CLOCK_MONOTONIC, t);
}
inline float elapsed_Time(timespec st, timespec ed) {
    return 1000.0f * (ed.tv_sec - st.tv_sec) + 0.000001f * (ed.tv_nsec - st.tv_nsec);
}

// typedef struct timeval timeval;
// inline float elapsed_Time(timeval st, timeval ed) {
//     return 1000.0f * (ed.tv_sec - st.tv_sec) + 0.001f * (ed.tv_usec - st.tv_usec);
// }
// inline void cur_Time(timeval *t) {
//     gettimeofday(t, NULL);
// }

struct pairInfo {
	uint32_t index;
	uint32_t length;
};
typedef struct pairInfo pairInfo;

void prepare_final_BFS();
uint32_t* final_BFS(const uint16_t* intra_nnz, const uint16_t* intra_ptr, const uint16_t* intra_idx, const uint16_t max_icn, const uint16_t upB);

void prepare_final_SPF();
uint32_t* final_SPF(const uint16_t* intra_nnz, const uint16_t* intra_ptr, const uint16_t* intra_idx, const uint16_t* intra_val, const uint16_t max_icn, const uint16_t upB);

float* global_BC(float* time);
void finalize_BC(uint8_t* cent_SG, float* cent_Global, int max_node, float* time);

float* global_PR(float* time);
void finalize_PR(float* rank_SG, float* rank_Global);