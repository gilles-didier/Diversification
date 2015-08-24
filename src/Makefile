
CC = gcc

LDFLAGS = -L/usr/local/lib -L/usr/lib/ -lm -lgsl -lgslcblas  -g


ALL = estimate sample assess complexity

all: estimate sample assess complexity

estimate: Estimate.c
	$(CC)  MinimizeNLOpt.c Model.c TreeExtras.c Uncertainty.c Utils.c Tree.c Fossil.c FossilInt.c  Estimate.c -o $@  -L/usr/local/lib -L/usr/lib/ -lm -lgsl -lgslcblas -lnlopt

sample: SampleTreeFossil.c
	$(CC) SimulTree.c SimulFossil.c Utils.c Tree.c Fossil.c SampleTreeFossil.c -o $@  -L/usr/local/lib -L/usr/lib/ -lm -lgsl -lgslcblas -lnlopt

assess: Assess.c
	$(CC)  -D_REENTRANT MinimizeNLOpt.c Model.c TreeExtras.c Uncertainty.c GNUFile.c SimulTree.c SimulFossil.c Utils.c Tree.c Fossil.c FossilInt.c  Assess.c -o $@  -L/usr/local/lib -L/usr/lib/ -lm -lgsl -lgslcblas -lnlopt -lpthread -O3 

complexity: Complexity.c
	$(CC)  Model.c TreeExtras.c Uncertainty.c SimulTree.c SimulFossil.c Utils.c Tree.c Fossil.c Complexity.c -o $@  -L/usr/local/lib -L/usr/lib/ -lm -lgsl -lgslcblas -lnlopt -Wl,--no-as-needed -lrt -O3 
	
