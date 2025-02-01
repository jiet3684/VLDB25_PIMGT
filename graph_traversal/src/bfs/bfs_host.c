#include "../../header/host.h"
#include "../../header/common.h"

char* visited;
int* queue;
int top = 0, bottom = 0;

void starting_BFS(const int starting_node) {
	uint32_t cur_sg = mat.sg_info[starting_node];
	uint32_t first_node = sg_Sizes[cur_sg];
	visited[starting_node] = 1;
	int temp_top = 0, temp_bottom = 0;
	int* temp_queue = malloc(SGs[cur_sg].num_node);
	temp_queue[top++] = starting_node;

	while (top > bottom) {
		int cur_node = temp_queue[temp_bottom++];

		if (is_ICN[cur_node]) {
			queue[top++] = cur_node;
		}

		for (int e = mat.ptr[cur_node]; e < mat.ptr[cur_node + 1]; ++e) {
			uint32_t dst_node = mat.idx[e];
			if (dst_node >= first_node && dst_node < first_node + SGs[cur_sg].num_node && !visited[dst_node]) {
				temp_queue[temp_top++] = dst_node;
				visited[dst_node] = 1;
			}
		}
	}
	
}

void prepare_final_BFS() {
    int starting_node;
	queue = malloc(mat.nr << 2);
	visited = calloc(mat.nr , 1);
	for (int i = 0; i < mat.nr; ++i) {
		if (is_ICN[i]) {
			starting_node = i;
			break;
		}
	}
	// printf("  -- Final Traversal\n\tStarting node is %d, ICN?: %d\n", starting_node, is_ICN[starting_node]);

	if (!is_ICN[starting_node]) {
		starting_BFS(starting_node);
	}
	else {
		queue[top++] = starting_node;
		visited[starting_node] = 1;
	}
}

uint32_t* final_BFS(const uint16_t* intra_nnz, const uint16_t* intra_ptr, const uint16_t* intra_idx, const uint16_t max_icn, const uint16_t upB) {
	uint32_t* length = malloc(mat.nr << 2);
	memset(length, 0xff, mat.nr << 2);

	timespec st, ed;
	float t0 = 0.0f, t1 = 0.0f, t2 = 0.0f;
	int count1 = 0, count2 = 0;
	int count11 = 0, count22 = 0;
	int count111 = 0, count222 = 0;
	int len_first = 0, len_last = 0;

	while (top > bottom) {
		uint32_t cur_node = queue[bottom++];
		uint32_t cur_sg = mat.sg_info[cur_node];
		uint32_t intra_node_offset = cur_sg * ALIGN4(max_icn + 1) + mapping_intra[cur_node];
		uint32_t intra_edge_offset = cur_sg * upB + intra_ptr[intra_node_offset];
		int* connected_ICEs = ICEs + mat.ptr[cur_node];

#pragma unroll(8)
		for (int i = intra_edge_offset; i < intra_edge_offset + intra_nnz[intra_node_offset]; ++i) {
			int dst = intra_idx[i] + sg_Sizes[cur_sg];

			if (!visited[dst]) {
				queue[top++] = dst;
				visited[dst] = 1;
			}
		}

#pragma unroll (4)
		for (int i = 0; i < num_ICEs[cur_node]; ++i) {
			int dst = connected_ICEs[i];

			if (!visited[dst]) {
				queue[top++] = dst;
				visited[dst] = 1;
			}
		}

	}
	free(length);
	return length;
}

