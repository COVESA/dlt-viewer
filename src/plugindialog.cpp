#include <QFileDialog>

#include "plugindialog.h"
#include "ui_plugindialog.h"

PluginDialog::PluginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PluginDialog) {
    ui->setupUi(this);
}

PluginDialog::~PluginDialog() {
    delete ui;
}

void PluginDialog::setName(QString name) {
    ui->lineEditName->setText(name);
}

void PluginDialog::setPluginVersion(QString version){
    ui->lineEditPluginVersion->setText(version);
}

void PluginDialog::setPluginInterfaceVersion(QString version){
    ui->lineEditPluginInterfaceVersion->setText(version);
}

void PluginDialog::setMode(int mode) {
    ui->comboBoxMode->setCurrentIndex(mode);
}

void PluginDialog::removeMode(int mode) {
    ui->comboBoxMode->removeItem(mode);
}

void PluginDialog::setType(int type) {
    ui->comboBoxType->setCurrentIndex(type);
}

void PluginDialog::setFilename(QString filename) {
    ui->lineEditFilename->setText(filename);
}

int PluginDialog::getMode() {
    return ui->comboBoxMode->currentIndex();
}

int PluginDialog::getType() {
    return ui->comboBoxType->currentIndex();
}

QString PluginDialog::getFilename() {
    return ui->lineEditFilename->text();
}

void PluginDialog::on_toolButton_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this,
        QString("Open ")+ui->lineEditName->text()+QString(" configuration file"), workingDirectory, tr("Plugin configuration (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    ui->lineEditFilename->setText(fileName);
}
