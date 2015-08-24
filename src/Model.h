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




#ifndef ModelF
#define ModelF

#include <stdlib.h>
#include <stdio.h>
#include <gsl/gsl_vector.h>
#include "Tree.h"
#include "Fossil.h"

#define RINFTY 1E99

typedef struct MODEL_PARAM {
	double birth, death, fossil;
} TypeModelParam;

typedef struct EVENT {
	char type;
	int n;
	double time;
} TypeEvent;

typedef struct LISTEEVENT {
	TypeEvent *list;
	int size;
	double minTime, maxTime;
} TypeListEvent;

typedef double TypeLikelihoodTreeFosFunction(TypeTree *, TypeFossilFeature *, TypeModelParam *);
typedef double TypeLikelihoodEventListFunction(TypeListEvent *, TypeModelParam *);

#ifdef __cplusplus
extern "C" {
#endif

TypeListEvent *getEventSequence(TypeTree *tree, TypeFossilFeature *fos);

void getStatEvent(TypeListEvent *event, int *b,  int *d,  int *f);
void freeListEvent(TypeListEvent *event);
void fprintEvent(FILE *f, TypeEvent *event);
void fprintListEvent(FILE *f, TypeListEvent *event);
/*get the total time*/
double getTotalTimeEvent(TypeListEvent *event);

double getAlpha(double birth, double death, double fossil);
double getBeta(double birth, double death, double fossil);
double getBirthProbLog(double t, double birth, double death, double fossil, double alpha, double beta);
double getDeathProbLog(double t, double birth, double death, double fossil, double alpha, double beta);
double getFossilProbLog(double t, double birth, double death, double fossil, double alpha, double beta);

double getObsProb(double t, double birth, double death, double fossil, double alpha, double beta);
double getObsProbLog(double t, double birth, double death, double fossil, double alpha, double beta);
double getLogCompProbObs(double t, double birth, double alpha, double beta);
double getLogProbObs(double t, double birth, double alpha, double beta);

double getEventFossilProbLogNee(TypeListEvent *event, double birth, double death, double fossil);

double getEventSamFosProbLog(TypeListEvent *event, double birth, double death, double fossil, double sample);
double getEventFossilProbLog(TypeListEvent *event, double birth, double death, double fossil);
double getEventPhyloProbLogNee(TypeListEvent *event, double birth, double death);
double getEventCompleteProbLog(TypeListEvent *event, double birth, double death, double fossil);
double getEventPhyloProbLog(TypeListEvent *event, double birth, double death);
double getEventPureProbLog(TypeListEvent *event, double birth, double death);
double getLogLikelihoodWithoutFossil(double startTime, double endTime, double maxTime, int k,  double birth, double death, double fossil);
double getSpecialLogLikelihoodWithoutFossil(double startTime, double endTime, double maxTime, int k,  double birth, double death, double fossil);

double getLogProbWaitFossilExactBis(double startTime, double endTime, double maxTime, int k,  double birth, double death, double fossil);
double getLogProbWaitFossilExact(double startTime, double endTime, double maxTime, int k,  double birth, double death, double fossil);
double getLogProbWaitFossilMore(double startTime, double endTime, int k,  double birth, double death, double fossil);


int estimateML(TypeListEvent *event, double *p);

void setFossilRate(double fos);
void setSample(double sam);
double funcLogU(double t, double birth, double death);
double funcLogP(double t, double birth, double death);
double funcLogCondition(double t, double birth, double death);
double funcFossilGSL(const gsl_vector *x, void *params);
double funcSamFosFixedGSL(const gsl_vector *x, void *params);
double funcFossilFixedGSL(const gsl_vector *x, void *params);
double funcFossilFixedNeeGSL(const gsl_vector *x, void *params);
double funcFossilBisGSL(const gsl_vector *x, void *params);
double funcCompleteGSL(const gsl_vector *x, void *params);
double funcPhyloGSL(const gsl_vector *x, void *params);
double funcPhyloNeeGSL(const gsl_vector *x, void *params);
double funcPureGSL(const gsl_vector *x, void *params);

double getEventPhyloProbLogX(TypeListEvent *event, TypeModelParam *pa);
double getEventFossilProbLogX(TypeListEvent *event, TypeModelParam *pa);
double getEventFossilProbLogY(TypeListEvent *event, TypeModelParam *pa);


#ifdef __cplusplus
}
#endif

#endif
