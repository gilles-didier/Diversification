#include <QPainter>
#include <iostream>
#include "qtreedrawwidget.h"

QTreeDrawWidget::QTreeDrawWidget(QTreeDrawer *p, QWidget *parent) : QWidget(parent)
{
    treePainter = p;
    setMinimumWidth(treePainter->getWidth());
    setMinimumHeight(treePainter->getHeight());
    setBackgroundRole(QPalette::Base);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QTreeDrawWidget::~QTreeDrawWidget()
{

}
void QTreeDrawWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.translate(0, 0);
    treePainter->paintTreeWidth(&painter, 0, 0, width());
}

