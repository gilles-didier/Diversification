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




#include "gminimizerthread.h"
#include <math.h>


GMinimizerThread::GMinimizerThread(TypeLikelihoodTreeFosFunction *l, TypeTree *t, TypeFossilIntFeature *f, TypeNLOptOption *o, QObject * parent) : QThread(parent) {
    likelihood = l;
    tree = t;
    fos = f;
    options = o;
}

void GMinimizerThread::run() Q_DECL_OVERRIDE {
    TypeEstimation estim;
    TypeTree treeTmp;
    TypeFossilFeature *ff;
    ff = sampleFossilInt(fos, tree->size);
    treeTmp.time = (double*) malloc(tree->size*sizeof(double));
    treeTmp.size = tree->size;
    treeTmp.sizeBuf = tree->sizeBuf;
    treeTmp.root = tree->root;
    treeTmp.node = tree->node;
    treeTmp.name = NULL;
    treeTmp.comment = NULL;
    treeTmp.info = NULL;
    treeTmp.parent = NULL;
    treeTmp.minTime = tree->minTimeInt.inf+UNIF_RAND*(tree->minTimeInt.sup-tree->minTimeInt.inf);
    treeTmp.maxTime = tree->maxTimeInt.inf+UNIF_RAND*(tree->maxTimeInt.sup-tree->maxTimeInt.inf);
    for(int n=0; n<tree->size; n++) {
        if(tree->node[n].child == NOSUCH) {
            switch(fos->status[n]) {
                case contempNodeStatus:
                    treeTmp.time[n] = treeTmp.maxTime;
                break;
                case extinctNodeStatus:
                    treeTmp.time[n] = NO_TIME;
                break;
                case unknownNodeStatus:
                    treeTmp.time[n] = fos->endTimeTable[fos->endTime[n]].inf+UNIF_RAND*(fos->endTimeTable[fos->endTime[n]].sup-fos->endTimeTable[fos->endTime[n]].inf);
                break;
                default:
                    fprintf(stderr, "Node %d has no status\n", n);
                    return;
            }
        }
    }
    if(!minimizeBirthDeathFossilFromTreeFossil(likelihood, &treeTmp, ff, options, &estim)) {
        estim.logLikelihood = sqrt(-1);
    }
    free((void*)treeTmp.time);
    if(treeTmp.parent != NULL)
        free((void*)treeTmp.parent);
    freeFossilFeature(ff);
    emit resultReady(estim);
}
