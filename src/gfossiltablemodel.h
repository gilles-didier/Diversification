#ifndef GFossilTableModel_H
#define GFossilTableModel_H
#include <QAbstractTableModel>
#include "FossilInt.h"


class GFossilTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    GFossilTableModel(double mi, double ma, QObject *parent=0);
    ~GFossilTableModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void setHeaderRow(int section, QString s) const;
    void add();
    void add(double inf, double sup);
    void add(TypeTimeInterval inter);
    void remove(int r);
    QList<TypeTimeInterval> *getList();
 private:
    QList<TypeTimeInterval> *list;
    double min, max;
};

#endif // GFossilTableModel_H
