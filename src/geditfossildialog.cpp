#include "geditfossildialog.h"
#include "ui_geditfossildialog.h"
#include "gfossiltablemodel.h"
#include "qspinboxdelegate.h"

GEditFossilDialog::GEditFossilDialog(int n, TypeFossilIntFeature *fos, double min, double max, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GEditFossilDialog)
{
    ui->setupUi(this);
    model = new GFossilTableModel(min, max, this);
    if(fos != NULL) {
        int f;
        for(f=fos->fossilInt[n]; f != NOSUCH; f=fos->fossilIntList[f].prec)
            model->add(fos->fossilIntList[f].fossilInt);
    }
    ui->tableView->setModel(model);
    ui->tableView->setItemDelegate(new QSpinBoxDelegate());
    ui->tableView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
}

GEditFossilDialog::~GEditFossilDialog()
{
    delete ui;
}

QList<TypeTimeInterval> *GEditFossilDialog::list()
{
    return model->getList();
}

void GEditFossilDialog::add()
{
    model->add();
}

void GEditFossilDialog::remove()
{
    QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();
    std::sort(selected.begin(), selected.end());
    for(int i=selected.size()-1; i>=0; i--)
        model->remove(selected.at(i).row());
}
