#include <QtWidgets>
#include "qhelpdialog.h"

QHelpDialog::QHelpDialog(QWidget * parent) :
    QDialog(parent, Qt::Dialog)
{
    baseUrl = QUrl();
    setupUi(this);
}

QHelpDialog::~QHelpDialog()
{

}
void QHelpDialog::setBaseUrl(const QUrl &url) {
    baseUrl = url;
}

void QHelpDialog::setContent(const QString &text) {
    webView->setHtml(text, baseUrl);
}

void QHelpDialog::savePDF() {
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec()) {
        QFile::copy(":/Help.pdf", dialog.selectedFiles().at(0));
    }
}
