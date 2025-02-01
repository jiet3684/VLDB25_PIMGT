#include "../segment_Graph.h"

int partition_int(int* arr, int l, int r) {
	int pivot = arr[l];
	int low = l, high = r + 1;
	int temp;

	do {
		do {
			low++;
		} while (low <= r && arr[low]> pivot);

		do {
			high--;
		} while (high >= l && arr[high]< pivot);

		if (low < high) {
			temp = arr[high];
			arr[high] = arr[low];
			arr[low] = temp;
		}
	} while (low < high);

	temp = arr[l];
	arr[l] = arr[high];
	arr[high] = temp;

	return high;
}

void sort_int(int* arr, int l, int r) {
	if (l < r) {
		int q = partition_int(arr, l, r);
		sort_int(arr, l, q - 1);
		sort_int(arr, q + 1, r);
	}
}

int partition(pairInfo* pair, int l, int r) {
	int pivot = pair[l].nnz;
	int low = l, high = r + 1;
	pairInfo temp;

	do {
		do {
			low++;
		} while (low <= r && pair[low].nnz < pivot); // > for large-first

		do {
			high--;
		} while (high >= l && pair[high].nnz > pivot);

		if (low < high) {
			temp = pair[high];
			pair[high] = pair[low];
			pair[low] = temp;
		}
	} while (low < high);

	temp = pair[l];
	pair[l] = pair[high];
	pair[high] = temp;

	return high;
}

void sort(pairInfo* pair, int l, int r) {
	if (l < r) {
		int q = partition(pair, l, r);
		sort(pair, l, q - 1);
		sort(pair, q + 1, r);
	}
}

pairInfo* make_Pair(csr* mat) {
	pairInfo* pair = malloc(mat->nr << 3);
	for (int i = 0; i < mat->nr; ++i) {
		pair[i].nnz = mat->nnz[i];
		pair[i].index = i;
	}
	return pair;
}

pairInfo* make_SortedPair(csr* mat) {
	pairInfo* pair = malloc(mat->nr << 3);
	for (int i = 0; i < mat->nr; ++i) {
		pair[i].nnz = mat->nnz[i];
		pair[i].index = i;
	}
	sort(pair, 0, mat->nr - 1);
	return pair;
}

pairInfo* find_hub_nodes(csr* mat) {
	int top_node = 0;
	int* nnz = mat->nnz;
	int div_factor = 100;
	pairInfo* hub_nodes = malloc(mat->nr << 3);

	for (int n = 0; n < mat->nr; ++n) {
		top_node = top_node >= nnz[n] ? top_node : nnz[n];
		// if (top_node < nnz[n]) top_node = nnz[n];
	}
	//printf("top node is %d\n", top_node);
	int offset = 0;
	int criteria = floor((float)top_node / div_factor);
	int low = criteria * (div_factor - 1);
	int high = top_node + 1;
	for (int i = 0; i < div_factor; ++i) {
		for (int n = 0; n < mat->nr; ++n) {
			if (nnz[n] >= low && nnz[n] < high) {
				hub_nodes[offset++].index = n;
			}
		}
		//printf("iter %d. %d ~ %d: %d\n", i, low, high, offset);
		high = low;
		low = criteria * (div_factor - i - 2);
	}

	return hub_nodes;
}

int push(pairInfo* pair, int* mapping, int current_size, int index, int nnz) {
	mapping[index] = current_size;
	pair[current_size].index = index;
	pair[current_size].nnz = nnz;

	// heapify(pair, mapping, index);
	return current_size + 1;
}

pairInfo pop(pairInfo* pair, int* mapping, int reduced_size) {
	pairInfo res = pair[0];
	mapping[res.index] = -2;

	pair[0] = pair[reduced_size];
	int parent = 0, left = 0, right = 0;
	int target;
	while (1) {
		left = parent * 2 + 1;
		right = parent * 2 + 2;
		if (left >= reduced_size) break;
		if (right >= reduced_size) target = left; 
		else target = pair[left].nnz < pair[right].nnz ? right : left;

		if (pair[parent].nnz <= pair[target].nnz) {
			mapping[pair[parent].index] = target;
			mapping[pair[target].index] = parent;
			pairInfo temp = pair[parent];
			pair[parent] = pair[target];
			pair[target] = temp;
			parent = target;
		}
		else break;
	}

	return res;
}

void heapify(pairInfo* pair, int* mapping, int index) {
	for (int i = index; i > 0; i = (i - 1) >> 1) {
		int parent = (i - 1) >> 1;
		if (pair[i].nnz > pair[parent].nnz) {
			mapping[pair[parent].index] = i;
			mapping[pair[i].index] = parent;
			pairInfo temp = pair[i];
			pair[i] = pair[parent];
			pair[parent] = temp;
		}
		else break;
	}
}