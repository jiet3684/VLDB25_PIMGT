all: hdv debug
base: metis fennel bpart ne hep topox fsm

hdv: segment_Graph_hdv.o hdv.o write_Storage.o sort.o adjust.o debug.o
	gcc -o bin/hdv -DHDV -O3 -fopenmp -w obj/segment_Graph_hdv.o obj/hdv.o obj/write_Storage.o obj/sort.o obj/adjust.o obj/debug.o -lm
debug: segment_Graph_debug.o hdv_debug.o write_Storage.o sort.o adjust.o debug_write.o baseline.o
	gcc -o bin/debug -DDEBUG -O3 -fopenmp -w obj/segment_Graph_debug.o obj/hdv_debug.o obj/write_Storage.o obj/sort.o obj/adjust.o obj/debug_write.o obj/baseline.o -lm

metis: segment_Graph_metis.o baseline.o write_Storage.o sort.o debug.o
	gcc -o bin/metis -DMETIS -O3 -fopenmp -w obj/segment_Graph_metis.o obj/baseline.o obj/write_Storage.o obj/sort.o obj/debug.o -lm
fennel: segment_Graph_fennel.o baseline.o write_Storage.o sort.o debug.o
	gcc -o bin/fennel -DFENNEL -O3 -fopenmp -w obj/segment_Graph_fennel.o obj/baseline.o obj/write_Storage.o obj/sort.o obj/debug.o -lm
bpart: segment_Graph_bpart.o baseline.o write_Storage.o sort.o debug.o
	gcc -o bin/bpart -DBPART -O3 -fopenmp -w obj/segment_Graph_bpart.o obj/baseline.o obj/write_Storage.o obj/sort.o obj/debug.o -lm
ne: segment_Graph_ne.o baseline.o write_Storage.o sort.o debug.o
	gcc -o bin/ne -DNE -O3 -fopenmp -w obj/segment_Graph_ne.o obj/baseline.o obj/write_Storage.o obj/sort.o obj/debug.o -lm
hep: segment_Graph_hep.o baseline.o write_Storage.o sort.o debug.o
	gcc -o bin/hep -DHEP -O3 -fopenmp -w obj/segment_Graph_hep.o obj/baseline.o obj/write_Storage.o obj/sort.o obj/debug.o -lm
topox: segment_Graph_topox.o baseline.o write_Storage.o sort.o debug.o
	gcc -o bin/topox -DTOPOX -O3 -fopenmp -w obj/segment_Graph_topox.o obj/baseline.o obj/write_Storage.o obj/sort.o obj/debug.o -lm
fsm: segment_Graph_fsm.o baseline.o write_Storage.o sort.o debug.o
	gcc -o bin/fsm -DFSM -O3 -fopenmp -w obj/segment_Graph_fsm.o obj/baseline.o obj/write_Storage.o obj/sort.o obj/debug.o -lm

segment_Graph_metis.o: src/segment_Graph.c segment_Graph.h
	gcc -o obj/segment_Graph_metis.o -O3 -w -DMETIS -c src/segment_Graph.c
segment_Graph_fennel.o: src/segment_Graph.c segment_Graph.h
	gcc -o obj/segment_Graph_fennel.o -O3 -w -DFENNEL -c src/segment_Graph.c
segment_Graph_bpart.o: src/segment_Graph.c segment_Graph.h
	gcc -o obj/segment_Graph_bpart.o -O3 -w -DBPART -c src/segment_Graph.c
segment_Graph_ne.o: src/segment_Graph.c segment_Graph.h
	gcc -o obj/segment_Graph_ne.o -O3 -w -DNE -c src/segment_Graph.c
segment_Graph_hep.o: src/segment_Graph.c segment_Graph.h
	gcc -o obj/segment_Graph_hep.o -O3 -w -DHEP -c src/segment_Graph.c
segment_Graph_topox.o: src/segment_Graph.c segment_Graph.h
	gcc -o obj/segment_Graph_topox.o -O3 -w -DTOPOX -c src/segment_Graph.c
segment_Graph_fsm.o: src/segment_Graph.c segment_Graph.h
	gcc -o obj/segment_Graph_fsm.o -O3 -w -DFSM -c src/segment_Graph.c
segment_Graph_hdv.o: src/segment_Graph.c segment_Graph.h
	gcc -o obj/segment_Graph_hdv.o -O3 -w -DHDV -c src/segment_Graph.c
segment_Graph_debug.o: src/segment_Graph.c segment_Graph.h
	gcc -o obj/segment_Graph_debug.o -O3 -w -DDEBUG -c src/segment_Graph.c

write_Storage.o: src/write_Storage.c segment_Graph.h
	gcc -o obj/write_Storage.o -O3 -w -c src/write_Storage.c
sort.o: src/sort.c segment_Graph.h
	gcc -o obj/sort.o -O3 -c src/sort.c
adjust.o: src/adjust.c segment_Graph.h
	gcc -o obj/adjust.o -O3 -c src/adjust.c
debug.o: src/debug.c segment_Graph.h
	gcc -o obj/debug.o -O3 -c src/debug.c
bfs.o: src/bfs.c segment_Graph.h
	gcc -o obj/bfs.o -O3 -w -Dbfs -c src/bfs.c
hdv.o: src/hdv.c segment_Graph.h
	gcc -o obj/hdv.o -O3 -w -fopenmp -c src/hdv.c
baseline.o: src/baseline.c segment_Graph.h
	gcc -o obj/baseline.o -O3 -w -c src/baseline.c

# FOR DEBUGGING
debug_write.o: src/debug.c segment_Graph.h
	gcc -o obj/debug_write.o -DDEBUG -c src/debug.c
hdv_debug.o: src/hdv.c segment_Graph.h
	gcc -o obj/hdv_debug.o -O3 -w -DDEBUG -fopenmp -c src/hdv.c
# baseline_debug.o: src/baseline.c segment_Graph.h
# 	gcc -o obj/baseline_debug.o -O3 -w -DDEBUG -c src/baseline.c

clean:
	rm bin/* obj/*
