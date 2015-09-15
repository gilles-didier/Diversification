#ifndef GLEAFDIALOG_H
#define GLEAFDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include "Fossil.h"

namespace Ui {
class GLeafDialog;
}

class GLeafDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GLeafDialog(QString name=0, TypeNodeStatus status=contempNodeStatus, double inf=0., double sup=0., QWidget *parent = 0);
    ~GLeafDialog();
    QString getName();
    TypeNodeStatus getStatus();
    double getInf();
    double getSup();
private slots:
    void enableSpinBoxes(bool b);
    void changeInf(double val);
    void changeSup(double val);
private:
    Ui::GLeafDialog *ui;
    QButtonGroup *group;
};

#endif // GLEAFDIALOG_H
