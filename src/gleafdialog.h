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




#ifndef GLEAFDIALOG_H
#define GLEAFDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include "Fossil.h"

namespace Ui {
class GLeafDialog;
}

class GLeafDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GLeafDialog(QString name=0, TypeNodeStatus status=contempNodeStatus, double inf=0., double sup=0., QWidget *parent = 0);
    ~GLeafDialog();
    QString getName();
    TypeNodeStatus getStatus();
    double getInf();
    double getSup();
private slots:
    void enableSpinBoxes(bool b);
    void changeInf(double val);
    void changeSup(double val);
private:
    Ui::GLeafDialog *ui;
    QButtonGroup *group;
};

#endif // GLEAFDIALOG_H