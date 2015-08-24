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




#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cfloat>
#include "qtableresultmodel.h"

#define NOTSET -1.
#define EPS 0.001

QTableResultModel::QTableResultModel(int r, int c, QObject *parent):QAbstractTableModel(parent)
{
	row = r;
	col = c;
	maxDigits = 9;
	table = new double*[row];
	for(int i=0; i<row; i++) {
		table[i] = new double[col];
		for(int j=0; j<col; j++)
			table[i][j] = NOTSET;
	}
	isInt = new bool*[row];
	for(int i=0; i<row; i++) {
		isInt[i] = new bool[col];
		for(int j=0; j<col; j++)
			isInt[i][j] = false;
	}
	headerRow = new QString[row];
	for(int i=0; i<row; i++)
		headerRow[i] = QString();
	headerCol = new QString[col];
	for(int i=0; i<col; i++)
		headerCol[i] = QString();
}

QTableResultModel::~QTableResultModel()
{
	for(int i=0; i<row; i++)
		delete[] table[i];
	delete[] table;
	for(int i=0; i<row; i++)
		delete[] isInt[i];
	delete[] isInt;
	delete[] headerRow;
	delete[] headerCol;
}


int QTableResultModel::rowCount(const QModelIndex & /*parent*/) const
{
   return row;
}

int QTableResultModel::columnCount(const QModelIndex & /*parent*/) const
{
	return col;
}

void QTableResultModel::setEntry(const QModelIndex &index, double val, bool isi) const
{
	if(index.row()<row && index.row()>=0 && index.column()<col && index.column()>=0)
		isInt[index.row()][index.column()] = isi;
	if(index.row()<row && index.row()>=0 && index.column()<col && index.column()>=0)
		table[index.row()][index.column()] = val;
}

void QTableResultModel::setEntry(const QModelIndex &index, double val) const
{
	if(index.row()<row && index.row()>=0 && index.column()<col && index.column()>=0)
		table[index.row()][index.column()] = val;
}

void QTableResultModel::setInt(const QModelIndex &index, bool isi) const
{
	if(index.row()<row && index.row()>=0 && index.column()<col && index.column()>=0)
		isInt[index.row()][index.column()] = isi;
}

QVariant QTableResultModel::data(const QModelIndex &index, int role) const
{
	switch(role){
		case Qt::DisplayRole:
			if(index.row()<row && index.row()>=0 && index.column()<col && index.column()>=0) {
				char buffer[50];
				double v = table[index.row()][index.column()], lv = log10(fabs(v));
				if(v != NOTSET) {
					if(isInt[index.row()][index.column()]) {
						if(lv<maxDigits)
							sprintf(buffer, "%.0lf", v);
						else
							sprintf(buffer, "%.2lE", v);
					} else {
						if(v == 0. || (lv<maxDigits && lv>0))
							sprintf(buffer, "%.2lf", v);
						else
							sprintf(buffer, "%.2lE", v);
					}
					return QString(buffer);
				}
			}
			break;
		case Qt::TextAlignmentRole:
			return Qt::AlignRight + Qt::AlignVCenter;
	}
	return QVariant();
}
void QTableResultModel::setHeaderRow(int section, QString s) const
{
	headerRow[section] = s;
}

void QTableResultModel::setHeaderColumn(int section, QString s) const
{
	headerCol[section] = s;
}

QVariant QTableResultModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Vertical) {
			return headerRow[section];
		}
		if (orientation == Qt::Horizontal) {
			return headerCol[section];
		}
	}
	return QVariant();
}
