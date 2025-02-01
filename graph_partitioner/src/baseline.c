#include "../segment_Graph.h"
#include <sys/wait.h>;
#define METIS_BIN "/home/jiet/local/bin/gpmetis"
#define FSM_BIN "/home/jiet/split-merge-partitioner/build/main"

FILE *fp_base;
FILE *fp_map;

void exec_METIS(char* file_name, int target, csr mat) {
    char input[128];
    char num_sg[8];
    sprintf(num_sg, "%d", target);
	strcpy(input, file_name);
	strcat(input, ".metis");
    if (access(input, F_OK) < 0) {
		puts("Input METIS File does not exist.");
        return;
	}
    memset(sg_size, 0, target << 2);

    int pid = fork();
    if (pid == 0) {
        int res = execl(METIS_BIN, METIS_BIN, "-niter=200", input, num_sg, (char *)0);
        if (res == -1) puts("Execute Error");
        exit(0);
    }
    else if (pid > 0) {
        wait(NULL);
    }

    strcat(input, ".part.");
    strcat(input, num_sg);
    if (access(input, F_OK) < 0) {
        printf("Partitioned METIS File does not exist.\n");
        return 0;
    }
	fp_base = fopen(input, "rt");
	for (int i = 0; i < mat.nr; ++i) {
		fscanf(fp_base, "%d\n", sg_index + i);
		sg_size[sg_index[i]]++;
		assert(sg_index[i] >= 0 && sg_index[i] < target);
	} 
	fclose(fp_base);

}

void exec_BPart(char* file_name, int target, csr mat) {
    char input[128];
    char num_sg[8];
    sprintf(num_sg, "%d", target);
	strcpy(input, file_name);
    memset(sg_size, 0, target << 2);

    int pid = fork();
    if (pid == 0) {
        int res = execl(FSM_BIN, FSM_BIN, "-p", num_sg, "-method", "bpart", "-filename", input, "-write", "onefile", (char *)0);
        if (res == -1) puts("Execute Error");
        exit(0);
    }
    else if (pid > 0) {
        wait(NULL);
    }

    strcat(input, ".vertexpart.bpart.");
    strcat(input, num_sg);
    if (access(input, F_OK) < 0) {
        printf("Input BPart File does not exist.\n");
        return 0;
    }
	fp_base = fopen(input, "rt");

    strcpy(input, file_name);
    strcat(input, ".map");
    if (access(input, F_OK) < 0) {
        printf("Input Mapping File does not exist.\n");
        return;
    }
    fp_map = fopen(input, "rt");

	for (int i = 0; i < mat.nr; ++i) {
        int node, sg;
        fscanf(fp_map, "%d\n", &node);
		fscanf(fp_base, "%d\n", &sg);
        sg_index[node] = sg;
        sg_size[sg_index[node]]++;
	}
	fclose(fp_base);
    fclose(fp_map);
}

void exec_Fennel(char* file_name, int target, csr mat) {
    char input[128];
    char num_sg[8];
    sprintf(num_sg, "%d", target);
	strcpy(input, file_name);
    memset(sg_size, 0, target << 2);

    int pid = fork();
    if (pid == 0) {
        int res = execl(FSM_BIN, FSM_BIN, "-p", num_sg, "-method", "fennel", "-filename", input, "-write", "onefile", (char *)0);
        if (res == -1) puts("Execute Error");
        exit(0);
    }
    else if (pid > 0) {
        wait(NULL);
    }

    strcat(input, ".vertexpart.fennel.");
    strcat(input, num_sg);
    if (access(input, F_OK) < 0) {
        printf("Input Fennel File does not exist.\n");
        return 0;
    }
	fp_base = fopen(input, "rt");

    strcpy(input, file_name);
    strcat(input, ".map");
    if (access(input, F_OK) < 0) {
        printf("Input Mapping File does not exist.\n");
        return;
    }
    fp_map = fopen(input, "rt");

	for (int i = 0; i < mat.nr; ++i) {
        int node, sg;
        fscanf(fp_map, "%d\n", &node);
		fscanf(fp_base, "%d\n", &sg);
        sg_index[node] = sg;
        sg_size[sg_index[node]]++;
	}
	fclose(fp_base);
    fclose(fp_map);
}

