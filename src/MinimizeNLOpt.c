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




#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <nlopt.h>

#include "Utils.h"

#include "MinimizeNLOpt.h"


//#define NLOPT_ALGO NLOPT_GN_ISRES
//#define NLOPT_ALGO NLOPT_GN_ESCH
#define NLOPT_ALGO NLOPT_LN_BOBYQA
//#define NLOPT_ALGO NLOPT_LN_COBYLA
//#define NLOPT_ALGO NLOPT_AUGLAG


#define MINVAL 0.01
#define INFTY 1E99
#define RINFTY 1E99
#define DEF 10
#define MIN_VAL 0.000001
#define TOLERANCE_CONSTRAINT 0.000000001
#define TOLERANCE_OPTIM 0.001



double toMinimizeTreeFossil(unsigned n, const double *x, double *grad, void *data);
double toMinimizeEventListA(unsigned n, const double *x, double *grad, void *data);
double toMinimizeEventListB(unsigned n, const double *x, double *grad, void *data);

typedef struct MINIMIZATION_TREE_FOS_DATA {
    TypeTree *tree;
    TypeFossilFeature *fos;
    TypeLikelihoodTreeFosFunction *likelihood;
} TypeMinimizationTreeFossilData;

typedef struct MINIMIZATION_EVENT_LIST_DATA {
    TypeListEvent *event;
    TypeLikelihoodEventListFunction *likelihood;
} TypeMinimizationEventListData;

#define TAG_SPE "SPE"
#define TAG_EXT "EXT"
#define TAG_FOS "FOS"
#define TAG_TRI "TRI"
#define TAG_TOL "TOL"
#define TAG_ITE "ITE"
#define SIZE_TAG 20
#define SIZE_VAL 100

void fprintNLoptOptionTag(FILE *f, TypeNLOptOption *option) {
    fprintf(f, ":%s [%lE;%lE]\n", TAG_SPE, option->infSpe, option->supSpe);
    fprintf(f, ":%s [%lE;%lE]\n", TAG_EXT, option->infExt, option->supExt);
    fprintf(f, ":%s [%lE;%lE]\n", TAG_FOS, option->infFos, option->supFos);
    fprintf(f, ":%s %d\n", TAG_TRI, option->trials);
    fprintf(f, ":%s %lE\n", TAG_TOL, option->tolOptim);
    fprintf(f, ":%s %d\n", TAG_ITE, option->maxIter);
}

void fscanNLoptOptionTag(FILE *f, TypeNLOptOption *option) {
    char c, tag[SIZE_TAG+1], val[SIZE_VAL+1];
    for(c=fgetc(f); c!=EOF && isspace(c); c=fgetc(f));
    while(c == ':') {
        int i;
        c=fgetc(f);
        for(i=0; c!=EOF && !isspace(c) && i<SIZE_TAG; c=fgetc(f))
            tag[i++] = c;
        tag[i] = '\0';
        if(i>=SIZE_TAG) {
            fprintf(stderr, "Error when reading an optimizer options file - Tag too long:\n%s...\n", tag);
            exit(1);
        }
        for(; c!=EOF && isspace(c); c=fgetc(f));
        for(i=0; c!=EOF && !isspace(c) && i<SIZE_VAL; c=fgetc(f))
            val[i++] = c;
        val[i] = '\0';
        if(i>=SIZE_VAL) {
            fprintf(stderr, "Error when reading an optimizer options file - value too long:\n%s...\n", val);
            exit(1);
        }
        if(strcmp(tag, TAG_SPE) == 0)
            toInterval(val, &(option->infSpe), &(option->supSpe));
        if(strcmp(tag, TAG_EXT) == 0)
            toInterval(val, &(option->infExt), &(option->supExt));
        if(strcmp(tag, TAG_FOS) == 0)
            toInterval(val, &(option->infFos), &(option->supFos));
        if(strcmp(tag, TAG_TRI) == 0)
            option->trials = atoi(val);
        if(strcmp(tag, TAG_TOL) == 0)
            option->tolOptim = atof(val);
        if(strcmp(tag, TAG_ITE) == 0)
            option->maxIter = atoi(val);
        for(; c!=EOF && isspace(c); c=fgetc(f));
    }
}

