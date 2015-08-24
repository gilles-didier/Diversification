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




#include "ui_gsaveoptiondialog.h"
#include "gsaveoptiondialog.h"
#include <QtWidgets>

GSaveOptionDialog::GSaveOptionDialog(GSaveOption *o, QWidget * parent) : QDialog(parent),
    ui(new Ui::GSaveOptionDialog)
{
    ui->setupUi(this);
    options = o;
    if(options == NULL || options->getType().size() == 0)
        return;
    combo = new QComboBox*[options->getType().size()];
    group = new QButtonGroup(this);
    group->setExclusive(true);
    for(int i=0; i<options->getType().size(); i++) {
        QRadioButton *radio = new QRadioButton(options->getType().at(i).getName(), this);
        group->addButton(radio, i);
        ui->gridLayout->addWidget(radio, i, 0, 1, 1, Qt::AlignLeft);
        if(options->getType().at(i).getFormat()->size()>1) {
            combo[i] = new QComboBox(this);
            combo[i]->setEditable(false);
            combo[i]->addItems(*(options->getType().at(i).getFormat()));
            ui->gridLayout->addWidget(combo[i], i, 1, 1, 1, Qt::AlignLeft);
        } else
            combo[i] = NULL;
    }
    group->	button(0)->toggle();
    ui->gridLayout->setSizeConstraint(QLayout::SetFixedSize);
    this->setFixedSize(this->size());
}

GSaveOptionDialog::~GSaveOptionDialog()
{
    delete[] combo;
}

int GSaveOptionDialog::getType() {
    return group->checkedId();
}

int GSaveOptionDialog::getFormat() {
    int id = getType();
    if(id>=0) {
        if(combo[id] != NULL)
            return combo[id]->currentIndex();
        else return 0;
    } else
        return -1;
}
