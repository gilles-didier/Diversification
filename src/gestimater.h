#ifndef GESTIMATER_H
#define GESTIMATER_H
#include <QThread>
#include "Tree.h"
#include "FossilInt.h"
#include "MinimizeNLOpt.h"


class GEstimater : public QObject
{
    Q_OBJECT
public:
    enum TypeEstimaterState {
        waiting = 0,
        busy,
        stopping
    };

    GEstimater(QObject *parent=NULL);
    ~GEstimater();
    void start(TypeLikelihoodTreeFosFunction *l, TypeTree *t=NULL, TypeFossilIntFeature *f=NULL, TypeNLOptOption *option=NULL, int it = 2);
    void stop();
public slots:
    void handleResult(TypeEstimation estim);
signals:
    void newEstimation(TypeEstimation mean, TypeEstimation std);
    void setProgress(int value);
    void estimationCompleted();
private:
    void update();
    void launchThread();

    enum TypeEstimaterState state;
    TypeTree *tree;
    TypeFossilIntFeature *fos;
    TypeNLOptOption *options;
    double tmin, tmax;
    int nIter, nComp, maxThread, nThread;
    TypeLikelihoodTreeFosFunction *likelihood;
    void warning(QString title, QString text);
    TypeEstimation *estimation;
};

#endif // GESTIMATER_H