void fprintNLoptOption(FILE *f, TypeNLOptOption *option) {
    fprintf(f, "Speciation rates are sampled in [%.2lE:%.2lE]\n", option->infSpe, option->supSpe);
    fprintf(f, "Extinction rates are sampled in [%.2lE:%.2lE]\n", option->infExt, option->supExt);
    fprintf(f, "Fossil rates are sampled in [%.2lE:%.2lE]\n", option->infFos, option->supFos);
    fprintf(f, "Optimizer runs %d trials and stops with tolerance %.lE or after more than %d iterations.\n", option->trials, option->tolOptim, option->maxIter);
}

void sprintNLoptOption(char *buffer, TypeNLOptOption *option) {
    buffer += sprintf(buffer, "Speciation rates are sampled in [%.2lE:%.2lE]\n", option->infSpe, option->supSpe);
    buffer += sprintf(buffer, "Extinction rates are sampled in [%.2lE:%.2lE]\n", option->infExt, option->supExt);
    buffer += sprintf(buffer, "Fossil rates are sampled in [%.2lE:%.2lE]\n", option->infFos, option->supFos);
    buffer += sprintf(buffer, "Optimizer runs %d trials and stops with tolerance %.lE or after more than %d iterations.\n", option->trials, option->tolOptim, option->maxIter);
}

double toMinimizeTreeFossil(unsigned n, const double *x, double *grad, void *data) {
    TypeModelParam param;
    param.birth = x[0]+x[1];
    param.death = x[1];
    param.fossil = x[2];
    return -((TypeMinimizationTreeFossilData*)data)->likelihood(((TypeMinimizationTreeFossilData*)data)->tree, ((TypeMinimizationTreeFossilData*)data)->fos, &param);
}

double toMinimizeEventListA(unsigned n, const double *x, double *grad, void *data) {
    TypeModelParam param;
    param.birth = x[0];
    param.death = x[1];
    param.fossil = x[2];
    return -((TypeMinimizationEventListData*)data)->likelihood(((TypeMinimizationEventListData*)data)->event, &param);
}

double toMinimizeEventListB(unsigned n, const double *x, double *grad, void *data) {
    TypeModelParam param;
    param.birth = x[1]+x[0];
    param.death = x[1];
    return -((TypeMinimizationEventListData*)data)->likelihood(((TypeMinimizationEventListData*)data)->event, &param);
}



int minimizeBirthDeathFossilFromTreeFossil(TypeLikelihoodTreeFosFunction *f, TypeTree *tree, TypeFossilFeature *fos, TypeNLOptOption *option, TypeEstimation *estim) {
    double x[3], minLikelihood;
    nlopt_opt opt;
    TypeMinimizationTreeFossilData data;
    int result, t;
//int m;
    data.tree = tree;
    data.fos = fos;
    data.likelihood = f;
    opt = nlopt_create(NLOPT_ALGO, 3); /* algorithm and dimensionality */
    nlopt_set_lower_bounds1(opt, 0.);
    nlopt_set_min_objective(opt, toMinimizeTreeFossil, &data);
    nlopt_set_xtol_abs1(opt, option->tolOptim);
    nlopt_set_maxeval(opt, option->maxIter);
    estim->logLikelihood = INFTY;
//m = -1;
    for(t=0; t<option->trials; t++) {
        double a, b;
        a = option->infSpe+UNIF_RAND*(option->supSpe-option->infSpe);
        b = option->infExt+UNIF_RAND*(option->supExt-option->infExt);
        if(a<b) {
            a = option->supSpe-a;
            b = option->supExt-b;
        }
        x[0] = a-b;
        x[1] = b;
        x[2] = option->infFos+UNIF_RAND*(option->supFos-option->infFos);
//fprintf(stdout, "S%d\tbirth %.2lf\tdeath %.2lf\tfossil %.2lf (%.2lE)\n", t, x[0]+x[1], x[1], x[2], toMinimizeTreeFossil(3, x, NULL, &data));
        if(((result = nlopt_optimize(opt, x, &minLikelihood)) >= 0) && minLikelihood < estim->logLikelihood) {
            estim->logLikelihood = minLikelihood;
            estim->param.birth = x[0]+x[1];
            estim->param.death = x[1];
            estim->param.fossil = x[2];
//            m = t;
        }
//fprintf(stdout, "R%d\tbirth %.2lf\tdeath %.2lf\tfossil %.2lf (%.2lE)\n", t, x[0]+x[1], x[1], x[2], toMinimizeTreeFossil(3, x, NULL, &data));
    }
    estim->logLikelihood = -estim->logLikelihood;
//printf("X%d\t%.2lf\t%.2lf\t%.2lf (%.2lE/%.2lE)\n\n", m, estim->param.birth, estim->param.death, estim->param.fossil, estim->logLikelihood, f(tree, fos, &(estim->param)));
//fflush(stdout);
    nlopt_destroy(opt);
    return result;
}

