/**
 * @licence app begin@
 * Copyright (C) 2014  BMW AG
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 * Contributions are licensed to the COVESA Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \author
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \file mainwindow.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "QMessageBox"
#include <QFile>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <qwaitcondition.h>

#include "example5.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    example5_init();

    server.listen(QHostAddress::Any,8888);
    connect(&server,SIGNAL(newConnection()),this,SLOT(newConnection()));

    ui->lineEditMessages->setText("100");

    socket = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonMessage_clicked()
{
    example5_test1();
}

void MainWindow::on_pushButtonMessage2_clicked()
{
    example5_test2();
}

void MainWindow::on_pushButtonMessage3_clicked()
{
    example5_test3(ui->lineEditMessages->text().toInt());
}


void MainWindow::on_pushButtonSend_clicked()
{
    unsigned char *DLTdata;
    int DLTlength;

    while((DLTlength = dlt_user_log_read(&DLTdata)) > 0)
    {
        //LOGGER_Send((char*)DLTdata,DLTlength);
        socket->write((const char*)DLTdata,DLTlength);
        dlt_user_log_read_ack(DLTlength);
    }

}

void MainWindow::newConnection()
{
    QMessageBox::information(this,"DLT Embedded Test","Connected");
    socket = server.nextPendingConnection();
}

void MainWindow::on_pushButtonSendFile_clicked()
{
    char buffer[100];
    int length;
    int size;

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open DLT Log file"), "", tr("DLT Files (*.dlt);;All files (*.*)"));

    if(fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);

    file.open(QIODevice::ReadOnly);

    size = file.size();

    //for(int num = 0;num < size;num+=51)
    {
        //file.seek(num);
        file.seek(ui->lineEditFileOffset->text().toInt());

        while (!file.atEnd()) {
            length = file.read(buffer,1);
            socket->write(buffer,length);

        }
    }

    file.close();
}
