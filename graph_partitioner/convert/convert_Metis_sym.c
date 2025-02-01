/** 
 * Read .mtx input file and convert to CSR format
 * Args: {fileName1, fileName2, mat.ptr, mat.idx, mat.val flags, #rows, #elements}
 * Read whole matrix and sort with from data.
 * Divide Matrix A to small blocks based of number of elements.
 * Convert each block to CSR format then let main process to compute them.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

struct edgeData {
    int from;
    int to;
    int val;
};
typedef struct edgeData edgeData;

struct csr {
    int *nnz;
    int *ptr;
    int *idx;
    int *val;
	int* back_nnz;
	int* back_ptr;
	int* back_idx;
    int* inc_nnz;
    int* inc_ptr;
    int* inc_idx;
    int nr;
    int nc;
    int ne;
};
typedef struct csr csr;

struct edgeData *queues[10];
int queue_Offsets[10];

void make_CSR(char *input_File, csr *mat);
void sort_From(edgeData *edges, int nr, int ne);
void sort_To(edgeData *edges, int nc, int ne);

void find_BackEdge(csr* mat) {
	mat->back_nnz = calloc(mat->nr, 4);
	mat->back_ptr = calloc(mat->nr + 1, 4);
	mat->back_idx = malloc(mat->ne << 2);

	for (int e = 0; e < mat->ne; ++e) {
		mat->back_nnz[mat->idx[e]]++;
	}
    for (int n = 0; n < mat->nr; ++n) {
        mat->back_ptr[n + 1] = mat->back_ptr[n] + mat->back_nnz[n];
    }

    int* offset = calloc(mat->nr, 4);
    for (int n = 0; n < mat->nr; ++n) {
        for (int e = mat->ptr[n]; e < mat->ptr[n + 1]; ++e) {
            int target = mat->idx[e];
            mat->back_idx[mat->back_ptr[target] + offset[target]++] = n;
        }
    }
    free(offset);
}

void incorporate_BackEdge(csr* mat) {
    mat->inc_nnz = calloc(mat->nr, 4);
    mat->inc_ptr = calloc(mat->nr + 1, 4);
    mat->inc_idx = malloc(mat->ne << 3); 

    // int offset = 0;
    // for (int n = 0; n < mat->nr; ++n) {
        // memcpy(mat->inc_idx + offset, mat->idx + mat->ptr[n], mat->nnz[n] << 2);
        // offset += mat->nnz[n];
        // memcpy(mat->inc_idx + offset, mat->back_idx + mat->back_ptr[n], mat->back_nnz[n] << 2);
        // offset += mat->back_nnz[n];
// 
        // mat->inc_nnz[n] = mat->nnz[n] + mat->back_nnz[n];
        // mat->inc_ptr[n + 1] = mat->inc_ptr[n] + mat->inc_nnz[n];
    // }
    int offset = 0;
    for (int n = 0; n < mat->nr; ++n) {
        int local_offset = 0;
        int f_off = mat->ptr[n], b_off = mat->back_ptr[n];
        while (f_off < mat->ptr[n + 1] && b_off < mat->back_ptr[n + 1]) {
            local_offset++;
            if (mat->idx[f_off] > mat->back_idx[b_off]) {
                mat->inc_idx[offset++] = mat->back_idx[b_off++];
            }
            else if (mat->idx[f_off] < mat->back_idx[b_off]) {
                mat->inc_idx[offset++] = mat->idx[f_off++];
            }
            else {
                mat->inc_idx[offset++] = mat->idx[f_off++];
                b_off++;
            }
        }
        if (f_off < mat->ptr[n + 1]) {
            memcpy(mat->inc_idx + offset, mat->idx + f_off, (mat->ptr[n + 1] - f_off) << 2);
            local_offset += mat->ptr[n + 1] - f_off;
            offset += mat->ptr[n + 1] - f_off;
        }
        if (b_off < mat->back_ptr[n + 1]) {
            memcpy(mat->inc_idx + offset, mat->back_idx + b_off, (mat->back_ptr[n + 1] - b_off) << 2);
            local_offset += mat->back_ptr[n + 1] - b_off;
            offset += mat->back_ptr[n + 1] - b_off;
        }
        mat->inc_nnz[n] = local_offset;
        mat->inc_ptr[n + 1] = mat->inc_ptr[n] + local_offset;
    }
}



int main(int argc, char** argv) {
    char input_File[64];
    strcpy(input_File, argv[1]);
    FILE *fp = fopen(input_File, "rt");
    csr mat;

    char line[200];
    bool symmetric = false;
    if (strstr(input_File, ".dat") == NULL) {
        fgets(line, 200, fp);
        if(strstr(line, "symmetric")) symmetric = true;
    }
    //else puts("Synthetic");

    while (fgets(line, 200, fp) != NULL) {
        if (line[0] == '%' || line[0] == '#') continue;   // Comment
        char delims[] = "\t, ";
        char *t;
        t = strtok(line, delims);
        if (t == NULL) {
            fprintf(stderr, "Wrong Format.\n");
            exit(-1);
        }
        mat.nr = atoi(t) + 1;
        
        t = strtok(NULL, delims);
        if (t == NULL) {
            fprintf(stderr, "Wrong Format.\n");
            exit(-1);
        }
        mat.nc = atoi(t) + 1;

        t = strtok(NULL, delims);
        if (t == NULL) {
            fprintf(stderr, "Wrong Format.\n");
            exit(-1);
        }
        mat.ne = atoi(t) << 1;
        // if (symmetric == true) mat.ne *= 2;

        break;
    }

    mat.nnz = (int*)malloc((mat.nr + 1) << 2);
    mat.ptr = (int*)malloc((mat.nr + 1) << 2);
    mat.idx = (int*)malloc(mat.ne << 2);

    edgeData *edges = (edgeData*)malloc(sizeof(edgeData) * mat.ne);
    for (int q = 0; q < 10; ++q) queues[q] = (edgeData*)malloc(sizeof(edgeData) * mat.ne);
    int offset = 0;

    while (fgets(line, 200, fp) != NULL) {
        char delims[] = "\t, ";
        char *t;
        t = strtok(line, delims);
        edges[offset].from = atoi(t);
        t = strtok(NULL, delims);
        edges[offset].to = atoi(t);

        if (edges[offset].from == edges[offset].to) {
            continue;
        }
        
        offset++;
    }
    mat.ne = offset;

    sort_To(edges, mat.nc, mat.ne);
    sort_From(edges, mat.nr, mat.ne);
    
    for (int q = 0; q < 10; ++q) {
        free(queues[q]);
    }

    int row_Offset = 0;
    int edges_in_row = 0;
    for (int e = 0; e < mat.ne; ++e) {
        if (edges[e].from > row_Offset) {
            mat.nnz[row_Offset] = edges_in_row;

            for (int empty_Rows = row_Offset + 1; empty_Rows < edges[e].from; ++empty_Rows) {
                mat.nnz[empty_Rows] = 0;
            }

            row_Offset = edges[e].from;
            edges_in_row = 0;
        }
        mat.idx[e] = edges[e].to;
        edges_in_row++;
    }
    mat.nnz[row_Offset] = edges_in_row;

    mat.ptr[0] = 0;
    mat.ptr[mat.nr] = mat.ne;
    for (int row = 0; row < mat.nr - 1; ++row) {
        mat.ptr[row + 1] = mat.ptr[row] + mat.nnz[row];
    }

    free(edges);
    fclose(fp);

    printf("\n\n----------------------------\nInput: %s\n\tNumber of Rows:\t\t%d\n\tNumber of Columns:\t%d\n\tNumber of Edges:\t%d\n\n", input_File, mat.nr, mat.nc, mat.ne);

    find_BackEdge(&mat);
    incorporate_BackEdge(&mat);
    printf("After Incorporation: Number of Edges: %d\n", mat.inc_ptr[mat.nr]);


    char input[64];
    strcpy(input, input_File);
    strcat(input, ".metis");
    fp = fopen(input, "wt");
    // int row = mat.nr, col = mat.nc, edge = mat.ne;
    // fwrite(&row, sizeof(int), 1, fp);
    // fwrite(&col, sizeof(int), 1, fp);
    // fwrite(&edge, sizeof(int), 1, fp);

    // fwrite(mat.nnz, sizeof(int), row, fp);
    // fwrite(mat.ptr, sizeof(int), row + 1, fp);
    // fwrite(mat.idx, sizeof(int), edge, fp);
    // fwrite(mat.back_nnz, sizeof(int), row, fp);
    // fwrite(mat.back_ptr, sizeof(int), row + 1, fp);
    // fwrite(mat.back_idx, sizeof(int), edge, fp);

    // int row = mat.nr, edge = mat.ne;
    // fprintf(fp, "%d %d", row, edge);
    // for (int n = 0; n < row; ++n) {
    //     // fprintf(fp, "\n");
    //     fputs("\n", fp);
    //     for (int e = mat.inc_ptr[n]; e < mat.inc_ptr[n + 1]; ++e) {
    //         fprintf(fp, " %d", mat.inc_idx[e]);
    //     }
    // }
    
    int row = mat.nr, edge = mat.inc_ptr[mat.nr];
    fprintf(fp, "%d %d", row - 1, edge);
    for (int n = 1; n < row; ++n) {
        fputs("\n", fp);
        for (int e = mat.inc_ptr[n]; e < mat.inc_ptr[n + 1]; ++e) {
            fprintf(fp, " %d", mat.inc_idx[e]);
        }
    }
    // fwrite(&row, sizeof(int), 1, fp);
    // fwrite(&edge, sizeof(int), 1, fp);
    // fwrite(mat.inc_nnz, sizeof(int), row, fp);
    // fwrite(mat.inc_ptr, sizeof(int), row + 1, fp);
    // fwrite(mat.inc_idx, sizeof(int), edge, fp);
    fclose(fp);

    free(mat.nnz);
    free(mat.ptr);
    free(mat.idx);
}


/** 
 * Radix Sort
 * Each edge has {from, to, value} data
 * Sort only with from data
 * Total Memory Usage = sizeof(struct edgeData) * mat.ne +                                               : Temporary Array(edges in main Function) to read input file
 *                      10 * sizeof(struct edgeData) * mat.ne +                                                      : Queue Size
 *                      sizeof(int) * mat.nr + sizeof(int) * mat.ne + sizeof(float) * mat.ne        : Output Array(CSR Format)
 *                      Actually, Ouput Array Size is hidden since memory allocated after freeing Queues
 * Can reduce memory usage by changing implemention to 2-bit radix sort
 * -> Queue Size = 2 * sizeof(int) * mat.ne
 * But, time complexity increases.
 */

