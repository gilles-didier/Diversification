#include <QMessageBox>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "gestimater.h"
#include "Uncertainty.h"
#include "SimulFossil.h"
#include "Random.h"
#include "MinimizeNLOpt.h"
#include "Fossil.h"
#include "Utils.h"
#include "gminimizerthread.h"

GEstimater::GEstimater(QObject * parent) : QObject(parent)
{
    maxThread = QThread::idealThreadCount();
    nThread = 0;
    estimation = NULL;
    state = waiting;
}



GEstimater::~GEstimater()
{
    if(estimation != NULL)
        delete[] estimation;
}


void GEstimater::handleResult(TypeEstimation estim)
{
    if(!isnan(estim.logLikelihood)) {
        estimation[nComp++] = estim;
        update();
    }
    emit setProgress(((nComp+1)*100)/nIter);
    nThread--;
    if(nComp+nThread<nIter && state != stopping)
        launchThread();
    if(nThread == 0) {
        state = waiting;
        emit estimationCompleted();
    }
}

void GEstimater::update() {
    TypeEstimation mean, std;
    int i;
    mean.param.birth = 0.;
    mean.param.death = 0.;
    mean.param.fossil = 0.;
    mean.logLikelihood = 0.;
    std.param.birth = 0.;
    std.param.death = 0.;
    std.param.fossil = 0.;
    std.logLikelihood = 0.;
      if(nComp>0) {
         for(i=0; i<nComp; i++) {
             mean.param.birth += estimation[i].param.birth;
             mean.param.death += estimation[i].param.death;
             mean.param.fossil += estimation[i].param.fossil;
             mean.logLikelihood += estimation[i].logLikelihood;
         }
         mean.param.birth /= (double) nComp;
         mean.param.death /= (double) nComp;
         mean.param.fossil /= (double) nComp;
         mean.logLikelihood /= (double) nComp;
         if(nComp>1) {
             for(i=0; i<nComp; i++) {
                 std.param.birth += pow(estimation[i].param.birth-mean.param.birth, 2.);
                 std.param.death += pow(estimation[i].param.death-mean.param.death, 2.);
                 std.param.fossil += pow(estimation[i].param.fossil-mean.param.fossil, 2.);
                 std.logLikelihood += pow(estimation[i].logLikelihood-mean.logLikelihood, 2.);
             }
             std.param.birth /= (double) nComp-1.;
             std.param.death /= (double) nComp-1.;
             std.param.fossil /= (double) nComp-1.;
             std.logLikelihood /= (double) nComp-1.;
             std.param.birth = sqrt(std.param.birth);
             std.param.death = sqrt(std.param.death);
             std.param.fossil = sqrt(std.param.fossil);
             std.logLikelihood = sqrt(std.logLikelihood);
         }
     }
     emit newEstimation(mean, std);
}

void GEstimater::launchThread() {
    GMinimizerThread *minimizer = new GMinimizerThread(likelihood, tree, fos, options, this);
    connect(minimizer, &GMinimizerThread::resultReady, this, &GEstimater::handleResult);
    connect(minimizer, &GMinimizerThread::finished, minimizer, &QObject::deleteLater);
    minimizer->start();
    nThread++;
}

void GEstimater::stop() {
    if(state == busy)
        state = stopping;
}


void GEstimater::start(TypeLikelihoodTreeFosFunction *l, TypeTree *t, TypeFossilIntFeature *f, TypeNLOptOption *o, int it) {
    if(state != waiting) {
        warning(QString(tr("Execution")), QString(tr("Previous estimation not completed. Please wait.")));
        return;
    }
    state = busy;
    if(t!=NULL) {
        tree = t;
        fos = f;
    } else {
        tree = NULL;
        fos = NULL;
    }
    nIter = it;
    options = o;
    likelihood = l;
    nComp = 0;
    estimation = new TypeEstimation[nIter];
    if(tree == NULL) {
       warning(QString(tr("Execution")), QString(tr("Please, load a tree !")));
       return;
   }
   if(fos == NULL) {
       warning(QString(tr("Execution")), QString(tr("Please, I do need fossils !")));
       return;
   }
    nThread = 0;
   while(nThread<utils_MIN(maxThread, nIter))
       launchThread();
 }



void GEstimater::warning(QString title, QString text)
{
    QMessageBox::warning(NULL, title, text );
}
