#include "qscaletree.h"

QScaleTree::QScaleTree(QTreeDrawer *p, QWidget *parent) : QWidget(parent)
{
    treePainter = p;
    setMinimumWidth(treePainter->getScaleWidth());
//    setMinimumHeight(treePainter->getScaleHeight());
//    setMinimumWidth(parent->width());
    setMinimumHeight(treePainter->getScaleHeight());
    setBackgroundRole(QPalette::Base);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QScaleTree::~QScaleTree()
{

}

void QScaleTree::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    setMinimumWidth(treePainter->getScaleWidth());
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.translate(0, 0);
    treePainter->drawScaleGenericDown(&painter, 0, 0);
}
