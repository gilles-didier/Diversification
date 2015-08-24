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

#include <gsl/gsl_rng.h>

#include "Utils.h"
#include "Tree.h"
#include "SimulTree.h"
#include "Fossil.h"
#include "SimulFossil.h"


#define STRING_SIZE 300
#define HELP_MESSAGE "\nusage: sample [options] [<output file>]\n\nsample simulates random trees and fossils finds and saves them in Newick format\n\nOptions:\n\t-h : display help\n\t-b <birth>\t: set birth rate\n\t-d <death>\t: set death rate\n\t-f <fossil>\t: set fossil find rate\n\t-m <min>\t: set minimum number of contemporary species of a simulation to be considered\n\t-M <size>\t: set maximum size of a simulation to be considered\n\t-i <niter>\t: set the number of simulations\n\t-t <time> : the end time of the diversification (start is always 0)\n"
// ./sampfos -b 2 -d 1 -f 1 -t 5 -s 4 simul.newick
// ./sampfos -b 2 -d 1 -f 1 -t 5 -s 1 aaa

int main(int argc, char **argv) {	
	char outputName[STRING_SIZE], *outputPrefix, option[256];
	FILE *fi, *fo, *fs;
	int i, nb, niter = 5, number, minContemp = 10, maxSizeTree = 500, minSizeTree = 150;
	double birth = 2., death = 1., fossil = 1., maxTime = 5.;
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
			if((i+1)<argc && sscanf(argv[i+1], "%lf", &fossil) == 1)
				i++;
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
		if(option['t']) {
			option['t'] = 0;
			if((i+1)<argc && sscanf(argv[i+1], "%lf", &maxTime) == 1)
				i++;
		}
		if(option['h']) {
			printf("%s\n", HELP_MESSAGE);
			exit(EXIT_SUCCESS);
		}
	}
	if(!(i<argc && sscanf(argv[i++], "%s", outputName) == 1)) {
		strcpy(outputName, "Tree_simul");
	}
	if((outputPrefix = strrchr(outputName, '.')) != NULL)
		outputPrefix[0] = '\0';
	if((outputPrefix=strrchr(outputName, '/')) == NULL)
		outputPrefix = outputName;
	else
		outputPrefix++;
	for(nb=1; nb <= niter; nb++) {
		TypeFossilFeature *fos;
		TypeTree *tree, *tree1, *tree2;
		FILE *ft;
		char outputName[STRING_SIZE];
		tree = NULL;
		do {
			if(tree != NULL)
				freeTree(tree);
			tree = simulTree(rg, birth, death, maxTime);
		} while(!(tree != NULL && (countContemp(tree))>=minContemp && tree->size<=maxSizeTree));
		fos = addFossils(rg, fossil, tree);
		tree1 = pruneFossil(tree, fos);
		freeFossilFeature(fos);
		freeTree(tree);
		tree2 = fixBinaryFossil(tree1, (TypeFossilFeature*) tree1->info);
		freeFossilFeature((TypeFossilFeature*) tree1->info);
		freeTree(tree1);
		sprintf(outputName, "%s_sim_%d.newick", outputPrefix, nb);
		if(ft = fopen(outputName, "w")) {
			int n;
			fprintf(ft, "\n\n");
			tree2->minTimeInt.inf = 0.;
			tree2->minTimeInt.sup = 0.;
			tree2->maxTimeInt.inf = maxTime;
			tree2->maxTimeInt.sup = maxTime;
			for(n=0; n<tree2->size; n++)
				if(tree2->time[n] != maxTime)
					tree2->time[n] = NO_TIME;
			tree2->minTime = 0.;
			tree2->maxTime = maxTime;
			tree2->minTimeInt.inf = 0.;
			tree2->minTimeInt.sup = 0.;
			tree2->maxTimeInt.inf = maxTime;
			tree2->maxTimeInt.sup = maxTime;
			for(n=0; n<tree2->size; n++)
				if(tree2->time[n] != maxTime)
					tree2->time[n] = NO_TIME;
			tree2->name = nameLeaves("leaf_", tree2);
			tree2->comment = (char**) malloc(tree2->sizeBuf*sizeof(char*));
			for(n=0; n<tree2->sizeBuf; n++)
				tree2->comment[n] = NULL;
			fillCommentFossil(tree2, (TypeFossilFeature*) tree2->info);
			fprintSubtreeNewick(ft, tree2->root, tree2);
			fprintf(ft, "\n");
			fclose(ft);
		}
		freeFossilFeature((TypeFossilFeature*) tree2->info);
		freeTree(tree2);
	}
	return 0;
}
