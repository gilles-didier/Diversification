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
#include "gfossiltablemodel.h"
#include <QDebug>

#define NOTSET -1.
#define EPS 0.001

GFossilTableModel::GFossilTableModel(double mi, double ma, QObject *parent):QAbstractTableModel(parent)
{
    min = mi;
    max =ma;
    list = new QList<TypeTimeInterval>();
}

GFossilTableModel::~GFossilTableModel()
{
    delete list;
}


int GFossilTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    if(list != 0)
        return list->size();
    else
        return 0;
}

int GFossilTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 2;
}

QModelIndex GFossilTableModel::index(int row, int column, const QModelIndex & parent) const
{
   return createIndex(row, column, (quintptr) row);
}

void GFossilTableModel::add(TypeTimeInterval inter)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    list->append(inter);
    endInsertRows();
}

void GFossilTableModel::add()
{
    TypeTimeInterval inter;
    inter.inf = (min+max)/2;
    inter.sup = (min+max)/2;
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    list->append(inter);
    endInsertRows();
}

void GFossilTableModel::add(double inf, double sup)
{
    TypeTimeInterval inter;
    inter.inf = inf;
    inter.sup = sup;
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    list->append(inter);
    endInsertRows();
}

void GFossilTableModel::remove(int r)
{
    beginRemoveRows(QModelIndex(), r, r);
    list->removeAt(r);
    endRemoveRows();
}

QList<TypeTimeInterval> *GFossilTableModel::getList() {
    return list;
}

Qt::ItemFlags GFossilTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;// | Qt::ItemIsUserCheckable | Qt::ItemIsTristate | Qt::ItemIsEditable;// | Qt::ItemIsEditable;
}

QVariant GFossilTableModel::data(const QModelIndex &index, int role) const
{
    switch(role){
        case Qt::DisplayRole:
            switch(index.column())
            {
                case 0: return QVariant(list->at(index.row()).inf);
                case 1: return QVariant(list->at(index.row()).sup);
                default: return QVariant();
            }
            break;
        case Qt::EditRole:
            switch(index.column())
            {
                case 0: return QVariant(list->at(index.row()).inf);
                case 1: return QVariant(list->at(index.row()).sup);
                default: return QVariant();
            }
            break;
       case Qt::TextAlignmentRole:
            switch(index.column())
            {
                case 0: return Qt::AlignRight + Qt::AlignVCenter;
                case 1: return Qt::AlignRight + Qt::AlignVCenter;
                default: return Qt::AlignRight + Qt::AlignVCenter;
            }
        case Qt::UserRole:
             switch(index.column())
             {
                 case 0: return QVariant(min);
                 case 1: return QVariant(list->at(index.row()).inf);
                 default: return QVariant(min);
             }
        case Qt::UserRole+1:
             switch(index.column())
             {
                 case 0: return QVariant(list->at(index.row()).sup);
                 case 1: return QVariant(max);
                 default: return QVariant(max);
             }
    }
    return QVariant();
}


bool GFossilTableModel::setData(const QModelIndex & index, const QVariant & value, int role) {
    if (!index.isValid())
        return true;
    switch(index.column())
    {
        case 0:
            list->operator[](index.row()).inf = value.toDouble();
            emit dataChanged(index,index);
            return true;
        case 1:
            list->operator[](index.row()).sup = value.toDouble();
            emit dataChanged(index,index);
            return true;
        default:
            return false;
    }
    return QAbstractItemModel::setData ( index,value,role );
}


QVariant GFossilTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch(orientation) {
        case Qt::Horizontal:
            switch(role){
                case Qt::DisplayRole:
                    switch(section)
                    {
                        case 0: return QString("Bound inf");
                        case 1: return QString("Bound sup");
                        default: return QVariant();
                    }
                    break;
                default: return QVariant();
            }
        default: return QVariant();
    }
}
