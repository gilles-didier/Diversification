#ifndef GHELPDIALOG_H
#define GHELPDIALOG_H

#include <QDialog>
#include <QUrl>

namespace Ui {
class GHelpDialog;
}

class GHelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GHelpDialog(QWidget *parent = 0);
    ~GHelpDialog();
    void setContent(const QUrl &url);
private:
    Ui::GHelpDialog *ui;
private slots:
    void savePDF();
};

#endif // GHELPDIALOG_H