void exec_NE(char* file_name, int target, csr mat) {
    char input[128];
    char num_sg[8];
    sprintf(num_sg, "%d", target);
	strcpy(input, file_name);
    memset(sg_size, 0, target << 2);
    int* degree = calloc(mat.nr * target, 4);

    int pid = fork();
    if (pid == 0) {
        int res = execl(FSM_BIN, FSM_BIN, "-p", num_sg, "-method", "ne", "-filename", input, "-write", "onefile", (char *)0);
        if (res == -1) puts("Execute Error");
        exit(0);
    }
    else if (pid > 0) {
        wait(NULL);
    }

    strcat(input, ".edgepart.ne.");
    strcat(input, num_sg);
    if (access(input, F_OK) < 0) {
        printf("Input NE File does not exist.\n");
        return;
    }
	fp_base = fopen(input, "rt");
    strcpy(input, file_name);
    strcat(input, ".map");
    if (access(input, F_OK) < 0) {
        printf("Input Mapping File does not exist.\n");
        return;
    }
    fp_map = fopen(input, "rt");

    int *map = malloc(mat.nr << 2);
    for (int i = 0; i < mat.nr; ++i) {
        fscanf(fp_map, "%d\n", map + i);
    }

	for (int i = 0; i < mat.ne; ++i) {
        int from, to, sg;
		fscanf(fp_base, "%d %d %d\n", &from, &to, &sg);
        from = map[from];
        to = map[to];
        degree[from * target + sg]++;
        degree[to * target + sg]++;
	} 

    int avg_node_ceil = ceil(1.02f * (mat.nr / target));
    for (int i = 0; i < mat.nr; ++i) {
        int max_val = -1;
        int max_idx = -1;
        for (int sg = 0; sg < target; ++sg) {
            if (max_val < degree[i * target + sg]) {
                max_val = degree[i * target + sg];
                max_idx = sg;
            }
            else if (max_val == degree[i * target + sg] && sg_size[max_idx] > sg_size[sg]) {
                max_val = degree[i * target + sg];
                max_idx = sg;
            }
        }
        if (max_idx == -1) {
            for (int sg = 0; sg < target; ++sg) {
                if (sg_size[sg] < avg_node_ceil) {
                    max_idx = sg;
                    break;
                }
            }
        }
        sg_index[i] = max_idx;
        sg_size[max_idx]++;
    }
    
	fclose(fp_base);
    fclose(fp_map);
    free(degree);
    free(map);
}

void exec_TopoX(char* file_name, int target, csr mat) {
    char input[128];
    char num_sg[8];
    sprintf(num_sg, "%d", target);
	strcpy(input, file_name);
    memset(sg_size, 0, target << 2);
    int* degree = calloc(mat.nr * target, 4);

    int pid = fork();
    if (pid == 0) {
        int res = execl(FSM_BIN, FSM_BIN, "-p", num_sg, "-method", "hybridbl", "-filename", input, "-write", "onefile", (char *)0);
        if (res == -1) puts("Execute Error");
        exit(0);
    }
    else if (pid > 0) {
        wait(NULL);
    }

    strcat(input, ".edgepart.hybridbl.");
    strcat(input, num_sg);
    if (access(input, F_OK) < 0) {
        printf("Input TopoX File does not exist.\n");
        return;
    }
	fp_base = fopen(input, "rt");
    strcpy(input, file_name);
    strcat(input, ".map");
    if (access(input, F_OK) < 0) {
        printf("Input Mapping File does not exist.\n");
        return;
    }
    fp_map = fopen(input, "rt");

    int *map = malloc(mat.nr << 2);
    for (int i = 0; i < mat.nr; ++i) {
        fscanf(fp_map, "%d\n", map + i);
    }

	for (int i = 0; i < mat.ne; ++i) {
        int from, to, sg;
		fscanf(fp_base, "%d %d %d\n", &from, &to, &sg);
        from = map[from];
        to = map[to];
        degree[from * target + sg]++;
        degree[to * target + sg]++;
	} 

    int avg_node_ceil = ceil(1.02f * (mat.nr / target));
    for (int i = 0; i < mat.nr; ++i) {
        int max_val = -1;
        int max_idx = -1;
        for (int sg = 0; sg < target; ++sg) {
            if (max_val < degree[i * target + sg]) {
                max_val = degree[i * target + sg];
                max_idx = sg;
            }
            else if (max_val == degree[i * target + sg] && sg_size[max_idx] > sg_size[sg]) {
                max_val = degree[i * target + sg];
                max_idx = sg;
            }
        }
        if (max_idx == -1) {
            for (int sg = 0; sg < target; ++sg) {
                if (sg_size[sg] < avg_node_ceil) {
                    max_idx = sg;
                    break;
                }
            }
        }
        sg_index[i] = max_idx;
        sg_size[max_idx]++;
    }
    
	fclose(fp_base);
    fclose(fp_map);
    free(degree);
    free(map);
}

