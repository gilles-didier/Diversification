#ifndef GSAVEOPTIONDIALOG_H
#define GSAVEOPTIONDIALOG_H

#include <QObject>
#include <QDialog>
#include <QButtonGroup>
#include <QComboBox>
#include "gsaveoption.h"

namespace Ui {
class GSaveOptionDialog;
}

class GSaveOptionDialog : public QDialog
{
    Q_OBJECT

public:
    GSaveOptionDialog(GSaveOption *o=0, QWidget * parent = 0);
    ~GSaveOptionDialog();
    int getType();
    int getFormat();
private:
    Ui::GSaveOptionDialog *ui;
    GSaveOption *options;
    QButtonGroup *group;
    QComboBox **combo;
};

#endif // GSAVEOPTIONDIALOG_H
