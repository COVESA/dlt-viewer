#include "filterdialog.h"
#include "ui_filterdialog.h"

FilterDialog::FilterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilterDialog)
{
    ui->setupUi(this);

    connect(ui->buttonSelectColor, SIGNAL(pressed()), this, SLOT(on_buttonSelectColor_clicked()));

    ui->buttonSelectColor->setEnabled(false);
    ui->labelSelectedColor->setVisible(false);

    recentFilters = new QList<FilterItem>();
}

FilterDialog::~FilterDialog()
{
    delete ui;
}
void FilterDialog::setRecentFilters(QList<FilterItem> *rcFilters){

    recentFilters = rcFilters;

    ui->comboBoxRecentFilters->clear();
    ui->comboBoxRecentFilters->addItem("");

    if(recentFilters->isEmpty()){
         ui->comboBoxRecentFilters->setEnabled(false);

     } else {

         ui->comboBoxRecentFilters->setEnabled(true);

         for(int i=0; i < recentFilters->size();i++)
         {
             FilterItem item = recentFilters->at(i);
             ui->comboBoxRecentFilters->addItem(item.name);
         }
     }
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

void FilterDialog::setFilterColour(QColor color)
{
   QPalette palette = ui->labelSelectedColor->palette();
   palette.setColor(QPalette::Background,color);
   ui->labelSelectedColor->setPalette(palette);

}

QColor FilterDialog::getFilterColour()
{
    return ui->labelSelectedColor->palette().background().color();
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

void FilterDialog::on_buttonSelectColor_clicked()
{
    QColor selectedBackgroundColor = QColorDialog::getColor();
    if(selectedBackgroundColor.isValid())
    {
        QPalette palette = ui->labelSelectedColor->palette();
        palette.setColor(QPalette::Background,selectedBackgroundColor);
        ui->labelSelectedColor->setPalette(palette);

    }
}

void FilterDialog::on_comboBoxType_currentIndexChanged(int index){

    switch(index){
    case 0:
    case 1:
            ui->buttonSelectColor->setEnabled(false);
            ui->labelSelectedColor->setVisible(false);

            break;
    case 2:
            ui->buttonSelectColor->setEnabled(true);
            ui->labelSelectedColor->setVisible(true);
            break;
    }
}

void FilterDialog::on_comboBoxRecentFilters_currentIndexChanged(int index)
{
    if(index>0)
    {
        FilterItem item = recentFilters->at(index-1);

        switch(item.type){
        case FilterItem::positive:
                setType(FilterItem::positive);
                on_comboBoxType_currentIndexChanged(FilterItem::positive);
                break;
        case FilterItem::negative:
                setType(FilterItem::negative);
                on_comboBoxType_currentIndexChanged(FilterItem::negative);
                break;
        case FilterItem::marker:
                setFilterColour( item.filterColour);
                setType(FilterItem::marker);
                on_comboBoxType_currentIndexChanged(FilterItem::marker);
                break;
        default:
                break;
        }

        setName(item.name);
        setEcuId(item.ecuId );
        setApplicationId(item.applicationId );
        setContextId(item.contextId);
        setHeaderText(item.headerText );
        setPayloadText(item.payloadText );

        setEnableEcuId(item.enableEcuId);
        setEnableApplicationId(item.enableApplicationId );
        setEnableContextId(item.enableContextId);
        setEnableHeaderText(item.enableHeaderText );
        setEnablePayloadText(item.enablePayloadText );
        setEnableCtrlMsgs(item.enableCtrlMsgs);
        setEnableLogLevelMax(item.enableLogLevelMax);
        setEnableLogLevelMin(item.enableLogLevelMin );

        setLogLevelMax(item.logLevelMax );
        setLogLevelMin(item.logLevelMin );
    }
}
