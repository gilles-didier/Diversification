#ifndef GTREEGEOMETRY_H
#define GTREEGEOMETRY_H

#include <QPointF>
#include "Tree.h"

class GTreeGeometry
{
public:
    GTreeGeometry(TypeTree *tree = NULL, double min = 0., double max = 1.);
    ~GTreeGeometry();
    void setMinTime(double mint);
    void setMaxTime(double maxt);
    void setMinMaxTime(double min, double max);
    void setTree(TypeTree *tree);
    void setLeafSep(int l);
    double fillTreeGeometry(int x0, int y0, int w, int l);
    QPointF endNode(int n);
    double *infTime();
    double *supTime();
    double getScale();
    double infTime(int n);
    double supTime(int n);
    double *getTimeTable();
protected:
    int leafSep, leafCur, xoffset, yoffset, width;
    double scale, minTime, maxTime, *time, *inf, *sup;
    TypeTree *tree;
    QPointF *start;
    void fillTime(int n, double tanc, double *min, double *max, int *dmax);
    virtual void fillBounds(int n, double tmin, double tmax, double *min, double *max, int *dmax);
    virtual double fillNodeGeometry(int n, int parent);
    virtual void fillUnknownTimes();
    void computeTime();
};

#endif // GTREEGEOMETRY_H
