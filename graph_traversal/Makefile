BASEDIR=.
SRCDIR=$(BASEDIR)/src
BFSSRCDIR=$(BASEDIR)/src/bfs
SPFSRCDIR=$(BASEDIR)/src/spf
BCSRCDIR=$(BASEDIR)/src/bc
PRSRCDIR=$(BASEDIR)/src/pr
HDRDIR=$(BASEDIR)/header
OBJDIR=$(BASEDIR)/obj

CPP=gcc
NUMTASKLETS=-DNR_TASKLETS=11
HOSTFLAGS=-O3 -fopenmp -w 
USEDPUS=-ldpu -I/usr/include/dpu
#USEDPUS=`dpu-pkg-config --cflags --libs dpu` $(NUMTASKLETS)
CLANG=dpu-upmem-dpurte-clang
PIMFLAGS=$(NUMTASKLETS) -O3 
HEADER=$(HDRDIR)/host.h $(HDRDIR)/common.h

all: bfs 

bfs: bfs_main bfs_kernel
spf: spf_main spf_kernel
bc: bc_main bc_kernel
pr: pr_main pr_kernel


# Breadth First Search
bfs_main: $(OBJDIR)/bfs_main.o $(OBJDIR)/readInput.o $(OBJDIR)/preProc.o $(OBJDIR)/bfs_host.o $(HEADER)
	$(CPP) $(HOSTFLAGS) -o bin/bfs $^ $(USEDPUS)

$(OBJDIR)/bfs_main.o: $(BFSSRCDIR)/main.c $(HEADER)
	$(CPP) $(HOSTFLAGS) -c -o $@ $< $(USEDPUS)

$(OBJDIR)/bfs_host.o: $(BFSSRCDIR)/bfs_host.c $(HEADER)
	$(CPP) $(HOSTFLAGS) -c -o $@ $< $(USEDPUS)

bfs_kernel: $(BFSSRCDIR)/bfs_kernel.c $(HEADER)
	$(CLANG) $(PIMFLAGS) -o $(BASEDIR)/bin/bfs_kernel $< 


# Shortest Path Finding
spf_main: $(OBJDIR)/spf_main.o $(OBJDIR)/readInput.o $(OBJDIR)/preProc.o $(OBJDIR)/spf_host.o $(HEADER)
	$(CPP) $(HOSTFLAGS) -o bin/spf $^ $(USEDPUS)

$(OBJDIR)/spf_main.o: $(SPFSRCDIR)/main.c $(HEADER)
	$(CPP) $(HOSTFLAGS) -c -o $@ $< $(USEDPUS)

$(OBJDIR)/spf_host.o: $(SPFSRCDIR)/spf_host.c $(HEADER)
	$(CPP) $(HOSTFLAGS) -c -o $@ $< $(USEDPUS)

spf_kernel: $(SPFSRCDIR)/spf_kernel.c $(HEADER)
	$(CLANG) $(PIMFLAGS) -o $(BASEDIR)/bin/spf_kernel $< 
	

# Betweenness Centrality
bc_main: $(OBJDIR)/bc_main.o $(OBJDIR)/readInput.o $(OBJDIR)/preProc.o $(OBJDIR)/bc_host.o $(HEADER)
	$(CPP) $(HOSTFLAGS) -o bin/bc $^ $(USEDPUS)

$(OBJDIR)/bc_main.o: $(BCSRCDIR)/main.c $(HEADER)
	$(CPP) $(HOSTFLAGS) -c -o $@ $< $(USEDPUS)

$(OBJDIR)/bc_host.o: $(BCSRCDIR)/bc_host.c $(HEADER)
	$(CPP) $(HOSTFLAGS) -c -o $@ $< $(USEDPUS)

bc_kernel: $(BCSRCDIR)/bc_kernel.c $(HEADER)
	$(CLANG) $(PIMFLAGS) -o $(BASEDIR)/bin/bc_kernel $< 
	


# Common
$(OBJDIR)/readInput.o: $(SRCDIR)/readInput.c $(HEADER)
	$(CPP) $(HOSTFLAGS) -c -o $@ $< $(USEDPUS)

$(OBJDIR)/preProc.o: $(SRCDIR)/preProc.c $(HEADER)
	$(CPP) $(HOSTFLAGS) -c -o $@ $< $(USEDPUS)


clean:
	rm $(OBJDIR)/*
	rm bin/*
