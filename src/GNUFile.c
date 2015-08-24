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




#include "GNUFile.h"


void fprintGNUFile(FILE *f, TypeGNUInfo *info) {
	int c;
	if(info->dataFileName == NULL || info->nColumn == 0)
		return;
	switch(info->type) {
		case EPS:
		default:
			fprintf(f, "set terminal postscript enhanced color eps\n");
	}
	if(info->xLabel != NULL)
		fprintf(f, "set xlabel \"%s\"\n",  info->xLabel);
	if(info->yLabel != NULL)
		fprintf(f, "set ylabel \"%s\"\n",  info->yLabel);
	if(info->outputFileName != NULL)
		fprintf(f, "set output \"%s\"\n", info->outputFileName);
	fprintf(f, "plot \"%s\"  using 1:2 with lines", info->dataFileName);
	if(info->title != NULL && info->title[0] != NULL)
		fprintf(f, " title '%s'", info->title[0]);
	for(c=1; c<info->nColumn; c++) {
		fprintf(f,", '' using 1:%d with lines", c+2);
		if(info->title != NULL && info->title[c] != NULL)
			fprintf(f," title '%s'", info->title[c]);
	}
}
