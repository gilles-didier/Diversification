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

#include "gtreewidget.h"
#include "gnode.h"
#include "gtreegeometry.h"
#include <math.h>
#include <QKeyEvent>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QtWidgets>

GTreeWidget::GTreeWidget(QWidget *parent) : QGraphicsView(parent)
{
    tree = NULL;
    targetNode = NULL;
    sourceNode = NULL;
    selectedNode = NULL;
    scaleTime = 1.;
    modified = false;
    setMinMaxTime(0., 1.);
    node = QVector<GNode*>(SIZE_VECTOR);
    font.setFamily(font.defaultFamily());
    leafSep = QFontMetrics(font).lineSpacing();
    setScene(NULL);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setAlignment(Qt::AlignCenter);
}

GTreeWidget::GTreeWidget(TypeTree *t, double mi, double ma, QWidget *parent) : QGraphicsView(parent)
{
    tree = NULL;
    targetNode = NULL;
    sourceNode = NULL;
    selectedNode = NULL;
    scaleTime = 1.;
    modified = false;
    setMinMaxTime(mi, ma);
    node = QVector<GNode*>(SIZE_VECTOR);
    font.setFamily(font.defaultFamily());
    leafSep = QFontMetrics(font).lineSpacing();
    setScene(NULL);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setTree(t);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setAlignment(Qt::AlignCenter);
}


bool GTreeWidget::isModified() {
    return modified;
}

void GTreeWidget::setModified(bool v) {
    modified = v;
}

void GTreeWidget::setTree(TypeTree *t) {
    setScene(new QGraphicsScene(this));
    scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
    if(tree != NULL) {
        Tree::freeTree(tree);
    }
    tree = t;
    Tree::setParent(tree);
    if(tree != NULL && tree->name == NULL)
        tree->name = Tree::newStringTab(tree->sizeBuf);
    computeLabelWidth();
    if(tree != NULL) {
        if(tree->time != NULL)
            free((void*) tree->time);
        tree->time = NULL;
        for(int n=0; n<tree->size; n++) {
            if(tree->name != NULL)
                node[n] = new GNode(n, tree->name[n], this);
            else
                node[n] = new GNode(n, NULL, this);
            scene()->addItem(node[n]);
        }
        layoutTree();
    }
    modified = false;
    emit change();
    show();
}

void GTreeWidget::setMinTimeInf(double min) {
    minTime = min;
    if(tree!=NULL)
        tree->minTimeInt.inf = min;
    modified = true;
    layoutTree();
}

void GTreeWidget::setMinTimeSup(double min) {
    if(tree!=NULL)
        tree->minTimeInt.sup = min;
    modified = true;
    layoutTree();
}

void GTreeWidget::setMaxTimeInf(double min) {
    if(tree!=NULL)
        tree->maxTimeInt.inf = min;
    modified = true;
    layoutTree();
}

void GTreeWidget::setMaxTimeSup(double max) {
    maxTime = max;
    if(tree!=NULL)
        tree->maxTimeInt.sup = max;
    modified = true;
    layoutTree();
}

void GTreeWidget::setMinMaxTime(double min, double max) {
    minTime = min;
    maxTime = max;
    modified = true;
//    layoutTree();
}

void GTreeWidget::computeLabelWidth() {
    int n;
    labelWidth = 0;
    if(tree == NULL || tree->name == NULL)
        return;
    for(n=0; n<tree->size; n++)
        if(tree->name[n] != NULL) {
            int w = QFontMetrics(font).width(QString(tree->name[n]));
            if(w>labelWidth)
                labelWidth = w;
        }
}

void GTreeWidget::layoutTree(){
    if(tree == NULL)
        return;
    GTreeGeometry geo = GTreeGeometry(tree);
    geo.fillTreeGeometry(0, 0, QWidget::width()-labelWidth-2*QFontMetrics(font).averageCharWidth()-GRAPHICVIEW_BAND, leafSep);
    scaleTime = geo.getScale();
    for(int n=0; n<tree->size; n++) {
        if(n != tree->root)
            node[n]->setGeometry(geo.endNode(n), geo.endNode(tree->parent[n]));
        else
            node[n]->setGeometry(geo.endNode(n), QPointF(0, geo.endNode(n).y()));
        node[n]->setTimeBounds(geo.infTime(n),geo.supTime(n));
    }
    scale(1., 1.);
}

void GTreeWidget::resizeEvent(QResizeEvent * event) {
    layoutTree();
    QGraphicsView::resizeEvent(event);
}

