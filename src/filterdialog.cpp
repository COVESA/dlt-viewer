#include "filterdialog.h"
#include "ui_filterdialog.h"

FilterDialog::FilterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilterDialog)
{
    ui->setupUi(this);
}

FilterDialog::~FilterDialog()
{
    delete ui;
}

void FilterDialog::setEcuId(QString id)
{
    ui->lineEditEcuId->setText(id);
}

QString FilterDialog::getEcuId()
{
    return ui->lineEditEcuId->text();
}

void FilterDialog::setEnableEcuId(bool state)
{
    ui->checkBoxEcuId->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableEcuId()
{
    return (ui->checkBoxEcuId->checkState() == Qt::Checked);
}

void FilterDialog::setApplicationId(QString id)
{
    ui->lineEditApplicationId->setText(id);
}

QString FilterDialog::getApplicationId()
{
    return ui->lineEditApplicationId->text();
}

void FilterDialog::setEnableApplicationId(bool state)
{
    ui->checkBoxApplicationId->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableApplicationId()
{
    return (ui->checkBoxApplicationId->checkState() == Qt::Checked);
}

void FilterDialog::setContextId(QString id)
{
    ui->lineEditContextId->setText(id);
}

QString FilterDialog::getContextId()
{
    return ui->lineEditContextId->text();
}

void FilterDialog::setEnableContextId(bool state)
{
    ui->checkBoxContextId->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableContextId()
{
    return (ui->checkBoxContextId->checkState() == Qt::Checked);
}

void FilterDialog::setHeaderText(QString id)
{
    ui->lineEditHeaderText->setText(id);
}

QString FilterDialog::getHeaderText()
{
    return ui->lineEditHeaderText->text();
}

void FilterDialog::setEnableHeaderText(bool state)
{
    ui->checkBoxHeaderText->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableHeaderText()
{
    return (ui->checkBoxHeaderText->checkState() == Qt::Checked);
}

void FilterDialog::setPayloadText(QString id)
{
    ui->lineEditPayloadText->setText(id);
}

QString FilterDialog::getPayloadText()
{
    return ui->lineEditPayloadText->text();
}

void FilterDialog::setEnablePayloadText(bool state)
{
    ui->checkBoxPayloadText->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnablePayloadText()
{
    return (ui->checkBoxPayloadText->checkState() == Qt::Checked);
}

void FilterDialog::setFilterColour(int value)
{
   ui->comboBoxFilterColour->setCurrentIndex(value);
}

int FilterDialog::getFilterColour()
{
    return ui->comboBoxFilterColour->currentIndex();
}
