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




#ifndef QTREEDRAWER_H
#define QTREEDRAWER_H
#include <QPainter>

#include "Tree.h"

class QTreeDrawer
{
public:
    QTreeDrawer(TypeTree *tree = NULL);
    ~QTreeDrawer();
    void setMinTime(double mint);
    void setMaxTime(double maxt);
    void setTree(TypeTree *tree);
//    void setFeature(TypeStandardFeature *std);
    void setWidth(int width);
    void setFont(QFont f);
    virtual double paintTree(QPainter *painter, int x0=0, int y0=0);
    double paintTreeWidth(QPainter *painter, int x0=0, int y0=0, int w = 500);
    int getWidth();
    int getHeight();
    int getScaleWidth();
    int getScaleHeight();
    void setMetrics();
    void setMetrics(QPainter *painter, int h);
    double drawScaleGenericUp(QPainter *painter, double x, double y);
    double drawScaleGenericDown(QPainter *painter, double x, double y);
protected:
    int leafSep, leafCur, labelSep, radius, xoffset, yoffset, height, width, labelWidth, tickLength, penWidth;
    double scale;
    double tmin, tmax, *time;
    TypeTree *tree;
    QFont font;
    void init();
    void fillTime(int n, double tanc, double *min, double *max, int *dmax);
    virtual void fillBounds(int n, double tmin, double tmax, double *min, double *max, int *dmax);
    void fillUnknownTimes(double tmin, double tmax);
    virtual double paintNode(QPainter *painter, int n, int parent);
    void paintEdge(QPainter *painter, double x1, double y1, double x2, double y2);
    void paintTreeLabel(QPainter *painter, double x, double y, char *text, const char *al);
    void computeLabelWidth();
    void computeLabelWidth(QPainter *painter);
    void computeScale();
    void computeTime();
};

#endif // QTREEDRAWER_H
