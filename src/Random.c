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
#include <stdio.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "Random.h"



gsl_rng *rgen;


void initRandom() {
	rgen = gsl_rng_alloc(gsl_rng_random_glibc2);
}

void endRandom() {
	gsl_rng_free(rgen);
}

/*return an uniform  random value in [0,1]*/
double getUniformStd() {
	return gsl_rng_uniform(rgen);
}
/*return an uniform  random value in [0, m]*/
double getUniformCont(double m) {
	return m*gsl_rng_uniform(rgen);
}

/*return an uniform discrete random value in {0, 1, m}*/
double getUniformDisc(int n) {
	return gsl_rng_uniform_int(rgen, n);
}
/*return an exponential distributed random value in {0, 1, m}*/
double getExponential(double l) {
    return gsl_ran_exponential(rgen, l);
}
