#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <assert.h>

typedef struct timeval timeval;
inline void cur_Time(timeval* tv) {gettimeofday(tv, NULL);}
inline float elapsed_Time(timeval st, timeval ed) {return (float)(ed.tv_sec - st.tv_sec) + (float)0.000001 * (ed.tv_usec - st.tv_usec);};

struct csr {
	int nr;
	int nc;
	int ne;
	int pooled_nr;
	int pooled_ne;
	int target;
	int *nnz;
	int *ptr;
	int *idx;
};
typedef struct csr csr;

struct subgraph {
	int size;
	int* ptr;
	int* idx;
	int* val;
};
typedef struct subgraph sg;

struct pairInfo {
	uint32_t index;
	uint32_t nnz;
};
typedef struct pairInfo pairInfo;

typedef enum Adjustment {NO_ADJUSTMENT, ICN_ADJUSTMENT, BALANCE_ADJUSTMENT} refine;

// Partition a Graph
int segment_base(csr* mat, int num_DPUs);
int half_Division(csr mat, int target, refine ref);
int segment_ne(csr mat, int target);
void exec_METIS(char* file_name, int target, csr mat);
void exec_BPart(char* file_name, int target, csr mat);
void exec_Fennel(char* file_name, int target, csr mat);
void exec_NE(char* file_name, int target, csr mat);
void exec_TopoX(char* file_name, int target, csr mat);
void exec_HEP(char* file_name, int target, csr mat);
void exec_FSM(char* file_name, int target, csr mat);

// Sorting Function
pairInfo* make_Pair(csr* mat);
pairInfo* make_SortedPair(csr* mat);
pairInfo* find_hub_nodes(csr* mat);
void sort_int(int* arr, int l, int r);
void sort(pairInfo* pair, int l, int r);
int push(pairInfo* pair, int* mapping, int current_size, int index, int nnz);
pairInfo pop(pairInfo* pair, int* mapping, int reduced_size);
void heapify(pairInfo* pair, int* mapping, int index);

// ICN Adjustment
void prepare_Adjustment(csr mat, int target);
void ICN_Adjustment(csr mat, int step);
void ICN_Balancing(csr mat);
void finish_Adjustment();

// Debuging
int find_ICN(csr mat, int* num_ICN);
void check_balance(csr mat, int total_ICN, int* num_ICN);
int check_Replication(csr mat);

// Write a Graph to Storage
void write_to_storage(csr mat, int target, char* filename, char* partname);

extern int* sg_index;
extern int* sg_size;
extern char* flag_ICN;
extern FILE* fp;
extern FILE* perf;
