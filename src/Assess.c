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
#include <pthread.h>
#include <omp.h>
#include <gsl/gsl_rng.h>

#include "Utils.h"
#include "Tree.h"
#include "SimulTree.h"
#include "Fossil.h"
#include "Model.h"
#include "Uncertainty.h"
#include "SimulFossil.h"
#include "MinimizeNLOpt.h"
#include "GNUFile.h"

#ifdef DO_PS
#endif

#define STRING_SIZE 300
#define INC_CUT 5
#define SEQ_SIZE 30
#define EXT_OUTPUT "_added.phy"


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
#define MAX_NF 10

#define M_MAX 6
#define M_MAX_F 4
#define MIN_TREE 20
#define MAX_PRED 7.5E+07
//./assess -f 1. -f 2. -f 3. -l 5 8 0.5 -i 1000 -M 500

#define HELP_MESSAGE "\nusage: assess [options] [<output file>]\n\nassess simulates random trees and fossils finds, estimates speciation and extinction rates and returns the mean absolute error\n\n\nOptions:\n\t-h : display help\n\t-b <birth>\t: set birth rate\n\t-d <death>\t: set death rate\n\t-f <fossil>\t: set fossil find rate\n\t-m <min>\t: set minimum number of contemporary species of a simulation to be considered\n\t-M <size>\t: set maximum size of a simulation to be considered\n\t-i <niter>\t: set the number of simulations\n\t-l <start> <end> <step> : the range of the end time of the diversification (start is always 0)\n\t-o <options file name>\t: load the settings of the optimizer. <options file name> has to be in the format:\n\t\t:SPE [0;1] :EXT [0;1] :FOS [0:1] :TRI 10 :TOL 1.E-7 :ITE 500\n\t-x <number>\t: set the number of threads\n\t-c <complexity>\t: set the max complexity index of a simulation to be considered\n\n\n"

typedef double TypeFuncGSL(const gsl_vector *, void *);

typedef double TypeErrorFunction(double, double);

typedef struct OBSERVATION {
	double length;
	int number;
} TypeObservation;


typedef struct NODE_FOSSIL {
	int node, fossil;
	double time;
} TypeNodeFossil;

typedef struct NODE_FOSSIL_TABLE {
	int size;
	TypeNodeFossil *table;
} TypeNodeFossilTable;


static TypeFuncGSL *funcCurrent;
static TypeListEvent *getEventSequenceStd(TypeTree *tree, TypeFossilFeature *fos);
static TypeListEvent *getEventSequenceBas(TypeTree *tree);
static void getNodeFossil(TypeNodeFossil start, TypeTree *tree, TypeFossilTab *fos, TypeNodeFossilTable *nf);
static void getWaitingTimesTable(TypeNodeFossil start, TypeTree *tree, TypeFossilTab *fos, int nfos, TypeObservation *obs);
static double getFossilRate(TypeTree *tree, TypeFossilFeature *fos);
static void funcCurrentGSL_df(const gsl_vector *v, void *params, gsl_vector *df);
static void funcCurrentGSL_fdf (const gsl_vector *v, void *params, double *f, gsl_vector *df);

static void getNodeFossilDebug(TypeNodeFossil start, TypeTree *tree, TypeFossilTab *fos, TypeNodeFossilTable *nf);
static void getWaitingTimesTableDebug(TypeNodeFossil start, TypeTree *tree, TypeFossilTab *fos, TypeNodeFossilTable *nf, TypeTableDouble *waitTimes);
static double getFossilRateDebug(TypeTree *tree, TypeFossilFeature *fos);
static double *getTableExpRandom(gsl_rng *rgen, int size, double rate);
static int simulate(gsl_rng *r, double birth, double death, double *fossil, int nf, double **resBirth, double **resDeath, double **resFossil, int **resStat, int niter, int fixed, int *err);
static int simulateMP(gsl_rng *r, double birth, double death, double *fossil, int nf, double **resBirth, double **resDeath, double **resFossil, int **resStat, int niter);


double AbsoluteError(double x, double y);
double StandardError(double x, double y);
double QuadraticError(double x, double y);
double getMeanError(double real_val, double *data, int size, TypeErrorFunction *error);
double getStdDevError(double mean, double real_val, double *data, int size, TypeErrorFunction *error);



typedef struct {
	double birth, death, *fossil, start, step, **resBirth, **resDeath, **resFossil;
	int **resStat;
	int nf, niter, size, *iter, srbd, srf;
	int minContemp, maxSizeTree;
	double maxComplexity;
	TypeNLOptOption nloptOption;
} TypeThreadArgument;


