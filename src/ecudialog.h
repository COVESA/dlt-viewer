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
 * \file ecudialog.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef ECUDIALOG_H
#define ECUDIALOG_H

#include <QDialog>
#include "project.h"

namespace Ui {
    class EcuDialog;
}

class EcuDialog : public QDialog {
    Q_OBJECT
public:
    EcuDialog(QString id,QString description,int interfacetype,QString hostname,unsigned int tcpport,QString port,BaudRateType baudrate,
              int loglevel, int tracestatus, int verbosemode, bool sendSerialHeaderTcp, bool sendSerialHeaderSerial,bool syncSerialHeaderTcp, bool syncSerialHeaderSerial,
              bool timingPackets, bool sendGetLogInfo, bool sendDefaultLogLevel, bool update, bool autoReconnect, int autoReconnectTimeout, QWidget *parent = 0);
    ~EcuDialog();

    QString id();
    QString description();
    int interfacetype();
    QString hostname();
    unsigned int tcpport();
    QString port();
    BaudRateType baudrate();
    int loglevel();
    int tracestatus();
    int verbosemode();
    int sendSerialHeaderTcp();
    int sendSerialHeaderSerial();
    int syncSerialHeaderTcp();
    int syncSerialHeaderSerial();
    int timingPackets();
    int sendGetLogInfo();
    int sendDefaultLogLevel();
    int update();
    int autoReconnect();
    int autoReconnectTimeout();

    QStringList getHostnameList();
    void setHostnameList(QStringList hostnames);

    QStringList getPortList();
    void setPortList(QStringList ports);

    void setDialogToEcuItem(EcuItem *item);
protected:
    void changeEvent(QEvent *e);

private slots:
    void on_checkBoxAutoReconnect_toggled(bool checked);

    void on_comboBoxInterface_currentIndexChanged(int index);

private:
    Ui::EcuDialog *ui;
};

#endif // ECUDIALOG_H
