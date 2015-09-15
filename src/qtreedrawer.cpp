#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <QFontMetrics>
#include <locale.h>
#include "qtreedrawer.h"
#include "Utils.h"


#define PREFERRED_WIDTH 500

QTreeDrawer::QTreeDrawer(TypeTree *t)
{
    tree = t;
    time = NULL;
    tmin = 0.;
    tmax = 1.;
    width = PREFERRED_WIDTH;
    font= QFont("Times", -1, QFont::Normal);
    computeTime();
    setMetrics();
    computeScale();
}


QTreeDrawer::~QTreeDrawer()
{
//    if(time != NULL)
//        free((void*)time);
}

int QTreeDrawer::getWidth() {
    return width;
}

int QTreeDrawer::getHeight() {
    return height;
}

void QTreeDrawer::setTree(TypeTree *t) {
    tree = t;
    computeTime();
    setMetrics();
}



void QTreeDrawer::setFont(QFont f) {
    font = f;
    setMetrics();
}
void QTreeDrawer::setWidth(int w) {
    width = w;
    computeScale();
}

void QTreeDrawer::setMinTime(double t) {
    tmin = t;
    computeScale();
}

void QTreeDrawer::setMaxTime(double t) {
    tmax = t;
    computeScale();
}

void QTreeDrawer::fillTime(int n, double tanc, double *min, double *max, int *dmax) {
    int c;
    if(time[n] == NO_TIME) {
        double tmp = utils_MAX(tanc, min[n]);
        if(max[n]<tanc)
            printf("\n\nProblem %.2lf %.2lf\n%d\n", max[n], tanc, n);
        if(tree->node[n].child>=0)
            time[n] = tmp+(max[n]-tmp)/((double)(2+dmax[n]));
        else
            time[n] = max[n];
    }
    for(c=tree->node[n].child; c>=0; c=tree->node[c].sibling)
        fillTime(c, time[n], min, max, dmax);
}

