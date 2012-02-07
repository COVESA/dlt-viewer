#include <QSettings>
#include <QFileDialog>
#include <qmessagebox.h>

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "version.h"
#include "dltsettingsmanager.h"

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

void SettingsDialog::assertSettingsVersion()
{
	DltSettingsManager *settings = DltSettingsManager::instance();

	int major = settings->value("startup/versionMajor").toInt();
	int minor = settings->value("startup/versionMinor").toInt();

    if(major == 0 && minor == 0)
        return; // The settings were empty already

    if(major < QString(PACKAGE_MAJOR_VERSION).toInt() ||
       minor < QString(PACKAGE_MINOR_VERSION).toInt())
    {
        QString msg;
        msg.append("Application version has changed. Settings might be incompatible.\n");
        msg.append("Would you like to remove all old settings and initialize new ones?\n");
        msg.append("Yes    - Reset settings to factory defaults.\n");
        msg.append("No     - Continue loading settings and risk crashing the application.\n");
        msg.append("Cancel - Exit the viewer now.\n");
        QMessageBox dlg("Warning", msg, QMessageBox::Warning,
                        QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
        int btn = dlg.exec();
        if(btn == QMessageBox::Yes)
        {
            resetSettings();
        }
        else if(btn == QMessageBox::Cancel)
        {
            exit(-1);
        }
    }
}

void SettingsDialog::resetSettings()
{
	DltSettingsManager *settings = DltSettingsManager::instance();
	settings->clear();
	QString fn(settings->fileName());
	DltSettingsManager::close();
	QFile fh(fn);
    if(fh.exists())
    {
        if(!fh.open(QIODevice::ReadWrite))
            return; // Could be a registry key on windows
        fh.close();
        if(!fh.remove())
        {
            QMessageBox err("Error", "Could not remove the settings file", QMessageBox::Critical, QMessageBox::Ok, 0, 0);
            err.exec();
        }
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
    ui->checkBoxAutoMarkFatalError->setCheckState(autoMarkFatalError?Qt::Checked:Qt::Unchecked);
    ui->checkBoxAutoMarkWarn->setCheckState(autoMarkWarn?Qt::Checked:Qt::Unchecked);

    /* table */
    ui->spinBoxFontSize->setValue(fontSize);
    ui->checkBoxIndex->setCheckState(showIndex?Qt::Checked:Qt::Unchecked);
    ui->checkBoxTime->setCheckState(showTime?Qt::Checked:Qt::Unchecked);
    ui->checkBoxTimestamp->setCheckState(showTimestamp?Qt::Checked:Qt::Unchecked);
    ui->checkBoxCount->setCheckState(showCount?Qt::Checked:Qt::Unchecked);

    ui->checkBoxEcuid->setCheckState(showEcuId?Qt::Checked:Qt::Unchecked);

    ui->groupBoxAppId->setChecked(showApId?Qt::Checked:Qt::Unchecked);
    if(ui->groupBoxAppId->isChecked()){
        ui->radioButtonAppId->setEnabled(true);
        ui->radioButtonAppIdDesc->setEnabled(true);
    }else{
        ui->radioButtonAppId->setEnabled(false);
        ui->radioButtonAppIdDesc->setEnabled(false);
    }
    switch(showApIdDesc){
    case 0:
        ui->radioButtonAppId->setChecked(true);
        ui->radioButtonAppIdDesc->setChecked(false);
        break;
    case 1:
        ui->radioButtonAppId->setChecked(false);
        ui->radioButtonAppIdDesc->setChecked(true);
        break;
    default:
        ui->radioButtonConId->setChecked(true);
        ui->radioButtonConIdDesc->setChecked(false);
        break;
    }

    ui->groupBoxConId->setChecked(showCtId?Qt::Checked:Qt::Unchecked);
    if(ui->groupBoxConId->isChecked()){
        ui->radioButtonConId->setEnabled(true);
        ui->radioButtonConIdDesc->setEnabled(true);
    }else{
        ui->radioButtonConId->setEnabled(false);
        ui->radioButtonConIdDesc->setEnabled(false);
    }
    switch(showCtIdDesc){
    case 0:
        ui->radioButtonConId->setChecked(true);
        ui->radioButtonConIdDesc->setChecked(false);
        break;
    case 1:
        ui->radioButtonConId->setChecked(false);
        ui->radioButtonConIdDesc->setChecked(true);
        break;
    default:
        ui->radioButtonConId->setChecked(true);
        ui->radioButtonConIdDesc->setChecked(false);
        break;
    }

    ui->checkBoxType->setCheckState(showType?Qt::Checked:Qt::Unchecked);

    ui->checkBoxSubtype->setCheckState(showSubtype?Qt::Checked:Qt::Unchecked);
    ui->checkBoxMode->setCheckState(showMode?Qt::Checked:Qt::Unchecked);
    ui->checkBoxNoar->setCheckState(showNoar?Qt::Checked:Qt::Unchecked);
    ui->checkBoxPayload->setCheckState(showPayload?Qt::Checked:Qt::Unchecked);

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
    autoMarkFatalError = (ui->checkBoxAutoMarkFatalError->checkState() == Qt::Checked);
    autoMarkWarn = (ui->checkBoxAutoMarkWarn->checkState() == Qt::Checked);

    /* table */
    fontSize = ui->spinBoxFontSize->value();
    showIndex =     ( ui->checkBoxIndex->checkState() == Qt::Checked);
    showTime =      ( ui->checkBoxTime->checkState() == Qt::Checked);
    showTimestamp = ( ui->checkBoxTimestamp->checkState() == Qt::Checked);
    showCount =     ( ui->checkBoxCount->checkState() == Qt::Checked);

    showEcuId =     ( ui->checkBoxEcuid->checkState() == Qt::Checked);
    showApId =      ( ui->groupBoxAppId->isChecked() == true ? 1:0);
    showApIdDesc =  ( ui->radioButtonAppIdDesc->isChecked()== true ? 1:0);
    showCtId =      ( ui->groupBoxConId->isChecked() == true ? 1:0);
    showCtIdDesc =  ( ui->radioButtonConIdDesc->isChecked()== true ? 1:0);
    showType =      ( ui->checkBoxType->checkState() == Qt::Checked);

    showSubtype = ( ui->checkBoxSubtype->checkState() == Qt::Checked);
    showMode = ( ui->checkBoxMode->checkState() == Qt::Checked);
    showNoar = ( ui->checkBoxNoar->checkState() == Qt::Checked);
    showPayload = ( ui->checkBoxPayload->checkState() == Qt::Checked);

    /* other */
    writeControl = (ui->checkBoxWriteControl->checkState() == Qt::Checked);

}

void SettingsDialog::writeSettings()
{
	DltSettingsManager *settings = DltSettingsManager::instance();

    /* startup */
	settings->setValue("startup/defaultProjectFile",defaultProjectFile);
	settings->setValue("startup/defaultProjectFileName",defaultProjectFileName);
	settings->setValue("startup/defaultLogFile",defaultLogFile);
	settings->setValue("startup/defaultLogFileName",defaultLogFileName);
	settings->setValue("startup/pluginsPath",pluginsPath);
	settings->setValue("startup/pluginsPathName",pluginsPathName);
	settings->setValue("startup/autoConnect",autoConnect);
	settings->setValue("startup/autoScroll",autoScroll);
	settings->setValue("startup/autoMarkFatalError",autoMarkFatalError);
	settings->setValue("startup/autoMarkWarn",autoMarkWarn);

    /* table */
	settings->setValue("startup/fontSize",fontSize);
	settings->setValue("startup/showIndex",showIndex);
	settings->setValue("startup/showTime",showTime);
	settings->setValue("startup/showTimestamp",showTimestamp);
	settings->setValue("startup/showCount",showCount);

	settings->setValue("startup/showEcuId",showEcuId);
	settings->setValue("startup/showApId",showApId);
	settings->setValue("startup/showApIdDesc",showApIdDesc);
	settings->setValue("startup/showCtId",showCtId);
	settings->setValue("startup/showCtIdDesc",showCtIdDesc);
	settings->setValue("startup/showType",showType);

	settings->setValue("startup/showSubtype",showSubtype);
	settings->setValue("startup/showMode",showMode);
	settings->setValue("startup/showNoar",showNoar);
	settings->setValue("startup/showPayload",showPayload);

    /* other */
	settings->setValue("startup/writeControl",writeControl);

    /* For settings integrity validation */
	settings->setValue("startup/versionMajor", QString(PACKAGE_MAJOR_VERSION).toInt());
	settings->setValue("startup/versionMinor", QString(PACKAGE_MINOR_VERSION).toInt());
	settings->setValue("startup/versionPatch", QString(PACKAGE_PATCH_LEVEL).toInt());
}

void SettingsDialog::readSettings()
{
	DltSettingsManager *settings = DltSettingsManager::instance();
    /* startup */
	defaultProjectFile = settings->value("startup/defaultProjectFile",0).toInt();
	defaultProjectFileName = settings->value("startup/defaultProjectFileName",QString("")).toString();
	defaultLogFile = settings->value("startup/defaultLogFile",0).toInt();
	defaultLogFileName = settings->value("startup/defaultLogFileName",QString("")).toString();
	pluginsPath = settings->value("startup/pluginsPath",0).toInt();
	pluginsPathName = settings->value("startup/pluginsPathName",QDir().currentPath()).toString();
	autoConnect = settings->value("startup/autoConnect",0).toInt();
	autoScroll = settings->value("startup/autoScroll",1).toInt();
	autoMarkFatalError = settings->value("startup/autoMarkFatalError",0).toInt();
	autoMarkWarn = settings->value("startup/autoMarkWarn",0).toInt();

    /* table */
	fontSize = settings->value("startup/fontSize",8).toInt();
	showIndex = settings->value("startup/showIndex",1).toInt();
	showTime = settings->value("startup/showTime",1).toInt();
	showTimestamp = settings->value("startup/showTimestamp",1).toInt();
	showCount = settings->value("startup/showCount",1).toInt();

	showEcuId = settings->value("startup/showEcuId",1).toInt();
	showApId = settings->value("startup/showApId",1).toInt();
	showApIdDesc = settings->value("startup/showApIdDesc",0).toInt();
	showCtId = settings->value("startup/showCtId",1).toInt();
	showCtIdDesc = settings->value("startup/showCtIdDesc",0).toInt();
	showType = settings->value("startup/showType",1).toInt();

	showSubtype = settings->value("startup/showSubtype",1).toInt();
	showMode = settings->value("startup/showMode",1).toInt();
	showNoar = settings->value("startup/showNoar",1).toInt();
	showPayload = settings->value("startup/showPayload",1).toInt();

    /* other */
	writeControl = settings->value("startup/writeControl",1).toInt();
}


void SettingsDialog::on_toolButtonDefaultLogFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open DLT Log file"), workingDirectory, tr("DLT Files (*.dlt);;All files (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    ui->lineEditDefaultLogFile->setText(fileName);

}

void SettingsDialog::on_toolButtonDefaultProjectFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open DLT Project file"), workingDirectory, tr("DLT Project Files (*.dlp);;All files (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    ui->lineEditDefaultProjectFile->setText(fileName);
}

void SettingsDialog::on_tooButtonPluginsPath_clicked()
{
    QString fileName = QFileDialog::getExistingDirectory(this,
        tr("Plugins directory"), workingDirectory+"/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    ui->lineEditPluginsPath->setText(fileName);

    QMessageBox::warning(0, QString("DLT Viewer"),
                         QString("Plugins will only be reloaded after restart of DLT Viewer!"));

}

void SettingsDialog::on_groupBoxConId_clicked(bool checked)
{
    if(checked){
        ui->radioButtonConId->setEnabled(true);
        ui->radioButtonConIdDesc->setEnabled(true);
    }else{
        ui->radioButtonConId->setEnabled(false);
        ui->radioButtonConIdDesc->setEnabled(false);
    }
}

void SettingsDialog::on_groupBoxAppId_clicked(bool checked)
{
    if(checked){
        ui->radioButtonAppId->setEnabled(true);
        ui->radioButtonAppIdDesc->setEnabled(true);
    }else{
        ui->radioButtonAppId->setEnabled(false);
        ui->radioButtonAppIdDesc->setEnabled(false);
    }
}
