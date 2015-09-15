#ifndef GTREEFOSSILGEOMETRY_H
#define GTREEFOSSILGEOMETRY_H
#include "gtreegeometry.h"
#include "FossilInt.h"
#include <QPointF>
#include <QLineF>

class GTreeFossilGeometry : public GTreeGeometry
{
public:
    GTreeFossilGeometry(TypeTree *tree = NULL, TypeFossilIntFeature *f = NULL, double min = 0., double max = 1.);
    ~GTreeFossilGeometry();
    double fillTreeGeometry(int x0, int y0, int w, int l);
    QLineF lineFossil(int f);
protected:
    QLineF *line;
    virtual void fillBounds(int n, double tmin, double tmax, double *min, double *max, int *dmax);
    virtual void fillUnknownTimes();
    virtual double fillNodeGeometry(int n, int parent);
    TypeFossilIntFeature *fos;
};

#endif // GTREEFOSSILGEOMETRY_H