typedef struct {
	int number, maxThreads, *cur;
	gsl_rng *r;
	TypeThreadArgument arg;
	pthread_mutex_t mutex_cur;
	pthread_mutex_t mutex_rand;
	pthread_mutex_t mutex_number;
	pthread_mutex_t mutex_arg;
	pthread_cond_t cond_number;
} TypeThreadData;

 
TypeThreadData set =
{
	.number = 0,
	.maxThreads = 40,
	.mutex_cur = PTHREAD_MUTEX_INITIALIZER,
	.mutex_rand = PTHREAD_MUTEX_INITIALIZER,
	.mutex_number = PTHREAD_MUTEX_INITIALIZER,
	.mutex_arg = PTHREAD_MUTEX_INITIALIZER,
	.cond_number = PTHREAD_COND_INITIALIZER
};

static void *simulThread(void *data) {
	TypeTree *tree = (TypeTree*) data;
	double *bufferB = (double*) malloc(set.arg.srbd*sizeof(double));
	double *bufferD = (double*) malloc(set.arg.srbd*sizeof(double));
	double *bufferF = (double*) malloc(set.arg.srf*sizeof(double));
	int *bufferS = (int*) malloc(3*sizeof(int)), i = (int) round((tree->maxTime-set.arg.start)/set.arg.step);;
	double totTime;
	double pred = 0.;
	TypeTree *tree1, *tree2;
	TypeListEvent *event;
	TypeEstimation estimation;
	int ok = 1;
	int m = 0, u, mc;
	event = getEventSequenceBas(tree);
	int nBirth, nDeath, nFossil, nContemp, curContemp;
	getStatEvent(event, &nBirth,  &nDeath,  &nFossil);
	totTime = getTotalTimeEvent(event);
	freeListEvent(event);
	bufferS[0] = nBirth;
	bufferS[1] = nDeath;
	bufferS[2] = curContemp;
	bufferB[m] = ((double)nBirth)/totTime;
	bufferD[m] = ((double)nDeath)/totTime;
	m++;
	mc = m; /*keep the comtemporary for later since it is almost always OK*/
	m++;
	for(u=0; ok && u<set.arg.nf; u++) {
		TypeTree *treef;
		TypeFossilFeature *fos;
		int n;	
		pthread_mutex_lock (&set.mutex_rand);
		fos = addFossils(set.r, set.arg.fossil[u], tree);
		pthread_mutex_unlock (&set.mutex_rand);
		event = getEventSequenceStd(tree, fos);
		getStatEvent(event, &nBirth,  &nDeath,  &nFossil);
		freeListEvent(event);
		bufferF[u] = ((double)nFossil)/totTime;
		tree1 = pruneFossil(tree, fos);
		freeFossilFeature(fos);
		tree2 = fixBinaryFossil(tree1,  (TypeFossilFeature*) tree1->info);
		freeFossilFeature((TypeFossilFeature*) tree1->info);
		freeTree(tree1);
		pred = getItemNumber(tree2, (TypeFossilFeature*) tree2->info);
		if(pred < set.arg.maxComplexity) {
			event = getEventSequenceStd(tree2, (TypeFossilFeature*) tree2->info);
			estimation.param.fossil = 0.;
			for(n=0; n<event->size; n++)
				if(event->list[n].type == 'f')
					estimation.param.fossil++;
			estimation.param.fossil /= getTotalTimeEvent(event);
			setFossilRate(estimation.param.fossil);
			bufferF[set.arg.nf+u] = estimation.param.fossil;
			if(ok && minimizeBirthDeathFromEventList(getEventFossilProbLogY, event, &(set.arg.nloptOption), &estimation)>=0) {
				bufferB[m+u] = estimation.param.birth;
				bufferD[m+u] = estimation.param.death;
			} else
				ok = 0;
			freeListEvent(event);
			estimation.param.birth = 0.;
			estimation.param.death = 0.;
			estimation.param.fossil = 0.;
			if(ok && minimizeBirthDeathFossilFromTreeFossil(getLogLikelihoodTreeFossil, tree2, (TypeFossilFeature*) tree2->info, &(set.arg.nloptOption), &estimation)) {
				bufferB[m+set.arg.nf+u] = estimation.param.birth;
				bufferD[m+set.arg.nf+u] = estimation.param.death;
				bufferF[2*set.arg.nf+u] = estimation.param.fossil;
			} else
				ok = 0;
		} else
			ok = 0;
		freeFossilFeature((TypeFossilFeature*) tree2->info);
		freeTree(tree2);
	}
	if(ok) {
		tree1 = pruneContemp(tree);
		tree2 = fixBinary(tree1);
		freeTree(tree1);
		event = getEventSequenceBas(tree2);
		freeTree(tree2);
		if(minimizeBirthDeathFromEventList(getEventPhyloProbLogX, event, &(set.arg.nloptOption), &estimation)>=0) {
			bufferB[mc] = estimation.param.birth;
			bufferD[mc] = estimation.param.death;
		} else
			ok = 0;
		freeListEvent(event);
	}
	if(ok) {
		pthread_mutex_lock (&set.mutex_arg);
		if(set.arg.iter[i]<set.arg.niter) {
			int m;
			for(m=0; m<3; m++)
				set.arg.resStat[m][i] = bufferS[m];
			for(m=0; m<set.arg.srbd; m++) {
				set.arg.resBirth[m][i] += AbsoluteError(set.arg.birth, bufferB[m]);
				set.arg.resDeath[m][i] += AbsoluteError(set.arg.death, bufferD[m]);
			}
			for(m=0; m<set.arg.nf; m++)
				set.arg.resFossil[m][i] += AbsoluteError(set.arg.fossil[m], bufferF[m]);
			for(m=0; m<set.arg.nf; m++)
				set.arg.resFossil[set.arg.nf+m][i] += AbsoluteError(set.arg.fossil[m], bufferF[set.arg.nf+m]);
			for(m=0; m<set.arg.nf; m++)
				set.arg.resFossil[2*set.arg.nf+m][i] += AbsoluteError(set.arg.fossil[m], bufferF[2*set.arg.nf+m]);
			set.arg.iter[i]++;
		}
		pthread_mutex_unlock (&set.mutex_arg);
fprintf(stderr, "ok thread %d\n", i);
	} else
fprintf(stderr, "ko thread %d (%.1lE)\n", i, pred);
	freeTree(tree);
	free((void*)bufferB);
	free((void*)bufferD);
	free((void*)bufferF);
	free((void*)bufferS);
	pthread_mutex_lock (&set.mutex_cur);
	set.cur[i]--;
fprintf(stderr, "fin thread %d -> %d\n", i, set.cur[i]);
	pthread_mutex_unlock (&set.mutex_cur);
	pthread_mutex_lock (&set.mutex_number);
	set.number--;
	pthread_cond_signal (&set.cond_number);
	pthread_mutex_unlock (&set.mutex_number);
	return NULL;
}

