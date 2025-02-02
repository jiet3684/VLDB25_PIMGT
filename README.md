# VLDB25 PIMGT
Artifacts for Proceedings of VLDB Endowment 2025

## Half-Division (HDV)

### Datasets
You can download the used datasets with "get_datasets.sh"
```
cd graph_partitioner
VLDB25_PIMGT/graph_partitioner$ sh get_datasets.sh
```

### Compilation
The source codes of Half-Division in PimGT are included in "graph_partitioner/src" directory.
We provide an object file and some source code files written by C.

You can generate the executable files with "graph_partitioner/Makefile".
```
VLDB25_PIMGT/graph_partitioner$ mkdir obj bin
VLDB25_PIMGT/graph_partitioner$ make	  	// generates all executables
VLDB25_PIMGT/graph_partitioner$ make base	// generates all baselines
```

### Baseline
We used 10 baselines for the experiments.
- METIS 5.1.0 [1]
- BPart [2]
- Fennel [3]
- NE [4]
- TopoX [5]
- HEP [6]
- FSM [7]
- HDV
- HDV+A (HDV with ICN adjustment)
- PimGT (HDV with ICN adjustment and ICN balancing)


### Usage
First, the baselines may need to be compiled on your local PC with reference to thess addresses;

 http://glaros.dtc.umn.edu/gkhome/metis/metis/download  (for METIS)

 https://github.com/lcj2021/split-merge-partitioner     (for all other baselines)

And you need to configure the path to the baselines.
```C
// in "VLDB25_PIMGT/graph_partitioner/src/baseline.c" line 3-4
#define METIS_BIN "/home/jiet/local/bin/gpmetis"
#define FSM_BIN "/home/jiet/split-merge-partitioner/build/main"
```


You can check the quality of all the partitioning methods with a command below (Recommended).
```	    
VLDB25_PIMGT/graph_partitioner$ sh check_All.sh	// Compares all baselines
```
The partitioning results will be stored in "quality.csv" and "performance.csv"


Or, you can run each partitioning technique with following commands.
option_adjustment: 0 for no adjustment(HDV), 2 for ICN adjustment(HDV+A), 3 for ICN adjustment + balancing(HDV+A+B).
```
VLDB25_PIMGT/graph_partitioner$ ./bin/hdv dataset/input_graph num_subgraphs option_refine
VLDB25_PIMGT/graph_partitioner$ ./bin/metis dataset/input_graph num_subgraphs 
  ......
```

Since the quality of ICN adjustment saturates quickly, the number of iterations is limited to 300.
Changing the number 300 will effect the number of iterations.
You can change the iteration limits by changing the code below
```C
// in "VLDB25_PIMGT/graph_partitioner/src/adjust.c" line 4
int NUM_ADJUSTMENT = 300;
```

## PimGT Framework

### Compilation
The source codes of graph traversal in PimGT (Graph Traversal on Processing-in-Memory) are included in "VLDB25_PIMGT/graph_traversal/src" directory.
We provide the Breadth-First Search Algorithm.

