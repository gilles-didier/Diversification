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
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multimin.h>

#include "Utils.h"
#include "Tree.h"
#include "SimulTree.h"
#include "Fossil.h"
#include "FossilInt.h"
#include "Model.h"
#include "Uncertainty.h"
#include "SimulFossil.h"
#include "MinimizeNLOpt.h"


#ifdef DO_PS
#endif

#define STRING_SIZE 300
#define INC_CUT 5
#define SEQ_SIZE 30
#define EXT_OUTPUT "_added.phy"
#define MAX_PRED 7.5E+07

#define SIZE_BUFFER_CHAR 300
#define INFTY 1E99
#define RINFTY 1E99
#define DEF 10
#define MIN_VAL 0.000001
#define DELTA 0.000001

#define MINVAL 0.01
#define TRIAL 10
#define FIX_VAL(x) (((x)<=0)?MINVAL:(x))

#define MAX_ITER 1000

#define M_MAX 6
#define M_MAX_F 4
#define MIN_TREE 20
#define MAX_TRIALS 1000

#define HELPMESSAGE "\n\nusage: estimate [options] <input file> [<output file>]\n\nEstimate the diversification rates of the tree contained in the input file.\nThe input file has to be in Newick format with special tags for fossils ages and origin and end of the diversification, \nit returns a text report with the estimates.\n\nOptions are:\n\t-o <options file name>\tload the settings of the optimizer. <options file name> has to be in the format:\n\t\t:SPE [0;1] :EXT [0;1] :FOS [0:1] :TRI 10 :TOL 1.E-7 :ITE 500\n\t-h\tdisplay help\n\n"


static void fprintReport(FILE *f, TypeEstimation estimationMean, TypeEstimation estimationStd, TypeNLOptOption nloptOption);

void fprintReport(FILE *f, TypeEstimation estimationMean, TypeEstimation estimationStd, TypeNLOptOption nloptOption) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char buffer[50];
	fprintf(f, "Diversification execution report\n%d/%d/%d at %d:%d\n\n", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
	fprintf(f, "\nEstimations\tMean\tStandard Deviation\n");
	sprintf(buffer, "%.3lE", estimationMean.param.birth);
	fprintf(f, "Speciation rate:\t%s", buffer);
	sprintf(buffer, "%.3lE", estimationStd.param.birth);
	fprintf(f, "\t%s\n", buffer);
	sprintf(buffer, "%.3lE", estimationMean.param.death);
	fprintf(f, "Extinction rate:\t%s", buffer);
	sprintf(buffer, "%.3lE", estimationStd.param.death);
	fprintf(f, "\t%s\n", buffer);
	sprintf(buffer, "%.3lE", estimationMean.param.fossil);
	fprintf(f, "Fossil find rate:\t%s", buffer);
	sprintf(buffer, "%.3lE", estimationStd.param.fossil);
	fprintf(f, "\t%s\n", buffer);
	sprintf(buffer, "%.3lE", estimationMean.logLikelihood);
	fprintf(f, "Log-likelihood:\t%s", buffer);
	sprintf(buffer, "%.3lE", estimationStd.logLikelihood);
	fprintf(f, "\t%s\n", buffer);
	fprintf(f, "\nOptimization settings\n");
	fprintNLoptOption(f, &nloptOption);
}

