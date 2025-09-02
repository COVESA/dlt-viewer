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
#include <QTreeWidgetItem>
#include <QFile>
#include <QProgressDialog>

#include "qdltparser.h"

extern const char* commandLineOptions;

class DltCon
{
public:
       DltCon() {  }

       QString conid;
       QString appid;
       QString description;
       QString context;
};

class DltFibexPdu
{
public:
       DltFibexPdu() { byteLength = 0; }

       QString id;
       QString description;
       int32_t byteLength;
       QString typeInfo;
};

class DltFibexPduRef
{
public:
       DltFibexPduRef() { ref = 0; }

       QString id;
       DltFibexPdu *ref;
};

/**
* The structure of a Fibex Frame information.
*/
class DltFibexFrame
{
public:
   DltFibexFrame() { byteLength=0;messageType=0;pduRefCounter=0;id=0;lineNumber=0; }

       QString idString;
       int id;
       int32_t byteLength;
       uint8_t messageType;
       QString messageInfo;
       QString appid;
       QString ctid;
       QString filename;
       int lineNumber;

       QList<DltFibexPdu*> pdureflist;
       QList<DltFibexPduRef*> pdurefreflist;
       uint32_t pduRefCounter;
};

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum parseType { parseTypeContexts,parseTypeMessages };

    explicit MainWindow(bool silent,QWidget *parent = 0);
    ~MainWindow();

    QStringList parseLine(QFile &file,QString &line,int &linecounter);

    bool parseDirectory(QString dirName, parseType type, bool convert, bool create, QProgressDialog &progress);

    void updateTree();
    void updateTreeApplications();
    void updateTreeContexts();
    void updateMessageIds();

    int argsParser();

    // options
    QString argParseFile,argParseDir,argParseCfg;
    QString argConverteFile,argConverteDir;
    QString argReadFibex;
    QString argWriteFibex,argWriteCsv,argWriteId,argWriteIdApp;
    QString argUpdateIdStart,argUpdateIdAppStart;
    QString argUpdateIdEnd,argUpdateIdAppEnd;
    bool checkDouble,checkDoubleApp;
    bool noGui;

    QString fibexFileName;

    QLabel *filenameWidget;


private:

    Ui::MainWindow *ui;

    QDltParser parser;

private slots:
    void on_actionInfo_triggered();
    void on_actionCommandline_options_triggered();
    void on_action_New_triggered();
    void on_action_Open_triggered();
    void on_actionExport_CSV_triggered();
    void on_actionExit_triggered();
    void on_actionCheck_double_IDs_triggered();
    void on_actionParseFile_triggered();
    void on_actionParseDirectory_triggered();
    void on_actionConvertFile_triggered();
    void on_actionConvertDirectory_triggered();
    void on_actionParseConfiguration_triggered();
    void on_actionGenerate_Id_Header_triggered();
    void on_actionGenerate_Fibex_XML_triggered();
    void on_actionCheck_double_IDs_per_context_triggered();
    void on_actionGenerate_Id_Header_per_context_triggered();
    void on_actionUpdate_IDs_triggered();
    void on_actionUpdate_IDs_per_application_triggered();
};

#endif // MAINWINDOW_H