static void *mainThread (void *data) {
	int i, cont = 1;
	set.cur = (int*) malloc(set.arg.size*sizeof(int));
	for(i=0; i<set.arg.size; i++)
		set.cur[i] = 0;
	cont = 1;
	i = 0;
	while(cont) {
		pthread_mutex_lock(&set.mutex_number);
		pthread_mutex_lock(&set.mutex_cur);
		pthread_mutex_lock(&set.mutex_arg);
		while(i < set.arg.size && set.number < set.maxThreads) {
			pthread_t thread;
			TypeTree *tree = NULL;
			double maxTime = set.arg.start+((double)i)*set.arg.step;
			do {
				if(tree != NULL)
					freeTree(tree);
				pthread_mutex_lock (&set.mutex_rand);
				tree = simulTree(set.r, set.arg.birth, set.arg.death, maxTime);
				pthread_mutex_unlock (&set.mutex_rand);
			} while(!(tree != NULL && (countContemp(tree))>=set.arg.minContemp && tree->size<=set.arg.maxSizeTree));
fprintf (stderr, "Launching thread %d on simulated tree with %.1lf, %.1lf during %.1lf processing %d, completed %d (%d)\n", set.number, set.arg.birth, set.arg.death, maxTime, set.cur[i], set.arg.iter[i], set.arg.niter);
			int ret = 0;
			if((ret = pthread_create(&thread, NULL, simulThread, (void*) tree)) == 0) {
				int err;
				if((err = pthread_detach(thread)) == 0) {
					set.number++;
					set.cur[i]++;
				} else {
					fprintf (stderr, "Error %d while detaching thread: %s\n", err, (char*) strerror (err));
					pthread_kill(thread, 0);
				}
			} else
				fprintf (stderr, "Error %d while creating thread: %s\n", ret, (char*) strerror (ret));
fprintf(stderr, "iter %d cur %d, comp %d", i, set.cur[i], set.arg.iter[i]);
			for(i=0; i<set.arg.size && (set.cur[i]+set.arg.iter[i])>=set.arg.niter; i++)
				;
fprintf(stderr, " -> %d\n", i);
		}
		cont = (set.number > 0) || (i < set.arg.size);
		pthread_mutex_unlock (& set.mutex_number);
		pthread_mutex_unlock(&set.mutex_arg);
		pthread_mutex_unlock(&set.mutex_cur);
		if(cont) {
			pthread_mutex_lock(&set.mutex_number);
			pthread_cond_wait (& set.cond_number, & set.mutex_number);
			pthread_mutex_unlock (& set.mutex_number);
			pthread_mutex_lock(&set.mutex_cur);
			pthread_mutex_lock(&set.mutex_arg);
			for(i=0; i<set.arg.size && (set.cur[i]+set.arg.iter[i])>=set.arg.niter; i++)
				;
			pthread_mutex_unlock(&set.mutex_arg);
			pthread_mutex_unlock(&set.mutex_cur);
			pthread_mutex_lock(&set.mutex_number);
			cont = (set.number > 0) || (i < set.arg.size);
			pthread_mutex_unlock (& set.mutex_number);
		}
	}
		pthread_mutex_lock(&set.mutex_cur);
		pthread_mutex_lock(&set.mutex_arg);
	for(i=0; i<set.arg.size && set.arg.iter[i]>=set.arg.niter; i++)
		printf("%d\titer %d, cur %d\n", i, set.arg.iter[i], set.cur[i]);
		pthread_mutex_unlock(&set.mutex_cur);
		pthread_mutex_unlock(&set.mutex_arg);
	free((void*)set.cur);
fprintf (stderr, "End %d\n", set.number);
	return NULL;
}

