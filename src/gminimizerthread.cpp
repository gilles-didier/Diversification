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