int main(int argc, char **argv) {	
	char inputName[STRING_SIZE], outputFileName[STRING_SIZE], outputName[STRING_SIZE], *outputPrefix, outputEstimate[STRING_SIZE], inputFileTimeName[STRING_SIZE], option[256];
	FILE *fi, *fo, *fs;
	int i, j, width=1000, height=500, Y_MAX=height-50, timeF = 0, nb, size = 5, number, minContemp = 20, maxSizeTree = 200;
	double birth = 2., death = 1., fossil = 1., maxTime = 5, iter = 1;
	TypeTree *tree;
	gsl_rng *rg = gsl_rng_alloc(gsl_rng_random_glibc2);
	TypeNLOptOption nloptOption;
		
    nloptOption.infSpe = 0.;
    nloptOption.supSpe = 1.;
    nloptOption.infExt = 0.;
    nloptOption.supExt = 1.;
    nloptOption.infFos = 0.;
    nloptOption.supFos = 1.;
    nloptOption.trials = 10;
    nloptOption.tolOptim = 0.00001;
    nloptOption.maxIter = 500;
	
	for(i=0; i<256; i++)
		option[i] = 0;
	   
	for(i=1; i<argc && *(argv[i]) == '-'; i++) {
		for(j=1; argv[i][j] != '\0'; j++)
			option[argv[i][j]] = 1;
		if(option['o']) {
			FILE *fopt;
			if((i+1)<argc) {
				if((fopt = fopen(argv[++i], "r"))) {
					fscanNLoptOptionTag(fopt, &nloptOption);
					fclose(fopt);
				} else {
					fprintf(stderr, "Can't open file %s\n", argv[++i]);
					exit(EXIT_FAILURE);
				}
			} else {
				fprintf(stderr, "File name missing after option -o\n");
				exit(EXIT_FAILURE);
			}
		}
		if(option['h']) {
			option['h'] = 0;
			printf("%s\n", HELPMESSAGE);
			exit(0);
		}
	}
	if(!(i<argc && sscanf(argv[i++], "%s", inputName) == 1)) {
		fprintf(stderr, "%s\n",  HELPMESSAGE);
		exit(1);
	}
	if(!(i<argc && sscanf(argv[i++], "%s", outputName) == 1)) {
		strcpy(outputName, "report.txt");
	}
    if(fi = fopen(inputName, "r")) {
		TypeFossilIntFeature *fos;
		TypeFossilFeature *ff;
		TypeEstimation mean, std, *estimation;
		double *timeSave;
		int i;
		estimation = (TypeEstimation*) malloc(iter*sizeof(TypeEstimation));
		mean.param.birth = 0.;
		mean.param.death = 0.;
		mean.param.fossil = 0.;
		mean.logLikelihood = 0.;
		std.param.birth = 0.;
		std.param.death = 0.;
		std.param.fossil = 0.;
		std.logLikelihood = 0.;
		
		int n;
        tree = readTree(fi);
        fclose(fi);
        if(tree == NULL)
            return;
        toBinary(tree);
        reorderTree(tree->name, tree);
        fos = fosToFossilInt(tree);
        fprintf(stdout, "\n");
        fprintTreeFossilInt(stdout, tree, fos);
        fprintf(stdout, "\n");
		tree->minTime = 0.;
		tree->maxTime = 5.;
		timeSave = tree->time;
		tree->time = (double*) malloc(tree->size*sizeof(double));
		for(n=0; n<tree->size; n++) {
			if(tree->node[n].child == NOSUCH) {
				switch(fos->status[n]) {
					case contempNodeStatus:
						tree->time[n] = tree->maxTime;
					break;
					case extinctNodeStatus:
						tree->time[n] = NO_TIME;
					break;
					case unknownNodeStatus:
						tree->time[n] = fos->endTimeTable[fos->endTime[n]].inf+UNIF_RAND*(fos->endTimeTable[fos->endTime[n]].sup-fos->endTimeTable[fos->endTime[n]].inf);
					break;
					default:
						fprintf(stderr, "Node %d has no status\n", n);
						return;
				}
			}
		}
		for(i=0; i<iter; i++) {
			TypeEstimation estim;
			tree->minTime = tree->minTimeInt.inf+UNIF_RAND*(tree->minTimeInt.sup-tree->minTimeInt.inf);
			tree->maxTime = tree->maxTimeInt.inf+UNIF_RAND*(tree->maxTimeInt.sup-tree->maxTimeInt.inf);
			ff = sampleFossilInt(fos, tree->size);
			if(!minimizeBirthDeathFossilFromTreeFossil(getLogLikelihoodTreeFossil, tree, ff, &nloptOption, &(estimation[i]))) {
				fprintf(stderr, "Estimation issue\n");
			}
		}
		for(i=0; i<iter; i++) {
			 mean.param.birth += estimation[i].param.birth;
			 mean.param.death += estimation[i].param.death;
			 mean.param.fossil += estimation[i].param.fossil;
			 mean.logLikelihood += estimation[i].logLikelihood;
		 }
		 mean.param.birth /= (double) iter;
		 mean.param.death /= (double) iter;
		 mean.param.fossil /= (double) iter;
		 mean.logLikelihood /= (double) iter;
		 if(iter>1) {
			 for(i=0; i<iter; i++) {
				 std.param.birth += pow(estimation[i].param.birth-mean.param.birth, 2.);
				 std.param.death += pow(estimation[i].param.death-mean.param.death, 2.);
				 std.param.fossil += pow(estimation[i].param.fossil-mean.param.fossil, 2.);
				 std.logLikelihood += pow(estimation[i].logLikelihood-mean.logLikelihood, 2.);
			 }
			 std.param.birth /= (double) iter-1.;
			 std.param.death /= (double) iter-1.;
			 std.param.fossil /= (double) iter-1.;
			 std.logLikelihood /= (double) iter-1.;
			 std.param.birth = sqrt(std.param.birth);
			 std.param.death = sqrt(std.param.death);
			 std.param.fossil = sqrt(std.param.fossil);
			 std.logLikelihood = sqrt(std.logLikelihood);
		 }
        if((fo = fopen(outputName, "w"))) {
			fprintReport(fo, mean, std, nloptOption);
			fclose(fo);
		}
	}
	return 0;
}
