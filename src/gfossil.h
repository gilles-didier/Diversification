#ifndef GFOSSIL_H
#define GFOSSIL_H
#include <QGraphicsLineItem>
#include "gnode.h"
#include "gtreefossilview.h"


class GFossil : public QGraphicsLineItem
{
public:
    GFossil(int ind = 0, GNode *n = NULL, GTreeFossilView *v = NULL);
    ~GFossil();
protected:
    GTreeFossilView *view;
    int index, radius;
};

#endif // GFOSSIL_H