int minimizeBirthDeathFossilFromEventList(TypeLikelihoodEventListFunction *f, TypeListEvent *event, TypeNLOptOption *option, TypeEstimation *estim) {
    double x[3], minLikelihood;
    nlopt_opt opt;
    TypeMinimizationEventListData data;
    int result, t;

    data.event = event;
    data.likelihood = f;
    opt = nlopt_create(NLOPT_ALGO, 3); /* algorithm and dimensionality */
    nlopt_set_lower_bounds1(opt, 0.);
    nlopt_set_min_objective(opt, toMinimizeEventListA, &data);
    nlopt_set_xtol_abs1(opt, option->tolOptim);
    nlopt_set_maxeval(opt, option->maxIter);
    estim->logLikelihood = INFTY;
    for(t=0; t<option->trials; t++) {
        double a, b;
        a = option->infSpe+UNIF_RAND*(option->supSpe-option->infSpe);
        b = option->infExt+UNIF_RAND*(option->supExt-option->infExt);
        if(a<b) {
            a = option->supSpe-a;
            b = option->supExt-b;
        }
        x[0] = a-b;
        x[1] = b;
        x[2] = option->infFos+UNIF_RAND*(option->supFos-option->infFos);
        if(((result = nlopt_optimize(opt, x, &minLikelihood)) >= 0) && minLikelihood < estim->logLikelihood) {
            estim->logLikelihood = minLikelihood;
            estim->param.birth = x[0]+x[1];
            estim->param.death = x[1];
            estim->param.fossil = x[2];
        }
    }
    estim->logLikelihood = -estim->logLikelihood;
    nlopt_destroy(opt);
    return result;
}


int minimizeBirthDeathFromEventList(TypeLikelihoodEventListFunction *f, TypeListEvent *event, TypeNLOptOption *option, TypeEstimation *estim) {
    double lb[2], x[2], minLikelihood;
    nlopt_opt opt;
    TypeMinimizationEventListData data;
    int result, t;

    data.event = event;
    data.likelihood = f;
    lb[0] = 0.;
    lb[1] = 0.;
    opt = nlopt_create(NLOPT_ALGO, 2); /* algorithm and dimensionality */
    nlopt_set_lower_bounds(opt, lb);
    nlopt_set_min_objective(opt, toMinimizeEventListB, &data);
    nlopt_set_lower_bounds1(opt, 0.);
    nlopt_set_xtol_abs1(opt, option->tolOptim);
    nlopt_set_maxeval(opt, option->maxIter);
    estim->logLikelihood = INFTY;
    for(t=0; t<option->trials; t++) {
        double a, b;
        a = option->infSpe+UNIF_RAND*(option->supSpe-option->infSpe);
        b = option->infExt+UNIF_RAND*(option->supExt-option->infExt);
        if(a<b) {
            a = option->supSpe-a;
            b = option->supExt-b;
        }
        x[0] = a-b;
        x[1] = b;
        if(((result = nlopt_optimize(opt, x, &minLikelihood)) >= 0) && minLikelihood < estim->logLikelihood) {
            estim->logLikelihood = minLikelihood;
            estim->param.birth = x[0]+x[1];
            estim->param.death = x[1];
        }
    }
    estim->logLikelihood = -estim->logLikelihood;
    nlopt_destroy(opt);
    return result;
}
