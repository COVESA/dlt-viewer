#include <QSettings>
#include <QFileDialog>
#include <qmessagebox.h>

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::changeEvent(QEvent *e)
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

void SettingsDialog::writeDlg()
{
    /* startup */
    ui->checkBoxDefaultProjectFile->setCheckState(defaultProjectFile?Qt::Checked:Qt::Unchecked);
    ui->lineEditDefaultProjectFile->setText(defaultProjectFileName);
    ui->checkBoxDefaultLogFile->setCheckState(defaultLogFile?Qt::Checked:Qt::Unchecked);
    ui->lineEditDefaultLogFile->setText(defaultLogFileName);
    ui->checkBoxPluginsPath->setCheckState(pluginsPath?Qt::Checked:Qt::Unchecked);
    ui->lineEditPluginsPath->setText(pluginsPathName);
    ui->checkBoxAutoConnect->setCheckState(autoConnect?Qt::Checked:Qt::Unchecked);
    ui->checkBoxAutoScroll->setCheckState(autoScroll?Qt::Checked:Qt::Unchecked);

    /* table */
    ui->checkBoxIndex->setCheckState(showIndex?Qt::Checked:Qt::Unchecked);
    ui->checkBoxTime->setCheckState(showTime?Qt::Checked:Qt::Unchecked);
    ui->checkBoxTimestamp->setCheckState(showTimestamp?Qt::Checked:Qt::Unchecked);
    ui->checkBoxCount->setCheckState(showCount?Qt::Checked:Qt::Unchecked);

    ui->checkBoxEcuid->setCheckState(showEcuId?Qt::Checked:Qt::Unchecked);
    ui->checkBoxAppid->setCheckState(showApId?Qt::Checked:Qt::Unchecked);
    ui->checkBoxConId->setCheckState(showCtId?Qt::Checked:Qt::Unchecked);
    ui->checkBoxType->setCheckState(showType?Qt::Checked:Qt::Unchecked);

    ui->checkBoxSubtype->setCheckState(showSubtype?Qt::Checked:Qt::Unchecked);
    ui->checkBoxMode->setCheckState(showMode?Qt::Checked:Qt::Unchecked);
    ui->checkBoxNoar->setCheckState(showNoar?Qt::Checked:Qt::Unchecked);
    ui->checkBoxPayload->setCheckState(showPayload?Qt::Checked:Qt::Unchecked);

    /* view */
    ui->checkBoxShowHeader->setCheckState(showHeader?Qt::Checked:Qt::Unchecked);
    ui->checkBoxShowPayloadAscii->setCheckState(showPayloadAscii?Qt::Checked:Qt::Unchecked);
    ui->checkBoxShowPayloadHex->setCheckState(showPayloadHex?Qt::Checked:Qt::Unchecked);
    ui->checkBoxShowPayloadMixed->setCheckState(showPayloadMixed?Qt::Checked:Qt::Unchecked);

    /* other */
    ui->checkBoxWriteControl->setCheckState(writeControl?Qt::Checked:Qt::Unchecked);
}

void SettingsDialog::readDlg()
{
    /* startup */
    defaultProjectFile = (ui->checkBoxDefaultProjectFile->checkState() == Qt::Checked);
    defaultProjectFileName = ui->lineEditDefaultProjectFile->text();
    defaultLogFile = (ui->checkBoxDefaultLogFile->checkState() == Qt::Checked);
    defaultLogFileName = ui->lineEditDefaultLogFile->text();
    pluginsPath = (ui->checkBoxPluginsPath->checkState() == Qt::Checked);
    pluginsPathName = ui->lineEditPluginsPath->text();
    autoConnect = (ui->checkBoxAutoConnect->checkState() == Qt::Checked);
    autoScroll = (ui->checkBoxAutoScroll->checkState() == Qt::Checked);

    /* table */
    showIndex =     ( ui->checkBoxIndex->checkState() == Qt::Checked);
    showTime =      ( ui->checkBoxTime->checkState() == Qt::Checked);
    showTimestamp = ( ui->checkBoxTimestamp->checkState() == Qt::Checked);
    showCount =     ( ui->checkBoxCount->checkState() == Qt::Checked);

    showEcuId =     ( ui->checkBoxEcuid->checkState() == Qt::Checked);
    showApId =      ( ui->checkBoxAppid->checkState() == Qt::Checked);
    showCtId =      ( ui->checkBoxConId->checkState() == Qt::Checked);
    showType =      ( ui->checkBoxType->checkState() == Qt::Checked);

    showSubtype = ( ui->checkBoxSubtype->checkState() == Qt::Checked);
    showMode = ( ui->checkBoxMode->checkState() == Qt::Checked);
    showNoar = ( ui->checkBoxNoar->checkState() == Qt::Checked);
    showPayload = ( ui->checkBoxPayload->checkState() == Qt::Checked);

    /* view */
    showHeader = (ui->checkBoxShowHeader->checkState() == Qt::Checked);
    showPayloadAscii = (ui->checkBoxShowPayloadAscii->checkState() == Qt::Checked);
    showPayloadHex = (ui->checkBoxShowPayloadHex->checkState() == Qt::Checked);
    showPayloadMixed = (ui->checkBoxShowPayloadMixed->checkState() == Qt::Checked);

    /* other */
    writeControl = (ui->checkBoxWriteControl->checkState() == Qt::Checked);

}

