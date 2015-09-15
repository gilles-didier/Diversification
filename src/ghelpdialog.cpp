#include "ghelpdialog.h"
#include "ui_ghelpdialog.h"
#include <QFileDialog>

GHelpDialog::GHelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GHelpDialog)
{
    baseUrl = QUrl();
    ui->setupUi(this);
}

GHelpDialog::~GHelpDialog()
{
    delete ui;
}



void GHelpDialog::setBaseUrl(const QUrl &url) {
    baseUrl = url;
}

void GHelpDialog::setContent(const QString &text) {
    ui->webView->setHtml(text, baseUrl);
}

void GHelpDialog::savePDF() {
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec()) {
        QFile::copy(":/Help.pdf", dialog.selectedFiles().at(0));
    }
}
