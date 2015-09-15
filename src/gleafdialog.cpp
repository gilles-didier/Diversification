#include "gleafdialog.h"
#include "ui_gleafdialog.h"

GLeafDialog::GLeafDialog(QString name, TypeNodeStatus status, double sup, double inf, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GLeafDialog)
{
    ui->setupUi(this);
    ui->lineEdit->setText(name);
    group = new QButtonGroup(this);
    group->setExclusive(true);
    group->addButton(ui->radioButton_contemp, 0);
    group->addButton(ui->radioButton_extinct, 1);
    group->addButton(ui->radioButton_unknown, 2);
    switch(status) {
        case contempNodeStatus:
            group->	button(0)->toggle();
            break;
        case extinctNodeStatus:
            group->	button(1)->toggle();
            break;
        case unknownNodeStatus:
            group->	button(2)->toggle();
            break;
        default:
            group->	button(0)->toggle();
    }
    ui->doubleSpinBox_inf->setValue(inf);
    ui->doubleSpinBox_sup->setValue(sup);
}

GLeafDialog::~GLeafDialog()
{
    delete ui;
}
void GLeafDialog::changeInf(double val)
{
    ui->doubleSpinBox_sup->setMinimum(val);
}


void GLeafDialog::changeSup(double val)
{
    ui->doubleSpinBox_inf->setMaximum(val);
}

QString GLeafDialog::getName()
{
    return ui->lineEdit->text();
}

void GLeafDialog::enableSpinBoxes(bool b) {
        ui->doubleSpinBox_inf->setEnabled(b);
        ui->doubleSpinBox_sup->setEnabled(b);
}

TypeNodeStatus GLeafDialog::getStatus() {
    switch(group->checkedId()) {
        case 0:
            return contempNodeStatus;
        case 1:
            return extinctNodeStatus;
        case 2:
            return unknownNodeStatus;
        default:
            return contempNodeStatus;
    }
}

double GLeafDialog::getInf() {
    return ui->doubleSpinBox_inf->value();
}

double GLeafDialog::getSup() {
    return ui->doubleSpinBox_sup->value();
}
