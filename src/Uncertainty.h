#ifndef UncertaintyF
#define UncertaintyF

#include <stdlib.h>
#include <stdio.h>
#include <gsl/gsl_vector.h>
#include "Utils.h"
#include "Tree.h"
#include "Fossil.h"
#include "Model.h"

typedef struct PARAM_UNCERTAINTY {
	TypeTree *tree;
	TypeFossilFeature *fos;
	double fossil;
} TypeParamUncertainty;

#ifdef __cplusplus
extern "C" {
#endif

void splitTreeFossil(TypeTree *tree, TypeFossilFeature *fos, TypeTree ***treeList, int *size);
double likelihood(double timeStart, double timeEnd, int nLeaves);
double getLogLikelihoodEvent(TypeTree *tree, TypeFossilFeature *fos, TypeModelParam *param);
double getLogLikelihoodTreeFossil(TypeTree *tree, TypeFossilFeature *fos, TypeModelParam *param);
double funcFossilUncertainGSL(const gsl_vector *x, void *params);
double funcFossilUncertainThreeParGSL(const gsl_vector *x, void *params);
double getItemNumber(TypeTree *tree, TypeFossilFeature *fos);
double getItemNumberSplitted(TypeTree *tree);

#ifdef __cplusplus
}
#endif


#endif
