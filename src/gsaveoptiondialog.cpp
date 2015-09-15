#include "ui_gsaveoptiondialog.h"
#include "gsaveoptiondialog.h"
#include <QtWidgets>

GSaveOptionDialog::GSaveOptionDialog(GSaveOption *o, QWidget * parent) : QDialog(parent),
    ui(new Ui::GSaveOptionDialog)
{
    ui->setupUi(this);
    options = o;
    if(options == NULL || options->getType().size() == 0)
        return;
    combo = new QComboBox*[options->getType().size()];
    group = new QButtonGroup(this);
    group->setExclusive(true);
    for(int i=0; i<options->getType().size(); i++) {
        QRadioButton *radio = new QRadioButton(options->getType().at(i).getName(), this);
        group->addButton(radio, i);
        ui->gridLayout->addWidget(radio, i, 0, 1, 1, Qt::AlignLeft);
        if(options->getType().at(i).getFormat()->size()>1) {
            combo[i] = new QComboBox(this);
            combo[i]->setEditable(false);
            combo[i]->addItems(*(options->getType().at(i).getFormat()));
            ui->gridLayout->addWidget(combo[i], i, 1, 1, 1, Qt::AlignLeft);
        } else
            combo[i] = NULL;
    }
    group->	button(0)->toggle();
    ui->gridLayout->setSizeConstraint(QLayout::SetFixedSize);
    this->setFixedSize(this->size());
}

GSaveOptionDialog::~GSaveOptionDialog()
{
    delete[] combo;
}

int GSaveOptionDialog::getType() {
    return group->checkedId();
}

int GSaveOptionDialog::getFormat() {
    int id = getType();
    if(id>=0) {
        if(combo[id] != NULL)
            return combo[id]->currentIndex();
        else return 0;
    } else
        return -1;
}
