/*
    'Diversification' and 'estimate' estimate diversification and fossilization rates frm tree shapes and fossil dates / 
    'sample' simulates random trees and fossils finds and saves them in Newick format / 
    'test' simulates random multiplex to test community detection approaches
	'complexity' simulates random trees and fossils finds, computes their complexity index and return a CSV file with lines
		<complexity index>	<likelihood computation time>	<tree size>	<fossil number>
	'assess' simulates random trees and fossils finds, estimates speciation and extinction rates and returns the mean absolute error

    Copyright (C) 2015  Gilles DIDIER

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/




#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <gsl/gsl_rng.h>

#include "Utils.h"
#include "Tree.h"
#include "SimulTree.h"
#include "Fossil.h"
#include "Model.h"
#include "Uncertainty.h"
#include "SimulFossil.h"


#define SIZE_BUFFER_CHAR 300
#define MAX_NF 10

//./complexity -f 2. -f 3. -l 5 5.2 0.5 -i 20 -M 500 -b 1.5

#define HELP_MESSAGE "\nusage: complexity [options] [<output file>]\n\ncomplexity simulates random trees and fossils finds an return a CSV file with lines\n\t<complexity index>\t<likelihood computation time>\t<tree size>\t<fossil number>\n\nOptions:\n\t-h : display help\n\t-b <birth>\t: set birth rate\n\t-d <death>\t: set death rate\n\t-f <fossil>\t: set fossil find rate\n\t-m <min>\t: set minimum number of contemporary species of a simulation to be considered\n\t-M <size>\t: set maximum size of a simulation to be considered\n\t-c <complexity>\t: set the max complexity index of a simulation to be considered\n\t-i <niter>\t: set the number of simulations\n\t-l <start> <end> <step> : the range of the end time of the diversification (start is always 0)\n"
static struct timespec diffTimespec(struct timespec start, struct timespec end);
static double toMSec(struct timespec t);

struct timespec diffTimespec(struct timespec start, struct timespec end) {
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

double toMSec(struct timespec t) {
	return 1000.*((double)t.tv_sec)+((double)t.tv_nsec)/1000000.;
}

int main(int argc, char **argv) {
	char  outputFileName[SIZE_BUFFER_CHAR], option[256];
	double birth = 2., death = 1., start = 2., end = 5., step=0.5, fossilTab[MAX_NF], maxComplexity= 1.E7;
	int i, niter = 20, nf = 0, minContemp = 10, maxSizeTree = 1000;;
	FILE *f;
	gsl_rng *rg = gsl_rng_alloc(gsl_rng_random_glibc2);

	for(i=0; i<256; i++)
		option[i] = 0;
	for(i=1; i<argc && *(argv[i]) == '-'; i++) {
		int j;
		for(j=1; argv[i][j] != '\0'; j++)
			option[argv[i][j]] = 1;
		if(option['b']) {
			option['b'] = 0;
			if((i+1)<argc && sscanf(argv[i+1], "%lf", &birth) == 1)
				i++;
		}
		if(option['d']) {
			option['d'] = 0;
			if((i+1)<argc && sscanf(argv[i+1], "%lf", &death) == 1)
				i++;
		}
		if(option['f']) {
			option['f'] = 0;
			if(nf < MAX_NF) {
				if((i+1)<argc && sscanf(argv[i+1], "%lf", &(fossilTab[nf])) == 1) {
					i++;
					nf++;
				} else
					printf("Can't read \"%s\" as fossil rate\n", argv[i+1]);
			} else
				printf("Can't handle more than %d fossil rates\n", MAX_NF);
		}
		if(option['i']) {
			option['i'] = 0;
			if((i+1)<argc && sscanf(argv[i+1], "%d", &niter) == 1)
				i++;
		}
		if(option['m']) {
			option['m'] = 0;
			if((i+1)<argc && sscanf(argv[i+1], "%d", &minContemp) == 1)
				i++;
		}
		if(option['M']) {
			option['M'] = 0;
			if((i+1)<argc && sscanf(argv[i+1], "%d", &maxSizeTree) == 1)
				i++;
		}
		if(option['c']) {
			option['c'] = 0;
			if((i+1)<argc && sscanf(argv[i+1], "%lE", &maxComplexity) == 1)
				i++;
		}
		if(option['l']) {
			option['l'] = 0;
			if((i+1)<argc && sscanf(argv[i+1], "%lf", &start) == 1)
				i++;
			if((i+1)<argc && sscanf(argv[i+1], "%lf", &end) == 1)
				i++;
			if((i+1)<argc && sscanf(argv[i+1], "%lf", &step) == 1)
				i++;
		}
		if(option['h']) {
			printf("%s\n", HELP_MESSAGE);
			exit(EXIT_SUCCESS);
		}
	}
	if (i>=argc || sscanf(argv[i++], "%s", outputFileName) != 1)
		strcpy(outputFileName, "TimeTable.csv");
	if(nf == 0)
		fossilTab[nf++] = 1.;
	if((f = fopen(outputFileName, "w"))) {
		double maxTime;
		TypeModelParam param;
		param.birth = birth;
		param.death = death;
		for(maxTime=start; maxTime<=end; maxTime += step) {
			int tot = 0;
			while(tot<niter) {
				TypeTree *tree;
				int nBirth, nDeath, nFossil, nContemp, curContemp;
				tree = simulTree(rg, birth, death, maxTime);
				if(tree != NULL && (curContemp = countContemp(tree))>=minContemp && tree->size<=maxSizeTree) {
					int u;
					for(u=0; u<nf; u++) {
						TypeTree *tree1, *tree2;
						TypeFossilFeature *fos;
						struct timespec tx, ty;
						double it;
						fos = addFossils(rg, fossilTab[u], tree);
						tree1 = pruneFossil(tree, fos);
						freeFossilFeature(fos);
						tree2 = fixBinaryFossil(tree1,  (TypeFossilFeature*) tree1->info);
						freeFossilFeature((TypeFossilFeature*) tree1->info);
						freeTree(tree1);
						it = getItemNumber(tree2, (TypeFossilFeature*) tree2->info);
						if(it<maxComplexity) {
							param.fossil = fossilTab[u];
							clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tx);
							getLogLikelihoodTreeFossil(tree2, (TypeFossilFeature*) tree2->info, &param);
							clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ty);
							fprintf(f, "%lE\t%lE\t%d\t%d\n", it, toMSec(diffTimespec(tx,ty)), tree->size, ((TypeFossilFeature*) tree2->info)->size);
						} else
							break;
						freeFossilFeature((TypeFossilFeature*) tree2->info);
						freeTree(tree2);
					}
					if(u>=nf)
						tot++;
				}
				freeTree(tree);
			}
		}
		fclose(f);
	}
	gsl_rng_free(rg);
	exit(EXIT_SUCCESS);
}

