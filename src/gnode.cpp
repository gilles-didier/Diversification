/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "gnode.h"
#include "gtreewidget.h"
#include "gnode.h"
#include <math.h>
#include <typeinfo>
#include <iostream>

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>
#include <QStyleOption>
#include <QDrag>
#include <QApplication>
#include <QMimeData>
#include <QMenu>
#include <QAction>

//! [0]
GNode::GNode(int ind, char *n, GTreeWidget *t):QGraphicsItem()
    //QGraphicsPathItem()
{
    nodeIndex = ind;
    treeWidget = t;
    name = n;
    adjust = 10;
    inf = 0.;
    sup = 0.;
    state = normalState;
    setFlag(ItemIsSelectable);
    setFlag(ItemClipsToShape);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setAcceptHoverEvents(true);
    setZValue(-1);
    font.setFamily(font.defaultFamily());
    penWidth = 3;
    hoverColor = QColor(20, 200, 20, 255);
    moveColor = QColor(200, 20, 20, 100);
    selectedColor = QColor(255, 0, 0, 255);
    subSelectedColor = QColor(200, 200, 200, 255);
    normalColor = QColor(0, 0, 0, 255);
    highlightColor = QColor(200, 20, 20, 100);
    color = normalColor;
    hasMoved = false;
}
#define RATIO 2.5

QRectF GNode::boundingRect() const
{
    qreal adjust = 20;
    qreal x, y, w, h;
    w = fabs(end.x()-start.x());
    h = fabs(end.y()-start.y());
    if(start.x()>end.x())
        x = -w;
    else
        x = 0.;
    if(start.y()>end.y())
        y = -h;
    else
        y = 0.;
    if(name != NULL) {
        QRect ra = QFontMetrics(font).boundingRect(QString(name));
        QRectF rb = QRectF(QFontMetrics(font).averageCharWidth()+ra.x(), QFontMetrics(font).ascent()/RATIO+ra.y(), ra.width(), ra.height());
        return QRectF( x - adjust, y - adjust, w + 2*adjust, h + 2*adjust).united(rb);
    } else
        return QRectF( x - adjust, y - adjust, w + 2*adjust, h + 2*adjust);
}

QPainterPath GNode::shape() const
{
    QPainterPath path;
    path.moveTo(0,0);
    path.lineTo(end.x()-start.x(), 0);
    path.lineTo(end.x()-start.x(), end.y()-start.y());
    if(name != NULL)
        path.addText(QFontMetrics(font).averageCharWidth(), QFontMetrics(font).ascent()/RATIO, font,QString(name));
    QPainterPathStroker stroker;
    stroker.setWidth(adjust);
    stroker.setCapStyle(Qt::FlatCap);
    return stroker.createStroke(path);
}


void GNode::setGeometry(QPointF s, QPointF e) {
    prepareGeometryChange();
    setPos(s);
    start = s;
    end = e;
}

void GNode::setTimeBounds(double i, double s) {
    inf = i;
    sup = s;
}

double GNode::infTime()
{
    return inf;
}

double GNode::supTime()
{
    return sup;
}

void GNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    painter->save();
    QPen pen;
    pen.setWidth(penWidth);
    pen.setColor(color);
    painter->setFont(font);
    painter->setPen(pen);
    painter->drawLine(0, 0, end.x()-start.x(), 0);
    painter->drawLine(end.x()-start.x(), 0, end.x()-start.x(), end.y()-start.y());
    if(name != NULL)
        painter->drawText(QFontMetrics(font).averageCharWidth(), QFontMetrics(font).ascent()/RATIO,QString(name));
    painter->restore();
}


int GNode::getIndex() {
    return nodeIndex;
}

void GNode::setColor(QColor c)
{
    color = c;
    update();
}

void GNode::setName(char *n)
{
    name = n;
}

void GNode::setNormal()
{
    switch(state) {
        case normalState:
            setColor(normalColor);
            break;
        case subSelectedState:
            setColor(subSelectedColor);
            break;
        case selectedState:
            setColor(selectedColor);
            break;
    }
}


void GNode::setState(enum NODE_STATE s) {
    state = s;
    setNormal();
}
void GNode::setHovered()
{
    setColor(hoverColor);
    update();
}
void GNode::setHighlighted()
{
    setColor(highlightColor);
    update();
}
void GNode::setEndPoint(QPointF e)
{
    prepareGeometryChange();
    end = e;
}

QPointF GNode::getStartPoint()
{
    return start;
}

void GNode::hoverEnterEvent(QGraphicsSceneHoverEvent * event) {
    setHovered();
}

void GNode::hoverLeaveEvent(QGraphicsSceneHoverEvent * event) {
    setNormal();
}

void GNode::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if(moving) {
        hasMoved = true;
        QGraphicsItem *hit = scene()->itemAt(event->scenePos(), scene()->views().at(0)->transform());
        if(hit != NULL && hit->type() == nodeType)
            treeWidget->setTarget((GNode*) hit);
        setEndPoint(event->scenePos());
        update();
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void GNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    moving = true;
    hasMoved = false;
    setHighlighted();
    treeWidget->startMoving(this);
    update();
    QGraphicsItem::mousePressEvent(event);
}

void GNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    moving = false;
    setNormal();
    if(hasMoved)
        treeWidget->endMoving();
    else
        treeWidget->selectNode(this);
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}

//A synchronous menu call, like QMenu::exec(), enters a modal event loop, and provides easy-to-read code like in the following example:
void GNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    treeWidget->nodeContextMenu(this, event->screenPos());
}