int main(int argc, char **argv) {
	int i, j, k;
	char  outputFileName[SIZE_BUFFER_CHAR], bufferOutput[SIZE_BUFFER_CHAR], *outtrunc, option[256], action = 't';
	double birth = 2., death = 1., fossil = 1., start=2, end=5, step=0.5;
	TypeTree *tree, *tree1, *tree2, *tree3, *tree4, *tree5;
	TypeListEvent *event;
	int nevent = 20, niter = 20, tot = 0, sizeBirth, sizeDeath, *eff, m;
	FILE *f;
	double *resBirth[M_MAX], *resDeath[M_MAX], *resFossil[M_MAX_F], **smBirth, **smDeath, **smFossil, fossilTab[MAX_NF];
	int **smStat, nf = 0, n;
	time_t t1, t2;
	gsl_rng *rg = gsl_rng_alloc(gsl_rng_random_glibc2);
	TypeNLOptOption nloptOption;
	int minContemp = 10, maxSizeTree = 1000;
	double maxComplexity = 7.5E+07;
		
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
		if(option['x']) {
			option['x'] = 0;
			if((i+1)<argc && sscanf(argv[i+1], "%d", &(set.maxThreads)) == 1)
				i++;
		}
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
			printf("%s\n", HELP_MESSAGE);
			exit(EXIT_SUCCESS);
		}
	}

	if (i>=argc || sscanf(argv[i++], "%s", outputFileName) != 1)
		strcpy(outputFileName, "out");
	if((outtrunc = strrchr(outputFileName, '/')) != NULL)
		outtrunc++;
	else
		outtrunc = outputFileName;