void sort_From(struct edgeData *edges, int nr, int ne) {
    int num_Digits = 1;
    while (nr) {
        nr /= 10;
        num_Digits *= 10;
    }
    //printf("Number of Digits: %d\n", num_Digits);

    for (int divide_To = 1; divide_To <= num_Digits; divide_To *= 10) {
        for (int q = 0; q < 10; ++q) queue_Offsets[q] = 0;
        
        for (int e = 0; e < ne; ++e) {
            int row = edges[e].from;
            int target_Queue = row % divide_To;
            target_Queue = (target_Queue * 10) / divide_To;
            queues[target_Queue][queue_Offsets[target_Queue]++] = edges[e];
        }

        int offset = 0;
        for (int q = 0; q < 10; ++q) {
            memcpy(edges + offset, queues[q], sizeof(struct edgeData) * queue_Offsets[q]);
            offset += queue_Offsets[q];
        }
    }
}

void sort_To(struct edgeData *edges, int nc, int ne) {
    int num_Digits = 1;
    while (nc) {
        nc /= 10;
        num_Digits *= 10;
    }
    //printf("Number of Digits: %d\n", num_Digits);

    for (int divide_To = 1; divide_To <= num_Digits; divide_To *= 10) {
        for (int q = 0; q < 10; ++q) queue_Offsets[q] = 0;
        
        for (int e = 0; e < ne; ++e) {
            int col = edges[e].to;
            int target_Queue = col % divide_To;
            target_Queue = (target_Queue * 10) / divide_To;
            queues[target_Queue][queue_Offsets[target_Queue]++] = edges[e];
        }

        int offset = 0;
        for (int q = 0; q < 10; ++q) {
            memcpy(edges + offset, queues[q], sizeof(struct edgeData) * queue_Offsets[q]);
            offset += queue_Offsets[q];
        }
    }
}
