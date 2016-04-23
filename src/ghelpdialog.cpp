#include "ghelpdialog.h"
#include "ui_ghelpdialog.h"
#include <QFileDialog>

GHelpDialog::GHelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GHelpDialog)
{
    ui->setupUi(this);
}

GHelpDialog::~GHelpDialog()
{
    delete ui;
}


void GHelpDialog::setContent(const QUrl &url) {
    ui->textBrowser->setSource(url);
    ui->textBrowser->show();
}

void GHelpDialog::savePDF() {
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec()) {
        QFile::copy(":/Help.pdf", dialog.selectedFiles().at(0));
    }
}
