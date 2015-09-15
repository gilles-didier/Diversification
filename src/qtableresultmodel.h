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