t1 = time(&t1);
	{
		FILE *fmb, *fsb, *fmd, *fsd, *fmf, *fsf, *fgnu, *ftmpb, *ftmpd, *fs;
		int err, size, l, srbd, srf;
		char BUFFER[3*MAX_NF][SIZE_BUFFER_CHAR], bufferO[SIZE_BUFFER_CHAR], bufferD[SIZE_BUFFER_CHAR], **TITLE;
		TypeGNUInfo gnu;
		srbd = 2+2*nf;
		srf = 3*nf;

		TITLE = (char**) malloc(utils_MAX(srbd, srf)*sizeof(char*));
/*Gnuplot general info*/
		gnu.type = EPS;
		gnu.xLabel = "time";
		gnu.yLabel = "absolute error";
		gnu.title = TITLE;
		gnu.outputFileName = bufferO;
		gnu.dataFileName = bufferD;
		TITLE[0] = "complete information";
		TITLE[1] = "contemporary lineages + divergence times";
		for(m=0; m<nf; m++) {
			sprintf(BUFFER[2+m], "contemporary lineages + divergence times + fossil finds with rate %.2lf", fossilTab[m]);
			TITLE[2+m] = BUFFER[2+m];
		}
		for(m=0; m<nf; m++) {
			sprintf(BUFFER[2+nf+m], "contemporary lineages + fossil finds with rate %.2lf", fossilTab[m]);
			TITLE[2+nf+m] = BUFFER[2+nf+m];
		}
		gnu.nColumn = 2+2*nf;
/*Birth files*/
		sprintf(bufferO, "%s_Abs_Mean_birth_r%.1lf_%d.eps", outtrunc, birth/death, niter);
		sprintf(bufferD, "%s_Abs_Mean_birth_r%.1lf_%d.csv", outputFileName, birth/death, niter);
		sprintf(bufferOutput, "%s_Abs_Mean_birth_r%.1lf_%d.gnu", outputFileName, birth/death, niter);
		if((fgnu = fopen(bufferOutput, "w"))) {
			fprintGNUFile(fgnu, &gnu);
			fclose(fgnu);
		} else {
			fprintf(stderr, "Can't open file %s\n", bufferOutput);
			exit(EXIT_FAILURE);
		}
		if(!(fmb = fopen(bufferD, "w"))) {
			fprintf(stderr, "Can't open file %s\n", bufferD);
			exit(EXIT_FAILURE);
		}
/*Death files*/
		sprintf(bufferO, "%s_Abs_Mean_death_r%.1lf_%d.eps", outtrunc, birth/death, niter);
		sprintf(bufferD, "%s_Abs_Mean_death_r%.1lf_%d.csv", outputFileName, birth/death, niter);
		sprintf(bufferOutput, "%s_Abs_Mean_death_r%.1lf_%d.gnu", outputFileName, birth/death, niter);
		if((fgnu = fopen(bufferOutput, "w"))) {
			fprintGNUFile(fgnu, &gnu);
			fclose(fgnu);
		} else {
			fprintf(stderr, "Can't open file %s\n", bufferOutput);
			exit(EXIT_FAILURE);
		}
		if(!(fmd = fopen(bufferD, "w"))) {
			fprintf(stderr, "Can't open file %s\n", bufferD);
			exit(EXIT_FAILURE);
		}
/*Fossil files*/
		gnu.nColumn = 3*nf;
		for(m=0; m<nf; m++) {
			sprintf(BUFFER[m], "complete information + fossil finds with rate %.2lf", fossilTab[m]);
			TITLE[m] = BUFFER[m];
		}
		for(m=0; m<nf; m++) {
			sprintf(BUFFER[nf+m], "contemporary lineages + divergence times + fossil finds with rate %.2lf", fossilTab[m]);
			TITLE[nf+m] = BUFFER[nf+m];
		}
		for(m=0; m<nf; m++) {
			sprintf(BUFFER[2*nf+m], "contemporary lineages + fossil finds with rate %.2lf", fossilTab[m]);
			TITLE[2*nf+m] = BUFFER[2*nf+m];
		}
		sprintf(bufferO, "%s_Abs_Mean_fossil_r%.1lf_%d.eps", outtrunc, birth/death, niter);
		sprintf(bufferD, "%s_Abs_Mean_fossil_r%.1lf_%d.csv", outputFileName, birth/death, niter);
		sprintf(bufferOutput, "%s_Abs_Mean_fossil_r%.1lf_%d.gnu", outputFileName, birth/death, niter);
		if((fgnu = fopen(bufferOutput, "w"))) {
			fprintGNUFile(fgnu, &gnu);
			fclose(fgnu);
		} else {
			fprintf(stderr, "Can't open file %s\n", bufferOutput);
			exit(EXIT_FAILURE);
		}
		if(!(fmf = fopen(bufferD, "w"))) {
			fprintf(stderr, "Can't open file %s\n", bufferD);
			exit(EXIT_FAILURE);
		}
		free((void*)TITLE);

		size = (int) floor((end-start+step/10.)/step)+1;
		smBirth = (double**) malloc(srbd*sizeof(double));
		smDeath = (double**) malloc(srbd*sizeof(double));
		smFossil = (double**) malloc(srf*sizeof(double));
		for(m=0; m<srbd; m++) {
			smBirth[m] = (double*) malloc(size*sizeof(double));
			smDeath[m] = (double*) malloc(size*sizeof(double));
			for(i=0; i<size; i++) {
				smBirth[m][i] = 0.;
				smDeath[m][i] = 0.;
			}
		}
		for(m=0; m<srf; m++) {
			smFossil[m] = (double*) malloc(size*sizeof(double));
			for(i=0; i<size; i++)
				smFossil[m][i] = 0.;
		}
		smStat = (int**) malloc(3*sizeof(int*));
		for(m=0; m<3; m++) {
			smStat[m] = (int*) malloc(size*sizeof(int));
			for(i=0; i<size; i++)
				smStat[m][i] = 0.;
		}
		set.r = rg;
		set.arg.minContemp = minContemp;
		set.arg.maxSizeTree = maxSizeTree;
		set.arg.maxComplexity = maxComplexity;
		set.arg.nloptOption = nloptOption;	
		set.arg.birth = birth;
		set.arg.death = death;
		set.arg.fossil = fossilTab;
		set.arg.start = start;
		set.arg.step = step;
		set.arg.resBirth = smBirth;
		set.arg.resDeath = smDeath;
		set.arg.resFossil = smFossil;
		set.arg.resStat = smStat;
		set.arg.nf = nf;
		set.arg.niter = niter;
		set.arg.size = size;
		set.arg.iter = (int*) malloc(size*sizeof(int));
		for(i=0; i<size; i++)
			set.arg.iter[i] = 0;
		set.arg.srbd = srbd;
		set.arg.srf = srf;
		int ret;
		pthread_t thread;
		if((ret = pthread_create (&thread, NULL, mainThread, NULL)) == 0) {
			pthread_join (thread, NULL);
		} else {
			fprintf (stderr, "Erreur %d %s\n", ret, (char*) strerror (ret));
			exit(1);
		}
		for(i=0; i<size; i++) {
			double r, mean, std, maxTime;
			int tot, m;
			maxTime = start+((double)i)*step;
			fprintf(fmb, "%lf", maxTime);
			for(m=0; m<srbd; m++) {
				double mean, std;
				mean = set.arg.resBirth[m][i]/((double)set.arg.iter[i]);
				fprintf(fmb, "\t%lE", mean);
			}
			fprintf(fmb, "\n");
			fprintf(fmd, "%lf", maxTime);
			for(m=0; m<srbd; m++) {
				double mean, std;
				mean = set.arg.resDeath[m][i]/((double)set.arg.iter[i]);
				fprintf(fmd, "\t%lE", mean);
			}
			fprintf(fmd, "\n");
			fprintf(fmf, "%lf", maxTime);
			for(m=0; m<srf; m++) {
				double mean, std;
				mean = set.arg.resFossil[m][i]/((double)set.arg.iter[i]);
				fprintf(fmf, "\t%lE", mean);
			}
			fprintf(fmf, "\n");
		}
		fclose(fmb);
		fclose(fmd);
		fclose(fmf);
		for(m=0; m<srbd; m++) {
			free((void*)smBirth[m]);
			free((void*)smDeath[m]);
		}
		for(m=0; m<srf; m++)
			free((void*)smFossil[m]);
		for(m=0; m<3; m++)
			free((void*)smStat[m]);
		free((void*)smBirth);
		free((void*)smDeath);
		free((void*)smFossil);
		free((void*)smStat);
	}
