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




#ifndef DIVERSIFICATIONWINDOW_H
#define DIVERSIFICATIONWINDOW_H

#include <QMainWindow>
#include "Tree.h"
#include "FossilInt.h"
#include "Model.h"
#include "qtreedrawer.h"
#include "qtreefossildrawer.h"
#include "qtreedrawwidget.h"
#include "qscaletree.h"
#include "ghelpdialog.h"
#include "gtreefossilview.h"
#include "qtableresultmodel.h"
#include "gestimater.h"
#include "MinimizeNLOpt.h"

namespace Ui {
class DiversificationWindow;
}

class DiversificationWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DiversificationWindow(QWidget *parent = 0);
    ~DiversificationWindow();

private slots:
    void openFossil();
    void openTree();
    bool saveAs();
    bool save();
    void newTree();
    bool saveImage(int type);
    void about();
    void help();
    void changeWidth();
    void compute();
    void computeThread();
    void changeTimeOrigin();
    void changeTimeOriginS();
    void changeTimeEnd();
    void changeTimeEndS();
    void changeTree();
    void setRates(TypeEstimation mean, TypeEstimation std);
    void stopCompute();
    void getOptimOptions();
    void restoreCompute();
private:
    Ui::DiversificationWindow *ui;
    TypeTree *tree;
    TypeFossilIntFeature *fossilInt;
    TypeEstimation estimationMean, estimationStd;
    GTreeFossilView *treeView;
    QTreeFossilDrawer *treeDrawer;
    QTreeDrawWidget *treeWidget;
    QScaleTree *scaleTree;
    GHelpDialog *helpDialog;
    QTableResultModel *model;
    TypeNLOptOption nloptOption;
    GEstimater *estimater;
    bool newed;
    bool maybeSave();
    void loadTree (QString inputTreeFileName);
    void loadFossil (QString input_file_name);
    void warning(QString title, QString text);
    bool saveReport();
    bool saveTable(int type);
    void writePDF(QString filename);
    void writePNG(QString filename);
    void writeSVG(QString filename);
    void writePSTricks(QString filename);
};

#endif // DIVERSIFICATIONWINDOW_H
