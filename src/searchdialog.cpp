#include "searchdialog.h"
#include "ui_searchdialog.h"

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{
    ui->setupUi(this);
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::setText(QString text) { ui->lineEditText->setText(text);}
void SearchDialog::selectText(){ui->lineEditText->selectAll();}
void SearchDialog::setHeader(bool header) { ui->checkBoxHeader->setCheckState(header?Qt::Checked:Qt::Unchecked);}
void SearchDialog::setPayload(bool payload) { ui->checkBoxPayload->setCheckState(payload?Qt::Checked:Qt::Unchecked);}
void SearchDialog::setCaseSensitive(bool caseSensitive) { ui->checkBoxCaseSensitive->setCheckState(caseSensitive?Qt::Checked:Qt::Unchecked);}
void SearchDialog::setRegExp(bool regExp) { ui->checkBoxRegExp->setCheckState(regExp?Qt::Checked:Qt::Unchecked);}

QString SearchDialog::getText() { return ui->lineEditText->text(); }
bool SearchDialog::getHeader() { return (ui->checkBoxHeader->checkState() == Qt::Checked); }
bool SearchDialog::getPayload() { return (ui->checkBoxPayload->checkState() == Qt::Checked); }
bool SearchDialog::getCaseSensitive() { return (ui->checkBoxCaseSensitive->checkState() == Qt::Checked); }
bool SearchDialog::getRegExp() { return (ui->checkBoxRegExp->checkState() == Qt::Checked); }

