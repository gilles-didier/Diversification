#ifndef GNODEDIALOG_H
#define GNODEDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class GNodeDialog;
}

class GNodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GNodeDialog(QString name = 0, QWidget *parent = 0);
    ~GNodeDialog();
    QString getName();

private:
    Ui::GNodeDialog *ui;
};

#endif // GNODEDIALOG_H
