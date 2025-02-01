#include "../segment_Graph.h"

int criteria = 0;
int total_ICE;

int find_ICN(csr mat, int* num_ICN) {
	memset(num_ICN, 0, mat.target << 2);
	memset(flag_ICN, 0, mat.nr);

	int total_ICN = 0;
	total_ICE = 0;
	for (int i = 0; i < mat.nr; ++i) {
		for (int e = mat.ptr[i]; e < mat.ptr[i + 1]; ++e) {
			int dst = mat.idx[e];
			if (sg_index[i] != sg_index[dst]) {
				total_ICE++;
				if (flag_ICN[i] == 0) {
					flag_ICN[i] = 1;
					num_ICN[sg_index[i]]++;
					total_ICN++;
				}
				if (flag_ICN[dst] == 0) {
					flag_ICN[dst] = 1;
					num_ICN[sg_index[dst]]++;
					total_ICN++;
				}
			}
		}
	}
	// printf("Ratio of ICN is %.2f, ICE is %.2f\n", (float)100 * total_ICN / mat.nr, (float)100 * total_ICE / mat.ne);

	return total_ICN;
}

int check_Replication(csr mat) {
	char* is_connected = malloc(mat.target);
	int replication = 0;

	for (int i = 0; i < mat.nr; ++i) {
		memset(is_connected, 0, mat.target);
		int cur_sg = sg_index[i];

		for (int e = mat.ptr[i]; e < mat.ptr[i + 1]; ++e) {
			int target_sg = sg_index[mat.idx[e]];
			if (target_sg != cur_sg && is_connected[target_sg] == 0) {
				is_connected[target_sg] = 1;
				replication++;
			}
		}
	}
	printf("Node-Cut Replication: %.3f\n\n", (float)(mat.nr + replication) / mat.nr);
	free(is_connected);
	return replication;
}

void check_balance(csr mat, int total_ICN, int* num_ICN) {
	int* temp_Node = malloc(mat.target << 2);
	int* temp_ICN = malloc(mat.target << 2);
	int avg_Node = mat.nr / mat.target;
	int avg_ICN = total_ICN / mat.target;

	memcpy(temp_Node, sg_size, mat.target << 2);
	memcpy(temp_ICN, num_ICN, mat.target << 2);
	sort_int(temp_Node, 0, mat.target - 1);
	sort_int(temp_ICN, 0, mat.target - 1);



#ifdef DEBUG
	fprintf(fp, "%.4f,%.4f,%.4f,%d,%d,%.4f,%d,%d\n", (float)total_ICN / mat.nr, (float)(mat.ne + total_ICE) / mat.ne, (float)temp_ICN[0] / ((float)total_ICN / mat.target), temp_ICN[0], total_ICN / mat.target, (float)temp_Node[0] / ((float)mat.nr / mat.target), temp_Node[0], mat.nr / mat.target);
#endif
	printf(" - The quality of partitioned subgraphs -\n");
	printf("\t100%%\t75%%\t50%%\t25%%\t0%%\tAVG\n");
	printf("Node :\t%d\t%d\t%d\t%d\t%d\t%d\n", temp_Node[0], temp_Node[mat.target >> 2], temp_Node[mat.target >> 1], temp_Node[(mat.target >> 2) * 3], temp_Node[mat.target - 1], avg_Node);
	printf("ICN  :\t%d\t%d\t%d\t%d\t%d\t%d\n", temp_ICN[0], temp_ICN[mat.target >> 2], temp_ICN[mat.target >> 1], temp_ICN[(mat.target >> 2) * 3], temp_ICN[mat.target - 1], avg_ICN);
    printf("ICN Imbalance Factor: %.4f\n", (float)temp_ICN[0] / ((float)total_ICN / mat.target));
	printf("Edge Replication Factor: %.4f\n", (float)(mat.ne + total_ICE) / mat.ne);
	// printf("Num ICE: %d, Edge: %d\n", total_ICE, mat.ne);
	printf("Ratio of ICN: %.2f%%\nRatio of ICE: %.2f%%\n\n", (float)100 * total_ICN / mat.nr, (float)100 * total_ICE / mat.ne);

	free(temp_Node);
	free(temp_ICN);
}
