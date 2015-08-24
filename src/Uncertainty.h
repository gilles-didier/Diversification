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
