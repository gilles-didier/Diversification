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




#include "ghelpdialog.h"
#include "ui_ghelpdialog.h"
#include <QFileDialog>

GHelpDialog::GHelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GHelpDialog)
{
    baseUrl = QUrl();
    ui->setupUi(this);
}

GHelpDialog::~GHelpDialog()
{
    delete ui;
}



void GHelpDialog::setBaseUrl(const QUrl &url) {
    baseUrl = url;
}

void GHelpDialog::setContent(const QString &text) {
    ui->webView->setHtml(text, baseUrl);
}

void GHelpDialog::savePDF() {
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec()) {
        QFile::copy(":/Help.pdf", dialog.selectedFiles().at(0));
    }
}
