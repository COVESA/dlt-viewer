#include "ecudialog.h"
#include "ui_ecudialog.h"

EcuDialog::EcuDialog(QString id,QString description,int interface,QString hostname,unsigned int tcpport,QString port,int baudrate,
                     int loglevel, int tracestatus,int verbosemode, bool sendSerialHeaderTcp, bool sendSerialHeaderSerial,bool syncSerialHeaderTcp, bool syncSerialHeaderSerial,
                     bool timingPackets, bool sendGetLogInfo, bool update, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EcuDialog)
{
    ui->setupUi(this);

    ui->idLineEdit->setText(id);
    ui->descriptionLineEdit->setText(description);
    ui->comboBoxInterface->setCurrentIndex(interface);
    ui->comboBoxHostname->setEditText(hostname);
    ui->lineEditTcpPort->setText(QString("%1").arg(tcpport));
    ui->comboBoxPort->setEditText(port);
    ui->comboBoxBaudrate->setCurrentIndex(baudrate);
    ui->loglevelComboBox->setCurrentIndex(loglevel);
    ui->tracestatusComboBox->setCurrentIndex(tracestatus);
    ui->comboBoxVerboseMode->setCurrentIndex(verbosemode);
    ui->checkBoxSendSerialHeaderSerial->setCheckState(sendSerialHeaderSerial?Qt::Checked:Qt::Unchecked);
    ui->checkBoxSendSerialHeaderTcp->setCheckState(sendSerialHeaderTcp?Qt::Checked:Qt::Unchecked);
    ui->checkBoxSyncToSerialHeaderSerial->setCheckState(syncSerialHeaderSerial?Qt::Checked:Qt::Unchecked);
    ui->checkBoxSyncToSerialHeaderTcp->setCheckState(syncSerialHeaderTcp?Qt::Checked:Qt::Unchecked);   
    ui->checkBoxTiming->setCheckState(timingPackets?Qt::Checked:Qt::Unchecked);

    ui->checkBoxGetLogInfo->setCheckState(sendGetLogInfo?Qt::Checked:Qt::Unchecked);

    ui->checkBoxUpdate->setCheckState(update?Qt::Checked:Qt::Unchecked);
}

EcuDialog::~EcuDialog()
{
    delete ui;
}

void EcuDialog::changeEvent(QEvent *e)
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

QString EcuDialog::id()
{
   return  ui->idLineEdit->text();
}

QString EcuDialog::description()
{
    return  ui->descriptionLineEdit->text();
}

int EcuDialog::interfacetype()
{
    return  ui->comboBoxInterface->currentIndex();
}

QString EcuDialog::hostname()
{
    return  ui->comboBoxHostname->currentText();
}

unsigned int EcuDialog::tcpport()
{
    return  ui->lineEditTcpPort->text().toUInt();
}

QString EcuDialog::port()
{
    QRegExp rx("^(com|COM)\\d\\d*$");    // matches from COM0 to COM99
    if(ui->comboBoxPort->currentText().contains(rx))
    {
        QString str = "\\\\.\\";
        return str.append(ui->comboBoxPort->currentText());
    }
    else
    {
        return  ui->comboBoxPort->currentText();
    }
}

int EcuDialog::baudrate()
{
    return  ui->comboBoxBaudrate->currentIndex();
}

int EcuDialog::loglevel()
{
    return  ui->loglevelComboBox->currentIndex();
}

int EcuDialog::tracestatus()
{
    return  ui->tracestatusComboBox->currentIndex();
}

int EcuDialog::verbosemode()
{
    return  ui->comboBoxVerboseMode->currentIndex();
}

int EcuDialog::sendSerialHeaderTcp()
{
    return (ui->checkBoxSendSerialHeaderTcp->checkState() == Qt::Checked);
}

int EcuDialog::sendSerialHeaderSerial()
{
    return (ui->checkBoxSendSerialHeaderSerial->checkState() == Qt::Checked);
}

int EcuDialog::syncSerialHeaderTcp()
{
    return (ui->checkBoxSyncToSerialHeaderTcp->checkState() == Qt::Checked);
}

int EcuDialog::syncSerialHeaderSerial()
{
    return (ui->checkBoxSyncToSerialHeaderSerial->checkState() == Qt::Checked);
}

int EcuDialog::timingPackets()
{
    return (ui->checkBoxTiming->checkState() == Qt::Checked);
}

int EcuDialog::sendGetLogInfo()
{
    return (ui->checkBoxGetLogInfo->checkState() == Qt::Checked);
}

int EcuDialog::update()
{
    return  ui->checkBoxUpdate->isChecked();
}

QStringList EcuDialog::getHostnameList()
{
    QStringList hostnames;

    hostnames.clear();
    for (int i = 0; i < ui->comboBoxHostname->count(); ++i)
    {
        hostnames.append(ui->comboBoxHostname->itemText(i));
    }
    return  hostnames;
}

void EcuDialog::setHostnameList(QStringList hostnames)
{
    ui->comboBoxHostname->clear();
    ui->comboBoxHostname->addItems(hostnames);
}

QStringList EcuDialog::getPortList()
{
    QStringList ports;

    ports.clear();
    for (int i = 0; i < ui->comboBoxPort->count(); ++i)
    {
        ports.append(ui->comboBoxPort->itemText(i));
    }
    return ports;
}

void EcuDialog::setPortList(QStringList ports)
{
    ui->comboBoxPort->clear();
    ui->comboBoxPort->addItems(ports);
}
