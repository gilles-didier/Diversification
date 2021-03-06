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
