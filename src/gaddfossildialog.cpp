#include "gaddfossildialog.h"

GAddFossilDialog::GAddFossilDialog(double mi, double ma, QWidget * parent) :
    QDialog(parent, Qt::Dialog)
{
    setupUi(this);
    QMetaObject::connectSlotsByName(this);
    double avg = (mi+ma)/2.;
    doubleSpinBox_inf->setMinimum(mi);
    doubleSpinBox_inf->setMaximum(avg);
    doubleSpinBox_inf->setValue(avg);
    doubleSpinBox_sup->setMinimum(avg);
    doubleSpinBox_sup->setMaximum(ma);
    doubleSpinBox_sup->setValue(avg);
}

GAddFossilDialog::~GAddFossilDialog()
{

}

void GAddFossilDialog::changeInf(double val)
{
    doubleSpinBox_sup->setMinimum(val);
}


void GAddFossilDialog::changeSup(double val)
{
    doubleSpinBox_inf->setMaximum(val);
}

double GAddFossilDialog::inf()
{
    return doubleSpinBox_inf->value();
}

double GAddFossilDialog::sup()
{
    return doubleSpinBox_sup->value();
}