double GTreeWidget::boundInf(int n) {
    if(tree->parent[n] != NOSUCH)
        return node[tree->parent[n]]->infTime();
    else
        return minTime;
}
void GTreeWidget::setTarget(GNode *n) {
    if(n != targetNode) {
        if(targetNode != NULL)
            targetNode->setNormal();
        if(n != NULL && !Tree::isDescendant(n->getIndex(), sourceNode->getIndex(), tree) && sourceNode->infTime()>=boundInf(n->getIndex())) {
            targetNode = n;
            targetNode->setHighlighted();
        } else {
            targetNode = NULL;
        }
    }
}

TypeTree *GTreeWidget::getTree() {
    return tree;
}

int GTreeWidget::getSelected() {
    if(selectedNode != NULL)
        return selectedNode->getIndex();
    else
        return NOSUCH;
}

void GTreeWidget::subSelectNode(int i)
{
    node[i]->setState(subSelectedState);
    for(int c=tree->node[i].child; c != NOSUCH; c = tree->node[c].sibling)
        subSelectNode(c);
}

void GTreeWidget::unsubSelectNode(int i)
{
    node[i]->setState(normalState);
    for(int c=tree->node[i].child; c != NOSUCH; c = tree->node[c].sibling)
        unsubSelectNode(c);
}

void GTreeWidget::selectNode(GNode *n)
{
    sourceNode = NULL;
    targetNode = NULL;
    if(selectedNode != NULL)
        unselectNode(selectedNode);
    selectedNode = n;
    if(selectedNode == NULL)
        return;
    selectedNode->setState(selectedState);
    for(int c=tree->node[selectedNode->getIndex()].child; c != NOSUCH; c = tree->node[c].sibling)
        subSelectNode(c);
}

void GTreeWidget::unselectNode(GNode *n)
{
    n->setState(normalState);
    for(int c=tree->node[n->getIndex()].child; c != NOSUCH; c = tree->node[c].sibling)
        unsubSelectNode(c);
}

void GTreeWidget::startMoving(GNode *n) {
    sourceNode = n;
}
void GTreeWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
    if(selectedNode != NULL) {
        unselectNode(selectedNode);
        selectedNode = NULL;
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

void GTreeWidget::endMoving() {
    if(targetNode != NULL) {
        if(selectedNode != NULL)
            unselectNode(selectedNode);
        selectedNode = NULL;
        targetNode->setNormal();
        Tree::transfer(sourceNode->getIndex(), targetNode->getIndex(), tree);
        targetNode = NULL;
        layoutTree();
        modified = true;
        emit change();
    } else {
        if(tree->parent[sourceNode->getIndex()] != NOSUCH)
            sourceNode->setEndPoint(node[tree->parent[sourceNode->getIndex()]]->getStartPoint());
        else
            sourceNode->setEndPoint(QPointF(0, node[sourceNode->getIndex()]->getStartPoint().y()));
    }
    sourceNode = NULL;
}

void GTreeWidget::itemMoved()
{
}

bool GTreeWidget::event(QEvent * e) {
    return QGraphicsView::event(e);
}

void GTreeWidget::mouseMoveEvent(QMouseEvent * mouseEvent) {
    double v = mapToScene(mouseEvent->pos()).x()/scaleTime+minTime;
    emit mouseMoved(QString::number(v,'f',2));
    QGraphicsView::mouseMoveEvent(mouseEvent);
}

void GTreeWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
         break;
    case Qt::Key_Down:
        break;
    case Qt::Key_Left:
        break;
    case Qt::Key_Right:
        break;
    case Qt::Key_Plus:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    case Qt::Key_Space:
    case Qt::Key_Enter:
         break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}



#ifndef QT_NO_WHEELEVENT

void GTreeWidget::wheelEvent(QWheelEvent *event)
{
    if(event->modifiers() == Qt::ControlModifier)
        scaleView(pow((double)1.5, -event->delta() / 240.0));
    else
        QGraphicsView::wheelEvent(event);
}

#endif


void GTreeWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
}

void GTreeWidget::clearRegisteredNodes() {
    selectedNode = NULL;
    sourceNode = NULL;
    targetNode = NULL;
}

void GTreeWidget::scaleView(qreal scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 5)
        return;
    scale(scaleFactor, scaleFactor);
}

void GTreeWidget::addNewLeaf() {
    int n = getSelected();
    if(tree==NULL || n==NOSUCH)
        return;
    Tree::addLeaf(n, strdup(""), tree);
    Tree::reorderTree(tree->name, tree);
    for(int n=tree->size-2; n<tree->size; n++) {
        if(tree->name != NULL)
            node[n] = new GNode(n, tree->name[n], this);
        else
            node[n] = new GNode(n, NULL, this);
        scene()->addItem(node[n]);
    }
    computeLabelWidth();
    layoutTree();
    modified = true;
    emit change();
}

