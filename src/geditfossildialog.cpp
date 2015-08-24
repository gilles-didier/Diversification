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




#include "geditfossildialog.h"
#include "ui_geditfossildialog.h"
#include "gfossiltablemodel.h"
#include "qspinboxdelegate.h"

GEditFossilDialog::GEditFossilDialog(int n, TypeFossilIntFeature *fos, double min, double max, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GEditFossilDialog)
{
    ui->setupUi(this);
    model = new GFossilTableModel(min, max, this);
    if(fos != NULL) {
        int f;
        for(f=fos->fossilInt[n]; f != NOSUCH; f=fos->fossilIntList[f].prec)
            model->add(fos->fossilIntList[f].fossilInt);
    }
    ui->tableView->setModel(model);
    ui->tableView->setItemDelegate(new QSpinBoxDelegate());
    ui->tableView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
}

GEditFossilDialog::~GEditFossilDialog()
{
    delete ui;
}

QList<TypeTimeInterval> *GEditFossilDialog::list()
{
    return model->getList();
}

void GEditFossilDialog::add()
{
    model->add();
}

void GEditFossilDialog::remove()
{
    QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();
    std::sort(selected.begin(), selected.end());
    for(int i=selected.size()-1; i>=0; i--)
        model->remove(selected.at(i).row());
}