t2 = time(&t2);
printf("\nTotal time %.1lf\n", difftime(t2, t1));
	gsl_rng_free(rg);
	exit(EXIT_SUCCESS);
}


double AbsoluteError(double x, double y) {
	return fabs(x-y);
}

double StandardError(double x, double y) {
	return (x-y);
}

double QuadraticError(double x, double y) {
	return (x-y);
}

double getMeanError(double real_val, double *data, int size, TypeErrorFunction *error) {
	double mean = 0.;
	int k;
	for(k=0; k<size; k++)
		mean += error(data[k], real_val);
	mean /= (double) size;
	return mean;
}

double getStdDevError(double mean, double real_val, double *data, int size, TypeErrorFunction *error) {
	double std = 0.;
	int k;
	for(k=0; k<size; k++)
		std += pow(error(data[k], real_val)-mean, 2.);
	std /= (double) size-1.;
	return std;
}


double *getTableExpRandom(gsl_rng *rgen, int size, double rate) {
	double *table;
	int i;
	table = (double*) malloc(size*sizeof(double));
	for(i=0; i<size; i++)
		table[i] = gsl_ran_exponential(rgen, 1./rate);
	return table;
}

void getNodeFossil(TypeNodeFossil start, TypeTree *tree, TypeFossilTab *fos, TypeNodeFossilTable *nf) {
	if(fos[start.node].size > start.fossil+1) {
		nf->table[nf->size].node = start.node;
		nf->table[nf->size].fossil = start.fossil+1;
		nf->table[nf->size].time = fos[nf->table[nf->size].node].time[nf->table[nf->size].fossil];
		nf->size++;
		return;
	} else {
		TypeNodeFossil tmp;
		tmp.fossil = -1;
		if(tree->node[start.node].child >= 0)
			for(tmp.node=tree->node[start.node].child; tmp.node>=0; tmp.node=tree->node[tmp.node].sibling)
				getNodeFossil(tmp, tree, fos, nf);
		else {
			nf->table[nf->size].node = start.node;
			nf->table[nf->size].fossil = -1;
			nf->table[nf->size].time = tree->time[start.node];
			nf->size++;
		}
	}
}
			
