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




#ifndef GESTIMATER_H
#define GESTIMATER_H
#include <QThread>
#include "Tree.h"
#include "FossilInt.h"
#include "MinimizeNLOpt.h"


class GEstimater : public QObject
{
    Q_OBJECT
public:
    enum TypeEstimaterState {
        waiting = 0,
        busy,
        stopping
    };

    GEstimater(QObject *parent=NULL);
    ~GEstimater();
    void start(TypeLikelihoodTreeFosFunction *l, TypeTree *t=NULL, TypeFossilIntFeature *f=NULL, TypeNLOptOption *option=NULL, int it = 2);
    void stop();
public slots:
    void handleResult(TypeEstimation estim);
signals:
    void newEstimation(TypeEstimation mean, TypeEstimation std);
    void setProgress(int value);
    void estimationCompleted();
private:
    void update();
    void launchThread();

    enum TypeEstimaterState state;
    TypeTree *tree;
    TypeFossilIntFeature *fos;
    TypeNLOptOption *options;
    double tmin, tmax;
    int nIter, nComp, maxThread, nThread;
    TypeLikelihoodTreeFosFunction *likelihood;
    void warning(QString title, QString text);
    TypeEstimation *estimation;
};

#endif // GESTIMATER_H
