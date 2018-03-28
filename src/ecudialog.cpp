/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file ecudialog.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include "ecudialog.h"
#include "ui_ecudialog.h"

#include <QSerialPort>

EcuDialog::EcuDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EcuDialog)
{
    ui->setupUi(this);

    ui->comboBoxPort->setEditable(true);

#if defined(Q_OS_UNIX) || defined(qdoc)
    ui->comboBoxBaudrate->addItem(QLatin1String("50"), 50 );                //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("75"), 75 );                //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("134"), 134 );              //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("150"), 150 );              //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("200"), 200 );              //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("1800"), 1800 );            //POSIX ONLY
#  if (defined(B230400) && defined(B4000000)) || defined(qdoc)
    ui->comboBoxBaudrate->addItem(QLatin1String("230400"), 230400 );        //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("460800"), 460800 );        //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("500000"), 500000 );        //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("576000"), 576000 );       //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("921600"), 921600 );        //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("100000"), 1000000 );      //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("115200"), 1152000 );      //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("150000"), 1500000 );      //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("200000"), 2000000 );      //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("250000"), 2500000 );     //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("300000"), 3000000 );      //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("350000"), 3500000 );      //POSIX ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("400000"), 4000000 );      //POSIX ONLY
#  endif
#endif //Q_OS_UNIX

#if defined(Q_OS_WIN) || defined(qdoc)
    ui->comboBoxBaudrate->addItem(QLatin1String("14400"), 14400 );          //WINDOWS ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("56000"), 56000 );          //WINDOWS ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("28000"), 128000 );        //WINDOWS ONLY
    ui->comboBoxBaudrate->addItem(QLatin1String("256000"), 256000 );       //WINDOWS ONLY
#endif  //Q_OS_WIN
    ui->comboBoxBaudrate->addItem(QLatin1String("110"), 110 );
    ui->comboBoxBaudrate->addItem(QLatin1String("300"), 300 );
    ui->comboBoxBaudrate->addItem(QLatin1String("600"), 600 );
    ui->comboBoxBaudrate->addItem(QLatin1String("1200"), QSerialPort::Baud1200 );
    ui->comboBoxBaudrate->addItem(QLatin1String("2400"), QSerialPort::Baud2400 );
    ui->comboBoxBaudrate->addItem(QLatin1String("4800"), QSerialPort::Baud4800 );
    ui->comboBoxBaudrate->addItem(QLatin1String("9600"), QSerialPort::Baud9600 );
    ui->comboBoxBaudrate->addItem(QLatin1String("19200"), QSerialPort::Baud19200 );
    ui->comboBoxBaudrate->addItem(QLatin1String("38400"),  QSerialPort::Baud38400 );
    ui->comboBoxBaudrate->addItem(QLatin1String("57600"), QSerialPort::Baud57600 );
    ui->comboBoxBaudrate->addItem(QLatin1String("115200"), QSerialPort::Baud115200 );

    ui->comboBoxBaudrate->setCurrentIndex(ui->comboBoxBaudrate->count()-1);

}

EcuDialog::~EcuDialog()
{
    delete ui;
}

