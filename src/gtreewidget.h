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

#ifndef GTREEWIDGET_H
#define GTREEWIDGET_H

#include <QGraphicsView>
#include "Tree.h"
#define GRAPHICVIEW_BAND 26
class GNode;

//! [0]
class GTreeWidget : public QGraphicsView
{
    Q_OBJECT
public:
    GTreeWidget(QWidget *parent = 0);
    GTreeWidget(TypeTree *t = NULL, double mi=0., double ma=1., QWidget *parent = 0);

    void itemMoved();
    void setWidth(int w);
    void setTree(TypeTree *t);
    void setTarget(GNode *n);
    void startMoving(GNode *n);
    void endMoving();
    void selectNode(GNode *n);
    void unselectNode(GNode *n);
    int getSelected();
    void addNewLeaf(GNode *n);
    void removeNode(GNode *n);
    void editNode(GNode *no);
    QString saveSubtree(int n);
    TypeTree *getTree();
    void setMinMaxTime(double min, double max);
    virtual void nodeContextMenu(GNode *n, QPoint pos);
    bool isModified();
    void setModified(bool v);
    void saveTree(QString);
public slots:
    void zoomIn();
    void zoomOut();
    void addNewLeaf();
    void removeNode();
    void setMinTimeInf(double min);
    void setMaxTimeInf(double max);
    void setMinTimeSup(double min);
    void setMaxTimeSup(double max);
signals:
    void change();
    void mouseMoved(QString text);
protected:
    void mouseDoubleClickEvent(QMouseEvent * event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent * mouseEvent) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
#endif
    bool event(QEvent * e);
    void resizeEvent(QResizeEvent * event);
    void drawBackground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;
    void scaleView(qreal scaleFactor);
    virtual void layoutTree();
    void computeLabelWidth();
    void subSelectNode(int i);
    void unsubSelectNode(int i);
    void clearRegisteredNodes();
    double boundInf(int n);

    TypeTree *tree;
    QVector<GNode*> node;
    GNode *targetNode, *sourceNode, *selectedNode;
    QFont font;
    int leafSep, labelWidth, fontSize;
    double scaleTime, minTime, maxTime;
    bool modified;
};

#define SIZE_VECTOR 500

#endif // GRAPHWIDGET_H