void GTreeWidget::addNewLeaf(GNode *no) {
    if(tree==NULL || no==NULL)
        return;
    int n = no->getIndex();
    char *leafName;
    bool ok;
    QString text = QInputDialog::getText(this, tr("New leaf"), tr("Name:"), QLineEdit::Normal, QString(""), &ok);
//    if (ok && !text.isEmpty())
    if (ok)
        leafName = strdup(qPrintable(text));
    else
        return;
    Tree::addLeaf(n, leafName, tree);
    Tree::reorderTree(tree->name, tree);
    for(int n=tree->size-2; n<tree->size; n++) {
        if(tree->name != NULL)
            node[n] = new GNode(n, tree->name[n], this);
        else
            node[n] = new GNode(n, NULL, this);
        scene()->addItem(node[n]);
    }
    computeLabelWidth();
    layoutTree();
    modified = true;
    emit change();
}

void GTreeWidget::editNode(GNode *no) {
    if(tree==NULL || no==NULL || tree->name == NULL)
        return;
    int n = no->getIndex();
    bool ok;
    QString text = QInputDialog::getText(this, tr("Edit node"), tr("Name:"), QLineEdit::Normal, QString(tree->name[n]), &ok);
    if (ok) {
        if(tree->name[n] != NULL)
            free((void*)tree->name[n]);
        if(text.isEmpty())
            tree->name[n] = NULL;
        else
            tree->name[n] = strdup(qPrintable(text));
        no->setName(tree->name[n]);
        computeLabelWidth();
        Tree::reorderTree(tree->name, tree);
        layoutTree();
        modified = true;
    } else
        return;
}

void GTreeWidget::saveTree(QString filename) {
    if(tree == NULL)
        return;
    char *name = strdup(qPrintable(filename));
    FILE *f = fopen(name, "w");
    if(f) {
        Tree::fprintSubtreeNewick(f, tree->root, tree);
        fclose(f);
    } else
        QMessageBox::warning(this, QString("I/O error"), QString("Unable to write in ").append(filename));
}
QString GTreeWidget::saveSubtree(int n) {
    if(tree == NULL)
        return 0;
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList files;
    if (dialog.exec())
        files = dialog.selectedFiles();
    else
        return 0;
    char *filename = strdup(qPrintable(files.at(0)));
    FILE *f = fopen(filename, "w");
    if(f) {
        Tree::fprintSubtreeNewick(f, n, tree);
        fclose(f);
        return files.at(0);
    } else {
        QMessageBox::warning(this, QString("I/O error"), QString("Unable to write in ").append(files.at(0)));
        return 0;
    }
}



void GTreeWidget::removeNode() {
    int n = getSelected(), oldSize = tree->size;
    if(tree==NULL || n==NOSUCH)
        return;
    clearRegisteredNodes();
    Tree::removeSubtree(n, tree);
    Tree::reorderTree(tree->name, tree);
    for(int n=0; n<tree->size; n++) {
        if(tree->name != NULL)
            node[n]->setName(tree->name[n]);
        else
            node[n]->setName(NULL);
        node[n]->setState(normalState);
    }
    for(int n=tree->size; n<oldSize; n++) {
        scene()->removeItem(node[n]);
        delete node[n];
    }
    computeLabelWidth();
    layoutTree();
    modified = true;
    emit change();
}


void GTreeWidget::removeNode(GNode *no) {
    if(tree==NULL || no==NULL || no->getIndex() == tree->root)
        return;
    int n = no->getIndex(), oldSize = tree->size;;
    clearRegisteredNodes();
    Tree::removeSubtree(n, tree);
    Tree::reorderTree(tree->name, tree);
    for(int n=0; n<tree->size; n++) {
        if(tree->name != NULL)
            node[n]->setName(tree->name[n]);
        else
            node[n]->setName(NULL);
        node[n]->setState(normalState);
    }
    for(int n=tree->size; n<oldSize; n++) {
        scene()->removeItem(node[n]);
        delete node[n];
    }
    computeLabelWidth();
    layoutTree();
    modified = true;
    emit change();
}


void GTreeWidget::zoomIn()
{
    scaleView(qreal(1.2));
}

void GTreeWidget::zoomOut()
{
    scaleView(1 / qreal(1.2));
}


void GTreeWidget::nodeContextMenu(GNode *n, QPoint pos) {
    if(n == NULL)
        return;
    QMenu menu;
    QAction *addAction = menu.addAction("Add leaf");
    QAction *editAction = menu.addAction("Edit node");
    QAction *removeAction = menu.addAction("Remove node");
    QAction *saveAction = menu.addAction("Save subtree as");
    QAction *a = menu.exec(pos);
    if(a==addAction)
        addNewLeaf(n);
    if(a==removeAction)
        removeNode(n);
    if(a==editAction)
        editNode(n);
    if(a==saveAction)
        saveSubtree(n->getIndex());
}
