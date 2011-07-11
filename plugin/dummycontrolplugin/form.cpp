#include "form.h"
#include "ui_form.h"
#include "dummycontrolplugin.h"

Form::Form(DummyControlPlugin *_plugin,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    plugin = _plugin;
}

Form::~Form()
{
    delete ui;
}

void Form::setConnections(QStringList list)
{
    ui->comboBoxConnections->clear();
    ui->comboBoxConnections->insertItems(0,list);
}

void Form::on_pushButton_clicked()
{
    if(plugin->dltControl)
        plugin->dltControl->sendInjection(ui->comboBoxConnections->currentIndex(),ui->lineEditApplicationId->text(),ui->lineEditContextId->text(),ui->lineEditServiceId->text().toInt(),ui->lineEditData->text().toAscii());
}
