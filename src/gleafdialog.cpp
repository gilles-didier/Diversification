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




#include "gleafdialog.h"
#include "ui_gleafdialog.h"

GLeafDialog::GLeafDialog(QString name, TypeNodeStatus status, double sup, double inf, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GLeafDialog)
{
    ui->setupUi(this);
    ui->lineEdit->setText(name);
    group = new QButtonGroup(this);
    group->setExclusive(true);
    group->addButton(ui->radioButton_contemp, 0);
    group->addButton(ui->radioButton_extinct, 1);
    group->addButton(ui->radioButton_unknown, 2);
    switch(status) {
        case contempNodeStatus:
            group->	button(0)->toggle();
            break;
        case extinctNodeStatus:
            group->	button(1)->toggle();
            break;
        case unknownNodeStatus:
            group->	button(2)->toggle();
            break;
        default:
            group->	button(0)->toggle();
    }
    ui->doubleSpinBox_inf->setValue(inf);
    ui->doubleSpinBox_sup->setValue(sup);
}

GLeafDialog::~GLeafDialog()
{
    delete ui;
}
void GLeafDialog::changeInf(double val)
{
    ui->doubleSpinBox_sup->setMinimum(val);
}


void GLeafDialog::changeSup(double val)
{
    ui->doubleSpinBox_inf->setMaximum(val);
}

QString GLeafDialog::getName()
{
    return ui->lineEdit->text();
}

void GLeafDialog::enableSpinBoxes(bool b) {
        ui->doubleSpinBox_inf->setEnabled(b);
        ui->doubleSpinBox_sup->setEnabled(b);
}

TypeNodeStatus GLeafDialog::getStatus() {
    switch(group->checkedId()) {
        case 0:
            return contempNodeStatus;
        case 1:
            return extinctNodeStatus;
        case 2:
            return unknownNodeStatus;
        default:
            return contempNodeStatus;
    }
}

double GLeafDialog::getInf() {
    return ui->doubleSpinBox_inf->value();
}

double GLeafDialog::getSup() {
    return ui->doubleSpinBox_sup->value();
}
