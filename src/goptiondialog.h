#ifndef GOPTIONDIALOG_H
#define GOPTIONDIALOG_H

#include <QDialog>
#include "MinimizeNLOpt.h"

namespace Ui {
class GOptionDialog;
}

class GOptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GOptionDialog(QWidget *parent = 0);
    ~GOptionDialog();
    TypeNLOptOption getOption();
    void setOption(TypeNLOptOption param);
public slots:
    void openOption();
    void saveOption();
    void changeInfSpe(double val);
    void changeSupSpe(double val);
    void changeInfExt(double val);
    void changeSupExt(double val);
    void changeInfFos(double val);
    void changeSupFos(double val);
private:
    Ui::GOptionDialog *ui;
};

#endif // GOPTIONDIALOG_H