void getWaitingTimesTable(TypeNodeFossil start, TypeTree *tree, TypeFossilTab *fos, int nfos, TypeObservation *obs) {
	int i;
	TypeNodeFossilTable nf;
	nf.size = 0;
	nf.table = (TypeNodeFossil*) malloc(nfos*sizeof(TypeNodeFossil));
	getNodeFossil(start, tree, fos, &nf);
	if(nf.size>0) {
		int ind = rand()%nf.size;
		obs->length += nf.table[ind].time-start.time;
		if(nf.table[ind].fossil>=0 && fos[nf.table[ind].node].time[nf.table[ind].fossil] < tree->time[nf.table[ind].node])
			obs->number++;
	}
	for(i=0; i<nf.size; i++) {
		if(!(nf.table[i].fossil<0 && nf.table[i].time == tree->time[nf.table[i].node]))
			getWaitingTimesTable(nf.table[i], tree, fos, nfos, obs);
	}
	free((void*)nf.table);
}
	
/*compute the fossil rate by selecting #fossils independent waiting times of the Poisson process*/
double getFossilRate(TypeTree *tree, TypeFossilFeature *fos) {
	TypeTableDouble waitTimes;
	TypeNodeFossil start;
	TypeObservation obs;
	int n, c, nevent = 0;
	TypeFossilTab *ftab;
	obs.length = 0.;
	obs.number = 0;
	ftab = listToFossilTab(fos, tree->size);
	waitTimes.size = 0;	
	waitTimes.sizeBuffer = fos->size;
	waitTimes.table = (double*) malloc(waitTimes.sizeBuffer*sizeof(double));
	if(tree->size == 0)
		return 0.;
	start.node = tree->root;
	start.fossil = -1;
	start.time = tree->minTime;
	getWaitingTimesTable(start, tree, ftab, fos->size, &obs);
	if(obs.length>0.) {
//		printf("number %d, length %.2lf (%.2lf)\n", obs.number, obs.length, ((double)obs.number)/obs.length);
		return ((double)obs.number)/obs.length;
	} else {
		return 0.;
	}
	freeFossilTab(ftab, tree->size);
}




/*return the sequence of speciation/extinction/fossil find events occurring in "tree", chronologically ordered in ascending order*/
/*event.list[i].n refers to the number of lineages alive just before the ith event occurs*/
TypeListEvent *getEventSequenceStd(TypeTree *tree, TypeFossilFeature *fos) {
	double time;
	int cur[MAX_CURRENT], ncur, i, ind = 0, cont = 1, nbuf = INC_SIZE;
	TypeListEvent *res;
	res = (TypeListEvent*) malloc(sizeof(TypeListEvent));
	res->size = 0;
	if(tree->size == 0) {
		res->list = NULL;
		return res;
	}
	res->list = (TypeEvent*) malloc(nbuf*sizeof(TypeEvent));
	if(tree->time[tree->root] == 0.) {
		ncur = 2;
		cur[0] = tree->node[tree->root].child;
		cur[1] = tree->node[tree->node[tree->root].child].sibling;
	} else {
		ncur = 1;
		cur[0] = tree->root;
	}
	time = 0.;
	while((tree->maxTime-time)>EPSILON) {
		double tf, tbd;
		int which;
		if(ind<fos->size)
			tf = fos->fossilList[ind].time;
		else
			tf = POS_INFTY;
		tbd = tree->maxTime;
		for(i=0; i<ncur; i++) {
			if(tree->time[cur[i]] < tbd) {
				tbd = tree->time[cur[i]];
				which = i;
			}
		}
		time = utils_MIN(tf,tbd);
		if((tree->maxTime-time)>EPSILON) {
			if(res->size>=nbuf) {
				nbuf += INC_SIZE;
				res->list = (TypeEvent*) realloc((void*)res->list, nbuf*sizeof(TypeEvent));
			}
			if(tf<tbd) {
				res->list[res->size].time = tf;
				res->list[res->size].n = ncur;
				res->list[res->size].type = 'f';
				(res->size)++;
				ind++;
			} else {
				if(tf==tbd)
					ind++;
				res->list[res->size].time = tbd;
				res->list[res->size].n = ncur;
				if(tree->node[cur[which]].child>=0 && tree->node[tree->node[cur[which]].child].sibling>=0) {
					if(ncur >= MAX_CURRENT) {
						printf("too much lineages in counting events");
						exit(EXIT_FAILURE);
					}
					res->list[res->size].type = 'b';
					(res->size)++;
					cur[ncur] = tree->node[tree->node[cur[which]].child].sibling;
					cur[which] = tree->node[cur[which]].child;
					ncur++;
				} else {
					res->list[res->size].type = 'd';
					(res->size)++;
					for(i=which+1; i<ncur; i++)
						cur[i-1] = cur[i];
					ncur--;
				}
			}
		}
	}
	if(res->size)
		res->list = (TypeEvent*) realloc((void*)res->list, (res->size)*sizeof(TypeEvent));
	else {
		free((void*)res->list);
		res->list = NULL;
	}
	res->minTime = tree->minTime;
	res->maxTime = tree->maxTime;
	return res;
}