Install the UPMEM SDK (https://sdk.upmem.com/) [4] and generate the executable files with "Makefile".
```
cd graph_traversal
VLDB25_PIMGT/graph_traversal$ mkdir obj bin
VLDB25_PIMGT/graph_traversal$ make
```

### Usage 
You can run the BFS algorithm with following commands.
```
VLDB25_PIMGT/graph_traversal$ ./bin/bfs dataset/metis/mario001.mtx 64
VLDB25_PIMGT/graph_traversal$ ./bin/bfs dataset/ne/mario001.mtx 64
VLDB25_PIMGT/graph_traversal$ ./bin/bfs dataset/hdv/mario001.mtx 64
```
However, the PIM kernel can only be executed on real-world PIM systems; UPMEM PIM [8].
Fortunately, the UPMEM PIM SDK [9] also provides a CPU-based simulator.
Therefore we adapted our code for the simulator.
However, since the simulator can only simulate 64 PIM units, we provide "mario001", which is small enout to be executed on 64 units.

The simulation results are shown below.
```
  -- Input File: dataset/metis/mario001.mtx	# METIS
	Number of Nodes:	38435
	Number of Edges:	206156


  -- Partitioning Result
	Total Number of Nodes: 38435
	Total Number of ICNs : 23924 
	Ratio of ICNs: 62.25 %
	Number of Nodes in SG: 583 ~ 617 (Avg: 600)
	Number of ICNs  in SG: 302 ~ 429 (Avg: 373)


  -- Execution Time Log
	Allocate 64 DPUs:	6.39 ms
	PIM transfer + Execute:	2376.86 ms
	- H-to-D Transfer:	0.51 ms
	- SG + ICE Analysis:	2375.55 ms
	- D-to-H Transfer:	0.80 ms
	Finalize Traversal:	2.34 ms
	Total Execution:	2379.19 ms
```
```
  -- Input File: dataset/ne/mario001.mtx	# Neighbor Expansion (NE)
	Number of Nodes:	38435
	Number of Edges:	206156


  -- Partitioning Result
	Total Number of Nodes: 38435
	Total Number of ICNs : 5396 
	Ratio of ICNs: 14.04 %
	Number of Nodes in SG: 600 ~ 601 (Avg: 600)
	Number of ICNs  in SG: 23 ~ 332 (Avg: 84)


  -- Execution Time Log
	Allocate 64 DPUs:	5.71 ms
	PIM transfer + Execute:	1285.41 ms
	- H-to-D Transfer:	0.47 ms
	- SG + ICE Analysis:	1283.97 ms
	- D-to-H Transfer:	0.97 ms
	Finalize Traversal:	0.86 ms
	Total Execution:	1286.27 ms
```
```
  -- Input File: dataset/hdv/mario001.mtx	# Half-Division (HDV)
	Number of Nodes:	38435
	Number of Edges:	206156


  -- Partitioning Result
	Total Number of Nodes: 38435
	Total Number of ICNs : 6060 
	Ratio of ICNs: 15.77 %
	Number of Nodes in SG: 477 ~ 626 (Avg: 600)
	Number of ICNs  in SG: 27 ~ 196 (Avg: 94)


  -- Execution Time Log
	Allocate 64 DPUs:	6.09 ms
	PIM transfer + Execute:	1273.12 ms
	- H-to-D Transfer:	0.57 ms
	- SG + ICE Analysis:	1271.90 ms
	- D-to-H Transfer:	0.65 ms
	Finalize Traversal:	0.65 ms
	Total Execution:	1273.77 ms
```
In contrast to real-world execution, NE can perform similarly or even slightly better than HDV in simulation.
This is because the simulation is performed on the CPU with dynamic scheduling, so balancing the workload across subgraphs is not critical.
Thus, METIS with the most total ICNs/ICEs is the slowest, and NE with the least ICNs/ICEs performs similar to HDV.
Additionally, the final traversal was also faster with lower total ICNs/ICEs.
However, in real-world PIM systems, HDV, which balances the workload across subgraphs, has always performed best.


### References
[1] George Karypis and Vipin Kumar. 1998. A Fast and High Quality Multi-level Scheme for Partitioning Irregular Graphs. SIAM Journal on Scientific Computing 20, 1 (1998), 359–392. https://doi.org/10.1137/S1064827595287997

[2] Shuai Lin, Rui Wang, Yongkun Li, Yinlong Xu, John C.S. Lui, Fei Chen, Pengcheng Wang, and Lei Han. 2023. Towards Fast Large-scale Graph Analysis via Two-dimensional Balanced Partitioning. In Proceedings of the 51st International Conference on Parallel Processing (ICPP '22). Association for Computing Machinery, New York, NY, USA, Article 37, 1–11. https://doi.org/10.1145/3545008.3545060

[3] Charalampos Tsourakakis, Christos Gkantsidis, Bozidar Radunovic, and Milan Vojnovic. 2014. FENNEL: streaming graph partitioning for massive scale graphs. In Proceedings of the 7th ACM international conference on Web search and data mining (WSDM '14). Association for Computing Machinery, New York, NY, USA, 333–342. https://doi.org/10.1145/2556195.2556213

[4] Chenzi Zhang, Fan Wei, Qin Liu, Zhihao Gavin Tang, and Zhenguo Li. 2017. Graph Edge Partitioning via Neighborhood Heuristic (KDD ’17). Association for Computing Machinery, New York, NY, USA, 605–614. https://doi.org/10.1145/3097983.3098033

[5] Dongsheng Li, Yiming Zhang, Jinyan Wang, and Kian-Lee Tan. 2019. TopoX: topology refactorization for efficient graph partitioning and processing. Proc. VLDB Endow. 12, 8 (April 2019), 891–905. https://doi.org/10.14778/3324301.3324306

[6] Ruben Mayer and Hans-Arno Jacobsen. 2021. Hybrid Edge Partitioner: Partitioning Large Power-Law Graphs under Memory Constraints. In Proceedings of the 2021 International Conference on Management of Data (SIGMOD '21). Association for Computing Machinery, New York, NY, USA, 1289–1302. https://doi.org/10.1145/3448016.3457300

[7] Chengjun Liu, Zhuo Peng, Weiguo Zheng, and Lei Zou. 2024. FSM: A Fine-Grained Splitting and Merging Framework for Dual-Balanced Graph Partition. Proc. VLDB Endow. 17, 9 (May 2024), 2378–2391. https://doi.org/10.14778/3665844.3665864

[8] F. Devaux. 2019. The true Processing In Memory accelerator. In 2019 IEEE Hot Chips 31 Symposium (HCS). IEEE Computer Society, Los Alamitos, CA, USA, 1–24. https://doi.org/10.1109/HOTCHIPS.2019.8875680

[9] https://sdk.upmem.com/

