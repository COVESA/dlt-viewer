#include "injectiondialog.h"
#include "ui_injectiondialog.h"

InjectionDialog::InjectionDialog(QString appid,QString conid,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InjectionDialog)
{
    ui->setupUi(this);
    ui->applicationidLineEdit->setText(appid);
    ui->contextidLineEdit->setText(conid);
}

InjectionDialog::~InjectionDialog()
{
    delete ui;
}

void InjectionDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void InjectionDialog::setApplicationId(QString text) { ui->applicationidLineEdit->setText(text); }
void InjectionDialog::setContextId(QString text) { ui->contextidLineEdit->setText(text); }
void InjectionDialog::setServiceId(QString text) { ui->serviceidLineEdit->setText(text); }
void InjectionDialog::setData(QString text) { ui->dataLineEdit->setText(text); }

QString InjectionDialog::getApplicationId() { return ui->applicationidLineEdit->text(); }
QString InjectionDialog::getContextId() { return ui->contextidLineEdit->text(); }
QString InjectionDialog::getServiceId() { return ui->serviceidLineEdit->text(); }
QString InjectionDialog::getData() { return ui->dataLineEdit->text(); }
