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




#ifndef GTREEGEOMETRY_H
#define GTREEGEOMETRY_H

#include <QPointF>
#include "Tree.h"

class GTreeGeometry
{
public:
    GTreeGeometry(TypeTree *tree = NULL, double min = 0., double max = 1.);
    ~GTreeGeometry();
    void setMinTime(double mint);
    void setMaxTime(double maxt);
    void setMinMaxTime(double min, double max);
    void setTree(TypeTree *tree);
    void setLeafSep(int l);
    double fillTreeGeometry(int x0, int y0, int w, int l);
    QPointF endNode(int n);
    double *infTime();
    double *supTime();
    double getScale();
    double infTime(int n);
    double supTime(int n);
    double *getTimeTable();
protected:
    int leafSep, leafCur, xoffset, yoffset, width;
    double scale, minTime, maxTime, *time, *inf, *sup;
    TypeTree *tree;
    QPointF *start;
    void fillTime(int n, double tanc, double *min, double *max, int *dmax);
    virtual void fillBounds(int n, double tmin, double tmax, double *min, double *max, int *dmax);
    virtual double fillNodeGeometry(int n, int parent);
    virtual void fillUnknownTimes();
    void computeTime();
};

#endif // GTREEGEOMETRY_H
