#ifndef QHELPDIALOG_H
#define QHELPDIALOG_H
#include <QDialog>
#include "ui_qhelpdialog.h"

class QHelpDialog : public QDialog, private Ui::QHelpDialog
{
public:
    QHelpDialog(QWidget *parent = 0);
    ~QHelpDialog();
    void setContent(const QUrl &url);
private slots:
    void savePDF();
};

#endif // QHELPDIALOG_H
