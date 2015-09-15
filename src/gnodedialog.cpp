#include "gnodedialog.h"
#include "ui_gnodedialog.h"

GNodeDialog::GNodeDialog(QString name, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GNodeDialog)
{
    ui->setupUi(this);
    ui->lineEdit->setText(name);
}

GNodeDialog::~GNodeDialog()
{
    delete ui;
}

QString GNodeDialog::getName()
{
    return ui->lineEdit->text();
}
