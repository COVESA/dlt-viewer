#include "multiplecontextdialog.h"
#include "ui_multiplecontextdialog.h"

MultipleContextDialog::MultipleContextDialog(int loglevel, int tracestatus, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MultipleContextDialog)
{
    ui->setupUi(this);
    ui->loglevelComboBox->setCurrentIndex(loglevel);
    ui->tracestatusComboBox->setCurrentIndex(tracestatus);
}

MultipleContextDialog::~MultipleContextDialog()
{
    delete ui;
}

int MultipleContextDialog::loglevel()
{
    return  ui->loglevelComboBox->currentIndex()-1;
}

int MultipleContextDialog::tracestatus()
{
    return  ui->tracestatusComboBox->currentIndex()-1;
}

int MultipleContextDialog::update()
{
    return  ui->updateCheckBox->isChecked();
}
