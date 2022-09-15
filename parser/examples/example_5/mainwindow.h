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
 * \file mainwindow.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QTcpServer server;
    QTcpSocket *socket;

private:
    Ui::MainWindow *ui;

private slots:
    void on_pushButtonSendFile_clicked();
    void on_pushButtonMessage3_clicked();
    void on_pushButtonMessage2_clicked();
    void on_pushButtonSend_clicked();
    void on_pushButtonMessage_clicked();
    void newConnection();
};

#endif // MAINWINDOW_H
