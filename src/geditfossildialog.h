#ifndef GEDITFOSSILDIALOG_H
#define GEDITFOSSILDIALOG_H
#include "FossilInt.h"
#include <QDialog>
#include "gfossiltablemodel.h"

namespace Ui {
class GEditFossilDialog;
}

class GEditFossilDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GEditFossilDialog(int n, TypeFossilIntFeature *fos, double min, double max, QWidget *parent = 0);
    ~GEditFossilDialog();
    QList<TypeTimeInterval> *list();
public slots:
    void add();
    void remove();
private:
    Ui::GEditFossilDialog *ui;
    GFossilTableModel *model;
};

#endif // GEDITFOSSILDIALOG_H
