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




#include <stdio.h>
#include "goptiondialog.h"
#include "ui_goptiondialog.h"

GOptionDialog::GOptionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GOptionDialog)
{
    ui->setupUi(this);
    QObject::connect(ui->doubleSpinBoxInfSpe, SIGNAL(valueChanged(double)), this, SLOT(changeInfSpe(double)));
    QObject::connect(ui->doubleSpinBoxSupSpe, SIGNAL(valueChanged(double)), this, SLOT(changeSupSpe(double)));
    QObject::connect(ui->doubleSpinBoxInfExt, SIGNAL(valueChanged(double)), this, SLOT(changeInfExt(double)));
    QObject::connect(ui->doubleSpinBoxSupExt, SIGNAL(valueChanged(double)), this, SLOT(changeSupExt(double)));
    QObject::connect(ui->doubleSpinBoxInfFos, SIGNAL(valueChanged(double)), this, SLOT(changeInfFos(double)));
    QObject::connect(ui->doubleSpinBoxSupFos, SIGNAL(valueChanged(double)), this, SLOT(changeSupFos(double)));
}

GOptionDialog::~GOptionDialog()
{
    delete ui;
}

void GOptionDialog::changeInfSpe(double val)
{
    ui->doubleSpinBoxSupSpe->setMinimum(val);
}


void GOptionDialog::changeSupSpe(double val)
{
    ui->doubleSpinBoxInfSpe->setMaximum(val);
}

void GOptionDialog::changeInfExt(double val)
{
    ui->doubleSpinBoxSupExt->setMinimum(val);
}


void GOptionDialog::changeSupExt(double val)
{
    ui->doubleSpinBoxInfExt->setMaximum(val);
}

void GOptionDialog::changeInfFos(double val)
{
    ui->doubleSpinBoxSupFos->setMinimum(val);
}


void GOptionDialog::changeSupFos(double val)
{
    ui->doubleSpinBoxInfFos->setMaximum(val);
}

TypeNLOptOption GOptionDialog::getOption() {
    TypeNLOptOption param;
    param.infSpe = ui->doubleSpinBoxInfSpe->value();
    param.supSpe = ui->doubleSpinBoxSupSpe->value();
    param.infExt = ui->doubleSpinBoxInfExt->value();
    param.supExt = ui->doubleSpinBoxSupExt->value();
    param.infFos = ui->doubleSpinBoxInfFos->value();
    param.supFos = ui->doubleSpinBoxSupFos->value();
    param.trials = ui->spinBoxTrials->value();
    param.tolOptim = ui->doubleSpinBoxTol->value();
    param.maxIter = ui->spinBoxIter->value();
    return param;
}

void GOptionDialog::setOption(TypeNLOptOption param) {
    ui->doubleSpinBoxSupSpe->setMinimum(param.infSpe);
    ui->doubleSpinBoxInfSpe->setMaximum(param.supSpe);
    ui->doubleSpinBoxSupExt->setMinimum(param.infExt);
    ui->doubleSpinBoxInfExt->setMaximum(param.supExt);
    ui->doubleSpinBoxSupFos->setMinimum(param.infFos);
    ui->doubleSpinBoxInfFos->setMaximum(param.supFos);

    ui->doubleSpinBoxInfSpe->setValue(param.infSpe);
    ui->doubleSpinBoxSupSpe->setValue(param.supSpe);
    ui->doubleSpinBoxInfExt->setValue(param.infExt);
    ui->doubleSpinBoxSupExt->setValue(param.supExt);
    ui->doubleSpinBoxInfFos->setValue(param.infFos);
    ui->doubleSpinBoxSupFos->setValue(param.supFos);
    ui->spinBoxTrials->setValue(param.trials);
    ui->doubleSpinBoxTol->setValue(param.tolOptim);
    ui->spinBoxIter->setValue(param.maxIter);



}