void EcuDialog::setData(EcuItem &item)
{
    ui->idLineEdit->setText(item.id);
    ui->descriptionLineEdit->setText(item.description);
    ui->comboBoxInterface->setCurrentIndex(item.interfacetype);
    ui->comboBoxHostname->setEditText(item.getHostname());
    ui->lineEditTcpPort->setText(QString("%1").arg(item.getIpport()));
    ui->comboBoxPort->setEditText(item.getPort());

    ui->comboBoxBaudrate->setCurrentIndex(ui->comboBoxBaudrate->count()-1);
    for(int i=0; i<ui->comboBoxBaudrate->count(); i++){
        if(item.getBaudrate() == (QSerialPort::BaudRate)ui->comboBoxBaudrate->itemData(i).toInt())
            ui->comboBoxBaudrate->setCurrentIndex(i);
    }

    ui->loglevelComboBox->setCurrentIndex(item.loglevel);
    ui->tracestatusComboBox->setCurrentIndex(item.tracestatus);
    ui->comboBoxVerboseMode->setCurrentIndex(item.verbosemode);
    ui->checkBoxSendSerialHeaderSerial->setCheckState(item.getSendSerialHeaderSerial()?Qt::Checked:Qt::Unchecked);
    ui->checkBoxSendSerialHeaderTcp->setCheckState(item.getSendSerialHeaderIp()?Qt::Checked:Qt::Unchecked);
    ui->checkBoxSyncToSerialHeaderSerial->setCheckState(item.getSyncSerialHeaderSerial()?Qt::Checked:Qt::Unchecked);
    ui->checkBoxSyncToSerialHeaderTcp->setCheckState(item.getSyncSerialHeaderIp()?Qt::Checked:Qt::Unchecked);
    ui->checkBoxTiming->setCheckState(item.timingPackets?Qt::Checked:Qt::Unchecked);

    ui->checkBoxGetLogInfo->setCheckState(item.sendGetLogInfo?Qt::Checked:Qt::Unchecked);
    ui->checkBoxDefaultLogLevel->setCheckState(item.sendDefaultLogLevel?Qt::Checked:Qt::Unchecked);
    ui->checkBoxGetSoftwareVersion->setCheckState(item.sendGetSoftwareVersion?Qt::Checked:Qt::Unchecked);

    ui->checkBoxUpdate->setCheckState(item.updateDataIfOnline?Qt::Checked:Qt::Unchecked);

    ui->checkBoxAutoReconnect->setCheckState(item.autoReconnect?Qt::Checked:Qt::Unchecked);
    ui->spinBoxAutoreconnect->setValue(item.autoReconnectTimeout);

    on_comboBoxInterface_currentIndexChanged(ui->comboBoxInterface->currentIndex());
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

QSerialPort::BaudRate EcuDialog::baudrate()
{
    return (QSerialPort::BaudRate)ui->comboBoxBaudrate->itemData(ui->comboBoxBaudrate->currentIndex()).toInt();
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

int EcuDialog::sendDefaultLogLevel()
{
    return (ui->checkBoxDefaultLogLevel->checkState() == Qt::Checked);
}

int EcuDialog::sendGetSoftwareVersion()
{
    return (ui->checkBoxGetSoftwareVersion->checkState() == Qt::Checked);
}

int EcuDialog::update()
{
    return  ui->checkBoxUpdate->isChecked();
}

int EcuDialog::autoReconnect()
{
    return  ui->checkBoxAutoReconnect->isChecked();
}

int EcuDialog::autoReconnectTimeout()
{
    return  ui->spinBoxAutoreconnect->value();
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

void EcuDialog::setDialogToEcuItem(EcuItem *item){
    item->id = this->id();
    item->description = this->description();
    item->interfacetype = this->interfacetype();
    item->setHostname(this->hostname());
    item->setIpport(this->tcpport());
    item->setPort(this->port());
    item->setBaudrate(this->baudrate());
    item->loglevel = this->loglevel();
    item->tracestatus = this->tracestatus();
    item->verbosemode = this->verbosemode();
    item->setSendSerialHeaderSerial(this->sendSerialHeaderSerial());
    item->setSendSerialHeaderIp(this->sendSerialHeaderTcp());
    item->setSyncSerialHeaderSerial(this->syncSerialHeaderSerial());
    item->setSyncSerialHeaderIp(this->syncSerialHeaderTcp());
    item->timingPackets = this->timingPackets();
    item->sendGetLogInfo = this->sendGetLogInfo();
    item->sendDefaultLogLevel = this->sendDefaultLogLevel();
    item->sendGetSoftwareVersion = this->sendGetSoftwareVersion();
    item->updateDataIfOnline = this->update();
    item->autoReconnect = this->autoReconnect();
    item->autoReconnectTimeout = this->autoReconnectTimeout();
    item->updateAutoReconnectTimestamp();

    /* new qdlt library */
    item->ipcon.setPort(this->tcpport());
    item->ipcon.setHostname(this->hostname());
    item->ipcon.setSendSerialHeader(this->sendSerialHeaderTcp());
    item->ipcon.setSyncSerialHeader(this->syncSerialHeaderTcp());
    item->serialcon.setBaudrate(this->baudrate());
    item->serialcon.setPort(this->port());
    item->serialcon.setSendSerialHeader(this->sendSerialHeaderSerial());
    item->serialcon.setSyncSerialHeader(this->syncSerialHeaderSerial());

}

void EcuDialog::on_checkBoxAutoReconnect_toggled(bool checked)
{
    ui->spinBoxAutoreconnect->setEnabled(checked);
}

void EcuDialog::on_comboBoxInterface_currentIndexChanged(int index)
{
    switch(index)
    {
        case EcuItem::INTERFACETYPE_TCP:
            //we have TCP set -> disable serial, enable TCP
            ui->tabWidget->setTabEnabled(1,true);
            ui->tabWidget->setTabEnabled(2,false);
            break;
        case EcuItem::INTERFACETYPE_UDP:
            //we have UDP set -> disable serial, enable UDP
            ui->tabWidget->setTabEnabled(1,true);
            ui->tabWidget->setTabEnabled(2,false);
            break;
        case EcuItem::INTERFACETYPE_SERIAL:
            ui->tabWidget->setTabEnabled(1,false);
            ui->tabWidget->setTabEnabled(2,true);
            break;
    }
}
