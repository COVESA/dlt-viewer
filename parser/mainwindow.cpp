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

#include <qfiledialog.h>
#include <qmessagebox.h>
#include <QFile>
#include <QStringList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QLabel>
#include <QInputDialog>
#include <QDebug>

#include <iostream>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "version.h"

const char* commandLineOptions = "" \
"Call: dlt-parser [options]\n" \
"DLT parser is used to generate Fibex XML files for non verbose DLT messages.\n" \
"Each option can only be used once. For more complex configurations use a configuration file.\n" \
"\n" \
"Options:\n" \
"  --no-gui                      start without gui\n" \
"  --parse-file filename         Parse a source code file\n" \
"  --parse-dir dirname           Parse a complete directory recursively (*.h;*.c;*.cpp;*.hpp)\n" \
"  --parse-cfg filename          Parse a configuration file contianing special commands.\n" \
"  --converte-file filename      Converte a source code file\n" \
"  --converte-dir dirname        Comverte a complete directory recursively (*.h;*.c;*.cpp;*.hpp)\n" \
"  --read-fibex filename         Read a fibex file to append new messages.\n" \
"  --update-id startId endId     Update the message IDs.\n" \
"  --update-id-app startId endId Update the message IDs unique per application.\n" \
"  --check-double                Check the mesage IDs if they are unique.\n" \
"  --check-double-app            Check the message IDs if they are unique per application.\n" \
"  --write-fibex filename        Generate the fibex XML file.\n" \
"  --write-csv filename          Generate a CSV file containng all parsed log messages.\n" \
"  --write-id dirname            Generate the id header\n" \
"  --write-id-app dirname        start without gui\n" \
"";

