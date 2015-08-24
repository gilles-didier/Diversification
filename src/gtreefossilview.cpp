/*
    'Diversification' and 'estimate' estimate diversification and fossilization rates frm tree shapes and fossil dates / 
    'sample' simulates random trees and fossils finds and saves them in Newick format / 
    'test' simulates random multiplex to test community detection approaches
	'complexity' simulates random trees and fossils finds, computes their complexity index and return a CSV file with lines
		<complexity index>	<likelihood computation time>	<tree size>	<fossil number>
	'assess' simulates random trees and fossils finds, estimates speciation and extinction rates and returns the mean absolute error

    Copyright (C) 2015  Gilles DIDIER

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/




#include "gtreefossilview.h"
#include "gtreefossilgeometry.h"
#include "gnode.h"
#include "gfossil.h"
#include "gaddfossildialog.h"
#include "geditfossildialog.h"
#include "gnodedialog.h"
#include "gleafdialog.h"
#include <QtWidgets>

GTreeFossilView::GTreeFossilView(TypeTree *t, TypeFossilIntFeature *f, double min ,double max, QWidget *parent): GTreeWidget(parent)
{
    tree = NULL;
    fos = NULL;
    targetNode = NULL;
    sourceNode = NULL;
    selectedNode = NULL;
    node = QVector<GNode*>(SIZE_VECTOR);
    fossil = QVector<GFossil*>(SIZE_VECTOR);
    font.setFamily(font.defaultFamily());
    leafSep = QFontMetrics(font).lineSpacing();
    setScene(NULL);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setMinMaxTime(min, max);
    setTreeFossil(t, f);
}

GTreeFossilView::~GTreeFossilView()
{
 if(tree != NULL)
  Tree::freeTree(tree);
 if(fos!=NULL)
  freeFossilIntFeature(fos);
}


void GTreeFossilView::setTree(TypeTree *t) {
//    GTreeWidget::setTree(t);
    setTreeFossil(t, NULL);
}

void GTreeFossilView::setFossil(TypeFossilIntFeature *fo) {
    if(fos != NULL)
        freeFossilIntFeature(fos);
    fos = fo;
    if(fos != NULL)
        reallocFossilIntNode(fos, tree->sizeBuf);
    fossil.resize(fos->sizeFossil);
    if(tree != NULL && fos != NULL) {
        for(int n=0; n<tree->size; n++)
            for(int f=fos->fossilInt[n]; f>=0; f=fos->fossilIntList[f].prec) {
                fossil[f] = new GFossil(f, NULL, this);
                scene()->addItem(fossil[f]);
            }
        layoutTree();
        modified = true;
        emit change();
        show();
    }
}

void GTreeFossilView::setTreeFossil(TypeTree *t, TypeFossilIntFeature *fo) {
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
    if(fos != NULL)
        freeFossilIntFeature(fos);
    fos = fo;
    if(fos != NULL)
        reallocFossilIntNode(fos, tree->sizeBuf);
    else
        fos = newFossilIntFeature(tree->sizeBuf);
    if(tree != NULL) {
        if(tree->size > node.size())
            node.resize(tree->size);
        for(int n=0; n<tree->size; n++) {
            int f;
            if(tree->name != NULL)
                node[n] = new GNode(n, tree->name[n], this);
            else
                node[n] = new GNode(n, NULL, this);
            if(fos != NULL)
                fossil.resize(fos->sizeFossil);
                for(f=fos->fossilInt[n]; f!=NOSUCH; f=fos->fossilIntList[f].prec) {
                    fossil[f] = new GFossil(f, NULL, this);
                    scene()->addItem(fossil[f]);
                }
            scene()->addItem(node[n]);
        }
        layoutTree();
    }
    modified = false;
    emit change();
    show();
}

void GTreeFossilView::layoutTree(){
    if(tree == NULL)
        return;
    GTreeFossilGeometry geo = GTreeFossilGeometry(tree, fos, minTime, maxTime);
    geo.fillTreeGeometry(0, 0, QWidget::width()-labelWidth-2*QFontMetrics(font).averageCharWidth()-GRAPHICVIEW_BAND, leafSep);
    scaleTime = geo.getScale();
    for(int n=0; n<tree->size; n++) {
        if(n != tree->root)
            node[n]->setGeometry(geo.endNode(n), geo.endNode(tree->parent[n]));
        else
            node[n]->setGeometry(geo.endNode(n), QPointF(0, geo.endNode(n).y()));
        node[n]->setTimeBounds(geo.infTime(n),geo.supTime(n));
    }
    if(fos != NULL)
        for(int f=0; f<fos->sizeFossil; f++)
            fossil[f]->setLine(geo.lineFossil(f));
    scale(1., 1.);
//    scene()->setSceneRect(QRectF());
}

void GTreeFossilView::nodeContextMenu(GNode *n, QPoint pos) {
    if(n == NULL)
        return;
    QMenu menu;
    QAction *editAction = menu.addAction("Edit");
    QAction *removeAction = menu.addAction("Remove");
    QAction *addAction = menu.addAction("Add leaf");
    QAction *addFossilAction = menu.addAction("Add Fossil");
    QAction *editFossilAction = menu.addAction("Edit Fossils");
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
    if(a==addFossilAction)
        addFossil(n);
    if(a==editFossilAction)
        editFossil(n);
}

void GTreeFossilView::editNode(GNode *no) {
    if(tree==NULL || no==NULL || tree->name == NULL)
        return;
    int n = no->getIndex();
    if(tree->node[n].child == NOSUCH) {/* leaf */
        GLeafDialog *dialog = new GLeafDialog(QString(tree->name[n]), fos->status[n],
                                              (fos->status[n]==unknownNodeStatus)?fos->endTimeTable[fos->endTime[n]].inf:no->supTime(),
                                              (fos->status[n]==unknownNodeStatus)?fos->endTimeTable[fos->endTime[n]].sup:no->infTime(), this);
        if (dialog->exec()) {
            if(tree->name[n] != NULL)
                free((void*)tree->name[n]);
            QString text = dialog->getName();
            if(text.isEmpty())
                tree->name[n] = NULL;
            else
                tree->name[n] = strdup(qPrintable(text));
            fos->status[n] =dialog->getStatus();
            if(fos->status[n] == unknownNodeStatus) {
                if(fos->endTime[n] == NOSUCH) {
                    if(fos->sizeTime>=fos->sizeBufTime) {
                        fos->sizeBufTime += INC_BUF_TIME_TABLE;
                        fos->endTimeTable = (TypeTimeInterval*) realloc((void*) fos->endTimeTable, fos->sizeBufTime*sizeof(TypeTimeInterval));
                    }
                    fos->endTimeTable[fos->sizeTime].inf = dialog->getInf();
                    fos->endTimeTable[fos->sizeTime].sup = dialog->getSup();
                    fos->endTime[n] = fos->sizeTime++;
                } else {
                    fos->endTimeTable[fos->endTime[n]].inf = dialog->getInf();
                    fos->endTimeTable[fos->endTime[n]].sup = dialog->getSup();
                }
            }
            no->setName(tree->name[n]);
            computeLabelWidth();
            Tree::reorderTree(tree->name, tree);
            layoutTree();
            modified = true;
        }
    } else {
        GNodeDialog *dialog = new GNodeDialog(QString(tree->name[n]), this);
        if (dialog->exec()) {
            if(tree->name[n] != NULL)
                free((void*)tree->name[n]);
            QString text = dialog->getName();
            if(text.isEmpty())
                tree->name[n] = NULL;
            else
                tree->name[n] = strdup(qPrintable(text));
            no->setName(tree->name[n]);
            computeLabelWidth();
            Tree::reorderTree(tree->name, tree);
            layoutTree();
            modified = true;
        }
    }
}

