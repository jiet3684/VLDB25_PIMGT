/**
 * @file bfs_kernel.c
 * @author Taehyeong Park (taehyeongpark@yonsei.ac.kr)
 * @brief Graph-partitioning BFS on UPMEM PIM
 * 
 * @copyright Copyright (c) 2023
 */

#include <attributes.h>
#include <stdio.h>
#include <stdint.h>
#include <defs.h>
#include <mram.h>
#include <mram_unaligned.h>
#include <alloc.h>
#include <barrier.h>
#include <mutex.h>
#include <seqread.h>
#include "../../header/common.h"

// #ifdef SEQREAD_CACHE_SIZE
//     #undef SEQREAD_CACHE_SIZE
//     #define SEQREAD_CACHE_SIZE 64
// #endif

BARRIER_INIT(barrier, NR_TASKLETS);
MUTEX_INIT(mutex);

__host argument arg;
uint32_t *icn_ptr;
uint8_t *icn_flag;
uint32_t icn_offset = NR_TASKLETS;
uint16_t *output_ptr;
uint16_t* output_nnz;

#define BUF_SIZE 128
#define COLLECTING_SIZE 120

int main() {
    if (arg.num_icn == 0 || arg.num_node == 0) return 0;
    uint32_t tid = me();

    uint32_t output_idx = (uint32_t)(DPU_MRAM_HEAP_POINTER) + (MUL2_ALIGN8(arg.max_icn + 1) << 1);
    uint32_t ptr_addr = output_idx + (arg.max_icn * MUL2_ALIGN8(arg.max_icn) << 1);
    uint32_t idx_addr = (uint32_t)(ptr_addr + MUL4_ALIGN8(arg.max_node + 1));

    uint32_t *current_edge;

    if (tid == 0) {
        mem_reset();
        uint32_t icn_addr = (uint32_t)(idx_addr + MUL4_ALIGN8(arg.max_edge));
        icn_ptr = mem_alloc(MUL4_ALIGN8(arg.num_icn));
        mram_read((__mram_ptr void const *)(icn_addr), icn_ptr, MUL4_ALIGN8(arg.num_icn));

		output_ptr = mem_alloc(MUL2_ALIGN8(arg.num_icn + 1));
        output_nnz = mem_alloc(MUL2_ALIGN8(arg.num_icn + 1));

        uint32_t size_flag_aligned = ALIGN8(arg.num_node);
        uint32_t flag_addr = (uint32_t)(icn_addr + MUL4_ALIGN8(arg.max_icn));
        icn_flag = mem_alloc(size_flag_aligned);
        mram_read((__mram_ptr void const *)(flag_addr), icn_flag, size_flag_aligned);
        // for (int i = 0; i < arg.num_icn + 1; ++i) output_ptr[i] = 0;
    }
    barrier_wait(&barrier);

    uint32_t *cache_ptr = mem_alloc(16);
    mram_read((__mram_ptr void const *)(ptr_addr), cache_ptr, 8);
    uint32_t start_edge = cache_ptr[0];

    seqreader_buffer_t cache_idx = seqread_alloc();
    seqreader_t seq_idx;

    int8_t* visited = mem_alloc(arg.num_node);
    uint16_t queue_size = ((61440 - (arg.num_icn << 3) - (arg.num_node << 1) - ((BUF_SIZE << 1) * NR_TASKLETS)) / NR_TASKLETS) >> 2;
    if (arg.num_node < queue_size) queue_size = arg.num_node;
    uint16_t* queue = mem_alloc(queue_size << 1);

    uint16_t* idx_buf = mem_alloc(BUF_SIZE);

    uint16_t current_icn = (uint16_t)tid;

    for (int n = 0; n < queue_size; ++n) {
        visited[n] = 0;
    }

    while (current_icn < arg.num_icn) {
        uint16_t top = 0, bottom = 0;
        uint16_t offset_buf = 0, offset_flush = 0;
        uint16_t start_icn = icn_ptr[current_icn] - arg.start_node;
        assert(start_icn >= 0 && start_icn < arg.num_node && icn_flag[start_icn]);
        queue[top++] = start_icn;
        visited[start_icn] = 1;

        // Start Breadth-First Search
        while (top > bottom) {
            uint16_t cur_node = queue[bottom++];
            uint32_t node_addr_aligned = ADDR_ALIGN8((uint32_t)cur_node << 2);
            uint32_t num_edge;
            if ((cur_node << 2) > node_addr_aligned) {
                mram_read((__mram_ptr void const *)(ptr_addr + node_addr_aligned), cache_ptr, 16);
                num_edge = cache_ptr[2] - cache_ptr[1];
                current_edge = seqread_init(cache_idx, (__mram_ptr void *)(idx_addr + ((cache_ptr[1] - start_edge) << 2)), &seq_idx);
            }
            else {
                mram_read((__mram_ptr void const *)(ptr_addr + node_addr_aligned), cache_ptr, 8);
                num_edge = cache_ptr[1] - cache_ptr[0];
                current_edge = seqread_init(cache_idx, (__mram_ptr void *)(idx_addr + ((cache_ptr[0] - start_edge) << 2)), &seq_idx);
            }

            if (icn_flag[cur_node] && cur_node != start_icn) {
                idx_buf[offset_buf] = cur_node;
                offset_buf++;

                if (offset_buf == (BUF_SIZE >> 1)) {
                    mram_write(idx_buf, (__mram_ptr void *)(output_idx + current_icn * MUL2_ALIGN8(arg.max_icn) + ((uint32_t)offset_flush << 1)), BUF_SIZE);
                    offset_flush += offset_buf;
                    offset_buf = 0;
                }
                // continue;
            }
            
            for (int e = 0; e < num_edge; ++e) {
                int32_t node_dst = (int32_t)(*current_edge) - (int32_t)arg.start_node;
                current_edge = seqread_get(current_edge, sizeof(*current_edge), &seq_idx);
                if (node_dst >= 0 && node_dst < arg.num_node && !visited[node_dst]) {
					queue[top++] = node_dst;
                    visited[node_dst] = 1;
                }
            }
        }
        output_nnz[current_icn] = offset_flush + offset_buf;
        
        if (offset_buf > 0) {
            mram_write(idx_buf, (__mram_ptr void *)(output_idx + current_icn * MUL2_ALIGN8(arg.max_icn) + ((uint32_t)offset_flush << 1)), MUL2_ALIGN8(offset_buf));
        }

        for (int n = 0; n < top; ++n) {
            visited[queue[n]] = 0;
        }
        // printf("current: %d, num: %d\n", start_icn, output_ptr[start_icn + 1]);

        mutex_lock(mutex);
        current_icn = icn_offset;
        icn_offset++;
        mutex_unlock(mutex);
    }
    barrier_wait(&barrier);

    if (tid == 0) {
        output_ptr[0] = 0;
        for (int i = 0; i < arg.num_icn; ++i) output_ptr[i + 1] = output_ptr[i] + ALIGN4(output_nnz[i]);
        mram_write(output_ptr, (__mram_ptr void *)(DPU_MRAM_HEAP_POINTER), MUL2_ALIGN8(arg.num_icn + 1));
    }
    else if (tid == 1) {
        mram_write(output_nnz, (__mram_ptr void *)(DPU_MRAM_HEAP_POINTER + MUL2_ALIGN8(arg.max_icn + 1)), MUL2_ALIGN8(arg.num_icn));
        icn_offset = NR_TASKLETS;
    }
    barrier_wait(&barrier);

	// Collecting
    current_icn = (uint16_t)tid;
    while (current_icn < arg.num_icn) {
        uint32_t idx_source = output_idx + current_icn * MUL2_ALIGN8(arg.max_icn);
        uint32_t idx_target = output_idx + arg.max_icn * MUL2_ALIGN8(arg.max_icn) + (output_ptr[current_icn] << 1);
        uint16_t size_edge = output_nnz[current_icn] << 1;

        uint16_t offset = 0;
        for (int i = 0; i < size_edge / BUF_SIZE; ++i) {
            mram_read((__mram_ptr void const *)(idx_source + offset), idx_buf, BUF_SIZE);
            mram_write(idx_buf, (__mram_ptr void *)(idx_target + offset), BUF_SIZE);
            offset += BUF_SIZE;
        }

        size_edge = size_edge - offset;
        if (size_edge > 0) {
            mram_read((__mram_ptr void const *)(idx_source + offset), idx_buf, ALIGN8(size_edge));
            mram_write(idx_buf, (__mram_ptr void *)(idx_target + offset), ALIGN8(size_edge));
        }

        mutex_lock(mutex);
        current_icn = icn_offset;
        icn_offset++;
        mutex_unlock(mutex);
    }

    return 0;
}
