#include "contextdialog.h"
#include "ui_contextdialog.h"

ContextDialog::ContextDialog(QString id,QString description,int loglevel, int tracestatus, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ContextDialog)
{
    ui->setupUi(this);

    ui->idLineEdit->setText(id);
    ui->descriptionLineEdit->setText(description);
    ui->loglevelComboBox->setCurrentIndex(loglevel+1);
    ui->tracestatusComboBox->setCurrentIndex(tracestatus+1);

}

ContextDialog::~ContextDialog()
{
    delete ui;
}

QString ContextDialog::id()
{
   return  ui->idLineEdit->text();
}

QString ContextDialog::description()
{
    return  ui->descriptionLineEdit->text();
}

int ContextDialog::loglevel()
{
    return  ui->loglevelComboBox->currentIndex()-1;
}

int ContextDialog::tracestatus()
{
    return  ui->tracestatusComboBox->currentIndex()-1;
}

int ContextDialog::update()
{
    return  ui->checkBoxUpdate->isChecked();
}