void GTreeFossilView::addNewLeaf(GNode *no) {
    if(tree==NULL || no==NULL)
        return;
    int n = no->getIndex();
    GLeafDialog *dialog = new GLeafDialog(QString(""), contempNodeStatus, no->supTime(), no->infTime(), this);
    if (dialog->exec()) {
        char *leafName;
        int ind;
        QString text = dialog->getName();
        if(text.isEmpty())
            leafName = NULL;
        else
            leafName = strdup(qPrintable(text));
        ind = Tree::addLeaf(n, leafName, tree);
        Tree::reorderTree(tree->name, tree);
        if(fos->sizeBufNode<tree->sizeBuf)
            reallocFossilIntNode(fos, tree->sizeBuf);
        fos->sizeNode = tree->size;
        fos->status[ind] = dialog->getStatus();
        if(fos->status[ind] == unknownNodeStatus) {
            if(fos->sizeTime>=fos->sizeBufTime) {
                fos->sizeBufTime += INC_BUF_TIME_TABLE;
                fos->endTimeTable = (TypeTimeInterval*) realloc((void*) fos->endTimeTable, fos->sizeBufTime*sizeof(TypeTimeInterval));
            }
            fos->endTimeTable[fos->sizeTime].inf = dialog->getInf();
            fos->endTimeTable[fos->sizeTime].sup = dialog->getSup();
            fos->endTime[ind] = fos->sizeTime++;
        }
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
}

void GTreeFossilView::removeNode(GNode *no) {
    if(tree==NULL || no==NULL || no->getIndex() == tree->root)
        return;
    clearRegisteredNodes();
    int oldTreeSize = tree->size, oldFossilSize = fos->sizeFossil, *index = Tree::removeSubtreeReturnIndex(no->getIndex(), tree);
    reindexFossilInt(fos, index);
    free((void*)index);
    for(int f=fos->sizeFossil; f<oldFossilSize; f++) {
        scene()->removeItem(fossil[f]);
        delete fossil[f];
    }
    for(int n=0; n<tree->size; n++) {
        if(tree->name != NULL)
            node[n]->setName(tree->name[n]);
        else
            node[n]->setName(NULL);
        node[n]->setState(normalState);
    }
    for(int n=tree->size; n<oldTreeSize; n++) {
        scene()->removeItem(node[n]);
        delete node[n];
    }
    computeLabelWidth();
    layoutTree();
    modified = true;
    emit change();
}

void GTreeFossilView::addFossil(GNode *n) {
    GAddFossilDialog *dialog = new GAddFossilDialog(boundInf(n->getIndex()), n->supTime(), this);
    qDebug() << tree->time[n->getIndex()] << " vs " << n->supTime();
    if (dialog->exec()) {
        if(fos == NULL) {
            fos = newFossilIntFeature(tree->sizeBuf);
        }
        addFossilInt(fos, n->getIndex(), dialog->inf(), dialog->sup());
        qDebug() << "size after " << fos->sizeFossil;
        fossil.resize(fos->sizeFossil);
        fossil[fos->sizeFossil-1] = new GFossil(fos->sizeFossil-1, NULL, this);
        scene()->addItem(fossil[fos->sizeFossil-1]);
        modified = true;
        layoutTree();
    }
    delete dialog;
}

void GTreeFossilView::editFossil(GNode *n) {
    GEditFossilDialog *dialog = new GEditFossilDialog(n->getIndex(), fos, boundInf(n->getIndex()), n->supTime(), this);
    if (dialog->exec()) {
        int i, f, g, r = 0, *l, *prec;
        QList<TypeTimeInterval> *list = dialog->list();
        prec = &(fos->fossilInt[n->getIndex()]);
        for(*prec=fos->fossilInt[n->getIndex()], i=0; *prec!=NOSUCH && i<list->size(); prec=&(fos->fossilIntList[*prec].prec), i++)
            fos->fossilIntList[*prec].fossilInt = list->at(i);
        if(*prec != NOSUCH) {
            f = *prec;
            *prec = NOSUCH;
            for(g=f; g!=NOSUCH ; g=fos->fossilIntList[g].prec)
                r++;
            l = (int*) malloc((r+1)*sizeof(int));
            r = 0;
            for(g=f, i=0; g!=NOSUCH ; g=fos->fossilIntList[g].prec)
                l[r++] = g;
            l[r] = NOSUCH;
            removeFossilIntFossil(fos, l);
            for(int f=fos->sizeFossil+r-1; f>=fos->sizeFossil; f--) {
                scene()->removeItem(fossil[f]);
                delete fossil[f];
            }
            i += r;
        }
         if(list->size()>i) {
            fossil.resize(fos->sizeFossil+list->size()-i);
            for(; i<list->size(); i++) {
                fossil[fos->sizeFossil] = new GFossil(fos->sizeFossil, NULL, this);
                scene()->addItem(fossil[fos->sizeFossil]);
                addFossilInt(fos, n->getIndex(), list->at(i).inf, list->at(i).sup);
            }
        } else
             fossil.resize(fos->sizeFossil);
         modified = true;
        layoutTree();
    }
    delete dialog;
}

TypeFossilIntFeature *GTreeFossilView::getFossil() {
    return fos;
}

void GTreeFossilView::saveTree(QString filename) {
    if(tree == NULL)
        return;
    char *name = strdup(qPrintable(filename));
    FILE *f = fopen(name, "w");
    if(f) {
        fprintTreeFossilInt(f, tree, fos);
        fclose(f);
    } else
        QMessageBox::warning(this, QString("I/O error"), QString("Unable to write in ").append(filename));
}

QString GTreeFossilView::saveSubtree(int n) {
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
        fprintSubTreeFossilInt(f, n, tree, fos);
        fclose(f);
        return files.at(0);
    } else {
        QMessageBox::warning(this, QString("I/O error"), QString("Unable to write in ").append(files.at(0)));
        return 0;
    }
}

