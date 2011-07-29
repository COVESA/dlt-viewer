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

void FilterDialog::setType(int value)
{
    ui->comboBoxType->setCurrentIndex(value);
}

int FilterDialog::getType()
{
    return ui->comboBoxType->currentIndex();
}

void FilterDialog::setName(QString name)
{
    ui->lineEditName->setText(name);
}

QString FilterDialog::getName()
{
    return ui->lineEditName->text();
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

void FilterDialog::setLogLevelMax(int value)
{
    ui->comboBoxLogLevelMax->setCurrentIndex(value);
}

int FilterDialog::getLogLevelMax()
{
    return ui->comboBoxLogLevelMax->currentIndex();
}

void FilterDialog::setEnableLogLevelMax(bool state)
{
    ui->checkBoxLogLevelMax->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableLogLevelMax()
{
    return (ui->checkBoxLogLevelMax->checkState() == Qt::Checked);
}

void FilterDialog::setLogLevelMin(int value)
{
    ui->comboBoxLogLevelMin->setCurrentIndex(value);
}

int FilterDialog::getLogLevelMin()
{
    return ui->comboBoxLogLevelMin->currentIndex();
}

void FilterDialog::setEnableLogLevelMin(bool state)
{
    ui->checkBoxLogLevelMin->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableLogLevelMin()
{
    return (ui->checkBoxLogLevelMin->checkState() == Qt::Checked);
}

void FilterDialog::setEnableCtrlMsgs(bool state)
{
    ui->checkBoxCtrlMsgs->setCheckState(state?Qt::Checked:Qt::Unchecked);
}

bool FilterDialog::getEnableCtrlMsgs()
{
    return (ui->checkBoxCtrlMsgs->checkState() == Qt::Checked);
}
