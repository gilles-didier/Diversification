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




#ifndef QTABLERESULTMODEL_H
#define QTABLERESULTMODEL_H
#include <QAbstractTableModel>


class QTableResultModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    QTableResultModel(int r=0, int c=0, QObject *parent=0);
    ~QTableResultModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void setHeaderRow(int section, QString s) const;
    void setHeaderColumn(int section, QString s) const;
    void setEntry(const QModelIndex &index, double val, bool isi) const;
    void setEntry(const QModelIndex &index, double val) const;
    void setInt(const QModelIndex &index, bool isi) const;
private:
    int row, col;
    double **table, maxDigits;
    bool **isInt;
    QString *headerRow, *headerCol;
};

#endif // QTABLERESULTMODEL_H
