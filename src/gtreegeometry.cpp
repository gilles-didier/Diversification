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
#include "gtreegeometry.h"
#include "Utils.h"


#define PREFERRED_WIDTH 500

GTreeGeometry::GTreeGeometry(TypeTree *t, double min, double max)
{
    tree = t;
    time = NULL;
    inf = NULL;
    sup = NULL;
    setMinMaxTime(min, max);
    if(tree != NULL && tree->size > 0) {
        computeTime();
        start = new QPointF[tree->size];
    } else
        start = NULL;
}


GTreeGeometry::~GTreeGeometry()
{
    if(start != NULL)
        delete[] start;
    if(time != NULL)
        free((void*)time);
    if(inf != NULL)
        free((void*)inf);
    if(sup != NULL)
        free((void*)sup);
}

double GTreeGeometry::getScale() {
    return scale;
}

void GTreeGeometry::setTree(TypeTree *t) {
    tree = t;
    computeTime();
}

void GTreeGeometry::setMinMaxTime(double min, double max) {
    minTime = min;
    maxTime = max;
}

void GTreeGeometry::setMinTime(double t) {
    minTime = t;
}

void GTreeGeometry::setMaxTime(double t) {
    maxTime = t;
}

void GTreeGeometry::setLeafSep(int l) {
    leafSep = l;
}

void GTreeGeometry::fillTime(int n, double tanc, double *min, double *max, int *dmax) {
    int c;
    if(time[n] == NO_TIME) {
        double tmp = utils_MAX(tanc, min[n]);
        if(max[n]<tanc)
            printf("\n\nProblem %.2lf %.2lf\n%d\n", max[n], tanc, n);
        if(tree->node[n].child != NOSUCH)
            time[n] = tmp+(max[n]-tmp)/((double)(2+dmax[n]));
        else
            time[n] = max[n];
    }
    for(c=tree->node[n].child; c>=0; c=tree->node[c].sibling)
        fillTime(c, time[n], min, max, dmax);
}

void GTreeGeometry::fillBounds(int n, double tmin, double tmax, double *min, double *max, int *dmax) {
    int c;
    if(time[n] != NO_TIME) {
        min[n] = time[n];
    } else {
        min[n] = tmin;
    }
    for(c=tree->node[n].child; c>=0; c=tree->node[c].sibling)
        fillBounds(c, min[n], tmax, min, max, dmax);
    if(time[n] != NO_TIME) {
        max[n] = time[n];
        dmax[n] = 0;
    } else {
        if(tree->node[n].child == NOSUCH) {
            max[n] = tmax;
            dmax[n] = 0;
        } else {
            max[n] = tmax+1;
            dmax[n] = 0;
            for(c=tree->node[n].child; c>=0; c=tree->node[c].sibling) {
                if((max[c])<(max[n])) {
                    max[n] = max[c];
                    dmax[n] = dmax[c]+1;
                }
            }
        }
    }
}
/*
void GTreeGeometry::fillUnknownTimes() {
    int *dmax;
    double *min, *max;

    min = (double*) malloc(tree->size*sizeof(double));
    max = (double*) malloc(tree->size*sizeof(double));
    dmax = (int*) malloc(tree->size*sizeof(int));
    fillBounds(tree->root, minTime, maxTime, min, max, dmax);
    fillTime(tree->root,  minTime, min, max, dmax);
    free((void*)min);
    free((void*)max);
    free((void*)dmax);
}
*/

void GTreeGeometry::fillUnknownTimes() {
    int *dmax;

    inf = (double*) malloc(tree->size*sizeof(double));
    sup = (double*) malloc(tree->size*sizeof(double));
    dmax = (int*) malloc(tree->size*sizeof(int));
    fillBounds(tree->root, minTime, maxTime, inf, sup, dmax);
    fillTime(tree->root,  minTime, inf, sup, dmax);
    free((void*)dmax);
}

double *GTreeGeometry::infTime()
{
    return inf;
}

double *GTreeGeometry::supTime()
{
    return sup;
}

double GTreeGeometry::infTime(int n)
{
/*    if(tree->parent[n] != NOSUCH)
        return inf[tree->parent[n]];
    else
        return minTime;
*/
    return inf[n];
}

/*return the maximum time at which an event can occur on branch ending with n*/
double GTreeGeometry::supTime(int n)
{
    return sup[n];
    if(tree->time[n] != NO_TIME)
        return tree->time[n];
    if(tree->node[n].child == NOSUCH)
        return maxTime;
    double t = sup[tree->node[n].child];
    for(int f=tree->node[tree->node[n].child].sibling; f!=NOSUCH; f=tree->node[f].sibling)
        if(t>sup[f])
            t = sup[f];
    return t;
}

void GTreeGeometry::computeTime() {
    int i;
    if(time != NULL)
        free((void*)time);
    if(tree == NULL) {
        time = NULL;
        return;
    }
    time = (double*) malloc(tree->size*sizeof(double));
    if(tree->time != NULL) {
        for(i=0; i<tree->size; i++)
            time[i] = tree->time[i];
    } else {
        for(i=0; i<tree->size; i++)
            time[i] = NO_TIME;
    }
    fillUnknownTimes();
}

double *GTreeGeometry::getTimeTable() {
    return time;
}

QPointF GTreeGeometry::endNode(int n) {
    return start[n];
}

double GTreeGeometry::fillTreeGeometry(int x0, int y0, int w, int l) {
    int tmp;
    double min, max, y;
    xoffset = x0;
    yoffset = y0;
    leafCur = 0;
    width = w;
    scale = ((double) width)/(maxTime- minTime);
    leafSep = l;
    if(tree == NULL || tree->size<=0)
        return 0.;
    if((tmp = tree->node[tree->root].child) >= 0) {
        min = fillNodeGeometry(tmp, tree->root);
        max = min;
        for(tmp = tree->node[tmp].sibling; tmp!=NOSUCH; tmp = tree->node[tmp].sibling) {
            max = fillNodeGeometry(tmp, tree->root);
        }
    } else {
        max = leafCur+leafSep/2.;
        min = max;
    }
    y = (min+max)/2;
    start[tree->root] = QPointF((time[tree->root]-minTime)*scale+xoffset, y+yoffset);
    return y;
}

double GTreeGeometry::fillNodeGeometry(int n, int parent) {
    double min, max, y;
    if(tree->node[n].child >= 0) {
        int tmp = tree->node[n].child;
        min = fillNodeGeometry(tmp, n);
        max = min;
        for(tmp = tree->node[tmp].sibling; tmp >= 0; tmp = tree->node[tmp].sibling)
            max = fillNodeGeometry(tmp, n);
        y = (min+max)/2;
    } else {
        leafCur += leafSep;
        y = leafCur;
    }
    start[n] = QPointF((time[n]-minTime)*scale+xoffset, yoffset+y);
    return y;
}
