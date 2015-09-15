#ifndef QTREEFOSSILDRAWER_H
#define QTREEFOSSILDRAWER_H
#include <QPainter>

#include "qtreedrawer.h"
#include "FossilInt.h"

class QTreeFossilDrawer : public QTreeDrawer
{
public:
    QTreeFossilDrawer(TypeFossilIntFeature *fos = NULL, TypeTree *tree = NULL, double min=0., double max=1.);
    ~QTreeFossilDrawer();
    void setFossil(TypeFossilIntFeature *fos);
    double paintTree(QPainter *painter, int x0, int y0);
    double paintTreeWidth(QPainter *painter, int x0, int y0, int w);
protected:
    TypeFossilIntFeature *fos;
    void paintFossil(QPainter *painter, int n, double y);
    void paintLineDot(QPainter *painter, double x1, double y1, double x2, double y2);
    void fillBounds(int n, double tmin, double tmax, double *min, double *max, int *dmax);
    double paintNode(QPainter *painter, int n, int parent);
    void computeTime();
};

#endif // QTREEFOSSILDRAWER_H
