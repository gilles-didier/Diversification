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




#include "gaddfossildialog.h"

GAddFossilDialog::GAddFossilDialog(double mi, double ma, QWidget * parent) :
    QDialog(parent, Qt::Dialog)
{
    setupUi(this);
    QMetaObject::connectSlotsByName(this);
    double avg = (mi+ma)/2.;
    doubleSpinBox_inf->setMinimum(mi);
    doubleSpinBox_inf->setMaximum(avg);
    doubleSpinBox_inf->setValue(avg);
    doubleSpinBox_sup->setMinimum(avg);
    doubleSpinBox_sup->setMaximum(ma);
    doubleSpinBox_sup->setValue(avg);
}

GAddFossilDialog::~GAddFossilDialog()
{

}

void GAddFossilDialog::changeInf(double val)
{
    doubleSpinBox_sup->setMinimum(val);
}


void GAddFossilDialog::changeSup(double val)
{
    doubleSpinBox_inf->setMaximum(val);
}

double GAddFossilDialog::inf()
{
    return doubleSpinBox_inf->value();
}

double GAddFossilDialog::sup()
{
    return doubleSpinBox_sup->value();
}