TypeListEvent *getEventSequenceBas(TypeTree *tree) {
	double time;
	int cur[MAX_CURRENT], ncur, i, ind = 0, cont = 1, nbuf = INC_SIZE;
	TypeListEvent *res;

	res = (TypeListEvent*) malloc(sizeof(TypeListEvent));
	res->size = 0;
	if(tree == NULL || tree->size == 0) {
fprintf(stderr, "Empty tree\n");
exit(1);
		res->list = NULL;
		return res;
	}
	res->list = (TypeEvent*) malloc(nbuf*sizeof(TypeEvent));
	if(tree->time[tree->root] == tree->minTime) {
		if(tree->node[tree->root].child != NOSUCH) {
			if(res->size>=nbuf) {
				nbuf += INC_SIZE;
				res->list = (TypeEvent*) realloc((void*)res->list, nbuf*sizeof(TypeEvent));
			}
			res->list[res->size].time = tree->minTime;
			res->list[res->size].n = 1;
			res->list[res->size].type = 'b';
			(res->size)++;
			ncur = 2;
			cur[0] = tree->node[tree->root].child;
			cur[1] = tree->node[tree->node[tree->root].child].sibling;
		} else {
			if(res->size>=nbuf) {
				nbuf += INC_SIZE;
				res->list = (TypeEvent*) realloc((void*)res->list, nbuf*sizeof(TypeEvent));
			}
			res->list[res->size].time = tree->minTime;
			res->list[res->size].n = 1;
			res->list[res->size].type = 'd';
			(res->size)++;
		}
	} else {
		ncur = 1;
		cur[0] = tree->root;
	}
	time = 0;
	while((tree->maxTime-time)>EPSILON) {
		int which;
		time = tree->maxTime;
		for(i=0; i<ncur; i++) {
			if(tree->time[cur[i]] < time) {
				time = tree->time[cur[i]];
				which = i;
			}
		}
		if((tree->maxTime-time)>EPSILON) {
			if(res->size>=nbuf) {
				nbuf += INC_SIZE;
				res->list = (TypeEvent*) realloc((void*)res->list, nbuf*sizeof(TypeEvent));
			}
			res->list[res->size].time = time;
			res->list[res->size].n = ncur;
			if(tree->node[cur[which]].child != NOSUCH) {
				if(tree->node[tree->node[cur[which]].child].sibling != NOSUCH) {
					if(ncur >= MAX_CURRENT) {
						printf("too much lineages in counting events");
						exit(EXIT_FAILURE);
					}
					res->list[res->size].type = 'b';
					(res->size)++;
					cur[ncur] = tree->node[tree->node[cur[which]].child].sibling;
					cur[which] = tree->node[cur[which]].child;
					ncur++;
				} else {
					fprintf(stderr, "Execution error: node %d with a single child\n", cur[which]);
					exit(1);
				}
			} else {
				res->list[res->size].type = 'd';
				(res->size)++;
				for(i=which+1; i<ncur; i++)
					cur[i-1] = cur[i];
				ncur--;
			}
		}
	}
	if(res->size)
		res->list = (TypeEvent*) realloc((void*)res->list, (res->size)*sizeof(TypeEvent));
	else {
		free((void*)res->list);
		res->list = NULL;
	}
	res->minTime = tree->minTime;
	res->maxTime = tree->maxTime;
if(res->size == 0) {
fprintf(stderr, "Empty tree -> contemp %d\n", countContemp(tree));
fprintTreeX(stderr, tree);
for(i=0; i<tree->size; i++)
fprintf(stderr, "parent[%d] = %d\n", i, tree->parent[i]);

exit(1);
}
	return res;
}

