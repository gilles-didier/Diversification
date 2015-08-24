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




#ifndef QTREEFOSSILDRAWER_H
#define QTREEFOSSILDRAWER_H
#include <QPainter>

#include "qtreedrawer.h"
#include "FossilInt.h"

class QTreeFossilDrawer : public QTreeDrawer
{
public:
    QTreeFossilDrawer(TypeFossilIntFeature *fos = NULL, TypeTree *tree = NULL, double min=0., double max=1.);
    ~QTreeFossilDrawer();
    void setFossil(TypeFossilIntFeature *fos);
    double paintTree(QPainter *painter, int x0, int y0);
    double paintTreeWidth(QPainter *painter, int x0, int y0, int w);
protected:
    TypeFossilIntFeature *fos;
    void paintFossil(QPainter *painter, int n, double y);
    void paintLineDot(QPainter *painter, double x1, double y1, double x2, double y2);
    void fillBounds(int n, double tmin, double tmax, double *min, double *max, int *dmax);
    double paintNode(QPainter *painter, int n, int parent);
    void computeTime();
};

#endif // QTREEFOSSILDRAWER_H