MainWindow::MainWindow(bool silent,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* show warning about limitations of software */
    if(!silent)
    {
        QMessageBox::warning(0, QString("DLT Parser"),
                             QString("Warning: This tool is a reference tool for parsing non verbose DLT messages.\nNot for usage in production."));
    }

    filenameWidget = new QLabel();
    statusBar()->addWidget(filenameWidget);

    checkDouble = false;
    checkDoubleApp = false;
    noGui = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::argsParser()
{
    if(!argReadFibex.isEmpty())
    {
        /* read Fibex */
        parser.readFibex(argWriteFibex);
    }
    if(!argParseCfg.isEmpty())
    {
        if(!parser.parseConfiguration(argParseCfg))
        {
            std::cout << "Error: Parse Cfg: "<< parser.getLastError().toLatin1().data() << std::endl;
            return -1;
        }

        // update message ids and application/context ids
        if(!parser.parseCheck())
        {
            std::cout << "Error: Parse Cfg: "<< parser.getLastError().toLatin1().data() << std::endl;
            return -1;
        }

        if(!noGui)
            /* update tree */
            updateTree();

    }
    if(!argParseFile.isEmpty())
    {
        /* parse file */
        if(!parser.parseFile(argParseFile))
        {
            std::cout << "Error: Parse File: "<< parser.getLastError().toLatin1().data() << std::endl;
            return -1;
        }

        // update message ids and application/context ids
        if(!parser.parseCheck())
        {
            std::cout << "Error: Parse File: "<< parser.getLastError().toLatin1().data() << std::endl;
            return -1;
        }

        if(!noGui)
            /* update tree */
            updateTree();

    }
    if(!argParseDir.isEmpty())
    {
        /* create progress dialog */
        QProgressDialog progress("Parsing Directory...", "Abort parsing", 0, 100, this);


        if(noGui){
            progress.setHidden(true);
        }
        else{
            progress.setWindowModality(Qt::WindowModal);
            progress.setMinimumDuration(1000);
            progress.setHidden(false);
        }

        /* go through all files in the directory */
        if(!parseDirectory(argParseDir,parseTypeContexts,false,true,progress))
        {
            progress.close();
            std::cout << "Error: Parse Directory: "<< parser.getLastError().toLatin1().data() << std::endl;
            return -1;
        }
        progress.close();

        // update message ids and application/context ids
        if(!parser.parseCheck())
        {
            std::cout << "Error: Parse Directory: "<< parser.getLastError().toLatin1().data() << std::endl;
            return -1;
        }

        if(!noGui)
            /* update tree */
            updateTree();

        progress.close();
    }
    if(!argConverteFile.isEmpty())
    {
        /* parse file */
        if(!parser.converteFile(argConverteFile))
        {
            std::cout << "Error: Converte File: "<< parser.getLastError().toLatin1().data() << std::endl;
            return -1;
        }
    }
    if(!argConverteDir.isEmpty())
    {
        /* create progress dialog */
        QProgressDialog progress("Parsing Directory...", "Abort parsing", 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setMinimumDuration(1000);

        /* go through all files in the directory */
        if(!parseDirectory(argConverteDir,parseTypeContexts,true,true,progress))
        {
            progress.close();
            std::cout << "Error: Converte Directory: "<< parser.getLastError().toLatin1().data() << std::endl;
            return -1;
        }
        progress.close();
    }
    if(!argUpdateIdStart.isEmpty() && !argUpdateIdEnd.isEmpty()) {
        /* update Ids */
        if(!parser.generateId(argUpdateIdStart.toUInt(),argUpdateIdEnd.toUInt(),false))
        {
            std::cout << "Error: Update IDs: "<< parser.getLastError().toLatin1().data() << std::endl;
            return -1;
        }
    }
    if(!argUpdateIdAppStart.isEmpty() && !argUpdateIdAppEnd.isEmpty()) {
        /* update Ids App */
        if(!parser.generateId(argUpdateIdAppStart.toUInt(),argUpdateIdAppEnd.toUInt(),true))
        {
            std::cout << "Error: Update IDs App: "<< parser.getLastError().toLatin1().data() << std::endl;
            return -1;
        }
    }
    if(checkDouble) {
        /* check for double Ids */
        QString text;
        if(!parser.checkDoubleIds(text,false))
        {
            std::cout << "Error: Check Double: "<< parser.getLastError().toLatin1().data() << std::endl;
            std::cout << "Double Ids: "<< text.toLatin1().data() << std::endl;
            return -1;
        }
    }
    if(checkDoubleApp) {
        /* check for double Ids App */
        QString text;
        if(!parser.checkDoubleIds(text,true))
        {
            std::cout << "Error: Check Double IDs App: "<< parser.getLastError().toLatin1().data() << std::endl;
            std::cout << "Double Ids: "<< text.toLatin1().data() << std::endl;
            return -1;
        }
    }
    if(!argWriteFibex.isEmpty())
    {
        /* write Fibex */
        if(!parser.writeFibex(argWriteFibex))
        {
            std::cout << "Error: Write Fibex: "<< parser.getLastError().toLatin1().data() << std::endl;
            return -1;
        }
    }
    if(!argWriteCsv.isEmpty())
    {
        /* write CSV */
        if(!parser.writeCsv(argWriteCsv))
        {
            std::cout << "Error: Write CSV: "<< parser.getLastError().toLatin1().data() << std::endl;
            return -1;
        }
    }
    if(!argWriteId.isEmpty())
    {
        /* write IDs Header */
        if(!parser.writeIdHeader(argWriteId,false))
        {
            std::cout << "Error: Write IDs Header: "<< parser.getLastError().toLatin1().data() << std::endl;
            return -1;
        }
    }
    if(!argWriteIdApp.isEmpty())
    {
        /* write IDs Header App */
        if(!parser.writeIdHeader(argWriteIdApp,true))
        {
            std::cout << "Error: Write IDs Header App: "<< parser.getLastError().toLatin1().data() << std::endl;
            return -1;
        }
    }
    return 0;
}

void MainWindow::updateTree()
{
    QTreeWidgetItem *item;
    QString text;
    QDltFibexFrame *frame;
    QDltFibexPdu *pdu;

    /* clear ID messages list */
    //frames->takeChildren();
    ui->treeWidget->clear();

    QList<QDltFibexFrame*> messages = parser.getMessages();

    /* add all DLT messages to list */
    for(int i = 0;i<messages.size();i++)
    {
        frame = messages.at(i);

        //text = QString("%1,%2,%3,%4,%5").arg(frame->id).arg("DLT_TYPE_LOG").arg(frame->messageInfo).arg(frame->appid).arg(frame->ctid);

        item = new QTreeWidgetItem(0);
        //frames->addChild(item);
        ui->treeWidget->addTopLevelItem(item);
        item->setText(0,frame->idString/*QString("%1").arg(frame->id,10,10,QLatin1Char(' '))*/); // message id
        item->setText(1,QString("%1").arg(frame->id,10,10,QLatin1Char(' '))); // message id
        item->setText(2,frame->context); // context
        item->setText(3,frame->ctid); // context id
        item->setText(4,frame->appid); // app id
        item->setText(5,frame->messageInfo); // log level
        item->setText(6,QString("%1").arg(frame->lineNumber)); // line number
        item->setText(7,frame->filename); // filename

        text.clear();

        for(int j = 0;j<frame->pdureflist.size();j++)
        {
            pdu= frame->pdureflist.at(j);

            //itempdu = new QTreeWidgetItem(item);
            //item->addChild(itempdu);

            if(j!=0)
                text += " ";

            if(!pdu->description.isEmpty())
            {
                text += QString("%1").arg(pdu->description);
            }
            else
            {
                text += QString("%%1").arg(j+1); // arg(pdu->byteLength).arg(pdu->typeInfo);
            }

            //itempdu->setText(0,text);
        }

        item->setText(8,text); // payload

    }

    updateTreeApplications();
    updateTreeContexts();
    updateMessageIds();
}

void MainWindow::updateMessageIds()
{
    ui->treeWidgetMessageIds->clear();

    QMap<QString,uint32_t> messageIds = parser.getMessageIds();

    QMapIterator<QString,uint32_t> i(messageIds);
    while (i.hasNext()) {
        i.next();

        QTreeWidgetItem *item = new QTreeWidgetItem(0);
        ui->treeWidgetMessageIds->addTopLevelItem(item);
        item->setText(0,i.key());
        item->setText(1,QString("%1").arg(i.value()));
    }
}

void MainWindow::updateTreeApplications()
{
    ui->treeWidgetApplications->clear();

    QMap<QString,QString> applications = parser.getApplications();

    QMapIterator<QString,QString> i(applications);
    while (i.hasNext()) {
        i.next();

        QTreeWidgetItem *item = new QTreeWidgetItem(0);
        ui->treeWidgetApplications->addTopLevelItem(item);
        item->setText(0,i.key());
        item->setText(1,i.value());
    }
}

void MainWindow::updateTreeContexts()
{
    ui->treeWidgetContexts->clear();

    QMap<QString,QDltCon*> contexts = parser.getContexts();

    QMapIterator<QString,QDltCon*> i(contexts);
    while (i.hasNext()) {
        i.next();

        QDltCon* con = i.value();

        QTreeWidgetItem *item = new QTreeWidgetItem(0);
        ui->treeWidgetContexts->addTopLevelItem(item);
        item->setText(0,con->context);
        item->setText(1,con->conid);
        item->setText(2,con->appid);
        item->setText(3,con->description);
    }

}


bool MainWindow::parseDirectory(QString dirName, parseType type,bool convert,bool create,QProgressDialog &progress)
{
    QFileInfoList list;
    QStringList filter;
    QDir dir(dirName);
    bool progressBarVisible=!progress.isHidden();

    /* create filter list for files */
    filter.append(QString("*.h"));
    filter.append(QString("*.c"));
    filter.append(QString("*.cpp"));
    filter.append(QString("*.cxx"));

    /* parse files in current directory */
    list = dir.entryInfoList(filter,QDir::Files);
    if(progressBarVisible)
        progress.setMaximum(progress.maximum()+list.size());

    for(int i = 0;i<list.size();i++)
    {
        /* increase progress bar */
        if(progressBarVisible)
            progress.setValue(progress.value()+1);

        if(convert)
        {
            if(!parser.converteFile(list.at(i).absoluteFilePath()))
                return false;
        }
        else
        {
            if(!parser.parseFile(list.at(i).absoluteFilePath()))
                return false;
        }
        if(progressBarVisible)
            progress.setLabelText(list.at(i).absoluteFilePath());
    }

    /* go recursive through all the subdiectories */
    list = dir.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
    for(int i = 0;i<list.size();i++)
    {
        if(!parseDirectory(list.at(i).absoluteFilePath(),type,convert,create,progress))
            return false;
    }

    return true;
}


void MainWindow::on_actionInfo_triggered()
{

    /* display information abouit SW status */
    QMessageBox::information(0, QString("DLT Parser"),
                             QString("Package Version : %1 %2\n").arg(PACKAGE_VERSION).arg(PACKAGE_VERSION_STATE)+
                             QString("Build Date: %1\n").arg(__DATE__)+
                             QString("Build Time: %1\n\n").arg(__TIME__)+
                             QString("(C) 2011,2014 BMW AG\n"));
}

void MainWindow::on_actionCommandline_options_triggered()
{
    /* display information abouit command line argumentss */
    QMessageBox::information(0, QString("Commandline arguments"),
                                QString(commandLineOptions));
}

void MainWindow::on_action_New_triggered()
{
    /* clear tree of contexts and log entries */
    parser.clear();
    updateTree();
    fibexFileName.clear();
    filenameWidget->setText("");
}

void MainWindow::on_action_Open_triggered()
{
    QString fileName;

    /* request filename from user */
    fileName = QFileDialog::getOpenFileName(this,
        tr("DLT Non verbose Fibex file"), "", tr("Description Files (*.xml);;All files (*.*)"));

    if(fileName.isEmpty())
    {
        /* no file selected from user */
        return;
    }

    /* update file name */
    fibexFileName = fileName;
    filenameWidget->setText(fibexFileName);

    /* read Fibex file */
    if(!parser.readFibex(fileName))
    {
        QMessageBox::warning(this,"Open Fibex",parser.getLastError());
    }

    /* update tree */
    updateTree();

}

void MainWindow::on_actionExport_CSV_triggered()
{
    QString fileName;
    QFile file;

    /* request filename from user */
    fileName = QFileDialog::getSaveFileName(this,
        tr("Export to CSV"), "", tr("Description Files (*.csv);;All files (*.*)"));

    if(fileName.isEmpty())
    {
        /* no file selected from user */
        return;
    }

    /* write CSV */
    if(!parser.writeCsv(fileName))
    {
        QMessageBox::warning(this,"Export CSV",parser.getLastError());
    }
}

void MainWindow::on_actionExit_triggered()
{
    QMessageBox::information(0, QString("File Exit"),QString("Feature not supported yet!"));
}

void MainWindow::on_actionCheck_double_IDs_triggered()
{
    QString text;

    if(!parser.checkDoubleIds(text,false))
    {
        /* double Ids found and display list of double Ids */
        QMessageBox msgBox;
        msgBox.setText("Double IDs found. Please check the details for further information.");
        msgBox.setDetailedText(text);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
    else
    {
        /* no double Ids found */
        QMessageBox msgBox;
        msgBox.setText("No double IDs found.");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
    }
}

void MainWindow::on_actionCheck_double_IDs_per_context_triggered()
{
    QString text;

    if(!parser.checkDoubleIds(text,true))
    {
        /* double Ids found and display list of double Ids */
        QMessageBox msgBox;
        msgBox.setText("Double IDs per context found. Please check the details for further information.");
        msgBox.setDetailedText(text);
        msgBox.setIcon(QMessageBox::Critical);
         msgBox.exec();
    }
    else
    {
        /* no double Ids found */
        QMessageBox msgBox;
        msgBox.setText("No double IDs found.");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
    }
}

void MainWindow::on_actionParseFile_triggered()
{
    QString fileName;

    /* request filename from user */
    fileName = QFileDialog::getOpenFileName(this,
        tr("Parse Source file"), "", tr("Source file (*.c *.cpp *.cxx *.h *.hpp *.hxx);;All files (*.*)"));

    if(fileName.isEmpty())
    {
        /* no directory selected, return */
        return;
    }

    // parse file
    if(!parser.parseFile(fileName))
    {
        QMessageBox::warning(this,"Parse File",parser.getLastError());
        return;
    }

    // update message ids and application/context ids
    if(!parser.parseCheck())
    {
        QMessageBox::warning(this,"Parse File",parser.getLastError());
        return;
    }

    /* update tree */
    updateTree();

}

void MainWindow::on_actionParseDirectory_triggered()
{
    QString dirName;

    /* request directory name from user */
    dirName = QFileDialog::getExistingDirectory(this,tr("Parse Directory"));

    if(dirName.isEmpty())
    {
        /* no directory selected, return */
        return;
    }

    /* create progress dialog */
    QProgressDialog progress("Parsing Directory...", "Abort parsing", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(1000);
    progress.setHidden(false);

    /* go through all files in the directory */
    if(!parseDirectory(dirName,parseTypeContexts,false,true,progress))
    {
        progress.close();
        QMessageBox::warning(this,"Parse Directory",parser.getLastError());
        return;
    }
    progress.close();

    // update message ids and application/context ids
    if(!parser.parseCheck())
    {
        QMessageBox::warning(this,"Parse Directory",parser.getLastError());
        return;
    }

    /* update tree */
    updateTree();

}

void MainWindow::on_actionParseConfiguration_triggered()
{
    QString fileName;

    /* request filename from user */
    fileName = QFileDialog::getOpenFileName(this,
        tr("Parse Configuration file"), "", tr("Configuration file (*.*);;All files (*.*)"));

    if(fileName.isEmpty())
    {
        /* no directory selected, return */
        return;
    }

    if(!parser.parseConfiguration(fileName))
    {
        QMessageBox::warning(this,"Parse Configuration",parser.getLastError());
        return;
    }

    /* update tree */
    updateTree();
}

void MainWindow::on_actionConvertFile_triggered()
{
    QString fileName;

    /* request filename from user */
    fileName = QFileDialog::getOpenFileName(this,
        tr("Parse and convert Source file"), "", tr("Source file (*.c;*.cpp;*.cxx);;All files (*.*)"));

    if(fileName.isEmpty())
    {
        /* no directory selected, return */
        return;
    }

    /* parse and convert file to find all log entries */
    if(!parser.converteFile(fileName))
    {
        QMessageBox::warning(this,"Converte File",parser.getLastError());
        return;
    }

    /* update tree */
    updateTree();

}

void MainWindow::on_actionConvertDirectory_triggered()
{
    QString dirName;

    /* request directory name from user */
    dirName = QFileDialog::getExistingDirectory(this,tr("Parse and convert Directory"));

    if(dirName.isEmpty())
    {
        /* no directory selected, return */
        return;
    }

    /* create progress dialog */
    QProgressDialog progress("Convert Directory...", "Abort converting", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setHidden(false);
    /* go through all files in the directory */
    if(!parseDirectory(dirName,parseTypeMessages,true,true,progress))
    {
        QMessageBox::warning(this,"Converte Directory",parser.getLastError());
        return;
    }

    /* update tree */
    updateTree();

    /* information about end of operation */
    QMessageBox::information(0, QString("DLT Parser"),
                         QString("Parsing and converting directory finished!"));
}

void MainWindow::on_actionGenerate_Id_Header_triggered()
{
    QString dirName;

    /* request directory name from user */
    dirName = QFileDialog::getExistingDirectory(this,tr("Generate Id Header Directory"));

    if(dirName.isEmpty())
    {
        /* no directory selected, return */
        return;
    }

    if(!parser.writeIdHeader(dirName,false))
    {
        QMessageBox::warning(this,"Generate Id Header:",parser.getLastError());
        return;
    }
}

void MainWindow::on_actionGenerate_Id_Header_per_context_triggered()
{
    QString dirName;

    /* request directory name from user */
    dirName = QFileDialog::getExistingDirectory(this,tr("Generate Id Header per application Directory"));

    if(dirName.isEmpty())
    {
        /* no directory selected, return */
        return;
    }

    if(!parser.writeIdHeader(dirName,true))
    {
        QMessageBox::warning(this,"Generate Id Header:",parser.getLastError());
        return;
    }
}

void MainWindow::on_actionGenerate_Fibex_XML_triggered()
{
    QString fileName;

    /* request filename from user */
    fileName = QFileDialog::getSaveFileName(this,
        tr("DLT Non verbose Fibex file"), "", tr("Description Files (*.xml);;All files (*.*)"));

    if(fileName.isEmpty())
    {
        /* no file selected from user */
        return;
    }
    fibexFileName = fileName;
    filenameWidget->setText(fibexFileName);

    if(!parser.writeFibex(fibexFileName))
    {
        QMessageBox::warning(this,"Save As",parser.getLastError());
    }
}


void MainWindow::on_actionUpdate_IDs_triggered()
{
    /* start with message id provided by user */
    bool ok;

    uint32_t messageIdStart=1;
    QString text = QInputDialog::getText(this, tr("Update IDs)"),
                                         tr("Start Message Id:"), QLineEdit::Normal,
                                         QString("%1").arg(1), &ok);
    if (ok)
        messageIdStart = text.toUInt();
    else
        return;

    uint32_t messageIdEnd=1;
    text = QInputDialog::getText(this, tr("Update IDs)"),
                                         tr("End Message Id:"), QLineEdit::Normal,
                                         QString("%1").arg(0xffffffff), &ok);
    if (ok)
        messageIdEnd = text.toUInt();
    else
        return;

    if(!parser.generateId(messageIdStart,messageIdEnd,false))
    {
        QMessageBox::warning(this,"Update IDs",parser.getLastError());
    }

    /* update tree */
    updateTree();
}

void MainWindow::on_actionUpdate_IDs_per_application_triggered()
{
    /* start with message id provided by user */
    bool ok;

    uint32_t messageIdStart=1;
    QString text = QInputDialog::getText(this, tr("Update IDs)"),
                                         tr("Start Message Id per App:"), QLineEdit::Normal,
                                         QString("%1").arg(1), &ok);
    if (ok)
        messageIdStart = text.toUInt();
    else
        return;

    uint32_t messageIdEnd=1;
    text = QInputDialog::getText(this, tr("Update IDs)"),
                                         tr("End Message Id per App:"), QLineEdit::Normal,
                                         QString("%1").arg(0xffffffff), &ok);
    if (ok)
        messageIdEnd = text.toUInt();
    else
        return;

    if(!parser.generateId(messageIdStart,messageIdEnd,true))
    {
        QMessageBox::warning(this,"Update IDs per application",parser.getLastError());
    }

    /* update tree */
    updateTree();
}
