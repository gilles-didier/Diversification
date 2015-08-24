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




#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <QFontMetrics>
#include <locale.h>
#include "qtreefossildrawer.h"
#include "Utils.h"


#define PREFERRED_WIDTH 500

QTreeFossilDrawer::QTreeFossilDrawer(TypeFossilIntFeature *f, TypeTree *t, double min, double max) : QTreeDrawer(t)
{
    fos = f;
    tree = t;
    time = NULL;
    tmin = min;
    tmax = max;
    radius = 10;
    width = PREFERRED_WIDTH;
    font= QFont("Times", 8, QFont::Normal);
//    font.setFamily(font.defaultFamily());
    computeTime();
    setMetrics();
    computeScale();
}


QTreeFossilDrawer::~QTreeFossilDrawer()
{
//    if(time != NULL)
//        free((void*)time);
}


void QTreeFossilDrawer::setFossil(TypeFossilIntFeature *f) {
    fos = f;
     computeTime();
}


void QTreeFossilDrawer::fillBounds(int n, double tmin, double tmax, double *min, double *max, int *dmax) {
    int c;
    if(tree->time[n] != NO_TIME) {
        min[n] = tree->time[n];
    } else {
        if(fos && fos->fossilInt[n] != -1) {
            int f;
            min[n] = fos->fossilIntList[fos->fossilInt[n]].fossilInt.sup;
            for(f=fos->fossilIntList[fos->fossilInt[n]].prec; f>=0; f=fos->fossilIntList[f].prec)
                if(fos->fossilIntList[f].fossilInt.sup>min[n])
                    min[n] = fos->fossilIntList[f].fossilInt.sup;
        } else
            min[n] = tmin;
    }
    for(c=tree->node[n].child; c>=0; c=tree->node[c].sibling)
        fillBounds(c, min[n], tmax, min, max, dmax);
    if(tree->time[n] != NO_TIME) {
        max[n] = tree->time[n];
        dmax[n] = 0;
    } else {
        if(tree->node[n].child<0) {
            max[n] = tmax;
            dmax[n] = 0;
        } else {
            max[n] = tmax+1;
            dmax[n] = 0;
            for(c=tree->node[n].child; c>=0; c=tree->node[c].sibling) {
                if(fos && fos->fossilInt[c] != -1) {
                    int f;
                    for(f=fos->fossilInt[c]; f>=0; f=fos->fossilIntList[f].prec)
                        if(fos->fossilIntList[f].fossilInt.inf<max[n]) {
                            max[n] = fos->fossilIntList[f].fossilInt.inf;
                            dmax[n] = 0;
                        }
                } else {
                    if((max[c])<(max[n])) {
                        max[n] = max[c];
                        dmax[n] = dmax[c]+1;
                    }
                }
            }
        }
    }
}

void QTreeFossilDrawer::computeTime() {
    int i;
    if(time != NULL)
        free((void*)time);
    if(tree == NULL) {
        time = NULL;
        return;
    }
    time = (double*) malloc(tree->size*sizeof(double));
    for(i=0; i<tree->size; i++)
        time[i] = tree->time[i];
    if(tree != NULL && fos != NULL)
        for(int n=0; n<tree->size; n++)
            if(tree->node[n].child == NOSUCH && time[n] == NO_TIME) {
                if(fos->fossilInt[n] != NOSUCH) {
                    int f;
                    time[n] = fos->fossilIntList[fos->fossilInt[n]].fossilInt.sup;
                    for(f=fos->fossilIntList[fos->fossilInt[n]].prec; f!=NOSUCH; f=fos->fossilIntList[f].prec)
                        if(fos->fossilIntList[f].fossilInt.sup>time[n])
                            time[n] = fos->fossilIntList[f].fossilInt.sup;
                } else
                    time[n] = tmax;
            }
    fillUnknownTimes(tmin, tmax);
}

double QTreeFossilDrawer::paintTreeWidth(QPainter *painter, int x0, int y0, int w) {
    double y = QTreeDrawer::paintTreeWidth(painter, x0, y0, w);
    paintFossil(painter, tree->root, y);
    return y;
}

double QTreeFossilDrawer::paintTree(QPainter *painter, int x0, int y0) {
    double y = QTreeDrawer::paintTree(painter, x0, y0);
    paintFossil(painter, tree->root, y);
    return y;
}

double QTreeFossilDrawer::paintNode(QPainter *painter, int n, int parent) {
    double y;
    y = QTreeDrawer::paintNode(painter, n, parent);
    paintFossil(painter, n, y);
    return y;
}

void QTreeFossilDrawer::paintFossil(QPainter *painter, int n, double y) {
    if(fos != NULL) {
        int f;
        for(f=fos->fossilInt[n]; f>=0; f=fos->fossilIntList[f].prec)
             paintLineDot(painter, (fos->fossilIntList[f].fossilInt.inf-tmin)*scale+xoffset,
            yoffset+y,
            (fos->fossilIntList[f].fossilInt.sup-tmin)*scale+xoffset,
            yoffset+y);
    }
}

void QTreeFossilDrawer::paintLineDot(QPainter *painter, double x1, double y1, double x2, double y2) {
    QPen pen;
    painter->save();
    pen.setColor(QColor(100, 0, 0, 100));
    pen.setCapStyle(Qt::RoundCap);
    pen.setWidth(radius);
    painter->setPen(pen);
    painter->drawLine(QPointF(x1,y1), QPointF(x2,y2));
    painter->restore();
}