void SettingsDialog::writeSettings()
{
    QSettings settings("BMW","DLT Viewer");

    /* startup */
    settings.setValue("startup/defaultProjectFile",defaultProjectFile);
    settings.setValue("startup/defaultProjectFileName",defaultProjectFileName);
    settings.setValue("startup/defaultLogFile",defaultLogFile);
    settings.setValue("startup/defaultLogFileName",defaultLogFileName);
    settings.setValue("startup/pluginsPath",pluginsPath);
    settings.setValue("startup/pluginsPathName",pluginsPathName);
    settings.setValue("startup/autoConnect",autoConnect);
    settings.setValue("startup/autoScroll",autoScroll);

    /* table */
    settings.setValue("startup/showIndex",showIndex);
    settings.setValue("startup/showTime",showTime);
    settings.setValue("startup/showTimestamp",showTimestamp);
    settings.setValue("startup/showCount",showCount);

    settings.setValue("startup/showEcuId",showEcuId);
    settings.setValue("startup/showApId",showApId);
    settings.setValue("startup/showCtId",showCtId);
    settings.setValue("startup/showType",showType);

    settings.setValue("startup/showSubtype",showSubtype);
    settings.setValue("startup/showMode",showMode);
    settings.setValue("startup/showNoar",showNoar);
    settings.setValue("startup/showPayload",showPayload);

    /* view */
    settings.setValue("startup/showHeader",showHeader);
    settings.setValue("startup/showPayloadAscii",showPayloadAscii);
    settings.setValue("startup/showPayloadHex",showPayloadHex);
    settings.setValue("startup/showPayloadMixed",showPayloadMixed);

    /* other */
    settings.setValue("startup/writeControl",writeControl);
}

void SettingsDialog::readSettings()
{
    QSettings settings("BMW","DLT Viewer");

    /* startup */
    defaultProjectFile = settings.value("startup/defaultProjectFile",0).toInt();
    defaultProjectFileName = settings.value("startup/defaultProjectFileName",QString("")).toString();
    defaultLogFile = settings.value("startup/defaultLogFile",0).toInt();
    defaultLogFileName = settings.value("startup/defaultLogFileName",QString("")).toString();
    pluginsPath = settings.value("startup/pluginsPath",0).toInt();
    pluginsPathName = settings.value("startup/pluginsPathName",QDir().currentPath()).toString();
    autoConnect = settings.value("startup/autoConnect",0).toInt();
    autoScroll = settings.value("startup/autoScroll",1).toInt();

    /* table */
    showIndex = settings.value("startup/showIndex",1).toInt();
    showTime = settings.value("startup/showTime",1).toInt();
    showTimestamp = settings.value("startup/showTimestamp",1).toInt();
    showCount = settings.value("startup/showCount",1).toInt();

    showEcuId = settings.value("startup/showEcuId",1).toInt();
    showApId = settings.value("startup/showApId",1).toInt();
    showCtId = settings.value("startup/showCtId",1).toInt();
    showType = settings.value("startup/showType",1).toInt();

    showSubtype = settings.value("startup/showSubtype",1).toInt();
    showMode = settings.value("startup/showMode",1).toInt();
    showNoar = settings.value("startup/showNoar",1).toInt();
    showPayload = settings.value("startup/showPayload",1).toInt();

    /* view */
    showHeader = settings.value("startup/showHeader",1).toInt();
    showPayloadAscii = settings.value("startup/showPayloadAscii",1).toInt();
    showPayloadHex = settings.value("startup/showPayloadHex",1).toInt();
    showPayloadMixed = settings.value("startup/showPayloadMixed",1).toInt();

    /* other */
    writeControl = settings.value("startup/writeControl",1).toInt();
}


void SettingsDialog::on_toolButtonDefaultLogFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open DLT Log file"), "", tr("DLT Files (*.dlt);;All files (*.*)"));

    if(fileName.isEmpty())
        return;

    ui->lineEditDefaultLogFile->setText(fileName);

}

void SettingsDialog::on_toolButtonDefaultProjectFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open DLT Project file"), "", tr("DLT Project Files (*.dlp);;All files (*.*)"));

    if(fileName.isEmpty())
        return;

    ui->lineEditDefaultProjectFile->setText(fileName);
}

void SettingsDialog::on_tooButtonPluginsPath_clicked()
{
    QString fileName = QFileDialog::getExistingDirectory(this,
        tr("Plugins directory"), pluginsPathName, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(fileName.isEmpty())
        return;

    ui->lineEditPluginsPath->setText(fileName);

    QMessageBox::warning(0, QString("DLT Viewer"),
                         QString("Plugins will only be reloaded after restart of DLT Viewer!"));


}
