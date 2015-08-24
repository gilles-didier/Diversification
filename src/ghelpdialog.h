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




#ifndef GHELPDIALOG_H
#define GHELPDIALOG_H

#include <QDialog>
#include <QUrl>

namespace Ui {
class GHelpDialog;
}

class GHelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GHelpDialog(QWidget *parent = 0);
    ~GHelpDialog();
    void setContent(const QString &text);
    void setBaseUrl(const QUrl &url);
private:
    Ui::GHelpDialog *ui;
    QUrl baseUrl;
private slots:
    void savePDF();
};

#endif // GHELPDIALOG_H