void exec_HEP(char* file_name, int target, csr mat) {
    char input[128];
    char num_sg[8];
    sprintf(num_sg, "%d", target);
	strcpy(input, file_name);
    memset(sg_size, 0, target << 2);
    int* degree = calloc(mat.nr * target, 4);

    int pid = fork();
    if (pid == 0) {
        int res = execl(FSM_BIN, FSM_BIN, "-p", num_sg, "-method", "hep", "-hdf", "10", "-filename", input, "-write", "onefile", (char *)0);
        if (res == -1) puts("Execute Error");
        exit(0);
    }
    else if (pid > 0) {
        wait(NULL);
    }

    strcat(input, ".edgepart.hep_hdf_10.");
    strcat(input, num_sg);
    if (access(input, F_OK) < 0) {
        printf("Input HEP File does not exist.\n");
        return;
    }
	fp_base = fopen(input, "rt");
    strcpy(input, file_name);
    strcat(input, ".map");
    if (access(input, F_OK) < 0) {
        printf("Input Mapping File does not exist.\n");
        return;
    }
    fp_map = fopen(input, "rt");

    int *map = malloc(mat.nr << 2);
    for (int i = 0; i < mat.nr; ++i) {
        fscanf(fp_map, "%d\n", map + i);
    }

	for (int i = 0; i < mat.ne; ++i) {
        int from, to, sg;
		fscanf(fp_base, "%d %d %d\n", &from, &to, &sg);
        from = map[from];
        to = map[to];
        degree[from * target + sg]++;
        degree[to * target + sg]++;
	} 

    int avg_node_ceil = ceil(1.02f * (mat.nr / target));
    for (int i = 0; i < mat.nr; ++i) {
        int max_val = -1;
        int max_idx = -1;
        for (int sg = 0; sg < target; ++sg) {
            if (max_val < degree[i * target + sg]) {
                max_val = degree[i * target + sg];
                max_idx = sg;
            }
            else if (max_val == degree[i * target + sg] && sg_size[max_idx] > sg_size[sg]) {
                max_val = degree[i * target + sg];
                max_idx = sg;
            }
        }
        if (max_idx == -1) {
            for (int sg = 0; sg < target; ++sg) {
                if (sg_size[sg] < avg_node_ceil) {
                    max_idx = sg;
                    break;
                }
            }
        }
        sg_index[i] = max_idx;
        sg_size[max_idx]++;
    }
    
	fclose(fp_base);
    fclose(fp_map);
    free(degree);
    free(map);
}

void exec_FSM(char* file_name, int target, csr mat) {
    char input[128];
    char num_sg[8];
    sprintf(num_sg, "%d", target);
	strcpy(input, file_name);
    memset(sg_size, 0, target << 2);
    int* degree = calloc(mat.nr * target, 4);

    int pid = fork();
    if (pid == 0) {
        // int res = execl(FSM_BIN, FSM_BIN, "-p", num_sg, "-method", "fsm_ne", "-k", "2", "-filename", input, "-write", "onefile", (char *)0);
        int res = execl(FSM_BIN, FSM_BIN, "-p", num_sg, "-method", "fsm_hep", "-k", "2", "-hdf", "10", "-filename", input, "-write", "onefile", "-fastmerge", "true", (char *)0);
        if (res == -1) puts("Execute Error");
        exit(0);
    }
    else if (pid > 0) {
        wait(NULL);
    }

    strcat(input, ".edgepart.fsm_hep_k_2.");
    strcat(input, num_sg);
    if (access(input, F_OK) < 0) {
        printf("Input FSM File does not exist.\n");
        return;
    }
	fp_base = fopen(input, "rt");
    strcpy(input, file_name);
    strcat(input, ".map");
    if (access(input, F_OK) < 0) {
        printf("Input Mapping File does not exist.\n");
        return;
    }
    fp_map = fopen(input, "rt");

    int *map = malloc(mat.nr << 2);
    for (int i = 0; i < mat.nr; ++i) {
        fscanf(fp_map, "%d\n", map + i);
    }

	for (int i = 0; i < mat.ne; ++i) {
        int from, to, sg;
		fscanf(fp_base, "%d %d %d\n", &from, &to, &sg);
        from = map[from];
        to = map[to];
        degree[from * target + sg]++;
        degree[to * target + sg]++;
	} 

    int avg_node_ceil = ceil(1.02f * (mat.nr / target));
    for (int i = 0; i < mat.nr; ++i) {
        int max_val = -1;
        int max_idx = -1;
        for (int sg = 0; sg < target; ++sg) {
            if (max_val < degree[i * target + sg]) {
                max_val = degree[i * target + sg];
                max_idx = sg;
            }
            else if (max_val == degree[i * target + sg] && sg_size[max_idx] > sg_size[sg]) {
                max_val = degree[i * target + sg];
                max_idx = sg;
            }
        }
        if (max_idx == -1) {
            for (int sg = 0; sg < target; ++sg) {
                if (sg_size[sg] < avg_node_ceil) {
                    max_idx = sg;
                    break;
                }
            }
        }
        sg_index[i] = max_idx;
        sg_size[max_idx]++;
    }
    
	fclose(fp_base);
    fclose(fp_map);
    free(degree);
    free(map);
}