void QTreeDrawer::fillBounds(int n, double tmin, double tmax, double *min, double *max, int *dmax) {
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
        if(tree->node[n].child<0) {
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

void QTreeDrawer::fillUnknownTimes(double tmin, double tmax) {
    int *dmax;
    double *min, *max;

    min = (double*) malloc(tree->size*sizeof(double));
    max = (double*) malloc(tree->size*sizeof(double));
    dmax = (int*) malloc(tree->size*sizeof(int));
    fillBounds(tree->root, tmin, tmax, min, max, dmax);
    fillTime(tree->root, tmin, min, max, dmax);
    free((void*)min);
    free((void*)max);
    free((void*)dmax);
}


void QTreeDrawer::computeTime() {
    int i;
    if(time != NULL)
        free((void*)time);
    if(tree == NULL) {
        time = NULL;
        return;
    }
    time = (double*) malloc(tree->size*sizeof(double));
    if(tree->time != NULL)
        for(i=0; i<tree->size; i++)
            time[i] = tree->time[i];
    else
        for(i=0; i<tree->size; i++)
            time[i] = NO_TIME;
    fillUnknownTimes(tmin, tmax);
}


#define NB_MIN 6

int QTreeDrawer::getScaleWidth() {
    return width-labelWidth-labelSep;
}

int QTreeDrawer::getScaleHeight() {
    return tickLength+leafSep;
}

double QTreeDrawer::drawScaleGenericUp(QPainter *painter, double x, double y) {
    int flag = 0;
    double start, end, step, cur, widthS;
    widthS = width-labelWidth-labelSep-x;

    paintEdge(painter, x, y+2*tickLength+1*leafSep, x+widthS, y+2*tickLength+1*leafSep);
    if((tmax-tmin) <= 0.)
        return 0.;
    step = pow(10., floor(log10(tmax-tmin)));
    if((tmax-tmin)/step<NB_MIN)
        step /= 2.;
    flag = step<1.;
    start = step*ceil(tmin/step);
    end = step*floor(tmax/step);
    for(cur=start; cur<=end; cur += step) {
        char tmp[500];
        paintEdge(painter, x+(cur-tmin)*scale, y+1*leafSep+tickLength, x+(cur-tmin)*scale, y+1*leafSep+2*tickLength);
         if(flag)
            sprintf(tmp, "%.1lf", cur);
        else
            sprintf(tmp, "%.0lf", cur);
        paintTreeLabel(painter, x+(cur-tmin)*scale, y+1*leafSep, tmp, "bc");
    }
    return y+2*tickLength+1*leafSep;
}

double QTreeDrawer::drawScaleGenericDown(QPainter *painter, double x, double y) {
    int flag = 0;
    double start, end, step, cur, widthS;
    widthS = width-labelWidth-labelSep-x;
    paintEdge(painter, x, y, x+widthS, y);
    if((tmax-tmin) <= 0.)
        return 0.;
    step = pow(10., floor(log10(tmax-tmin)));
    if((tmax-tmin)/step<NB_MIN)
        step /= 2.;
    flag = step<1.;
    start = step*ceil(tmin/step);
    end = step*floor(tmax/step);
    for(cur=start; cur<=end; cur += step) {
        char tmp[500];
        paintEdge(painter, x+(cur-tmin)*scale, y, x+(cur-tmin)*scale, y+tickLength);
 //       paintEdge(painter, x+(cur-tmin)*scale, y, x+(cur-tmin)*scale, y+10);
        if(flag)
            sprintf(tmp, "%.1lf", cur);
        else
            sprintf(tmp, "%.0lf", cur);
        paintTreeLabel(painter, x+(cur-tmin)*scale, y+tickLength, tmp, "tc");
    }
    return y+tickLength+leafSep;
}

void QTreeDrawer::computeLabelWidth() {
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

void QTreeDrawer::computeLabelWidth(QPainter *painter) {
    int n;
    labelWidth = 0;
    if(tree->name == NULL)
        return;
    for(n=0; n<tree->size; n++)
        if(tree->name[n] != NULL) {
            int w = painter->fontMetrics().width(QString(tree->name[n]));
            if(w>labelWidth)
                labelWidth = w;
        }
}

void QTreeDrawer::computeScale(){
    scale = (width-labelWidth-2*labelSep)/(tmax-tmin);
}

#define RADIUS 8
#define MINIMUM_TREE_WIDTH 200

void QTreeDrawer::setMetrics() {
    leafSep = QFontMetrics(font).lineSpacing();
    computeLabelWidth();
    if((labelWidth+MINIMUM_TREE_WIDTH)>width) {
        width = labelWidth+MINIMUM_TREE_WIDTH;
        computeScale();
    }
    tickLength = QFontMetrics(font).height()/3; //4*QFontMetrics(font).leading();
    labelSep = QFontMetrics(font).averageCharWidth();
    height = leafSep*(Tree::countLeaves(tree)+1);
    radius = QFontMetrics(font).height()/2;
    penWidth = QFontMetrics(font).height()/15;
    xoffset = 0;
    yoffset = 0;
}


void QTreeDrawer::setMetrics(QPainter *painter, int h) {
    painter->save();
    int n = Tree::countLeaves(tree);
    int s = 11;
    do {
        font.setPointSize(--s);
        painter->setFont(font);
        leafSep = painter->fontMetrics().lineSpacing();
    } while(s>5 && leafSep*n>h);
    computeLabelWidth(painter);
    if((labelWidth+MINIMUM_TREE_WIDTH)>width)
        width = labelWidth+MINIMUM_TREE_WIDTH;
    tickLength = painter->fontMetrics().height()/3; //4*QFontMetrics(font).leading();
    labelSep = painter->fontMetrics().averageCharWidth();
    height = leafSep*(Tree::countLeaves(tree)+1)+2*tickLength+1*leafSep;
    radius = painter->fontMetrics().height()/2;
    penWidth = painter->fontMetrics().height()/15;
    painter->restore();
    xoffset = 0;
    yoffset = 0;
}

double QTreeDrawer::paintTreeWidth(QPainter *painter, int x0, int y0, int w) {
    setWidth(w);
    return paintTree(painter, x0, y0);
}

double QTreeDrawer::paintTree(QPainter *painter, int x0, int y0) {
    int tmp;
    double min, max, y;
    xoffset = x0;
    yoffset = y0;
    leafCur = 0;

    if(tree == NULL || tree->size<=0)
        return 0.;
    if((tmp = tree->node[tree->root].child) >= 0) {
        min = paintNode(painter, tmp, tree->root);
        max = min;
        for(tmp = tree->node[tmp].sibling; tmp >= 0; tmp = tree->node[tmp].sibling) {
            max = paintNode(painter, tmp, tree->root);
        }
    } else {
        max = leafCur+leafSep/2.;
        min = max;
    }
    paintEdge(painter, (time[tree->root]-tmin)*scale+xoffset, yoffset+min, (time[tree->root]-tmin)*scale+xoffset, yoffset+max);
    y = (min+max)/2;
    paintEdge(painter, (time[tree->root]-tmin)*scale+xoffset, yoffset+y, xoffset, yoffset+y);
    if(tree->name != NULL && tree->name[tree->root])
        paintTreeLabel(painter, (time[tree->root]-tmin)*scale+xoffset+labelSep, y+yoffset, tree->name[tree->root], "cl");
    return y;
}

double QTreeDrawer::paintNode(QPainter *painter, int n, int parent) {
    double min, max, y;
    if(tree->node[n].child >= 0) {
        int tmp = tree->node[n].child;
        min = paintNode(painter, tmp, n);
        max = min;
        for(tmp = tree->node[tmp].sibling; tmp >= 0; tmp = tree->node[tmp].sibling)
            max = paintNode(painter, tmp, n);
        y = (min+max)/2;
        paintEdge(painter, (time[n]-tmin)*scale+xoffset, yoffset+min, (time[n]-tmin)*scale+xoffset, yoffset+max);
        if(tree->name && tree->name[n]) {
             paintTreeLabel(painter, (time[n]-tmin)*scale+xoffset+labelSep, y+yoffset, tree->name[n], "cl");
        }
    } else {
        leafCur += leafSep;
        y = leafCur;
        if(tree->name && tree->name[n])
            paintTreeLabel(painter, (time[n]-tmin)*scale+xoffset+labelSep, yoffset+y, tree->name[n], "cl");
    }
    paintEdge(painter, (time[n]-tmin)*scale+xoffset, yoffset+y, (time[parent]-tmin)*scale+xoffset, yoffset+y);
     return y;
}

void QTreeDrawer::paintEdge(QPainter *painter, double x1, double y1, double x2, double y2) {
    painter->save();
    QPen pen;
    pen.setColor(QColor(0, 0, 0, 255));
    pen.setWidth(penWidth);
    painter->setPen(pen);
    painter->drawLine(QPointF(x1,y1), QPointF(x2,y2));
    painter->restore();
}

void QTreeDrawer::paintTreeLabel(QPainter *painter, double x, double y, char *text, const char *al) {
    double xs, ys;
    if(text == NULL)
        return;
    QString st = QString(text);
    painter->save();
    painter->setFont(font);
    if(al != NULL && strlen(al)>0) {
        switch(al[0]) {
            case 'B':
                ys = y+painter->fontMetrics().descent();
                break;
            case 'b':
                ys = y;
                break;
            case 't':
                ys = y+painter->fontMetrics().descent()+QFontMetrics(font).ascent();
                break;
            case 'c':
            default:
                ys = y+painter->fontMetrics().ascent()/4.;
        }
        switch(al[1]) {
            case 'l':
                xs = x;
                break;
            case 'r':
                xs = x-painter->fontMetrics().width(st);
                break;
            case 'c':
            default:
                xs = x-painter->fontMetrics().width(st)/2.;
        }
    }
    painter->setPen(QColor(50,50,50,255));
    painter->drawText(QPointF(xs,ys), st);
    painter->restore();
}

