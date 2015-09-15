#ifndef QHELPDIALOG_H
#define QHELPDIALOG_H
#include <QDialog>
#include "ui_qhelpdialog.h"

class QHelpDialog : public QDialog, private Ui::QHelpDialog
{
public:
    QHelpDialog(QWidget * parent = 0);
    ~QHelpDialog();
    void setContent(const QString &text);
    void setBaseUrl(const QUrl &url);
private:
    QUrl baseUrl;
private slots:
    void savePDF();
};

#endif // QHELPDIALOG_H
