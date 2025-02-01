#include "../segment_Graph.h"

csr new_graph;
char *new_ICN;

void reorder_Input(const csr mat, int num_SGs) {
	new_ICN = calloc(mat.nr, 1);
	int* ptr_nodes = calloc(num_SGs + 1, sizeof(int));
	for (int n = 0; n < num_SGs; ++n) {
		ptr_nodes[n + 1] = ptr_nodes[n] + sg_size[n];
	}
	int* ptr_edges = calloc(num_SGs + 1, sizeof(int));
	for (int n = 0; n < mat.nr; ++n) {
		ptr_edges[sg_index[n] + 1] += mat.nnz[n];
	}
	int max_size_node = 0;
	for (int sg = 0; sg < num_SGs; ++sg) {
		if (max_size_node < sg_size[sg]) max_size_node = sg_size[sg];
	}
	int max_size_edge = 0;
	for (int sg = 1; sg <= num_SGs; ++sg) {
		if (max_size_edge < ptr_edges[sg]) max_size_edge = ptr_edges[sg];
	}
	for (int sg = 0; sg < num_SGs; ++sg) {
		ptr_edges[sg + 1] += ptr_edges[sg];
	}

    new_graph.nnz = (int*)malloc((mat.nr + max_size_node) << 2);
    new_graph.ptr = (int*)malloc((mat.nr + max_size_node) << 2);
    new_graph.idx = (int*)malloc((mat.ne + max_size_edge) << 2);
	new_graph.ptr[0] = 0;

	new_graph.nr = mat.nr;
	new_graph.nc = mat.nc;
	new_graph.ne = mat.ne;
	new_graph.pooled_nr = mat.nr + max_size_node;
	new_graph.pooled_ne = mat.ne + max_size_edge;
    new_graph.target = mat.target;

	int* offsets_node = calloc(num_SGs, sizeof(int));
	int* offsets_edge = calloc(num_SGs, sizeof(int));
	int* mapping = malloc(mat.nr * sizeof(int));

	for (int n = 0; n < mat.nr; ++n) {
		int cur_sg = sg_index[n];
		assert(cur_sg >= 0 && cur_sg < num_SGs);
		int node_offset = ptr_nodes[cur_sg] + offsets_node[cur_sg];
		assert(node_offset >= 0 && node_offset < mat.nr);

		new_graph.nnz[node_offset] = mat.nnz[n];
		mapping[n] = node_offset;
		offsets_node[cur_sg]++;
	}
	for (int n = 0; n < mat.nr; ++n) {
		int cur_sg = sg_index[n];
		int edge_offset = ptr_edges[cur_sg] + offsets_edge[cur_sg];

		for (int e = 0; e < mat.nnz[n]; ++e) {
			new_graph.idx[edge_offset + e] = mapping[mat.idx[mat.ptr[n] + e]];
		}
		offsets_edge[cur_sg] += mat.nnz[n];
	}
	for (int n = 0; n < mat.nr; ++n) {
		new_graph.ptr[n + 1] = new_graph.ptr[n] + new_graph.nnz[n];
	}

	for (int i = 0; i < mat.nr; ++i) new_ICN[mapping[i]] = flag_ICN[i];

	free(ptr_nodes);
	free(ptr_edges);
	free(offsets_node);
	free(offsets_edge);
	free(mapping);

	return;
}

void write_to_storage(csr mat, int target, char* filename, char* partname) {
	reorder_Input(mat, target);
	char input[128];
    char num_SGs[8];
	strcpy(input, filename);
	strcat(input, ".pimgt.");
	strcat(input, partname);
	sprintf(num_SGs, ".%d", new_graph.target);
    strcat(input, num_SGs);

	int* ptr_nodes = calloc(target + 1, sizeof(int));
	for (int n = 0; n < target; ++n) {
		ptr_nodes[n + 1] = ptr_nodes[n] + sg_size[n];
	}
	
	FILE *fp = fopen(input, "wb");
	fwrite(&(new_graph.nr), sizeof(int), 1, fp);
	fwrite(&(new_graph.pooled_nr), sizeof(int), 1, fp);
	fwrite(&(new_graph.ne), sizeof(int), 1, fp);
	fwrite(&(new_graph.pooled_ne), sizeof(int), 1, fp);
	fwrite(new_graph.nnz, sizeof(int), new_graph.nr, fp);
	fwrite(new_graph.ptr, sizeof(int), new_graph.nr + 1, fp);
	fwrite(new_graph.idx, sizeof(int), new_graph.ne, fp);
	// fwrite(sg_size, sizeof(int), new_graph.target, fp);
	fwrite(ptr_nodes, sizeof(int), new_graph.target + 1, fp);
	fwrite(new_ICN, 1, new_graph.nr, fp);
	fclose(fp);
	free(new_graph.nnz);
	free(new_graph.ptr);
	free(new_graph.idx);
	free(ptr_nodes);
	free(new_ICN);
}