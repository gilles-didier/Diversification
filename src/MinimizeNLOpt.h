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




#ifndef MinimizeNLOptF
#define MinimizeNLOptF

#include "Model.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NLOPT_OPTION {
    double infSpe, supSpe, infExt, supExt, infFos, supFos, tolOptim;
    int trials, maxIter;
} TypeNLOptOption;

typedef struct ESTIMATION {
    TypeModelParam param;
    double logLikelihood;
} TypeEstimation;

int minimizeBirthDeathFossilFromTreeFossil(TypeLikelihoodTreeFosFunction *f, TypeTree *tree, TypeFossilFeature *fos, TypeNLOptOption *option, TypeEstimation *estim);
int minimizeBirthDeathFossilFromEventList(TypeLikelihoodEventListFunction *f, TypeListEvent *event, TypeNLOptOption *option, TypeEstimation *estim);
int minimizeBirthDeathFromEventList(TypeLikelihoodEventListFunction f, TypeListEvent *event, TypeNLOptOption *option, TypeEstimation *estim);
void fprintNLoptOption(FILE *f, TypeNLOptOption *option);
void sprintNLoptOption(char *buffer, TypeNLOptOption *option);
void fprintNLoptOptionTag(FILE *f, TypeNLOptOption *option);
void fscanNLoptOptionTag(FILE *f, TypeNLOptOption *option);
#ifdef __cplusplus
}
#endif

#endif
