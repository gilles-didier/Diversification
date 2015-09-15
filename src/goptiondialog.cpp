#include <stdio.h>
#include <QFileDialog>
#include "goptiondialog.h"
#include "ui_goptiondialog.h"

GOptionDialog::GOptionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GOptionDialog)
{
    ui->setupUi(this);
    QObject::connect(ui->doubleSpinBoxInfSpe, SIGNAL(valueChanged(double)), this, SLOT(changeInfSpe(double)));
    QObject::connect(ui->doubleSpinBoxSupSpe, SIGNAL(valueChanged(double)), this, SLOT(changeSupSpe(double)));
    QObject::connect(ui->doubleSpinBoxInfExt, SIGNAL(valueChanged(double)), this, SLOT(changeInfExt(double)));
    QObject::connect(ui->doubleSpinBoxSupExt, SIGNAL(valueChanged(double)), this, SLOT(changeSupExt(double)));
    QObject::connect(ui->doubleSpinBoxInfFos, SIGNAL(valueChanged(double)), this, SLOT(changeInfFos(double)));
    QObject::connect(ui->doubleSpinBoxSupFos, SIGNAL(valueChanged(double)), this, SLOT(changeSupFos(double)));
}

GOptionDialog::~GOptionDialog()
{
    delete ui;
}

void GOptionDialog::openOption()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        FILE *fi;
        if((fi = fopen(qPrintable(fileName), "r"))) {
            TypeNLOptOption option = getOption();
            setlocale(LC_ALL, "C");
            fscanNLoptOptionTag(fi, &option);
            fclose(fi);
            setOption(option);
        }
    }
}

void GOptionDialog::saveOption()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList files;
    if (dialog.exec()) {
        FILE *f;
        if((f = fopen(qPrintable(dialog.selectedFiles().at(0)), "w"))) {
            TypeNLOptOption option = getOption();
            setlocale(LC_ALL, "C");
            fprintNLoptOptionTag(f, &option);
            fclose(f);
        }
    }
}


void GOptionDialog::changeInfSpe(double val)
{
    ui->doubleSpinBoxSupSpe->setMinimum(val);
}


void GOptionDialog::changeSupSpe(double val)
{
    ui->doubleSpinBoxInfSpe->setMaximum(val);
}

void GOptionDialog::changeInfExt(double val)
{
    ui->doubleSpinBoxSupExt->setMinimum(val);
}


void GOptionDialog::changeSupExt(double val)
{
    ui->doubleSpinBoxInfExt->setMaximum(val);
}

void GOptionDialog::changeInfFos(double val)
{
    ui->doubleSpinBoxSupFos->setMinimum(val);
}


void GOptionDialog::changeSupFos(double val)
{
    ui->doubleSpinBoxInfFos->setMaximum(val);
}

TypeNLOptOption GOptionDialog::getOption() {
    TypeNLOptOption param;
    param.infSpe = ui->doubleSpinBoxInfSpe->value();
    param.supSpe = ui->doubleSpinBoxSupSpe->value();
    param.infExt = ui->doubleSpinBoxInfExt->value();
    param.supExt = ui->doubleSpinBoxSupExt->value();
    param.infFos = ui->doubleSpinBoxInfFos->value();
    param.supFos = ui->doubleSpinBoxSupFos->value();
    param.trials = ui->spinBoxTrials->value();
    param.tolOptim = ui->doubleSpinBoxTol->value();
    param.maxIter = ui->spinBoxIter->value();
    return param;
}

void GOptionDialog::setOption(TypeNLOptOption param) {
    ui->doubleSpinBoxSupSpe->setMinimum(param.infSpe);
    ui->doubleSpinBoxInfSpe->setMaximum(param.supSpe);
    ui->doubleSpinBoxSupExt->setMinimum(param.infExt);
    ui->doubleSpinBoxInfExt->setMaximum(param.supExt);
    ui->doubleSpinBoxSupFos->setMinimum(param.infFos);
    ui->doubleSpinBoxInfFos->setMaximum(param.supFos);

    ui->doubleSpinBoxInfSpe->setValue(param.infSpe);
    ui->doubleSpinBoxSupSpe->setValue(param.supSpe);
    ui->doubleSpinBoxInfExt->setValue(param.infExt);
    ui->doubleSpinBoxSupExt->setValue(param.supExt);
    ui->doubleSpinBoxInfFos->setValue(param.infFos);
    ui->doubleSpinBoxSupFos->setValue(param.supFos);
    ui->spinBoxTrials->setValue(param.trials);
    ui->doubleSpinBoxTol->setValue(param.tolOptim);
    ui->spinBoxIter->setValue(param.maxIter);
}

