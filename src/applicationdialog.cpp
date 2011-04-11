#include "applicationdialog.h"
#include "ui_applicationdialog.h"

ApplicationDialog::ApplicationDialog(QString id,QString description,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ApplicationDialog)
{
    ui->setupUi(this);

    ui->idLineEdit->setText(id);
    ui->descriptionLineEdit->setText(description);
}

ApplicationDialog::~ApplicationDialog()
{
    delete ui;
}

QString ApplicationDialog::id()
{
   return  ui->idLineEdit->text();
}

QString ApplicationDialog::description()
{
    return  ui->descriptionLineEdit->text();
}
