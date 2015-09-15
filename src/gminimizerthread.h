#ifndef GMINIMIZERTHREAD_H
#define GMINIMIZERTHREAD_H
#include <QThread>
#include "Tree.h"
#include "FossilInt.h"
#include "MinimizeNLOpt.h"


class GMinimizerThread : public QThread
{
    Q_OBJECT
public:
    GMinimizerThread(TypeLikelihoodTreeFosFunction *l, TypeTree *t, TypeFossilIntFeature *f, TypeNLOptOption *o, QObject * parent);
signals:
    void resultReady(TypeEstimation estim);
protected:
    void run() Q_DECL_OVERRIDE;
private:
    TypeLikelihoodTreeFosFunction *likelihood;
    TypeTree *tree;
    TypeFossilIntFeature *fos;
    TypeNLOptOption *options;

};

#endif // GMINIMIZERTHREAD_H
