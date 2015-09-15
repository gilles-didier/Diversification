#include "gfossil.h"

GFossil::GFossil(int ind, GNode *n, GTreeFossilView *v) : QGraphicsLineItem(n)
{
    QPen pen;
    index = ind;
    view = v;
    radius = 10;
    pen.setColor(QColor(180, 0, 0, 75));
    pen.setCapStyle(Qt::RoundCap);
    pen.setWidth(radius);
    setPen(pen);
}

GFossil::~GFossil()
{

}

