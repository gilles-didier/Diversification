#ifndef GADDFOSSILDIALOG_H
#define GADDFOSSILDIALOG_H
#include <QDialog>
#include "ui_gaddfossildialog.h"


class GAddFossilDialog : public QDialog, private Ui::GAddFossilDialog
{
     Q_OBJECT
public:
    GAddFossilDialog(double mi=0., double ma=1., QWidget * parent = 0);
    ~GAddFossilDialog();
    double inf();
    double sup();
public slots:
    void changeInf(double val);
    void changeSup(double val);
};

#endif // GADDFOSSILDIALOG_H
