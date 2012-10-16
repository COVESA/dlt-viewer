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
 * \file mainwindow.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QTreeView>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QTemporaryFile>
#include <QPluginLoader>
#include <QPushButton>
#include <QKeyEvent>
#include <QClipboard>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QLineEdit>
#include <QUrl>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ecudialog.h"
#include "applicationdialog.h"
#include "contextdialog.h"
#include "multiplecontextdialog.h"
#include "plugindialog.h"
#include "settingsdialog.h"
#include "injectiondialog.h"
#include "qextserialenumerator.h"
#include "version.h"
#include "commandplugindialog.h"
#include "threadplugin.h"
#include "threaddltindex.h"
#include "threadfilter.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer(this),
    qcontrol(this)
{
    ui->setupUi(this);
    setAcceptDrops(true);

    /* Settings */
    settings = new SettingsDialog(&qfile,this);
    settings->assertSettingsVersion();
    settings->readSettings();
    recentFiles = settings->getRecentFiles();
    recentProjects = settings->getRecentProjects();
    recentFilters = settings->getRecentFilters();
    workingDirectory = settings->getWorkingDirectory();

    /* Initialize recent files */
    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
        ui->menuRecent_files->addAction(recentFileActs[i]);
    }

    /* Initialize recent projects */
    for (int i = 0; i < MaxRecentProjects; ++i) {
        recentProjectActs[i] = new QAction(this);
        recentProjectActs[i]->setVisible(false);
        connect(recentProjectActs[i], SIGNAL(triggered()), this, SLOT(openRecentProject()));
        ui->menuRecent_projects->addAction(recentProjectActs[i]);
    }

    /* Initialize recent filters */
    for (int i = 0; i < MaxRecentFilters; ++i) {
        recentFiltersActs[i] = new QAction(this);
        recentFiltersActs[i]->setVisible(false);
        connect(recentFiltersActs[i], SIGNAL(triggered()), this, SLOT(openRecentFilters()));
        ui->menuRecent_Filters->addAction(recentFiltersActs[i]);
    }

    /* Update recent file and project actions */
    updateRecentFileActions();
    updateRecentProjectActions();
    updateRecentFiltersActions();

    /* initialise DLT file handling */
    tableModel = new TableModel("Hello Tree");
    tableModel->qfile = &qfile;
    tableModel->project = &project;

    /* initialise project configuration */
    project.ecu = ui->configWidget;
    project.filter = ui->filterWidget;
    project.plugin = ui->pluginWidget;
    project.settings = settings;

    /* Load Plugins before loading default project */
    loadPlugins();

    searchDlg = new SearchDialog(this);
    searchDlg->file = &qfile;
    searchDlg->table = ui->tableView;
    searchDlg->plugin = project.plugin;

    /* initialize injection */
    injectionAplicationId.clear();
    injectionContextId.clear();
    injectionServiceId.clear();
    injectionData.clear();

    /* set table size and en */
    ui->tableView->setModel(tableModel);
    ui->tableView->setColumnWidth(0,50);
    ui->tableView->setColumnWidth(1,150);
    ui->tableView->setColumnWidth(2,70);
    ui->tableView->setColumnWidth(3,40);
    ui->tableView->setColumnWidth(4,40);
    ui->tableView->setColumnWidth(5,40);
    ui->tableView->setColumnWidth(6,40);
    ui->tableView->setColumnWidth(7,50);
    ui->tableView->setColumnWidth(8,50);
    ui->tableView->setColumnWidth(9,40);
    ui->tableView->setColumnWidth(10,40);
    ui->tableView->setColumnWidth(11,400);

    /* Enable column sorting of config widget */
    ui->configWidget->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
    ui->configWidget->setSortingEnabled(true);             // should cause sort on add
    ui->configWidget->setHeaderHidden(false);
    ui->filterWidget->setHeaderHidden(false);
    ui->pluginWidget->setHeaderHidden(false);

    /* initialise statusbar */
    totalBytesRcvd = 0;
    totalByteErrorsRcvd = 0;
    statusFilename = new QLabel("no log file loaded");
    statusBytesReceived = new QLabel("Recv: 0");
    statusByteErrorsReceived = new QLabel("Recv Errors: 0");
    statusBar()->addWidget(statusFilename);
    statusBar()->addWidget(statusBytesReceived);
    statusBar()->addWidget(statusByteErrorsReceived);

    /* Inform filterWidget about FilterButton */
    ui->filterWidget->setFilterButton(ui->filterButton);

    /* initialize tool bar */
    QAction *action;
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/document-new.png"), tr("&New"));
    connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFile_New_triggered()));
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/document-open.png"), tr("&Open"));
    connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFile_Open_triggered()));
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/edit-clear.png"), tr("&Clear"));
    connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFile_Clear_triggered()));
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/document-save-as3.png"), tr("&Save logfile"));
    connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFile_SaveAs_triggered()));
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/document-save-as2.png"), tr("&Save project"));
    connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuProject_Save_triggered()));
    ui->mainToolBar->addSeparator();
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/network-transmit-receive.png"), tr("Connec&t"));
    connect(action, SIGNAL(triggered()), this, SLOT(connectAll()));
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/network-offline.png"), tr("&Disconnect"));
    connect(action, SIGNAL(triggered()), this, SLOT(disconnectAll()));
    ui->mainToolBar->addSeparator();
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/preferences-desktop.png"), tr("S&ettings"));
    connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFile_Settings_triggered()));

    ui->mainToolBar->addSeparator();

    scrollbutton = new QPushButton(QIcon(":/toolbar/png/go-bottom.png"),tr(""),this);
    scrollbutton->setFlat(true);
    scrollbutton->setCheckable(true);
    scrollbutton->setToolTip(tr("Auto Scroll"));
    action = ui->mainToolBar->addWidget(scrollbutton);
    connect(scrollbutton, SIGNAL(toggled(bool)), this, SLOT(autoscrollToggled(bool)));
    updateScrollButton();

    ui->mainToolBar->addSeparator();

    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/system-search.png"), tr("Find"));
    connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuSearch_Find_triggered()));
    //action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/go-previous.png"), tr("Search previous"));
    //connect(action, SIGNAL(triggered()), this, SLOT(on_actionSearch_Continue_triggered()));
    searchTextToolbar = new QLineEdit(ui->mainToolBar);
    searchDlg->appendLineEdit(searchTextToolbar);
    connect(searchTextToolbar, SIGNAL(textEdited(QString)),searchDlg,SLOT(textEditedFromToolbar(QString)));
    connect(searchTextToolbar, SIGNAL(returnPressed()),searchDlg,SLOT(findNextClicked()));
    action = ui->mainToolBar->addWidget(searchTextToolbar);
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/go-previous.png"), tr("Find Previous"));
    connect(action, SIGNAL(triggered()), searchDlg, SLOT(findPreviousClicked()));
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/go-next.png"), tr("Find Next"));
    connect(action, SIGNAL(triggered()), searchDlg, SLOT(findNextClicked()));


    /* Apply loaded settings */
    applySettings();

    connect((QObject*)(ui->tableView->verticalScrollBar()), SIGNAL(valueChanged(int)), this, SLOT(tableViewValueChanged(int)));
    connect(ui->tableView->horizontalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(sectionInTableDoubleClicked(int)));

    /* Process Project */
    if(OptManager::getInstance()->isProjectFile())
    {
        openDlpFile(OptManager::getInstance()->getProjectFile());

    } else {
        /* Load default project file */
        this->setWindowTitle(QString("DLT Viewer - unnamed project - Version : %1 %2").arg(PACKAGE_VERSION).arg(PACKAGE_VERSION_STATE));
        if(settings->defaultProjectFile)
        {
            if(!openDlpFile(settings->defaultProjectFileName)){
                QMessageBox::critical(0, QString("DLT Viewer"),
                                      QString("Cannot load default project \"%1\"")
                                      .arg(settings->defaultProjectFileName));
            }
        }
    }

    /* Process Logfile */
    if(OptManager::getInstance()->isLogFile())
    {
        openDltFile(OptManager::getInstance()->getLogFile());
    } else {
        /* load default log file */
        statusFilename->setText("no log file loaded");
        if(settings->defaultLogFile)
        {
            openDltFile(settings->defaultLogFileName);

        }
        else
        {
            /* create temporary file */
            QTemporaryFile file;
            if (file.open()) {
                QString filename;
                filename = file.fileName();
                file.setAutoRemove(false);
                file.close();
                outputfile.setFileName(filename);
                if(outputfile.open(QIODevice::WriteOnly|QIODevice::Truncate))
                    reloadLogFile();
                else
                    QMessageBox::critical(0, QString("DLT Viewer"),
                                          QString("Cannot load temporary log file \"%1\"\n%2")
                                          .arg(filename)
                                          .arg(outputfile.errorString()));
            }
            file.close();
        }
    }

    if(OptManager::getInstance()->isFilterFile()){
        if(project.LoadFilter(OptManager::getInstance()->getFilterFile(),true))
        {
            filterUpdate();
            setCurrentFilters(OptManager::getInstance()->getFilterFile());
        }
    }

    if(OptManager::getInstance()->isConvert())
    {
        commandLineConvertToASCII();
        exit(0);
    }

    /* Command plugin */
    if(OptManager::getInstance()->isPlugin())
    {
        commandLineExecutePlugin(OptManager::getInstance()->getPluginName(),
                                 OptManager::getInstance()->getCommandName(),
                                 OptManager::getInstance()->getCommandParams());
    }

    /* auto connect */
    if(settings->autoConnect)
    {
        connectAll();
    }

    /* start timer for autoconnect */
    connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer.start(1000);

    /* start timer for serial read */
    connect(&timerRead, SIGNAL(timeout()), this, SLOT(timeoutRead()));
    timerRead.start(100);

    restoreGeometry(DltSettingsManager::getInstance()->value("geometry").toByteArray());
    restoreState(DltSettingsManager::getInstance()->value("windowState").toByteArray());
}

MainWindow::~MainWindow()
{
    DltSettingsManager *settings = DltSettingsManager::getInstance();

    /* store last working directory */
    settings->setValue("work/workingDirectory",workingDirectory);
    DltSettingsManager::close();

    delete ui;
    delete tableModel;
    delete searchDlg;
}

void MainWindow::commandLineConvertToASCII(){
    QByteArray data;
    QDltMsg msg;
    QString text;

    openDltFile(OptManager::getInstance()->getConvertSourceFile());

    QFile asciiFile(OptManager::getInstance()->getConvertDestFile());
    if(!asciiFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //Error output
        exit(0);
    }

    for(int num = 0;num< qfile.sizeFilter();num++)
    {

        /* get message form log file */
        data = qfile.getMsgFilter(num);
        msg.setMsg(data);

        /* decode message is necessary */
        iterateDecodersForMsg(msg,1);

        /* get message ASCII text */
        text.clear();
        text += QString("%1 ").arg(qfile.getMsgFilterPos(num));
        text += msg.toStringHeader();
        text += " ";
        text += msg.toStringPayload();
        text += "\n";

        /* write to file */
        asciiFile.write(text.toAscii().constData());

    }

    asciiFile.close();
}

void MainWindow::commandLineExecutePlugin(QString plugin, QString cmd, QStringList params)
{
    bool plugin_found = false;
    for(int i = 0;i < project.plugin->topLevelItemCount();i++)
    {
        PluginItem *item = (PluginItem*)project.plugin->topLevelItem(i);
        if(item->getName() == plugin)
        {
            plugin_found = true;
            /* Check that this is a command plugin */
            QDltPluginCommandInterface *cmdif = item->plugincommandinterface;
            if(cmdif == NULL)
            {
                QMessageBox::critical(this, "Error", plugin + " is not a command plugin.");
                exit(-1);
            }
            if(!cmdif->command(cmd, params))
            {
                // Show error
                QMessageBox::warning(this, "Error", plugin);
                return;
            }

            // Show progress dialog
            QProgressDialog progress(plugin, "Cancel", 0, 100, this);
            progress.show();
            while(cmdif->commandProgress() < 100)
            {
                progress.setValue(cmdif->commandProgress());
                if(progress.wasCanceled())
                {
                    cmdif->cancel();
                    return;
                }
                // TODO: This caused crashing someplace else. Use caution.
                QApplication::processEvents();
            }
            progress.close();

            // Show return value
            QMessageBox::information(this, plugin, cmdif->commandReturnValue());
        }
    }
    if(!plugin_found)
    {
        QMessageBox::critical(this, "Error", plugin + " is not a command plugin.");
        exit(-1);
    }
    exit(0);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings->writeSettings(this);

    QMainWindow::closeEvent(event);
}

void MainWindow::on_action_menuFile_New_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("New DLT Log file"), workingDirectory, tr("DLT Files (*.dlt);;All files (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    /* close existing file */
    if(outputfile.isOpen())
        outputfile.close();

    /* create new file; truncate if already exist */
    outputfile.setFileName(fileName);
    setCurrentFile(fileName);
    if(outputfile.open(QIODevice::WriteOnly|QIODevice::Truncate))
        reloadLogFile();
    else
        QMessageBox::critical(0, QString("DLT Viewer"),
                              QString("Cannot create new log file \"%1\"\n%2")
                              .arg(fileName)
                              .arg(outputfile.errorString()));
}

void MainWindow::on_action_menuFile_Open_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open DLT Log file"), workingDirectory, tr("DLT Files (*.dlt);;All files (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    openDltFile(fileName);

    searchDlg->setMatch(false);
    searchDlg->setOnceClicked(false);
    searchDlg->setStartLine(-1);
}

void MainWindow::openDltFile(QString fileName)
{
    /* close existing file */
    if(outputfile.isOpen())
        outputfile.close();

    /* open existing file and append new data */
    outputfile.setFileName(fileName);
    setCurrentFile(fileName);
    if(outputfile.open(QIODevice::WriteOnly|QIODevice::Append))
        reloadLogFile();
    else
        QMessageBox::critical(0, QString("DLT Viewer"),
                              QString("Cannot open log file \"%1\"\n%2")
                              .arg(fileName)
                              .arg(outputfile.errorString()));
}

void MainWindow::on_action_menuFile_Import_DLT_Stream_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Import DLT Stream"), workingDirectory, tr("DLT Stream file (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    if(!outputfile.isOpen())
        return;

    DltFile importfile;

    dlt_file_init(&importfile,0);

    /* open DLT stream file */
    dlt_file_open(&importfile,fileName.toAscii(),0);

    /* parse and build index of complete log file and show progress */
    while (dlt_file_read_raw(&importfile,false,0)>=0)
    {
        outputfile.write((char*)importfile.msg.headerbuffer,importfile.msg.headersize);
        outputfile.write((char*)importfile.msg.databuffer,importfile.msg.datasize);
        outputfile.flush();

    }

    dlt_file_free(&importfile,0);

    if(importfile.error_messages>0)
    {
        QMessageBox::warning(this, QString("DLT Stream import"),
                             QString("At least %1 corrupted messages during import found!").arg(importfile.error_messages));
    }

    reloadLogFile();

}

void MainWindow::on_action_menuFile_Import_DLT_Stream_with_Serial_Header_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Import DLT Stream with serial header"), workingDirectory, tr("DLT Stream file (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    if(!outputfile.isOpen())
        return;

    DltFile importfile;

    dlt_file_init(&importfile,0);

    /* open DLT stream file */
    dlt_file_open(&importfile,fileName.toAscii(),0);

    /* parse and build index of complete log file and show progress */
    while (dlt_file_read_raw(&importfile,true,0)>=0)
    {
        outputfile.write((char*)importfile.msg.headerbuffer,importfile.msg.headersize);
        outputfile.write((char*)importfile.msg.databuffer,importfile.msg.datasize);
        outputfile.flush();

    }

    dlt_file_free(&importfile,0);

    if(importfile.error_messages>0)
    {
        QMessageBox::warning(this, QString("Import DLT Stream with serial header"),
                             QString("%1 corrupted messages during import found!").arg(importfile.error_messages));
    }

    reloadLogFile();
}

void MainWindow::on_action_menuFile_Append_DLT_File_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Append DLT File"), workingDirectory, tr("DLT File (*.dlt)"));

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    if(!outputfile.isOpen())
        return;

    DltFile importfile;

    dlt_file_init(&importfile,0);

    QProgressDialog progress("Append log file", "Cancel Loading", 0, 100, this);
    int num = 0;

    /* open DLT log file with same filename as output file */
    if (dlt_file_open(&importfile,fileName.toAscii() ,0)<0)
    {
        return;
    }

    /* get number of files in DLT log file */
    while (dlt_file_read(&importfile,0)>=0)
    {
        num++;
        if ((num%1000) == 0)
            progress.setValue(importfile.file_position*100/importfile.file_length);
        if (progress.wasCanceled())
        {
            dlt_file_free(&importfile,0);
            return;
        }
    }

    /* read DLT messages and append to current output file */
    for(int pos = 0 ; pos<num ; pos++)
    {
        if ((pos%1000) == 0)
            progress.setValue(pos*100/num);
        if (progress.wasCanceled())
        {
            dlt_file_free(&importfile,0);
            reloadLogFile();
            return;
        }
        dlt_file_message(&importfile,pos,0);
        outputfile.write((char*)importfile.msg.headerbuffer,importfile.msg.headersize);
        outputfile.write((char*)importfile.msg.databuffer,importfile.msg.datasize);
    }
    outputfile.flush();

    dlt_file_free(&importfile,0);

    /* reload log file */
    reloadLogFile();

}

void MainWindow::on_action_menuFile_Export_ASCII_triggered()
{
    QDltMsg msg;
    QByteArray data;
    QString text;
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export to ASCII"), workingDirectory, tr("ASCII Files (*.txt);;All files (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    QFile outfile(fileName);
    if(!outfile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QProgressDialog fileprogress("Export to ASCII...", "Cancel", 0, qfile.sizeFilter(), this);
    fileprogress.setWindowTitle("DLT Viewer");
    fileprogress.setWindowModality(Qt::WindowModal);
    fileprogress.show();
    const int qsz = qfile.sizeFilter();
    for(int num = 0;num< qsz;num++)
    {
        if((num%(qsz/300))==0)
            fileprogress.setValue(num);

        /* get message form log file */
        data = qfile.getMsgFilter(num);
        msg.setMsg(data);

        /* decode message is necessary */
        iterateDecodersForMsg(msg,1);

        /* get message ASCII text */
        text.clear();
        text += QString("%1 ").arg(qfile.getMsgFilterPos(num));
        text += msg.toStringHeader();
        text += " ";
        text += msg.toStringPayload();
        text += "\n";

        /* write to file */
        outfile.write(text.toAscii().constData());

    }

    outfile.close();
}

void MainWindow::on_action_menuFile_Export_Selection_triggered()
{
    exportSelection(false,true);
}

void MainWindow::on_action_menuFile_Export_Selection_ASCII_triggered()
{
    exportSelection(true,true);
}

void MainWindow::exportSelection(bool ascii = true,bool file = false)
{
    QModelIndexList list = ui->tableView->selectionModel()->selection().indexes();
    QDltMsg msg;
    QByteArray data;
    QString textExport;
    QString text;

    if(list.count()<=0)
    {
        QMessageBox::critical(0, QString("DLT Viewer"),
                              QString("No messages selected"));
        return;
    }

    QString fileName;

    if(file)
    {
        if(ascii)
        {
            fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export Selection"), workingDirectory, tr("Text Files (*.txt)"));
        }
        else
        {
            fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export Selection"), workingDirectory, tr("DLT Files (*.dlt)"));
        }
        if(fileName.isEmpty())
            return;
    }

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    QFile outfile(fileName);
    if(file)
    {
        if(!outfile.open(QIODevice::WriteOnly))
            return;
    }

    QProgressDialog fileprogress("Export...", "Cancel", 0, list.count(), this);
    fileprogress.setWindowTitle("DLT Viewer");
    fileprogress.setWindowModality(Qt::WindowModal);
    fileprogress.show();
    for(int num=0; num < list.count();num++)
    {
        if((num%(list.count()/300))==0)
            fileprogress.setValue(num);

        QModelIndex index = list[num];

        /* get the message with the selected item id */
        if(index.column()==0)
        {
            data = qfile.getMsgFilter(index.row());

            if(ascii)
            {
                msg.setMsg(data);

                /* decode message is necessary */
                iterateDecodersForMsg(msg,1);

                /* get message ASCII text */
                text.clear();
                text += QString("%1 ").arg(qfile.getMsgFilterPos(index.row()));
                text += msg.toStringHeader();
                text += " ";
                text += msg.toStringPayload();
                text += "\n";

                if(file)
                {
                    // write to file
                    outfile.write(text.toAscii().constData());
                }
                else
                {
                    // write to clipboard
                    textExport += text;
                }
            }
            else
            {
                if(file)
                {
                    // write to file
                    outfile.write(data);
                }
                else
                {
                    // write to clipboard
                }
            }
        }
    }

    if(file)
    {
        outfile.close();
    }
    else
    {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(textExport);
    }

}

void MainWindow::on_action_menuFile_SaveAs_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save DLT Log file"), workingDirectory, tr("DLT Files (*.dlt);;All files (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    qfile.close();
    outputfile.close();

    bool success = true;
    QFile sourceFile( outputfile.fileName() );
    QFile destFile( fileName );
    success &= sourceFile.open( QFile::ReadOnly );
    success &= destFile.open( QFile::WriteOnly | QFile::Truncate );
    success &= destFile.write( sourceFile.readAll() ) >= 0;
    sourceFile.close();
    destFile.close();

    if(!success)
    {
        QMessageBox::critical(0, QString("DLT Viewer"),
                              QString("Save as failed!"));

        return;
    }

    outputfile.setFileName(fileName);
    setCurrentFile(fileName);
    if(outputfile.open(QIODevice::WriteOnly|QIODevice::Append))
        reloadLogFile();
    else
        QMessageBox::critical(0, QString("DLT Viewer"),
                              QString("Cannot rename log file \"%1\"\n%2")
                              .arg(fileName)
                              .arg(outputfile.errorString()));
}

void MainWindow::on_action_menuFile_Clear_triggered()
{

    int ret = QMessageBox::question(this, tr("Please confirm"),
                                    tr("Are you sure to clear the table (the dlt file will be cleared too)?"),
                                    QMessageBox::Cancel | QMessageBox::Ok);

    switch (ret) {
    case QMessageBox::Cancel:
        // Cancel was clicked
        break;
    case QMessageBox::Ok:

        /* close existing file */
        if(outputfile.isOpen())
            outputfile.close();

        /* reopen file and truncate */
        if(outputfile.open(QIODevice::WriteOnly|QIODevice::Truncate))
        {
            reloadLogFile();
            //ui->textBrowser->setText("");
        }
        else
            QMessageBox::critical(0, QString("DLT Viewer"),
                                  QString("Cannot clear log file \"%1\"\n%2")
                                  .arg(outputfile.fileName())
                                  .arg(outputfile.errorString()));
        break;
    default:
        // should never be reached
        break;
    }
}



void MainWindow::threadpluginFinished(){
    threadIsRunnging = false;
}

void MainWindow::reloadLogFile()
{

    PluginItem *item = 0;
    QList<PluginItem*> activeViewerPlugins;
    QList<PluginItem*> activeDecoderPlugins;

#ifdef DEBUG_PERFORMANCE
    QTime t;
#endif

    QProgressDialog fileprogress("Parsing DLT file...", "Cancel", 0, 0, this);
    fileprogress.setWindowTitle("DLT Viewer");
    fileprogress.setWindowModality(Qt::WindowModal);

    QList<QPushButton *> fileprogressButtons =fileprogress.findChildren<QPushButton *>();
    fileprogressButtons.at(0)->setEnabled(false);

    fileprogress.show();

    qfile.open(outputfile.fileName());
    qfile.clearIndex();

    ThreadDltIndex threadDltIndex;
    QString filename = outputfile.fileName();
    threadDltIndex.setFilename(filename);

    connect(&threadDltIndex, SIGNAL(updateProgressText(QString)), &fileprogress, SLOT(setLabelText(QString)));
    connect(&threadDltIndex, SIGNAL(finished()), this, SLOT(threadpluginFinished()));

    threadIsRunnging = true;

#ifdef DEBUG_PERFORMANCE
    t.start();
#endif

    /* Using now seperate thread to create DLT index which is faster.
       To use old behaviour, use methode qfile.createIndex.*/

    //qfile.createIndex();

    /* ----> Thread usage to create DLT index starts here <---- */
    threadDltIndex.start();
    threadDltIndex.setPriority(QThread::HighestPriority);

    while(threadIsRunnging){
        QApplication::processEvents();
    }

#ifdef DEBUG_PERFORMANCE
    qDebug() << "Time to create index: " << t.elapsed()/1000 << "s" ;
#endif

    QList<unsigned long> indexDltList = threadDltIndex.getIndexAll();
    qfile.setDltIndex(indexDltList);
    /* ----> Thread usage to create DLT index ends here <---- */


    fileprogress.setMaximum(qfile.size());
    fileprogressButtons.at(0)->setEnabled(true);


    for(int i = 0; i < project.plugin->topLevelItemCount(); i++)
    {
        item = (PluginItem*)project.plugin->topLevelItem(i);

        if(item->getMode() != PluginItem::ModeDisable)
        {
            if(item->plugindecoderinterface)
            {
                activeDecoderPlugins.append(item);
            }
            if(item->pluginviewerinterface)
            {

#ifdef DEBUG_PERFORMANCE
                t.start();
#endif
                item->pluginviewerinterface->initFileStart(&qfile);

#ifdef DEBUG_PERFORMANCE
                qDebug() << "Time for initFileStart: " << item->getName() << ": " << t.elapsed()/1000 << "s" ;
#endif
                activeViewerPlugins.append(item);
            }
        }
    }

    // Possible to use several threads to process the splitted  trace file
    // But this is slower than using one thread.
    // Reason: Random IO on disk is very slow
    // Example to use more than one threads
    //int threadCount = 1;
    //if(qfile.size()!=0)
    //    threadCount = QThread::idealThreadCount();
    //if(threadCount < 1)
    //        threadCount = 1;
    //qDebug()<<"Threads: "<<threadCount;
    //int chunkSize = qfile.size() / threadCount;
    //qDebug()<<"messages: " <<  qfile.size() << "chunkSize: " << chunkSize;

    if(activeViewerPlugins.isEmpty() && activeDecoderPlugins.isEmpty())
    {
        fileprogress.cancel();
        qDebug() << "No plugins active - no thread to process messages started";
    }
    else
    {

        fileprogress.setLabelText(QString("Applying Plugins for Message 0/%1").arg(qfile.size()));

        ThreadPlugin thread;
        thread.setQDltFile(&qfile);
        thread.setActiveDecoderPlugins(&activeDecoderPlugins);
        thread.setActiveViewerPlugins(&activeViewerPlugins);
        thread.setStartIndex(0);
        thread.setStopIndex( qfile.size());

        connect(&thread, SIGNAL(percentageComplete(int)), &fileprogress, SLOT(setValue(int)));
        connect(&thread, SIGNAL(updateProgressText(QString)), &fileprogress, SLOT(setLabelText(QString)));
        connect(&thread, SIGNAL(finished()), this, SLOT(threadpluginFinished()));
        connect(&fileprogress, SIGNAL(canceled()), &thread, SLOT(stopProcessMsg()));

        threadIsRunnging = true;

#ifdef DEBUG_PERFORMANCE
        t.start();
#endif

        thread.start();
        thread.setPriority(QThread::HighestPriority);

        while(threadIsRunnging){
            QApplication::processEvents();
        }

#ifdef DEBUG_PERFORMANCE
        qDebug() << "Time to initMsg,isMsg,decodeMsg,checkFilter,initMsgDecoded: " << t.elapsed()/1000 << "s" ;
#endif

        for(int i = 0; i < activeViewerPlugins.size(); i++){
            item = (PluginItem*)activeViewerPlugins.at(i);

#ifdef DEBUG_PERFORMANCE
            t.start();
#endif

            item->pluginviewerinterface->initFileFinish();

#ifdef DEBUG_PERFORMANCE
            qDebug() << "Time for initFileFinish: " << item->getName() << ": " << t.elapsed()/1000 << "s" ;
#endif
        }

        if(fileprogress.wasCanceled())
        {
            QMessageBox::warning(this, tr("DLT Viewer"), tr("You canceled the initialisation progress. Not all messages could be processed by the enabled Plugins!"), QMessageBox::Ok);
        }
    }

    ui->tableView->selectionModel()->clear();
    tableModel->modelChanged();

    /* set name of opened log file in status bar */
    statusFilename->setText(outputfile.fileName());
}

void MainWindow::applySettings()
{
    QFont tableViewFont = ui->tableView->font();
    tableViewFont.setPointSize(settings->fontSize);
    ui->tableView->setFont(tableViewFont);
    // Rescale the height of a row to choosen font size + 8 pixels
    ui->tableView->verticalHeader()->setDefaultSectionSize(settings->fontSize+8);

    settings->showIndex?ui->tableView->showColumn(0):ui->tableView->hideColumn(0);
    settings->showTime?ui->tableView->showColumn(1):ui->tableView->hideColumn(1);
    settings->showTimestamp?ui->tableView->showColumn(2):ui->tableView->hideColumn(2);
    settings->showCount?ui->tableView->showColumn(3):ui->tableView->hideColumn(3);

    settings->showEcuId?ui->tableView->showColumn(4):ui->tableView->hideColumn(4);
    settings->showApId?ui->tableView->showColumn(5):ui->tableView->hideColumn(5);
    settings->showType?ui->tableView->showColumn(7):ui->tableView->hideColumn(7);

    settings->showSubtype?ui->tableView->showColumn(8):ui->tableView->hideColumn(8);
    settings->showMode?ui->tableView->showColumn(9):ui->tableView->hideColumn(9);
    settings->showNoar?ui->tableView->showColumn(10):ui->tableView->hideColumn(10);
    settings->showPayload?ui->tableView->showColumn(11):ui->tableView->hideColumn(11);

}

void MainWindow::on_action_menuFile_Settings_triggered()
{
    /* show settings dialog */
    settings->writeDlg();
    settings->workingDirectory = workingDirectory;

    if(settings->exec()==1)
    {
        /* change settings and store settings persistently */
        settings->readDlg();
        settings->writeSettings(this);
        workingDirectory = settings->workingDirectory;

        /* Apply settings to table */
        applySettings();

        updateScrollButton();
    }
}

void MainWindow::on_action_menuFile_Quit_triggered()
{
    /* TODO: Add quit code here */
    this->close();

}


void MainWindow::on_action_menuProject_New_triggered()
{
    /* TODO: Ask for saving project if changed */

    /* create new project */

    this->setWindowTitle(QString("DLT Viewer - unnamed project - Version : %1 %2").arg(PACKAGE_VERSION).arg(PACKAGE_VERSION_STATE));
    project.Clear();

    /* Update the ECU list in control plugins */
    updatePluginsECUList();

}

void MainWindow::on_action_menuProject_Open_triggered()
{
    /* TODO: Ask for saving project if changed */

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open DLT Project file"), workingDirectory, tr("DLT Project Files (*.dlp);;All files (*.*)"));

    /* open existing project */
    if(!fileName.isEmpty())
    {
        /* change current working directory */
        workingDirectory = QFileInfo(fileName).absolutePath();

        openDlpFile(fileName);
    }

}

bool MainWindow::openDlpFile(QString fileName)
{
    /* Open existing project */
    if(project.Load(fileName))
    {
        /* Applies project settings and save it to registry */
        applySettings();
        settings->writeSettings(this);

        this->setWindowTitle(QString("DLT Viewer - "+fileName+" - Version : %1 %2").arg(PACKAGE_VERSION).arg(PACKAGE_VERSION_STATE));

        /* Load the plugins description files after loading project */
        updatePlugins();

        setCurrentProject(fileName);

        /* Update the ECU list in control plugins */
        updatePluginsECUList();

        /* After loading the project file update the filters */
        filterUpdate();

        return true;
    } else {
        return false;
    }
}

void MainWindow::on_action_menuProject_Save_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save DLT Project file"), workingDirectory, tr("DLT Project Files (*.dlp);;All files (*.*)"));

    /* save project */
    if(!fileName.isEmpty() && project.Save(fileName))
    {
        /* change current working directory */
        workingDirectory = QFileInfo(fileName).absolutePath();

        this->setWindowTitle(QString("DLT Viewer - "+fileName+" - Version : %1 %2").arg(PACKAGE_VERSION).arg(PACKAGE_VERSION_STATE));

        setCurrentProject(fileName);
    }
}

QStringList MainWindow::getSerialPortsWithQextEnumartor(){

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    QStringList portList;
#ifdef Q_OS_WIN
    for (int i = 0; i < ports.size(); i++) {
        portList << ports.at(i).portName;
    }
#else
    for (int i = 0; i < ports.size(); i++) {
        portList << ports.at(i).physName;
    }
#endif
    return portList;
}

void MainWindow::on_action_menuConfig_ECU_Add_triggered()
{   
    QStringList hostnameListPreset;
    hostnameListPreset << "localhost";

    QStringList portListPreset = getSerialPortsWithQextEnumartor();

    /* show ECU configuration dialog */
    EcuDialog dlg("ECU","A new ECU",0,"localhost",DLT_DAEMON_TCP_PORT,"COM0",BAUD115200,DLT_LOG_INFO,DLT_TRACE_STATUS_OFF,1,
                  false,true,false,true,false,false,true,true,5);

    /* Read settings for recent hostnames and ports */
    recentHostnames = DltSettingsManager::getInstance()->value("other/recentHostnameList",hostnameListPreset).toStringList();
    recentPorts = DltSettingsManager::getInstance()->value("other/recentPortList",portListPreset).toStringList();

    dlg.setHostnameList(recentHostnames);
    dlg.setPortList(recentPorts);

    if(dlg.exec()==1)
    {
        /* add new ECU to configuration */
        EcuItem* ecuitem = new EcuItem(0);
        dlg.setDialogToEcuItem(ecuitem);

        /* update ECU item */
        ecuitem->update();

        /* add ECU to configuration */
        project.ecu->addTopLevelItem(ecuitem);

        /* Update settings for recent hostnames and ports */
        setCurrentHostname(ecuitem->hostname);
        setCurrentPort(ecuitem->port);

        /* Update the ECU list in control plugins */
        updatePluginsECUList();

        for(int pnum = 0; pnum < project.plugin->topLevelItemCount (); pnum++) {
            PluginItem *item = (PluginItem*)project.plugin->topLevelItem(pnum);

            if(item->plugincontrolinterface)
            {
                item->plugincontrolinterface->stateChanged(project.ecu->indexOfTopLevelItem(ecuitem), QDltConnection::QDltConnectionOffline);
            }

        }
    }
}

void MainWindow::on_action_menuConfig_ECU_Edit_triggered()
{
    /* find selected ECU in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        QStringList hostnameListPreset;
        hostnameListPreset << "localhost";

        QStringList portListPreset = getSerialPortsWithQextEnumartor();

        EcuItem* ecuitem = (EcuItem*) list.at(0);

        /* show ECU configuration dialog */
        EcuDialog dlg(ecuitem->id,ecuitem->description,ecuitem->interfacetype,ecuitem->hostname,ecuitem->tcpport,ecuitem->port,ecuitem->baudrate,
                      ecuitem->loglevel,ecuitem->tracestatus,ecuitem->verbosemode,ecuitem->sendSerialHeaderTcp,ecuitem->sendSerialHeaderSerial,ecuitem->syncSerialHeaderTcp,ecuitem->syncSerialHeaderSerial,
                      ecuitem->timingPackets,ecuitem->sendGetLogInfo,ecuitem->updateDataIfOnline,ecuitem->autoReconnect,ecuitem->autoReconnectTimeout);

        /* Read settings for recent hostnames and ports */
        recentHostnames = DltSettingsManager::getInstance()->value("other/recentHostnameList",hostnameListPreset).toStringList();
        recentPorts = DltSettingsManager::getInstance()->value("other/recentPortList",portListPreset).toStringList();

        setCurrentHostname(ecuitem->hostname);
        setCurrentPort(ecuitem->port);

        dlg.setHostnameList(recentHostnames);
        dlg.setPortList(recentPorts);

        if(dlg.exec())
        {
            bool interfaceChanged = false;
            if((ecuitem->interfacetype != dlg.interfacetype() ||
                ecuitem->hostname != dlg.hostname() ||
                ecuitem->tcpport != dlg.tcpport() ||
                ecuitem->port != dlg.port() ||
                ecuitem->baudrate != dlg.baudrate()) &&
                    ecuitem->tryToConnect)
            {
                interfaceChanged = true;
                disconnectECU(ecuitem);
            }

            dlg.setDialogToEcuItem(ecuitem);

            /* update ECU item */
            ecuitem->update();

            /* if interface settings changed, reconnect */
            if(interfaceChanged)
            {
                connectECU(ecuitem);
            }

            /* send new default log level to ECU, if connected and if selected in dlg */
            if(ecuitem->connected && ecuitem->updateDataIfOnline)
            {
                sendUpdates(ecuitem);
            }

            /* Update settings for recent hostnames and ports */
            setCurrentHostname(ecuitem->hostname);
            setCurrentPort(ecuitem->port);

            /* Update the ECU list in control plugins */
            updatePluginsECUList();

        }
    }
}

void MainWindow::on_action_menuConfig_ECU_Delete_triggered()
{
    /* find selected ECU in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        /* disconnect, if connected */
        disconnectECU((EcuItem*)list.at(0));

        /* delete ECU from configuration */
        delete project.ecu->takeTopLevelItem(project.ecu->indexOfTopLevelItem(list.at(0)));

        /* Update the ECU list in control plugins */
        updatePluginsECUList();
    }
}

void MainWindow::on_action_menuConfig_Delete_All_Contexts_triggered()
{
    /* find selected ECU in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        /* delete all applications from ECU from configuration */
        EcuItem* ecuitem = (EcuItem*) list.at(0);

        (ecuitem->takeChildren()).clear();
    }
}

void MainWindow::on_action_menuConfig_Application_Add_triggered()
{
    /* find selected ECU in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        /* show Application configuration dialog */
        ApplicationDialog dlg("APP","A new Application");
        EcuItem* ecuitem = (EcuItem*)list.at(0);
        if(dlg.exec()==1)
        {
            /* change settings of application configuration */
            ApplicationItem* appitem = new ApplicationItem(ecuitem);
            appitem->id = dlg.id();
            appitem->description = dlg.description();

            /* update application item */
            appitem->update();

            /* add new application to ECU */
            ecuitem->addChild(appitem);
        }
    }
}

void MainWindow::on_action_menuConfig_Application_Edit_triggered()
{
    /* find selected application in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == application_type))
    {
        ApplicationItem* appitem = (ApplicationItem*) list.at(0);

        /* show Application configuration dialog */
        ApplicationDialog dlg(appitem->id,appitem->description);
        if(dlg.exec())
        {
            appitem->id = dlg.id();
            appitem->description = dlg.description();

            /* update application item */
            appitem->update();
        }
    }

}

void MainWindow::on_action_menuConfig_Application_Delete_triggered()
{
    /* find selected application in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == application_type))
    {
        ApplicationItem* appitem = (ApplicationItem*) list.at(0);

        /* remove application */
        delete appitem->parent()->takeChild(appitem->parent()->indexOfChild(appitem));
    }

}

void MainWindow::on_action_menuConfig_Context_Add_triggered()
{
    /* find selected application in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == application_type))
    {

        /* show Context configuration dialog */
        ContextDialog dlg("CON","A new Context",-1,-1);
        ApplicationItem* appitem = (ApplicationItem*)list.at(0);
        if(dlg.exec()==1)
        {
            ContextItem* conitem = new ContextItem(appitem);
            conitem->id = dlg.id();
            conitem->description = dlg.description();
            conitem->loglevel = dlg.loglevel();
            conitem->tracestatus = dlg.tracestatus();

            /* update context item */
            conitem->update();

            /* add new context to application */
            appitem->addChild(conitem);

            /* send new default log level to ECU, if connected and if selected in dlg */
            if(dlg.update())
            {
                EcuItem* ecuitem = (EcuItem*) appitem->parent();
                controlMessage_SetLogLevel(ecuitem,appitem->id,conitem->id,conitem->loglevel);
                controlMessage_SetTraceStatus(ecuitem,appitem->id,conitem->id,conitem->tracestatus);

                /* update status */
                conitem->status = ContextItem::valid;
                conitem->update();
            }
        }
    }


}

void MainWindow::on_action_menuConfig_Context_Edit_triggered()
{
    /* find selected context in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == context_type))
    {
        ContextItem* conitem = (ContextItem*) list.at(0);

        /* show Context configuration dialog */
        ContextDialog dlg(conitem->id,conitem->description,conitem->loglevel,conitem->tracestatus);
        if(dlg.exec())
        {
            conitem->id = dlg.id();
            conitem->description = dlg.description();
            conitem->loglevel = dlg.loglevel();
            conitem->tracestatus = dlg.tracestatus();

            /* update context item */
            conitem->update();

            /* send new log level to ECU, if connected and if selected in dlg */
            if(dlg.update())
            {
                ApplicationItem* appitem = (ApplicationItem*) conitem->parent();
                EcuItem* ecuitem = (EcuItem*) appitem->parent();

                if(ecuitem->connected)
                {
                    controlMessage_SetLogLevel(ecuitem,appitem->id,conitem->id,conitem->loglevel);
                    controlMessage_SetTraceStatus(ecuitem,appitem->id,conitem->id,conitem->tracestatus);

                    /* update status */
                    conitem->status = ContextItem::valid;
                    conitem->update();
                }
            }
        }
    }
}

void MainWindow::on_action_menuDLT_Edit_All_Log_Levels_triggered()
{

    MultipleContextDialog dlg(0,0);

    if(dlg.exec())
    {

        QList<QTreeWidgetItem *> list = project.ecu->selectedItems();

        if(list.at(0)->type() == context_type){
            //Nothing to do
        }

        if(list.at(0)->type() == application_type){
            //qDebug()<<"Application selected";
            ApplicationItem *applicationItem;
            for(int i=0; i<list.count(); i++){
                applicationItem = (ApplicationItem*) list.at(i);
                ContextItem *contextItem;
                for(int j=0; j<applicationItem->childCount();j++){
                    contextItem = (ContextItem*)applicationItem->child(j);
                    contextItem->setSelected(true);
                }
                applicationItem->setSelected(false);
            }
        }


        if(list.at(0)->type() == ecu_type){
            //qDebug()<<"ECU selected";
            EcuItem *ecuItem;
            for(int i=0; i<list.count(); i++){
                ecuItem = (EcuItem*) list.at(i);
                ApplicationItem *applicationItem;
                for(int j=0; j<ecuItem->childCount(); j++){
                    applicationItem = (ApplicationItem*) ecuItem->child(j);
                    ContextItem *contextItem;
                    for(int k=0; k<applicationItem->childCount();k++){
                        contextItem = (ContextItem*)applicationItem->child(k);
                        contextItem->setSelected(true);
                    }

                }
                ecuItem->setSelected(false);
            }
        }

        list = project.ecu->selectedItems();

        if((list.count() >= 1))
        {
            ContextItem *conitem;
            for(int i=0; i<list.count(); i++){
                if(list.at(i)->type() == context_type){

                    conitem = (ContextItem*) list.at(i);

                    conitem->loglevel = dlg.loglevel();
                    conitem->tracestatus = dlg.tracestatus();

                    /* update context item */
                    conitem->update();

                    /* send new log level to ECU, if connected and if selected in dlg */
                    if(dlg.update())
                    {
                        ApplicationItem* appitem = (ApplicationItem*) conitem->parent();
                        EcuItem* ecuitem = (EcuItem*) appitem->parent();

                        if(ecuitem->connected)
                        {
                            controlMessage_SetLogLevel(ecuitem,appitem->id,conitem->id,conitem->loglevel);
                            controlMessage_SetTraceStatus(ecuitem,appitem->id,conitem->id,conitem->tracestatus);

                            /* update status */
                            conitem->status = ContextItem::valid;
                            conitem->update();
                        }
                    }
                    conitem->setSelected(false);
                }
            }

        }
    }
}

void MainWindow::on_action_menuConfig_Context_Delete_triggered()
{
    /* find selected context in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == context_type))
    {
        ContextItem* conitem = (ContextItem*) list.at(0);

        /* delete context from application */
        delete conitem->parent()->takeChild(conitem->parent()->indexOfChild(conitem));
    }

}


void MainWindow::on_configWidget_customContextMenuRequested(QPoint pos)
{

    /* show custom pop menu  for configuration */
    QPoint globalPos = ui->configWidget->mapToGlobal(pos);
    QMenu menu(project.ecu);
    QAction *action;
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();

    if(list.count() > 1 && (list.at(0)->type() == context_type))
    {
        action = new QAction("&Edit All Log Levels...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Edit_All_Log_Levels_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("DLT &Set Log Levels...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Set_Log_Level_triggered()));
        menu.addAction(action);
    }
    else if((list.count() > 1) && (list.at(0)->type() == ecu_type))
    {
        action = new QAction("&Edit All Log Levels...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Edit_All_Log_Levels_triggered()));
        menu.addAction(action);
    }
    else if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        /* ECU is selected */

        action = new QAction("ECU Add...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_ECU_Add_triggered()));
        menu.addAction(action);

        action = new QAction("ECU Edit...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_ECU_Edit_triggered()));
        menu.addAction(action);

        action = new QAction("ECU Delete", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_ECU_Delete_triggered()));
        menu.addAction(action);

        action = new QAction("&ECU Edit All Log Levels...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Edit_All_Log_Levels_triggered()));
        menu.addAction(action);

        action = new QAction("ECU Delete All Contexts", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Delete_All_Contexts_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("Application Add...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Application_Add_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("ECU Connect", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Connect_triggered()));
        menu.addAction(action);

        action = new QAction("ECU Disconnect", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Disconnect_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("Expand All ECUs", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Expand_All_ECUs_triggered()));
        menu.addAction(action);

        action = new QAction("Collapse All ECUs", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Collapse_All_ECUs_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("DLT Get Log Info", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Get_Log_Info_triggered()));
        menu.addAction(action);

        action = new QAction("DLT Set All Log Levels", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Set_All_Log_Levels_triggered()));
        menu.addAction(action);

        action = new QAction("DLT Get Default Log Level", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Get_Default_Log_Level_triggered()));
        menu.addAction(action);

        action = new QAction("DLT Set Default Log Level", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Set_Default_Log_Level_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("Store Config", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Store_Config_triggered()));
        menu.addAction(action);

        action = new QAction("Reset to Factory Default", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Reset_to_Factory_Default_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("Send Injection...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Send_Injection_triggered()));
        menu.addAction(action);

        action = new QAction("Get Software Version", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Get_Software_Version_triggered()));
        menu.addAction(action);

        action = new QAction("Get Local Time", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Get_Local_Time_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("&Filter Add", this);
        connect(action, SIGNAL(triggered()), this, SLOT(filterAdd()));
        menu.addAction(action);




    }
    else if((list.count() > 1) && (list.at(0)->type() == application_type))
    {
        action = new QAction("&Edit All Log Levels...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Edit_All_Log_Levels_triggered()));
        menu.addAction(action);
    }
    else if((list.count() == 1) && (list.at(0)->type() == application_type))
    {
        /* Application is selected */

        action = new QAction("&Application Edit...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Application_Edit_triggered()));
        menu.addAction(action);

        action = new QAction("A&pplication Delete...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Application_Delete_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("&Context Add...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Context_Add_triggered()));
        menu.addAction(action);

        action = new QAction("&Edit All Log Levels...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Edit_All_Log_Levels_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("&Filter Add", this);
        connect(action, SIGNAL(triggered()), this, SLOT(filterAdd()));
        menu.addAction(action);


    }
    else if((list.count() == 1) && (list.at(0)->type() == context_type))
    {
        /* Context is selected */

        action = new QAction("&Context Edit...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Context_Edit_triggered()));
        menu.addAction(action);

        action = new QAction("C&ontext Delete...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Context_Delete_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("DLT &Set Log Level...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Set_Log_Level_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("&Filter Add", this);
        connect(action, SIGNAL(triggered()), this, SLOT(filterAdd()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("Send Injection...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Send_Injection_triggered()));
        menu.addAction(action);
    }
    else
    {
        /* nothing is selected */
        action = new QAction("ECU Add...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_ECU_Add_triggered()));
        menu.addAction(action);

    }

    /* show popup menu */
    menu.exec(globalPos);

}


void MainWindow::on_filterWidget_customContextMenuRequested(QPoint pos)
{

    /* show custom pop menu  for filter configuration */
    QPoint globalPos = ui->filterWidget->mapToGlobal(pos);
    QMenu menu(project.ecu);
    QAction *action;
    QList<QTreeWidgetItem *> list = project.filter->selectedItems();


    action = new QAction("Save Filter(s)...", this);
    if(project.filter->topLevelItemCount() <= 0)
        action->setEnabled(false);
    else
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Save_As_triggered()));
    menu.addAction(action);

    action = new QAction("Load Filter(s)...", this);
    connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Load_triggered()));
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("Filter Add...", this);
    connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Add_triggered()));
    menu.addAction(action);

    action = new QAction("Filter Edit...", this);
    if(list.size() != 1)
        action->setEnabled(false);
    else
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Edit_triggered()));
    menu.addAction(action);

    action = new QAction("Filter Delete", this);
    if(list.size() != 1)
        action->setEnabled(false);
    else
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Delete_triggered()));
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("Filter Duplicate...", this);
    if(list.size() != 1)
        action->setEnabled(false);
    else
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Duplicate_triggered()));
    menu.addAction(action);

    /* show popup menu */
    menu.exec(globalPos);

}

void MainWindow::on_pluginWidget_customContextMenuRequested(QPoint pos)
{
    /* show custom pop menu for plugin configuration */
    QPoint globalPos = ui->pluginWidget->mapToGlobal(pos);
    QMenu menu(project.ecu);
    QAction *action;
    QList<QTreeWidgetItem *> list = project.plugin->selectedItems();

    if((list.count() == 1) ) {
        PluginItem* item = (PluginItem*) list.at(0);

        action = new QAction("Plugin Edit...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuPlugin_Edit_triggered()));
        menu.addAction(action);
        if(item->pluginviewerinterface)
        {
            action = new QAction("Plugin Show", this);
            connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuPlugin_Show_triggered()));
            menu.addAction(action);
            action = new QAction("Plugin Hide", this);
            connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuPlugin_Hide_triggered()));
            menu.addAction(action);
        }
        if(item->plugincommandinterface)
        {
            action = new QAction("Execute Command...", this);
            connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuPlugin_ExecuteCommand_triggered()));
            menu.addAction(action);
        }
        menu.addSeparator();
        action = new QAction("Plugin Disable", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuPlugin_Disable_triggered()));
        menu.addAction(action);
        /* show popup menu */
        menu.exec(globalPos);
    }
}

void MainWindow::connectAll()
{
    if(project.ecu->topLevelItemCount() == 0)
    {
        on_action_menuConfig_ECU_Add_triggered();
    }

    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
        connectECU(ecuitem);
    }
}

void MainWindow::disconnectAll()
{
    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
        disconnectECU(ecuitem);
    }
}

void MainWindow::disconnectECU(EcuItem *ecuitem)
{
    if(ecuitem->tryToConnect == true)
    {
        /* disconnect from host */
        ecuitem->tryToConnect = false;
        ecuitem->connected = false;
        ecuitem->connectError.clear();
        ecuitem->update();
        on_configWidget_itemSelectionChanged();

        /* update conenction state */
        if(ecuitem->interfacetype == 0)
        {
            /* TCP */
            if (ecuitem->socket.state()!=QAbstractSocket::UnconnectedState)
                ecuitem->socket.disconnectFromHost();
        }
        else
        {
            /* Serial */
            ecuitem->serialport->close();
            delete ecuitem->serialport;
            ecuitem->serialport = 0;
        }

        ecuitem->InvalidAll();
    }
}

void MainWindow::on_action_menuConfig_Connect_triggered()
{
    /* get selected ECU from configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        EcuItem* ecuitem = (EcuItem*) list.at(0);

        /* connect to host */
        connectECU(ecuitem);
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No ECU selected in configuration!"));

}

void MainWindow::on_action_menuConfig_Disconnect_triggered()
{
    /* get selected ECU from configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        EcuItem* ecuitem = (EcuItem*) list.at(0);
        disconnectECU(ecuitem);
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No ECU selected in configuration!"));
}

void MainWindow::connectECU(EcuItem* ecuitem,bool force)
{
    if(ecuitem->tryToConnect == false || force)
    {
        ecuitem->tryToConnect = true;
        ecuitem->connected = false;
        //ecuitem->connectError.clear();
        ecuitem->update();
        on_configWidget_itemSelectionChanged();

        /* reset receive buffer */
        ecuitem->totalBytesRcvd = 0;
        ecuitem->totalBytesRcvdLastTimeout = 0;
        ecuitem->data.clear();

        /* start socket connection to host */
        if(ecuitem->interfacetype == 0)
        {
            /* TCP */
            /* connect socket signals with window slots */
            if (ecuitem->socket.state()==QAbstractSocket::UnconnectedState)
            {
                disconnect(&ecuitem->socket,0,0,0);
                connect(&ecuitem->socket,SIGNAL(connected()),this,SLOT(connected()));
                connect(&ecuitem->socket,SIGNAL(disconnected()),this,SLOT(disconnected()));
                connect(&ecuitem->socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error(QAbstractSocket::SocketError)));
                connect(&ecuitem->socket,SIGNAL(readyRead()),this,SLOT(readyRead()));
                connect(&ecuitem->socket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(stateChangedTCP(QAbstractSocket::SocketState)));

                ecuitem->socket.connectToHost(ecuitem->hostname,ecuitem->tcpport);
            }
        }
        else
        {
            /* Serial */
            if(!ecuitem->serialport)
            {
                PortSettings settings = {ecuitem->baudrate, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10}; //Before timeout was 1

                ecuitem->serialport = new QextSerialPort(ecuitem->port,settings);
                connect(ecuitem->serialport, SIGNAL(readyRead()), this, SLOT(readyRead()));
                connect(ecuitem->serialport,SIGNAL(dsrChanged(bool)),this,SLOT(stateChangedSerial(bool)));

            }

            if(ecuitem->serialport->isOpen())
            {
                ecuitem->serialport->close();
                ecuitem->serialport->setBaudRate(ecuitem->baudrate);
            }

            ecuitem->serialport->open(QIODevice::ReadWrite);

            if(ecuitem->serialport->isOpen())
            {
                ecuitem->connected = true;
                ecuitem->update();
                on_configWidget_itemSelectionChanged();

                /* send new default log level to ECU, if selected in dlg */
                if (ecuitem->updateDataIfOnline)
                {
                    sendUpdates(ecuitem);
                }

            }

        }

        if(  (settings->showCtId && settings->showCtIdDesc) || (settings->showApId && settings->showApIdDesc) ){
            controlMessage_GetLogInfo(ecuitem);
        }
    }
}

void MainWindow::connected()
{
    /* signal emited when connected to host */

    /* find socket which emited signal */
    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
        if( &(ecuitem->socket) == sender())
        {
            /* update connection state */
            ecuitem->connected = true;
            ecuitem->connectError.clear();
            ecuitem->update();
            on_configWidget_itemSelectionChanged();

            /* reset receive buffer */
            ecuitem->totalBytesRcvd = 0;
            ecuitem->totalBytesRcvdLastTimeout = 0;
            ecuitem->data.clear();
        }
    }
}

void MainWindow::disconnected()
{
    /* signal emited when disconnected to host */

    /* find socket which emited signal */
    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
        if( &(ecuitem->socket) == sender())
        {
            /* update connection state */
            ecuitem->connected = false;
            ecuitem->connectError.clear();
            ecuitem->InvalidAll();
            ecuitem->update();
            on_configWidget_itemSelectionChanged();

            /* disconnect socket signals from window slots */
            disconnect(&ecuitem->socket,0,0,0);
        }
    }
}


void MainWindow::timeout()
{
    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);

        if(ecuitem->isAutoReconnectTimeoutPassed())
        {
            //qDebug() << "totalBytesRcvd:"<<ecuitem->totalBytesRcvd << " - totalBytesRcvdLastTimeout:" << ecuitem->totalBytesRcvdLastTimeout;

            if(ecuitem->autoReconnect && ecuitem->totalBytesRcvd == ecuitem->totalBytesRcvdLastTimeout)
            {
                //qDebug() << "reconnect";
                disconnectECU(ecuitem);
                ecuitem->tryToConnect = true;
            }
            ecuitem->totalBytesRcvdLastTimeout = ecuitem->totalBytesRcvd;
        }

        if( ecuitem->tryToConnect && !ecuitem->connected)
        {
            connectECU(ecuitem,true);
        }
    }
}

void MainWindow::timeoutRead()
{
    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);

        if( ecuitem->connected)
        {
            read(ecuitem);
        }
    }

}

void MainWindow::error(QAbstractSocket::SocketError /* socketError */)
{
    /* signal emited when connection to host is not possible */

    /* find socket which emited signal */
    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
        if( &(ecuitem->socket) == sender())
        {
            /* save error */
            ecuitem->connectError = ecuitem->socket.errorString();

            if(ecuitem->interfacetype == 0)
            {
                /* disconnect socket */
                ecuitem->socket.disconnectFromHost();
            }

            /* update connection state */
            ecuitem->connected = false;
            ecuitem->update();

            //            for(int pnum = 0; pnum < project.plugin->topLevelItemCount (); pnum++) {
            //                PluginItem *item = (PluginItem*)project.plugin->topLevelItem(pnum);

            //                if(item->plugincontrolinterface)
            //                {
            //                        item->plugincontrolinterface->stateChanged(num,QDltConnection::QDltConnectionError);
            //                }
            //            }

            on_configWidget_itemSelectionChanged();
        }

    }
}

void MainWindow::readyRead()
{
    /* signal emited when socket received data */

    /* find socket which emited signal */
    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
        if( (&(ecuitem->socket) == sender()) || (ecuitem->serialport == sender()))
        {
            read(ecuitem);
        }
    }
}

/**
 * @brief MainWindow::skipSerialHeader
 * @param ecu
 * If buffer size in ecuitem exceeds DLT_BUFFER_CORRUPT_TRESHOLD
 * while using serial connection, this function is used to
 * skil one serial header and allow a resync
 */
void MainWindow::skipSerialHeader(EcuItem *ecu)
{
    const char *buf = ecu->data.data();
    unsigned int offset = 0;
    unsigned const int datalen = ecu->data.length();

    while(offset < datalen - sizeof(dltSerialHeader))
    {
        if(memcmp(buf, dltSerialHeader, sizeof(dltSerialHeader)) == 0)
        {
            ecu->data.remove(offset, sizeof(dltSerialHeader));
            return;
        }
    }
    // Unrecoverable error condition. Disconnect ECU
    QMessageBox::warning(this, "Buffer corrupted irrecoverably",
                         "Input buffer corrupted. An attempt was made to resynchronize it, but it failed. Buffer will be flushed.");
    ecu->data.clear();

}

void MainWindow::read(EcuItem* ecuitem)
{
    uint8_t *buf;
    int32_t bytesRcvd = 0;
    DltMessage msg;
    QDltMsg qmsg;
    PluginItem *item = 0;
    QList<PluginItem*> activeViewerPlugins;
    QList<PluginItem*> activeDecoderPlugins;

    if (!ecuitem)
        return;

    dlt_message_init(&msg,0);

    if(ecuitem->interfacetype == 0)
    {
        /* TCP */
        bytesRcvd = ecuitem->socket.bytesAvailable();
        ecuitem->data += ecuitem->socket.readAll();
    }
    else if(ecuitem->serialport)
    {
        /* serial */
        bytesRcvd = ecuitem->serialport->bytesAvailable();
        ecuitem->data += ecuitem->serialport->readAll();
    }
    buf = (uint8_t*) ecuitem->data.data();

    /* reading data; new data is added to the current buffer */
    if (bytesRcvd>0)
    {

        bytesRcvd = ecuitem->data.size();
        ecuitem->totalBytesRcvd += bytesRcvd;
        totalBytesRcvd += bytesRcvd;
        statusBytesReceived->setText(QString("Recv: %1").arg(totalBytesRcvd));

        /* read so many messages from the buffer as in the buffer; keep rest of the data in the buffer */
        while (dlt_message_read(&msg,buf,bytesRcvd,(ecuitem->interfacetype == 0)?ecuitem->syncSerialHeaderTcp:ecuitem->syncSerialHeaderSerial,0)==0)
        {
            /* prepare storage header */
            if (DLT_IS_HTYP_WEID(msg.standardheader->htyp))
                dlt_set_storageheader(msg.storageheader,msg.headerextra.ecu);
            else
                dlt_set_storageheader(msg.storageheader,ecuitem->id.toAscii());

            bool is_ctrl_msg = DLT_MSG_IS_CONTROL(&msg);

            /* check if message is matching the filter */
            if (outputfile.isOpen())
            {

                if ((settings->writeControl && is_ctrl_msg) || (!is_ctrl_msg))
                {
                    outputfile.write((char*)msg.headerbuffer,msg.headersize);
                    outputfile.write((char*)msg.databuffer,msg.datasize);
                    outputfile.flush();
                }
            }

            /* remove read message from buffer */
            if(msg.found_serialheader)
            {
                bytesRcvd = bytesRcvd - (msg.headersize+msg.datasize-sizeof(DltStorageHeader)+sizeof(dltSerialHeader));
                buf = buf + (msg.headersize+msg.datasize-sizeof(DltStorageHeader)+sizeof(dltSerialHeader));
            }
            else
            {
                bytesRcvd = bytesRcvd - (msg.headersize+msg.datasize-sizeof(DltStorageHeader));
                buf = buf + (msg.headersize+msg.datasize-sizeof(DltStorageHeader));
            }
            if(msg.resync_offset>0)
            {
                bytesRcvd -= msg.resync_offset;
                buf += msg.resync_offset;

                totalByteErrorsRcvd += msg.resync_offset;
                statusByteErrorsReceived->setText(QString("Recv Errors: %1").arg(totalByteErrorsRcvd));
            }

            /* analyse received message, check if DLT control message response */
            if ( is_ctrl_msg && DLT_MSG_IS_CONTROL_RESPONSE(&msg))
            {
                controlMessage_ReceiveControlMessage(ecuitem,msg);
            }

        }

        /* remove parsed data block from buffer */
        ecuitem->data.remove(0,ecuitem->data.size()-bytesRcvd);

        /* If buffer is too large after parsing, try to resync it */
        if(ecuitem->interfacetype != 0 &&
                ecuitem->serialport &&
                ecuitem->data.length() > DLT_BUFFER_CORRUPT_TRESHOLD)
        {
            skipSerialHeader(ecuitem);
        }

        if (outputfile.isOpen() )
        {

            for(int i = 0; i < project.plugin->topLevelItemCount(); i++)
            {
                item = (PluginItem*)project.plugin->topLevelItem(i);

                if(item->getMode() != PluginItem::ModeDisable)
                {
                    if(item->plugindecoderinterface)
                    {
                        activeDecoderPlugins.append(item);
                    }
                    else if(item->pluginviewerinterface)
                    {
                        item->pluginviewerinterface->updateFileStart();
                        activeViewerPlugins.append(item);
                    }
                }
            }


            /* read received messages in DLT file parser and update DLT message list view */
            /* update indexes  and table view */
            int oldsize = qfile.size();
            qfile.updateIndex();

            for(int num=oldsize;num<qfile.size();num++) {
                qmsg.setMsg(qfile.getMsg(num));

                for(int i = 0; i < activeViewerPlugins.size(); i++){
                    item = (PluginItem*)activeViewerPlugins.at(i);
                    item->pluginviewerinterface->updateMsg(num,qmsg);
                }

                for(int i = 0; i < activeDecoderPlugins.size(); i++)
                {
                    item = (PluginItem*)activeDecoderPlugins.at(i);

                    if(item->plugindecoderinterface->isMsg(qmsg,0))
                    {
                        item->plugindecoderinterface->decodeMsg(qmsg,0);
                        break;
                    }
                }

                if(qfile.checkFilter(qmsg)) {
                    qfile.addFilterIndex(num);
                }

                for(int i = 0; i < activeViewerPlugins.size(); i++){
                    item = (PluginItem*)activeViewerPlugins.at(i);
                    item->pluginviewerinterface->updateMsgDecoded(num,qmsg);
                }
            }

            tableModel->modelChanged();
            //Line below would resize the payload column automatically so that the whole content is readable
            //ui->tableView->resizeColumnToContents(11); //Column 11 is the payload column
            if(settings->autoScroll) {
                ui->tableView->scrollToBottom();
            }

            for(int i = 0; i < activeViewerPlugins.size(); i++){
                item = (PluginItem*)activeViewerPlugins.at(i);
                item->pluginviewerinterface->updateFileFinish();
            }
        }
    }
}

void MainWindow::on_tableView_clicked(QModelIndex index)
{
    PluginItem *item = 0;
    QList<PluginItem*> activeViewerPlugins;
    QList<PluginItem*> activeDecoderPlugins;
    QDltMsg msg;
    //qfile.getMsg(qfile.getMsgFilterPos(index.row()), msg);
    msg.setMsg(qfile.getMsgFilter(index.row()));

    int msgIndex = qfile.getMsgFilterPos(index.row());

    for(int i = 0; i < project.plugin->topLevelItemCount(); i++)
    {
        item = (PluginItem*)project.plugin->topLevelItem(i);

        if(item->getMode() != PluginItem::ModeDisable)
        {
            if(item->plugindecoderinterface)
            {
                activeDecoderPlugins.append(item);
            }
            if(item->pluginviewerinterface)
            {
                activeViewerPlugins.append(item);
            }
        }
    }

    if(activeViewerPlugins.isEmpty() && activeDecoderPlugins.isEmpty())
    {
        return;
    }

    // Update plugins
    for(int i = 0; i < activeViewerPlugins.size() ; i++)
    {
        item = (PluginItem*)activeViewerPlugins.at(i);
        item->pluginviewerinterface->selectedIdxMsg(msgIndex,msg);

    }

    bool decoded = false;
    for(int i = 0; i < activeDecoderPlugins.size(); i++)
    {
        item = (PluginItem*)activeDecoderPlugins.at(i);

        if(item->plugindecoderinterface->isMsg(msg,0))
        {
            item->plugindecoderinterface->decodeMsg(msg,0);
            decoded = true;
            break;
        }
    }

    if(decoded){
        for(int i = 0; i < activeViewerPlugins.size(); i++){
            item = (PluginItem*)activeViewerPlugins.at(i);
            item->pluginviewerinterface->selectedIdxMsgDecoded(msgIndex,msg);
        }
    }
}

void MainWindow::controlMessage_ReceiveControlMessage(EcuItem *ecuitem, DltMessage &msg)
{
    uint8_t * ptr = msg.databuffer;
    int32_t length = msg.datasize;

    /* control message was received */
    uint32_t service_id=0, service_id_tmp=0;
    DLT_MSG_READ_VALUE(service_id_tmp,ptr,length,uint32_t);
    service_id=DLT_ENDIAN_GET_32(msg.standardheader->htyp, service_id_tmp);

    switch (service_id)
    {
    case DLT_SERVICE_ID_GET_LOG_INFO:
    {
        /* Only status 1,2,6,7,8 is supported yet! */

        uint8_t status=0;
        DLT_MSG_READ_VALUE(status,ptr,length,uint8_t); /* No endian conversion necessary */

        /* Support for status=8 */
        if (status==8)
        {
            ecuitem->InvalidAll();
        }

        /* Support for status=6 and status=7 */
        if ((status==6) || (status==7))
        {
            uint16_t count_app_ids=0,count_app_ids_tmp=0;
            DLT_MSG_READ_VALUE(count_app_ids_tmp,ptr,length,uint16_t);
            count_app_ids=DLT_ENDIAN_GET_16(msg.standardheader->htyp, count_app_ids_tmp);

            for (int32_t num=0;num<count_app_ids;num++)
            {
                char apid[DLT_ID_SIZE+1];
                apid[DLT_ID_SIZE] = 0;

                DLT_MSG_READ_ID(apid,ptr,length);

                uint16_t count_context_ids=0,count_context_ids_tmp=0;
                DLT_MSG_READ_VALUE(count_context_ids_tmp,ptr,length,uint16_t);
                count_context_ids=DLT_ENDIAN_GET_16(msg.standardheader->htyp, count_context_ids_tmp);

                for (int32_t num2=0;num2<count_context_ids;num2++)
                {
                    QString contextDescription;
                    char ctid[DLT_ID_SIZE+1];
                    ctid[DLT_ID_SIZE] = 0;

                    DLT_MSG_READ_ID(ctid,ptr,length);

                    int8_t log_level=0;
                    DLT_MSG_READ_VALUE(log_level,ptr,length,int8_t); /* No endian conversion necessary */

                    int8_t trace_status=0;
                    DLT_MSG_READ_VALUE(trace_status,ptr,length,int8_t); /* No endian conversion necessary */

                    if (status==7)
                    {
                        uint16_t context_description_length=0,context_description_length_tmp=0;
                        DLT_MSG_READ_VALUE(context_description_length_tmp,ptr,length,uint16_t);
                        context_description_length=DLT_ENDIAN_GET_16(msg.standardheader->htyp,context_description_length_tmp);

                        if (length<context_description_length)
                        {
                            length = -1;
                        }
                        else
                        {
                            contextDescription = QString(QByteArray((char*)ptr,context_description_length));
                            ptr+=context_description_length;
                            length-=context_description_length;
                        }
                    }

                    controlMessage_SetContext(ecuitem,QString(apid),QString(ctid),contextDescription,log_level,trace_status);
                }

                if (status==7)
                {
                    QString applicationDescription;
                    uint16_t application_description_length=0,application_description_length_tmp=0;
                    DLT_MSG_READ_VALUE(application_description_length_tmp,ptr,length,uint16_t);
                    application_description_length=DLT_ENDIAN_GET_16(msg.standardheader->htyp,application_description_length_tmp);
                    applicationDescription = QString(QByteArray((char*)ptr,application_description_length));
                    controlMessage_SetApplication(ecuitem,QString(apid),applicationDescription);
                    ptr+=application_description_length;
                }
            }
        }

        char com_interface[DLT_ID_SIZE];
        DLT_MSG_READ_ID(com_interface,ptr,length);

        if (length<0)
        {
            // wxMessageBox(_("Control Message corrupted!"),_("Receive Control Message"));
        }

        break;
    }
    case DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL:
    {
        uint8_t status=0;
        DLT_MSG_READ_VALUE(status,ptr,length,uint8_t); /* No endian conversion necessary */

        uint8_t loglevel=0;
        DLT_MSG_READ_VALUE(loglevel,ptr,length,uint8_t); /* No endian conversion necessary */

        switch (status)
        {
        case 0: /* OK */
        {
            ecuitem->loglevel = loglevel;
            ecuitem->status = EcuItem::valid;
        }
            break;
        case 1: /* NOT_SUPPORTED */
        {
            ecuitem->status = EcuItem::unknown;
        }
            break;
        case 2: /* ERROR */
        {
            ecuitem->status = EcuItem::invalid;
        }
            break;
        }
        /* update status */
        ecuitem->update();

        break;
    }
    case DLT_SERVICE_ID_SET_LOG_LEVEL:
    {
        uint8_t status=0;
        DLT_MSG_READ_VALUE(status,ptr,length,uint8_t); /* No endian conversion necessary */

        switch (status)
        {
        case 0: /* OK */
        {
            //conitem->status = EcuItem::valid;
        }
            break;
        case 1: /* NOT_SUPPORTED */
        {
            //conitem->status = EcuItem::unknown;
        }
            break;
        case 2: /* ERROR */
        {
            //conitem->status = EcuItem::invalid;
        }
            break;
        }

        /* update status*/
        //conitem->update();

        break;
    }
    } // switch
}

void MainWindow::controlMessage_SendControlMessage(EcuItem* ecuitem,DltMessage &msg, QString appid, QString contid)
{
    QByteArray data;
    QDltMsg qmsg;

    /* prepare storage header */
    msg.storageheader = (DltStorageHeader*)msg.headerbuffer;
    dlt_set_storageheader(msg.storageheader,ecuitem->id.toAscii());

    /* prepare standard header */
    msg.standardheader = (DltStandardHeader*)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_WEID | DLT_HTYP_WTMS | DLT_HTYP_UEH | DLT_HTYP_PROTOCOL_VERSION1 ;

#if (BYTE_ORDER==BIG_ENDIAN)
    msg.standardheader->htyp = (msg.standardheader->htyp | DLT_HTYP_MSBF);
#endif

    msg.standardheader->mcnt = 0;

    /* Set header extra parameters */
    dlt_set_id(msg.headerextra.ecu,ecuitem->id.toAscii());
    msg.headerextra.tmsp = dlt_uptime();

    /* Copy header extra parameters to headerbuffer */
    dlt_message_set_extraparameters(&msg,0);

    /* prepare extended header */
    msg.extendedheader = (DltExtendedHeader*)(msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp) );
    msg.extendedheader->msin = DLT_MSIN_CONTROL_REQUEST;
    msg.extendedheader->noar = 1; /* number of arguments */
    if (appid.isEmpty())
    {
        dlt_set_id(msg.extendedheader->apid,"APP");       /* application id */
    }
    else
    {
        dlt_set_id(msg.extendedheader->apid, appid.toAscii());
    }
    if (contid.isEmpty())
    {
        dlt_set_id(msg.extendedheader->ctid,"CON");       /* context id */
    }
    else
    {
        dlt_set_id(msg.extendedheader->ctid, contid.toAscii());
    }

    /* prepare length information */
    msg.headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + sizeof(DltExtendedHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp);
    msg.standardheader->len = DLT_HTOBE_16(msg.headersize - sizeof(DltStorageHeader) + msg.datasize);

    /* send message to daemon */
    if (ecuitem->interfacetype == 0 && ecuitem->socket.isOpen())
    {
        /* Optional: Send serial header, if requested */
        if (ecuitem->sendSerialHeaderTcp)
            ecuitem->socket.write((const char*)dltSerialHeader,sizeof(dltSerialHeader));

        /* Send data */
        ecuitem->socket.write((const char*)msg.headerbuffer+sizeof(DltStorageHeader),msg.headersize-sizeof(DltStorageHeader));
        ecuitem->socket.write((const char*)msg.databuffer,msg.datasize);
    }
    else if (ecuitem->interfacetype == 1 && ecuitem->serialport && ecuitem->serialport->isOpen())
    {
        /* Optional: Send serial header, if requested */
        if (ecuitem->sendSerialHeaderSerial)
            ecuitem->serialport->write((const char*)dltSerialHeader,sizeof(dltSerialHeader));

        /* Send data */
        ecuitem->serialport->write((const char*)msg.headerbuffer+sizeof(DltStorageHeader),msg.headersize-sizeof(DltStorageHeader));
        ecuitem->serialport->write((const char*)msg.databuffer,msg.datasize);
    }
    else
    {
        /* ECU is not connected */
        return;
    }

    /* store ctrl message in log file */
    if (outputfile.isOpen())
    {
        if (settings->writeControl)
        {
            outputfile.write((const char*)msg.headerbuffer,msg.headersize);
            outputfile.write((const char*)msg.databuffer,msg.datasize);
            outputfile.flush();
        }
    }

    /* read received messages in DLT file parser and update DLT message list view */
    /* update indexes  and table view */
    int oldsize = qfile.size();
    qfile.updateIndex();
    for(int num=oldsize;num<qfile.size();num++) {
        data = qfile.getMsg(num);
        qmsg.setMsg(data);
        iterateDecodersForMsg(qmsg,0);
        if(qfile.checkFilter(qmsg)) {
            qfile.addFilterIndex(num);
        }
    }
    tableModel->modelChanged();
    if(settings->autoScroll) {
        ui->tableView->scrollToBottom();
    }

}

void MainWindow::on_action_menuDLT_Get_Default_Log_Level_triggered()
{
    /* get selected ECU in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        EcuItem* ecuitem = (EcuItem*) list.at(0);

        /* send get default log level request */
        ControlServiceRequest(ecuitem,DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL);
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No ECU selected in configuration!"));
}

void MainWindow::on_action_menuDLT_Set_Default_Log_Level_triggered()
{
    /* get selected ECU in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        EcuItem* ecuitem = (EcuItem*) list.at(0);

        /* send set default log level request */
        controlMessage_SetDefaultLogLevel(ecuitem,ecuitem->loglevel);
        controlMessage_SetDefaultTraceStatus(ecuitem,ecuitem->tracestatus);

        /* update status */
        ecuitem->status = EcuItem::valid;
        ecuitem->update();
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No ECU selected in configuration!"));
}

void MainWindow::on_action_menuDLT_Set_Log_Level_triggered()
{
    /* get selected context in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == context_type))
    {
        ContextItem* conitem = (ContextItem*) list.at(0);
        ApplicationItem* appitem = (ApplicationItem*) conitem->parent();
        EcuItem* ecuitem = (EcuItem*) appitem->parent();

        /* send set log level and trace status request */
        controlMessage_SetLogLevel(ecuitem,appitem->id,conitem->id,conitem->loglevel);
        controlMessage_SetTraceStatus(ecuitem,appitem->id,conitem->id,conitem->tracestatus);

        /* update status */
        conitem->status = ContextItem::valid;
        conitem->update();

    }else if( (list.count() > 1) && (list.at(0)->type() == context_type) ){
        ContextItem* conitem;

        for(int i=0; i<list.count(); i++){
            if(list.at(i)->type() == context_type){
                conitem = (ContextItem*) list.at(i);

                ApplicationItem* appitem = (ApplicationItem*) conitem->parent();
                EcuItem* ecuitem = (EcuItem*) appitem->parent();

                /* send set log level and trace status request */
                controlMessage_SetLogLevel(ecuitem,appitem->id,conitem->id,conitem->loglevel);
                controlMessage_SetTraceStatus(ecuitem,appitem->id,conitem->id,conitem->tracestatus);

                /* update status */
                conitem->status = ContextItem::valid;
                conitem->update();
            }
        }
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No Context selected in configuration!"));
}

void MainWindow::on_action_menuDLT_Set_All_Log_Levels_triggered()
{
    /* get selected ECU in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        EcuItem* ecuitem = (EcuItem*) list.at(0);

        /* iterate through all applications */
        for(int numapp = 0; numapp < ecuitem->childCount(); numapp++)
        {
            ApplicationItem * appitem = (ApplicationItem *) ecuitem->child(numapp);

            /* iterate through all contexts */
            for(int numcontext = 0; numcontext < appitem->childCount(); numcontext++)
            {
                ContextItem * conitem = (ContextItem *) appitem->child(numcontext);

                /* set log level and trace status of this context */
                controlMessage_SetLogLevel(ecuitem,appitem->id,conitem->id,conitem->loglevel);
                controlMessage_SetTraceStatus(ecuitem,appitem->id,conitem->id,conitem->tracestatus);

                /* update status */
                conitem->status = ContextItem::valid;
                conitem->update();

            }
        }
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No ECU selected in configuration!"));
}

void MainWindow::on_action_menuDLT_Get_Log_Info_triggered()
{
    /* get selected ECU in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        EcuItem* ecuitem = (EcuItem*) list.at(0);
        controlMessage_GetLogInfo(ecuitem);
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No ECU selected in configuration!"));
}

void MainWindow::controlMessage_SetLogLevel(EcuItem* ecuitem, QString app, QString con,int log_level)
{
    DltMessage msg;

    /* initialise new message */
    dlt_message_init(&msg,0);

    /* prepare payload */
    msg.datasize = sizeof(DltServiceSetLogLevel);
    if (msg.databuffer) free(msg.databuffer);
    msg.databuffer = (uint8_t *) malloc(msg.datasize);
    DltServiceSetLogLevel *req;
    req = (DltServiceSetLogLevel*) msg.databuffer;
    req->service_id = DLT_SERVICE_ID_SET_LOG_LEVEL;
    dlt_set_id(req->apid,app.toAscii());
    dlt_set_id(req->ctid,con.toAscii());
    req->log_level = log_level;
    dlt_set_id(req->com,"remo");

    /* send message */
    controlMessage_SendControlMessage(ecuitem,msg,QString(""),QString(""));

    /* free message */
    dlt_message_free(&msg,0);
}

void MainWindow::controlMessage_SetDefaultLogLevel(EcuItem* ecuitem, int status)
{
    DltMessage msg;

    /* initialise new message */
    dlt_message_init(&msg,0);

    /* prepare payload */
    msg.datasize = sizeof(DltServiceSetDefaultLogLevel);
    if (msg.databuffer) free(msg.databuffer);
    msg.databuffer = (uint8_t *) malloc(msg.datasize);
    DltServiceSetDefaultLogLevel *req;
    req = (DltServiceSetDefaultLogLevel*) msg.databuffer;
    req->service_id = DLT_SERVICE_ID_SET_DEFAULT_LOG_LEVEL;
    req->log_level = status;
    dlt_set_id(req->com,"remo");

    /* send message */
    controlMessage_SendControlMessage(ecuitem,msg,QString(""),QString(""));

    /* free message */
    dlt_message_free(&msg,0);
}

void MainWindow::controlMessage_SetTraceStatus(EcuItem* ecuitem,QString app, QString con,int status)
{
    DltMessage msg;

    /* initialise new message */
    dlt_message_init(&msg,0);

    /* prepare payload */
    msg.datasize = sizeof(DltServiceSetLogLevel);
    if (msg.databuffer) free(msg.databuffer);
    msg.databuffer = (uint8_t *) malloc(msg.datasize);
    DltServiceSetLogLevel *req;
    req = (DltServiceSetLogLevel*) msg.databuffer;
    req->service_id = DLT_SERVICE_ID_SET_TRACE_STATUS;
    dlt_set_id(req->apid,app.toAscii());
    dlt_set_id(req->ctid,con.toAscii());
    req->log_level = status;
    dlt_set_id(req->com,"remo");

    /* send message */
    controlMessage_SendControlMessage(ecuitem,msg,QString(""),QString(""));

    /* free message */
    dlt_message_free(&msg,0);

}

void MainWindow::controlMessage_SetDefaultTraceStatus(EcuItem* ecuitem, int status)
{
    DltMessage msg;

    /* initialise new message */
    dlt_message_init(&msg,0);

    /* prepare payload */
    msg.datasize = sizeof(DltServiceSetDefaultLogLevel);
    if (msg.databuffer) free(msg.databuffer);
    msg.databuffer = (uint8_t *) malloc(msg.datasize);
    DltServiceSetDefaultLogLevel *req;
    req = (DltServiceSetDefaultLogLevel*) msg.databuffer;
    req->service_id = DLT_SERVICE_ID_SET_DEFAULT_TRACE_STATUS;
    req->log_level = status;
    dlt_set_id(req->com,"remo");

    /* send message */
    controlMessage_SendControlMessage(ecuitem,msg,QString(""),QString(""));

    /* free message */
    dlt_message_free(&msg,0);
}

void MainWindow::controlMessage_SetVerboseMode(EcuItem* ecuitem, int verbosemode)
{
    DltMessage msg;

    /* initialise new message */
    dlt_message_init(&msg,0);

    /* prepare payload */
    msg.datasize = sizeof(DltServiceSetVerboseMode);
    if (msg.databuffer) free(msg.databuffer);
    msg.databuffer = (uint8_t *) malloc(msg.datasize);
    DltServiceSetVerboseMode *req;
    req = (DltServiceSetVerboseMode*) msg.databuffer;
    req->service_id = DLT_SERVICE_ID_SET_VERBOSE_MODE;
    req->new_status = verbosemode;
    //dlt_set_id(req->com,"remo");

    /* send message */
    controlMessage_SendControlMessage(ecuitem,msg,QString(""),QString(""));

    /* free message */
    dlt_message_free(&msg,0);
}

void MainWindow::controlMessage_SetTimingPackets(EcuItem* ecuitem, bool enable)
{
    DltMessage msg;
    uint8_t new_status=(enable?1:0);

    /* initialise new message */
    dlt_message_init(&msg,0);

    /* prepare payload of data */
    msg.datasize = sizeof(DltServiceSetVerboseMode);
    if (msg.databuffer) free(msg.databuffer);
    msg.databuffer = (uint8_t *) malloc(msg.datasize);
    DltServiceSetVerboseMode *req;
    req = (DltServiceSetVerboseMode*) msg.databuffer;
    req->service_id = DLT_SERVICE_ID_SET_TIMING_PACKETS;
    req->new_status = new_status;

    /* send message */
    controlMessage_SendControlMessage(ecuitem,msg,QString(""),QString(""));

    /* free message */
    dlt_message_free(&msg,0);
}

void MainWindow::controlMessage_GetLogInfo(EcuItem* ecuitem)
{
    DltMessage msg;

    /* initialise new message */
    dlt_message_init(&msg,0);

    /* prepare payload */
    msg.datasize = sizeof(DltServiceGetLogInfoRequest);
    if (msg.databuffer) free(msg.databuffer);
    msg.databuffer = (uint8_t *) malloc(msg.datasize);
    DltServiceGetLogInfoRequest *req;
    req = (DltServiceGetLogInfoRequest*) msg.databuffer;
    req->service_id = DLT_SERVICE_ID_GET_LOG_INFO;

    req->options = 7;

    dlt_set_id(req->apid, "");
    dlt_set_id(req->ctid, "");

    dlt_set_id(req->com,"remo");

    /* send message */
    controlMessage_SendControlMessage(ecuitem,msg,QString(""),QString(""));

    /* free message */
    dlt_message_free(&msg,0);
}

void MainWindow::ControlServiceRequest(EcuItem* ecuitem, int service_id )
{
    DltMessage msg;

    /* initialise new message */
    dlt_message_init(&msg,0);

    /* prepare payload of data */
    msg.datasize = sizeof(uint32_t);
    if (msg.databuffer) free(msg.databuffer);
    msg.databuffer = (uint8_t *) malloc(msg.datasize);
    uint32_t sid = service_id;
    memcpy(msg.databuffer,&sid,sizeof(sid));

    /* send message */
    controlMessage_SendControlMessage(ecuitem,msg,QString(""),QString(""));

    /* free message */
    dlt_message_free(&msg,0);
}

void MainWindow::SendInjection(EcuItem* ecuitem)
{
    unsigned int serviceID;
    unsigned int size;
    bool ok;

    if (injectionAplicationId.isEmpty() || injectionContextId.isEmpty() || injectionServiceId.isEmpty() )
        return;

    serviceID = (unsigned int)injectionServiceId.toInt(&ok, 0);

    if ((DLT_SERVICE_ID_CALLSW_CINJECTION<= serviceID) && (serviceID!=0))
    {
        DltMessage msg;

        /* initialise new message */
        dlt_message_init(&msg,0);

        // Request parameter:
        // data_length uint32
        // data        uint8[]

        /* prepare payload of data */
        size = (injectionData.length() + 1);
        msg.datasize = 4 + 4 + size;
        if (msg.databuffer) free(msg.databuffer);
        msg.databuffer = (uint8_t *) malloc(msg.datasize);

        memcpy(msg.databuffer  , &serviceID,sizeof(serviceID));
        memcpy(msg.databuffer+4, &size, sizeof(size));
        memcpy(msg.databuffer+8, injectionData.toUtf8(), size);

        /* send message */
        controlMessage_SendControlMessage(ecuitem,msg,injectionAplicationId,injectionContextId);

        /* free message */
        dlt_message_free(&msg,0);
    }
}

void MainWindow::on_action_menuDLT_Store_Config_triggered()
{
    /* get selected ECU from configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        EcuItem* ecuitem = (EcuItem*) list.at(0);

        ControlServiceRequest(ecuitem,DLT_SERVICE_ID_STORE_CONFIG);
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No ECU selected in configuration!"));
}

void MainWindow::on_action_menuDLT_Reset_to_Factory_Default_triggered()
{
    /* get selected ECU from configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        EcuItem* ecuitem = (EcuItem*) list.at(0);

        ControlServiceRequest(ecuitem,DLT_SERVICE_ID_RESET_TO_FACTORY_DEFAULT);
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No ECU selected in configuration!"));
}

void MainWindow::on_action_menuDLT_Get_Software_Version_triggered()
{
    /* get selected ECU from configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        EcuItem* ecuitem = (EcuItem*) list.at(0);

        ControlServiceRequest(ecuitem,DLT_SERVICE_ID_GET_SOFTWARE_VERSION);
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No ECU selected in configuration!"));
}

void MainWindow::on_action_menuDLT_Get_Local_Time_triggered()
{
    /* get selected ECU from configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        EcuItem* ecuitem = (EcuItem*) list.at(0);

        ControlServiceRequest(ecuitem,DLT_SERVICE_ID_GET_LOCAL_TIME);
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No ECU selected in configuration!"));
}


void MainWindow::getSelectedItems(EcuItem **ecuitem,ApplicationItem** appitem,ContextItem** conitem)
{
    *ecuitem = 0;
    *appitem = 0;
    *conitem = 0;

    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if(list.count() != 1)
    {
        return;
    }

    if(list.at(0)->type() == ecu_type)
    {
        *ecuitem = (EcuItem*) list.at(0);
    }
    if(list.at(0)->type() == application_type)
    {
        *appitem = (ApplicationItem*) list.at(0);
        *ecuitem = (EcuItem*) (*appitem)->parent();
    }
    if(list.at(0)->type() == context_type)
    {
        *conitem = (ContextItem*) list.at(0);
        *appitem = (ApplicationItem*) (*conitem)->parent();
        *ecuitem = (EcuItem*) (*appitem)->parent();
    }

}

void MainWindow::sendInjection(int index,QString applicationId,QString contextId,int serviceId,QByteArray data)
{
    EcuItem* ecuitem = (EcuItem*) project.ecu->topLevelItem(index);

    injectionAplicationId = applicationId;
    injectionContextId = contextId;

    if(ecuitem)
    {

        unsigned int serviceID;
        unsigned int size;

        serviceID = serviceId;

        if ((DLT_SERVICE_ID_CALLSW_CINJECTION<= serviceID) && (serviceID!=0))
        {
            DltMessage msg;

            /* initialise new message */
            dlt_message_init(&msg,0);

            // Request parameter:
            // data_length uint32
            // data        uint8[]

            /* prepare payload of data */
            size = (data.size());
            msg.datasize = 4 + 4 + size;
            if (msg.databuffer) free(msg.databuffer);
            msg.databuffer = (uint8_t *) malloc(msg.datasize);

            memcpy(msg.databuffer  , &serviceID,sizeof(serviceID));
            memcpy(msg.databuffer+4, &size, sizeof(size));
            memcpy(msg.databuffer+8, data.constData(), data.size());

            /* send message */
            controlMessage_SendControlMessage(ecuitem,msg,injectionAplicationId,injectionContextId);

            /* free message */
            dlt_message_free(&msg,0);
        }
    }
}

void MainWindow::on_action_menuDLT_Send_Injection_triggered()
{
    /* get selected ECU from configuration */
    EcuItem* ecuitem = 0;
    ApplicationItem* appitem = 0;
    ContextItem* conitem = 0;

    getSelectedItems(&ecuitem,&appitem,&conitem);

    if(!ecuitem)
    {
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("Nothing selected in configuration!"));
        return;
    }
    else
    {
        /* show Injection dialog */
        InjectionDialog dlg("","");

        if(conitem)
        {
            dlg.setApplicationId(appitem->id);
            dlg.setContextId(conitem->id);
        }
        else if(appitem)
        {
            dlg.setApplicationId(appitem->id);
            dlg.setContextId(injectionContextId);
        }
        else
        {
            dlg.setApplicationId(injectionAplicationId);
            dlg.setContextId(injectionContextId);
        }
        dlg.setServiceId(injectionServiceId);
        dlg.setData(injectionData);

        if(dlg.exec())
        {
            injectionAplicationId = dlg.getApplicationId();
            injectionContextId = dlg.getContextId();
            injectionServiceId = dlg.getServiceId();
            injectionData = dlg.getData();

            SendInjection(ecuitem);
        }
    }
    //else
    //    QMessageBox::warning(0, QString("DLT Viewer"),
    //                         QString("No ECU selected in configuration!"));
}

void MainWindow::controlMessage_SetApplication(EcuItem *ecuitem, QString apid, QString appdescription)
{
    /* Try to find App */
    for(int numapp = 0; numapp < ecuitem->childCount(); numapp++)
    {
        ApplicationItem * appitem = (ApplicationItem *) ecuitem->child(numapp);

        if(appitem->id == apid)
        {
            appitem->description = appdescription;
            appitem->update();

            return;
        }
    }

    /* No app and no con found */
    ApplicationItem* appitem = new ApplicationItem(ecuitem);
    appitem->id = apid;
    appitem->description = appdescription;
    appitem->update();
    ecuitem->addChild(appitem);
}

void MainWindow::controlMessage_SetContext(EcuItem *ecuitem, QString apid, QString ctid,QString ctdescription,int log_level,int trace_status)
{
    /* First try to find existing context */
    for(int numapp = 0; numapp < ecuitem->childCount(); numapp++)
    {
        ApplicationItem * appitem = (ApplicationItem *) ecuitem->child(numapp);

        for(int numcontext = 0; numcontext < appitem->childCount(); numcontext++)
        {
            ContextItem * conitem = (ContextItem *) appitem->child(numcontext);

            if(appitem->id == apid && conitem->id == ctid)
            {
                /* set new log level and trace status */
                conitem->loglevel = log_level;
                conitem->tracestatus = trace_status;
                conitem->description = ctdescription;
                conitem->status = ContextItem::valid;
                conitem->update();
                return;
            }
        }
    }

    /* Try to find App */
    for(int numapp = 0; numapp < ecuitem->childCount(); numapp++)
    {
        ApplicationItem * appitem = (ApplicationItem *) ecuitem->child(numapp);

        if(appitem->id == apid)
        {
            /* Add new context */
            ContextItem* conitem = new ContextItem(appitem);
            conitem->id = ctid;
            conitem->loglevel = log_level;
            conitem->tracestatus = trace_status;
            conitem->description = ctdescription;
            conitem->status = ContextItem::valid;
            conitem->update();
            appitem->addChild(conitem);

            return;
        }
    }

    /* No app and no con found */
    ApplicationItem* appitem = new ApplicationItem(ecuitem);
    appitem->id = apid;
    appitem->description = QString("");
    appitem->update();
    ecuitem->addChild(appitem);
    ContextItem* conitem = new ContextItem(appitem);
    conitem->id = ctid;
    conitem->loglevel = log_level;
    conitem->tracestatus = trace_status;
    conitem->description = ctdescription;
    conitem->status = ContextItem::valid;
    conitem->update();
    appitem->addChild(conitem);
}

void MainWindow::on_action_menuHelp_Info_triggered()
{
    QMessageBox::information(0, QString("DLT Viewer"),
                             QString("Package Version : %1 %2\n").arg(PACKAGE_VERSION).arg(PACKAGE_VERSION_STATE)+
                             QString("Package Revision: %1\n\n").arg(PACKAGE_REVISION)+
                             QString("Build Date: %1\n").arg(__DATE__)+
                             QString("Build Time: %1\n").arg(__TIME__)+
                             QString("Qt Version: %1\n\n").arg(QT_VERSION_STR)+
                         #if (BYTE_ORDER==BIG_ENDIAN)
                             QString("Architecture: Big Endian\n\n")+
                         #else
                             QString("Architecture: Little Endian\n\n")+
                         #endif
                             QString("(C) 2010 BMW AG\n"));
}


void MainWindow::on_action_menuHelp_Command_Line_triggered()
{
    // Please copy changes to OptManager::getInstance().cpp - printUsage()

    QMessageBox::information(0, QString("DLT Viewer - Command line usage"),
                         #if (WIN32)
                             QString("Usage: dlt_viewer.exe [OPTIONS]\n\n")+
                             QString("Options:\n")+
                         #else
                             QString("Usage: dlt_viewer [OPTIONS]\n\n")+
                             QString("Options:\n")+
                             QString(" -h \t\tPrint usage\n")+
                         #endif
                             QString(" -p projectfile \t\tLoading project file on startup (must end with .dlp)\n")+
                             QString(" -l logfile \t\tLoading logfile on startup (must end with .dlt)\n")+
                             QString(" -f filterfile \t\tLoading filterfile on startup (must end with .dlf)\n")+
                             QString(" -c logfile textfile \tConvert logfile file to textfile (logfile must end with .dlt)\n")+
                             QString(" -e \"plugin|command|param1|..|param<n>\" \tExecute a command plugin with <n> parameters.")
                             );
}

void MainWindow::on_pluginWidget_itemSelectionChanged()
{
    QList<QTreeWidgetItem *> list = project.plugin->selectedItems();

    if((list.count() >= 1) ) {
        ui->action_menuPlugin_Edit->setEnabled(true);
        ui->action_menuPlugin_Hide->setEnabled(true);
        ui->action_menuPlugin_Show->setEnabled(true);
        ui->action_menuPlugin_Disable->setEnabled(true);
    }
}
void MainWindow::on_filterWidget_itemSelectionChanged()
{
    ui->action_menuFilter_Load->setEnabled(true);

    if( project.filter->topLevelItemCount() > 0 ){
        ui->action_menuFilter_Save_As->setEnabled(true);
        ui->action_menuFilter_Clear_all->setEnabled(true);
    }else{
        ui->action_menuFilter_Save_As->setEnabled(false);
        ui->action_menuFilter_Clear_all->setEnabled(false);
    }

    if((project.filter->selectedItems().count() >= 1) ) {
        ui->action_menuFilter_Delete->setEnabled(true);
        ui->action_menuFilter_Edit->setEnabled(true);
        ui->action_menuFilter_Add->setEnabled(true);
        ui->action_menuFilter_Duplicate->setEnabled(true);
    }else{
        ui->action_menuFilter_Delete->setEnabled(false);
        ui->action_menuFilter_Edit->setEnabled(false);
        ui->action_menuFilter_Add->setEnabled(false);
        ui->action_menuFilter_Duplicate->setEnabled(false);
    }
}

void MainWindow::on_configWidget_itemSelectionChanged()
{
    /* get selected ECU from configuration */
    EcuItem* ecuitem = 0;
    ApplicationItem* appitem = 0;
    ContextItem* conitem = 0;

    getSelectedItems(&ecuitem,&appitem,&conitem);

    ui->action_menuDLT_Get_Default_Log_Level->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->action_menuDLT_Set_Default_Log_Level->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->action_menuDLT_Get_Local_Time->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->action_menuDLT_Get_Software_Version->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->action_menuDLT_Store_Config->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->action_menuDLT_Get_Log_Info->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->action_menuDLT_Set_Log_Level->setEnabled(conitem && ecuitem->connected);
    ui->action_menuDLT_Set_All_Log_Levels->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->action_menuDLT_Reset_to_Factory_Default->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->action_menuDLT_Send_Injection->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->action_menuDLT_Edit_All_Log_Levels->setEnabled(ecuitem);

    ui->action_menuConfig_Application_Add->setEnabled(ecuitem && !appitem);
    ui->action_menuConfig_Application_Edit->setEnabled(appitem && !conitem);
    ui->action_menuConfig_Application_Delete->setEnabled(appitem && !conitem);
    ui->action_menuConfig_Context_Add->setEnabled(appitem && !conitem);
    ui->action_menuConfig_Context_Edit->setEnabled(conitem);
    ui->action_menuConfig_Context_Delete->setEnabled(conitem);
    ui->action_menuConfig_ECU_Add->setEnabled(true);
    ui->action_menuConfig_ECU_Edit->setEnabled(ecuitem && !appitem);
    ui->action_menuConfig_ECU_Delete->setEnabled(ecuitem && !appitem);
    ui->action_menuConfig_Delete_All_Contexts->setEnabled(ecuitem && !appitem);
    ui->action_menuConfig_Connect->setEnabled(ecuitem && !appitem && !ecuitem->tryToConnect);
    ui->action_menuConfig_Disconnect->setEnabled(ecuitem && !appitem && ecuitem->tryToConnect);
    ui->action_menuConfig_Expand_All_ECUs->setEnabled(ecuitem && !appitem );
    ui->action_menuConfig_Collapse_All_ECUs->setEnabled(ecuitem && !appitem );

}

void MainWindow::autoscrollToggled(bool state)
{
    int autoScrollOld = settings->autoScroll;

    // Mapping: button to variable
    settings->autoScroll = (state?Qt::Checked:Qt::Unchecked);

    if (autoScrollOld!=settings->autoScroll)
        settings->writeSettings(this);
}

void MainWindow::updateScrollButton()
{
    // Mapping: variable to button
    if (settings->autoScroll == Qt::Unchecked )
        scrollbutton->setChecked(false);
    else
        scrollbutton->setChecked(true);
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QString fileName;

    if (action)
    {
        fileName = action->data().toString();

        if(fileName.isEmpty())
        {
            removeCurrentFile(fileName);
            return;
        }

        /* close existing file */
        if(outputfile.isOpen())
            outputfile.close();

        /* open existing file and append new data */
        outputfile.setFileName(fileName);

        setCurrentFile(fileName);

        if(outputfile.open(QIODevice::WriteOnly|QIODevice::Append))
            reloadLogFile();
        else
        {
            QMessageBox::critical(0, QString("DLT Viewer"),
                                  QString("Cannot open log file \"%1\"\n%2")
                                  .arg(fileName)
                                  .arg(outputfile.errorString()));
            removeCurrentFile(fileName);
        }
    }
}

void MainWindow::updateRecentFileActions()
{
    int numRecentFiles = qMin(recentFiles.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(recentFiles[i]);
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(recentFiles[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);

    ui->menuRecent_files->setEnabled(recentFiles.size()>0);
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    recentFiles.removeAll(fileName);
    recentFiles.prepend(fileName);
    while (recentFiles.size() > MaxRecentFiles)
        recentFiles.removeLast();

    updateRecentFileActions();

    // write settings
    DltSettingsManager::getInstance()->setValue("other/recentFileList",recentFiles);
}

void MainWindow::removeCurrentFile(const QString &fileName)
{
    recentFiles.removeAll(fileName);
    updateRecentFileActions();

    // write settings
    DltSettingsManager::getInstance()->setValue("other/recentFileList",recentFiles);
}

void MainWindow::openRecentProject()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QString projectName;

    if (action)
    {
        projectName = action->data().toString();

        /* Open existing project */
        if(!projectName.isEmpty() && project.Load(projectName))
        {
            /* Applies project settings and save it to registry */
            applySettings();
            settings->writeSettings(this);

            /* Change current working directory */
            workingDirectory = QFileInfo(projectName).absolutePath();

            this->setWindowTitle(QString("DLT Viewer - "+projectName+" - Version : %1 %2").arg(PACKAGE_VERSION).arg(PACKAGE_VERSION_STATE));
            /* Load the plugins description files after loading project */
            updatePlugins();

            setCurrentProject(projectName);
        }
        else
        {
            removeCurrentProject(projectName);
            return;
        }
    }
}

void MainWindow::updateRecentProjectActions()
{
    int numRecentProjects = qMin(recentProjects.size(), (int)MaxRecentProjects);

    for (int i = 0; i < numRecentProjects; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(recentProjects[i]);
        recentProjectActs[i]->setText(text);
        recentProjectActs[i]->setData(recentProjects[i]);
        recentProjectActs[i]->setVisible(true);
    }
    for (int j = numRecentProjects; j < MaxRecentProjects; ++j)
        recentProjectActs[j]->setVisible(false);

    ui->menuRecent_projects->setEnabled(recentProjects.size()>0);
}

void MainWindow::setCurrentProject(const QString &projectName)
{
    recentProjects.removeAll(projectName);
    recentProjects.prepend(projectName);
    while (recentProjects.size() > MaxRecentProjects)
        recentProjects.removeLast();

    updateRecentProjectActions();

    // write settings
    DltSettingsManager::getInstance()->setValue("other/recentProjectList",recentProjects);
}

void MainWindow::removeCurrentProject(const QString &projectName)
{
    recentProjects.removeAll(projectName);
    updateRecentProjectActions();

    // write settings
    DltSettingsManager::getInstance()->setValue("other/recentProjectList",recentProjects);
}


void MainWindow::openRecentFilters()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QString fileName;

    if (action)
    {
        fileName = action->data().toString();

        if(!fileName.isEmpty() && project.LoadFilter(fileName,true))
        {
            ui->filterButton->setIcon(QIcon(":/toolbar/png/weather-storm.png"));
            ui->filterStatus->setText("Filters changed. Please enable filtering.");

            ui->filterButton->setChecked(Qt::Unchecked);
            ui->filterButton->setText("Enable filters");

            //filterUpdate();
            setCurrentFilters(fileName);
            //reloadLogFile();
        }
    }
}

void MainWindow::updateRecentFiltersActions()
{
    int numRecentFilters = qMin(recentFilters.size(), (int)MaxRecentFilters);

    for (int i = 0; i < numRecentFilters; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(recentFilters[i]);

        recentFiltersActs[i]->setText(text);
        recentFiltersActs[i]->setData(recentFilters[i]);
        recentFiltersActs[i]->setVisible(true);
    }

    for (int j = numRecentFilters; j < MaxRecentFilters; ++j)
    {
        recentFiltersActs[j]->setVisible(false);
    }

    ui->menuRecent_Filters->setEnabled(recentFilters.size()>0);
}

void MainWindow::setCurrentFilters(const QString &filtersName)
{
    recentFilters.removeAll(filtersName);
    recentFilters.prepend(filtersName);
    while (recentFilters.size() > MaxRecentFilters)
        recentFilters.removeLast();

    updateRecentFiltersActions();

    // write settings
    DltSettingsManager::getInstance()->setValue("other/recentFiltersList",recentFilters);
}

void MainWindow::removeCurrentFilters(const QString &filtersName)
{
    recentFilters.removeAll(filtersName);
    updateRecentFiltersActions();

    // write settings
    DltSettingsManager::getInstance()->setValue("other/recentFiltersList",filtersName);
}


void MainWindow::setCurrentHostname(const QString &hostName)
{
    recentHostnames.removeAll(hostName);
    recentHostnames.prepend(hostName);
    while (recentHostnames.size() > MaxRecentHostnames)
        recentHostnames.removeLast();

    /* Write settings for recent hostnames*/
    DltSettingsManager::getInstance()->setValue("other/recentHostnameList",recentHostnames);
}

void MainWindow::setCurrentPort(const QString &portName)
{
    recentPorts.removeAll(portName);
    recentPorts.prepend(portName);
    while (recentPorts.size() > MaxRecentPorts)
        recentPorts.removeLast();

    /* Write settings for recent ports */
    DltSettingsManager::getInstance()->setValue("other/recentPortList",recentPorts);
}

void MainWindow::tableViewValueChanged(int value)
{
    int maximum = ((QAbstractSlider *)(ui->tableView->verticalScrollBar()))->maximum();

    if (value==maximum)
    {
        /* Only enable, if disabled */
        if (settings->autoScroll==Qt::Unchecked)
        {
            /* enable scrolling */
            autoscrollToggled(Qt::Checked);
            updateScrollButton();
        }
    }
    else
    {
        /* Only disable, if enabled */
        if (settings->autoScroll==Qt::Checked)
        {
            /* disable scrolling */
            autoscrollToggled(Qt::Unchecked);
            updateScrollButton();
        }
    }
}

void MainWindow::sendUpdates(EcuItem* ecuitem)
{
    /* update default log level, trace status and timing packets */
    controlMessage_SetDefaultLogLevel(ecuitem,ecuitem->loglevel);
    controlMessage_SetDefaultTraceStatus(ecuitem,ecuitem->tracestatus);
    controlMessage_SetVerboseMode(ecuitem,ecuitem->verbosemode);
    controlMessage_SetTimingPackets(ecuitem,ecuitem->timingPackets);

    if (ecuitem->sendGetLogInfo)
        controlMessage_GetLogInfo(ecuitem);

    /* update status */
    ecuitem->status = EcuItem::valid;
    ecuitem->update();

}

void MainWindow::stateChangedSerial(bool dsrChanged){
    /* signal emited when connection state changed */

    /* find socket which emited signal */
    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
        if( ecuitem->serialport == sender())
        {
            /* update ECU item */
            ecuitem->update();

            if(dsrChanged)
            {
                /* send new default log level to ECU, if selected in dlg */
                if (ecuitem->updateDataIfOnline)
                {
                    sendUpdates(ecuitem);
                }
            }

            for(int pnum = 0; pnum < project.plugin->topLevelItemCount (); pnum++) {
                PluginItem *item = (PluginItem*)project.plugin->topLevelItem(pnum);

                if(item->plugincontrolinterface)
                {
                    if(dsrChanged){
                        item->plugincontrolinterface->stateChanged(num,QDltConnection::QDltConnectionOnline);
                    }else{
                        item->plugincontrolinterface->stateChanged(num,QDltConnection::QDltConnectionOffline);
                    }

                }
            }

        }
    }
}

void MainWindow::stateChangedTCP(QAbstractSocket::SocketState socketState)
{
    /* signal emited when connection state changed */

    /* find socket which emited signal */
    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
        if( &(ecuitem->socket) == sender())
        {
            /* update ECU item */
            ecuitem->update();

            if (socketState==QAbstractSocket::ConnectedState)
            {
                /* send new default log level to ECU, if selected in dlg */
                if (ecuitem->updateDataIfOnline)
                {
                    sendUpdates(ecuitem);
                }
            }

            for(int pnum = 0; pnum < project.plugin->topLevelItemCount (); pnum++) {
                PluginItem *item = (PluginItem*)project.plugin->topLevelItem(pnum);

                if(item->plugincontrolinterface)
                {
                    switch(socketState){
                    case QAbstractSocket::UnconnectedState:
                        item->plugincontrolinterface->stateChanged(num,QDltConnection::QDltConnectionOffline);
                        break;
                    case QAbstractSocket::ConnectingState:
                        item->plugincontrolinterface->stateChanged(num,QDltConnection::QDltConnectionConnecting);
                        break;
                    case QAbstractSocket::ConnectedState:
                        item->plugincontrolinterface->stateChanged(num,QDltConnection::QDltConnectionOnline);
                        break;
                    case QAbstractSocket::ClosingState:
                        item->plugincontrolinterface->stateChanged(num,QDltConnection::QDltConnectionOffline);
                        break;
                    default:
                        item->plugincontrolinterface->stateChanged(num,QDltConnection::QDltConnectionOffline);
                        break;
                    }

                }
            }

        }
    }
}

//----------------------------------------------------------------------------
// Search functionalities
//----------------------------------------------------------------------------

void MainWindow::on_action_menuSearch_Find_triggered()
{

    searchDlg->open();
    searchDlg->selectText();
}

//----------------------------------------------------------------------------
// Plugin functionalities
//----------------------------------------------------------------------------

void MainWindow::loadPlugins()
{
    QDir pluginsDir;

    /* The viewer loosk in the relativ to the executable in the ./plugins directory */
    pluginsDir.setPath(QCoreApplication::applicationDirPath());
    if(pluginsDir.cd("plugins"))
    {
        loadPluginsPath(pluginsDir);
    }

    if(settings->pluginsPath)
    {
        pluginsDir.setPath(settings->pluginsPathName);
        if(pluginsDir.exists() && pluginsDir.isReadable())
        {
            loadPluginsPath(pluginsDir);
        }
    }

    /* Load plugins from system directory in linux */
    pluginsDir.setPath("/usr/share/dlt-viewer/plugins");
    if(pluginsDir.exists() && pluginsDir.isReadable())
    {
        loadPluginsPath(pluginsDir);
    }
}

void MainWindow::loadPluginsPath(QDir dir)
{
    foreach (QString fileName, dir.entryList(QDir::Files))
    {
        QPluginLoader pluginLoader(dir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin)
        {
            QDLTPluginInterface *plugininterface = qobject_cast<QDLTPluginInterface *>(plugin);
            if (plugininterface)
            {
                if(QString::compare( plugininterface->pluginInterfaceVersion(),PLUGIN_INTERFACE_VERSION, Qt::CaseSensitive) == 0){

                    PluginItem* item = new PluginItem(0);
                    item->plugininterface = plugininterface;
                    item->setName(plugininterface->name());
                    item->setPluginVersion( plugininterface->pluginVersion() );
                    item->setPluginInterfaceVersion( plugininterface->pluginInterfaceVersion() );
                    item->setMode( item->getPluginModeFromSettings());

                    QDltPluginViewerInterface *pluginviewerinterface = qobject_cast<QDltPluginViewerInterface *>(plugin);
                    if(pluginviewerinterface)
                    {
                        item->pluginviewerinterface = pluginviewerinterface;
                        item->widget = item->pluginviewerinterface->initViewer();

                        if(item->widget)
                        {
                            //item->dockWidget = new QDockWidget(item->getName(),this);
                            item->dockWidget = new MyPluginDockWidget(item,this);
                            item->dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
                            item->dockWidget->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
                            item->dockWidget->setWidget(item->widget);
                            item->dockWidget->setObjectName(item->getName());

                            addDockWidget(Qt::LeftDockWidgetArea, item->dockWidget);

                            if(item->getMode() != PluginItem::ModeShow)
                            {
                                item->dockWidget->hide();
                            }
                        }
                    }
                    QDLTPluginDecoderInterface *plugindecoderinterface = qobject_cast<QDLTPluginDecoderInterface *>(plugin);
                    if(plugindecoderinterface)
                    {
                        item->plugindecoderinterface = plugindecoderinterface;
                    }
                    QDltPluginControlInterface *plugincontrolinterface = qobject_cast<QDltPluginControlInterface *>(plugin);
                    if(plugincontrolinterface)
                    {
                        item->plugincontrolinterface = plugincontrolinterface;
                    }
                    QDltPluginCommandInterface *plugincommandinterface = qobject_cast<QDltPluginCommandInterface *>(plugin);
                    if(plugincommandinterface)
                    {
                        item->plugincommandinterface = plugincommandinterface;
                    }
                    item->update();

                    project.plugin->addTopLevelItem(item);

                } else {

                    QMessageBox::warning(0, QString("DLT Viewer"),QString(tr("Error: Plugin could not be loaded!\nMismatch with plugin interface version of DLT Viewer.\n\nPlugin name: %1\nPlugin version: %2\nPlugin interface version: %3\nPlugin path: %4\n\nDLT Viewer - Plugin interface version: %5")).arg(plugininterface->name()).arg(plugininterface->pluginVersion()).arg(plugininterface->pluginInterfaceVersion()).arg(dir.absolutePath()).arg(PLUGIN_INTERFACE_VERSION));
                }


            }
        }
    }
}

void MainWindow::updatePluginsECUList()
{
    QStringList list;

    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);

        list.append(ecuitem->id + " (" + ecuitem->description + ")");
    }
    for(int num = 0; num < project.plugin->topLevelItemCount (); num++) {
        PluginItem *item = (PluginItem*)project.plugin->topLevelItem(num);

        if(item->plugincontrolinterface)
        {
            item->plugincontrolinterface->initControl(&qcontrol);
            item->plugincontrolinterface->initConnections(list);
        }
    }
}

void MainWindow::updatePlugins() {
    for(int num = 0; num < project.plugin->topLevelItemCount (); num++) {
        PluginItem *item = (PluginItem*)project.plugin->topLevelItem(num);

        updatePlugin(item);
    }

}

void MainWindow::updatePlugin(PluginItem *item) {
    item->takeChildren();

    item->plugininterface->loadConfig(item->getFilename());

    /*    if(item->plugincontrolinterface)
        item->plugincontrolinterface->initControl(&qcontrol);
*/
    QStringList list = item->plugininterface->infoConfig();
    for(int num=0;num<list.size();num++) {
        item->addChild(new QTreeWidgetItem(QStringList(list.at(num))));
    }

    item->update();

    if(item->dockWidget) {
        if(item->getMode() == PluginItem::ModeShow) {
            item->dockWidget->show();
        }
        else {
            item->dockWidget->hide();
        }
    }
}

void MainWindow::on_action_menuPlugin_Edit_triggered() {
    /* get selected plugin */
    bool callInitFile = false;

    QList<QTreeWidgetItem *> list = project.plugin->selectedItems();
    if((list.count() == 1) ) {
        QTreeWidgetItem *treeitem = list.at(0);
        if(treeitem->parent())
        {
            /* This is not a plugin item */
            return;
        }
        PluginItem* item = (PluginItem*) treeitem;

        /* show plugin dialog */
        PluginDialog dlg;
        dlg.setName(item->getName());
        dlg.setPluginVersion(item->getPluginVersion());
        dlg.setPluginInterfaceVersion(item->getPluginInterfaceVersion());
        dlg.setFilename(item->getFilename());
        dlg.setMode(item->getMode());
        if(!item->pluginviewerinterface)
            dlg.removeMode(2); // remove show mode, if no viewer plugin
        dlg.setType(item->getType());
        dlg.workingDirectory = workingDirectory;
        if(dlg.exec()) {
            workingDirectory = dlg.workingDirectory;
            item->setFilename( dlg.getFilename() );

            if(item->getMode() == PluginItem::ModeDisable && dlg.getMode() != PluginItem::ModeDisable)
                callInitFile = true;

            item->setMode( dlg.getMode() );
            item->setType( dlg.getType() );

            /* update plugin item */
            updatePlugin(item);
            item->savePluginModeToSettings();


        }
        if(callInitFile)
        {
            processMsgAfterPluginmodeChange(item);
        }
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No Plugin selected!"));

}

void MainWindow::processMsgAfterPluginmodeChange(PluginItem *item){

    QList<PluginItem*> activeDecoderPlugins;
    QList<PluginItem*> activeViewerPlugins;

#ifdef DEBUG_PERFORMANCE
    QTime t;
#endif

    if(!item){
        qDebug()<<"Error: Plugin could not be initizialised. Item null.";
        return;
    }

    QProgressDialog pluginprogress(QString("Applying Plugins for message 0/%1").arg(qfile.size()), "Cancel", 0, qfile.size(), this);
    pluginprogress.setWindowTitle("DLT Viewer");
    pluginprogress.setWindowModality(Qt::WindowModal);
    pluginprogress.show();

    if(item->pluginviewerinterface)
    {
        activeViewerPlugins.append(item);

#ifdef DEBUG_PERFORMANCE
        t.start();
#endif
        item->pluginviewerinterface->initFileStart(&qfile);

#ifdef DEBUG_PERFORMANCE
        qDebug() << "Time for initFileStart: " << item->getName() << ": " << t.elapsed()/1000 << "s" ;
#endif

        PluginItem *decoderItem = 0;
        for(int i = 0; i < project.plugin->topLevelItemCount(); i++)
        {
            decoderItem = (PluginItem*)project.plugin->topLevelItem(i);

            if(decoderItem->getMode() != PluginItem::ModeDisable && decoderItem->plugindecoderinterface)
            {

                activeDecoderPlugins.append(decoderItem);
            }
        }

    }
    // Possible to use several threads to process the splitted  trace file
    // But this is slower than using one thread.
    // Reason: Random IO on disk is very slow
    // Example to use more than one threads
    //if(qfile.size()!=0)
    //    threadCount = QThread::idealThreadCount();
    //if(threadCount < 1)
    //        threadCount = 1;
    //int chunkSize = qfile.size() / threadCount;
    //qDebug()<<"messages: " <<  qfile.size() << "chunkSize: " << chunkSize;

    ThreadPlugin thread;

    thread.setQDltFile(&qfile);
    thread.setActiveDecoderPlugins(&activeDecoderPlugins);
    thread.setActiveViewerPlugins(&activeViewerPlugins);
    thread.setStartIndex(0);
    thread.setStopIndex( qfile.size());

    connect(&pluginprogress, SIGNAL(canceled()), &thread, SLOT(stopProcessMsg()));
    connect(&thread, SIGNAL(percentageComplete(int)), &pluginprogress, SLOT(setValue(int)));
    connect(&thread, SIGNAL(updateProgressText(QString)), &pluginprogress, SLOT(setLabelText(QString)));
    connect(&thread, SIGNAL(finished()), this, SLOT(threadpluginFinished()));

    threadIsRunnging = true;

#ifdef DEBUG_PERFORMANCE
    t.start();
#endif

    thread.start();
    thread.setPriority(QThread::HighPriority);

    while(threadIsRunnging){
        QApplication::processEvents();
    }

    if(item->pluginviewerinterface)
    {
#ifdef DEBUG_PERFORMANCE
    qDebug() << "Time to initMsg,isMsg,decodeMsg,checkFilter,initMsgDecoded: " << t.elapsed()/1000 << "s" ;
    t.start();
#endif

        item->pluginviewerinterface->initFileFinish();

#ifdef DEBUG_PERFORMANCE
    qDebug() << "Time for initFileFinish: " << item->getName() << ": " << t.elapsed()/1000 << "s" ;
#endif
    }


    if(pluginprogress.wasCanceled())
    {
        QMessageBox::warning(this, tr("DLT Viewer"), tr("You canceled the initialisation progress. Not all messages could be processed by the enabled Plugins!"), QMessageBox::Ok);
    }
}


void MainWindow::on_action_menuPlugin_Show_triggered() {

    /* get selected plugin */
    QList<QTreeWidgetItem *> list = project.plugin->selectedItems();
    if((list.count() == 1) ) {
        PluginItem* item = (PluginItem*) list.at(0);

        if(item->getMode() != PluginItem::ModeShow){
            int oldMode = item->getMode();

            item->setMode( PluginItem::ModeShow );
            item->savePluginModeToSettings();
            updatePlugin(item);

            if(oldMode == PluginItem::ModeDisable){
                processMsgAfterPluginmodeChange(item);
            }
        }else{
            QMessageBox::warning(0, QString("DLT Viewer"),
                                 QString("The selected Plugin is already active."));
        }
    }
    else {
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No Plugin selected!"));
    }

}

void MainWindow::on_action_menuPlugin_Hide_triggered() {
    /* get selected plugin */
    QList<QTreeWidgetItem *> list = project.plugin->selectedItems();
    if((list.count() == 1) ) {
        PluginItem* item = (PluginItem*) list.at(0);

        if(item->getMode() == PluginItem::ModeShow){
            item->setMode( PluginItem::ModeEnable );
            item->savePluginModeToSettings();
            updatePlugin(item);
        }else{
            QMessageBox::warning(0, QString("DLT Viewer"),
                                 QString("The selected Plugin is already hidden or deactivated."));
        }
    }
    else {
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No Plugin selected!"));
    }

}

void MainWindow::on_action_menuPlugin_Disable_triggered()
{
    /* get selected plugin */
    QList<QTreeWidgetItem *> list = project.plugin->selectedItems();
    if((list.count() == 1) ) {
        PluginItem* item = (PluginItem*) list.at(0);

        if(item->getMode() != PluginItem::ModeDisable){
            item->setMode( PluginItem::ModeDisable );
            item->savePluginModeToSettings();
            updatePlugin(item);
        }else{
            QMessageBox::warning(0, QString("DLT Viewer"),
                                 QString("The selected Plugin is already deactivated."));
        }
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No Plugin selected!"));
}

void MainWindow::on_action_menuPlugin_ExecuteCommand_triggered()
{
    QList<QTreeWidgetItem *> list = project.plugin->selectedItems();
    PluginItem* item = (PluginItem*) list.at(0);
    QDltPluginCommandInterface *plugin = item->plugincommandinterface;
    QDLTPluginInterface *plugin_core = item->plugininterface;

    if(!plugin)
    {
        QMessageBox::warning(this, QString("DLT Viewer"),
                             QString("Not a Command Plugin."));
        return;
    }

    CommandPluginDialog dlg(this);
    dlg.setPlugin(plugin);
    int ret = dlg.exec();
    if(ret == QDialog::Accepted)
    {
        // Start processing the command
        QString cmd = dlg.command();
        QList<QString> params = dlg.params();
        if(!plugin->command(cmd, params))
        {
            // Show error
            QMessageBox::warning(this, "Error", plugin_core->error());
            return;
        }

        // Show progress dialog
        QProgressDialog progress(plugin_core->name(), "Cancel", 0, 100, this);
        progress.show();
        while(plugin->commandProgress() < 100)
        {
            progress.setValue(plugin->commandProgress());
            if(progress.wasCanceled())
            {
                plugin->cancel();
                return;
            }
            // TODO: This caused crashing someplace else. Use caution.
            QApplication::processEvents();
        }
        progress.close();

        // Show return value
        QMessageBox::information(this, plugin_core->name(), plugin->commandReturnValue());
    }
}

//----------------------------------------------------------------------------
// Filter functionalities
//----------------------------------------------------------------------------

void MainWindow::filterAddTable() {
    QModelIndexList list = ui->tableView->selectionModel()->selection().indexes();
    QDltMsg msg;
    QByteArray data;

    if(list.count()<=0)
    {
        QMessageBox::critical(0, QString("DLT Viewer"),
                              QString("No message selected"));
        return;
    }

    QModelIndex index;
    for(int num=0; num < list.count();num++)
    {
        index = list[num];
        if(index.column()==0)
        {
            break;
        }
    }

    data = qfile.getMsgFilter(index.row());
    msg.setMsg(data);

    /* decode message if necessary */
    iterateDecodersForMsg(msg,1);

    /* show filter dialog */
    FilterDialog dlg;
    dlg.setEnableEcuId(!msg.getEcuid().isEmpty());
    dlg.setEcuId(msg.getEcuid());
    dlg.setEnableApplicationId(!msg.getApid().isEmpty());
    dlg.setApplicationId(msg.getApid());
    dlg.setEnableContextId(!msg.getCtid().isEmpty());
    dlg.setContextId(msg.getCtid());
    dlg.setHeaderText(msg.toStringHeader());
    dlg.setPayloadText(msg.toStringPayload());

    if(dlg.exec()==1) {
        FilterItem* item = new FilterItem(0);
        project.filter->addTopLevelItem(item);
        filterDialogRead(dlg,item);
    }
}

void MainWindow::filterAdd() {
    EcuItem* ecuitem = 0;
    ContextItem* conitem = 0;
    ApplicationItem* appitem = 0;

    /* add filter triggered from popupmenu in Context list */
    /* get selected context from configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        ecuitem = (EcuItem*) list.at(0);
    }
    if((list.count() == 1) && (list.at(0)->type() == application_type))
    {
        appitem = (ApplicationItem*) list.at(0);
        ecuitem = (EcuItem*) appitem->parent();
    }
    if((list.count() == 1) && (list.at(0)->type() == context_type))
    {
        conitem = (ContextItem*) list.at(0);
        appitem = (ApplicationItem*) conitem->parent();
        ecuitem = (EcuItem*) appitem->parent();
    }

    /* show filter dialog */
    FilterDialog dlg;

    if(ecuitem)
    {
        dlg.setEnableEcuId(true);
        dlg.setEcuId(ecuitem->id);
    }

    if(appitem)
    {
        dlg.setEnableApplicationId(true);
        dlg.setApplicationId(appitem->id);
    }

    if(conitem)
    {
        dlg.setEnableContextId(true);
        dlg.setContextId(conitem->id);
    }

    if(dlg.exec()==1) {
        FilterItem* item = new FilterItem(0);
        project.filter->addTopLevelItem(item);
        filterDialogRead(dlg,item);
    }
}

void MainWindow::on_action_menuFilter_Save_As_triggered()
{

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save DLT Filters"), workingDirectory, tr("DLT Filter File (*.dlf);;All files (*.*)"));

    if(!fileName.isEmpty())
    {
        project.SaveFilter(fileName);
        setCurrentFilters(fileName);
    }
}


void MainWindow::on_action_menuFilter_Load_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Load DLT Filter file"), workingDirectory, tr("DLT Filter Files (*.dlf);;All files (*.*)"));

    if(!fileName.isEmpty() && project.LoadFilter(fileName,true))
    {
        ui->filterButton->setIcon(QIcon(":/toolbar/png/weather-storm.png"));
        ui->filterStatus->setText("Filters changed. Please enable filtering.");

        ui->filterButton->setChecked(Qt::Unchecked);
        ui->filterButton->setText("Enable filters");

        setCurrentFilters(fileName);

    }

    on_filterWidget_itemSelectionChanged();
}

void MainWindow::on_action_menuFilter_Add_triggered() {
    /* show filter dialog */
    FilterDialog dlg;

    if(dlg.exec()==1) {
        FilterItem* item = new FilterItem(0);
        project.filter->addTopLevelItem(item);
        filterDialogRead(dlg,item);
    }
}

void MainWindow::filterDialogWrite(FilterDialog &dlg,FilterItem* item)
{
    dlg.setType((int)(item->type));

    dlg.setName(item->name);
    dlg.setEcuId(item->ecuId);
    dlg.setApplicationId(item->applicationId);
    dlg.setContextId(item->contextId);
    dlg.setHeaderText(item->headerText);
    dlg.setPayloadText(item->payloadText);

    dlg.setActive(item->enableFilter);
    dlg.setEnableEcuId(item->enableEcuId);
    dlg.setEnableApplicationId(item->enableApplicationId);
    dlg.setEnableContextId(item->enableContextId);
    dlg.setEnableHeaderText(item->enableHeaderText);
    dlg.setEnablePayloadText(item->enablePayloadText);
    dlg.setEnableCtrlMsgs(item->enableCtrlMsgs);
    dlg.setEnableLogLevelMax(item->enableLogLevelMax);
    dlg.setEnableLogLevelMin(item->enableLogLevelMin);

    dlg.setFilterColour(item->filterColour);

    dlg.setLogLevelMax(item->logLevelMax);
    dlg.setLogLevelMin(item->logLevelMin);
}

void MainWindow::filterDialogRead(FilterDialog &dlg,FilterItem* item)
{
    item->type = (FilterItem::FilterType)(dlg.getType());

    item->name = dlg.getName();
    item->ecuId = dlg.getEcuId();
    item->applicationId = dlg.getApplicationId();
    item->contextId = dlg.getContextId();
    item->headerText = dlg.getHeaderText();
    item->payloadText = dlg.getPayloadText();

    item->enableFilter = dlg.getEnableActive();
    item->enableEcuId = dlg.getEnableEcuId();
    item->enableApplicationId = dlg.getEnableApplicationId();
    item->enableContextId = dlg.getEnableContextId();
    item->enableHeaderText = dlg.getEnableHeaderText();
    item->enablePayloadText = dlg.getEnablePayloadText();
    item->enableCtrlMsgs = dlg.getEnableCtrlMsgs();
    item->enableLogLevelMax = dlg.getEnableLogLevelMax();
    item->enableLogLevelMin = dlg.getEnableLogLevelMin();

    item->filterColour = dlg.getFilterColour();
    item->logLevelMax = dlg.getLogLevelMax();
    item->logLevelMin = dlg.getLogLevelMin();

    /* update filter item */
    item->update();

    ui->filterButton->setIcon(QIcon(":/toolbar/png/weather-storm.png"));
    ui->filterStatus->setText("Filters changed. Please enable filtering.");

    ui->filterButton->setChecked(Qt::Unchecked);
    ui->filterButton->setText("Enable filters");

    on_filterWidget_itemSelectionChanged();
}

void MainWindow::on_action_menuFilter_Duplicate_triggered() {
    QTreeWidget *widget;

    /* get currently visible filter list in user interface */
    if(ui->tabPFilter->isVisible()) {
        widget = project.filter;
    }
    else
        return;

    /* get selected filter form list */
    QList<QTreeWidgetItem *> list = widget->selectedItems();
    if((list.count() == 1) ) {
        FilterItem* item = (FilterItem*) list.at(0);

        /* show filter dialog */
        FilterDialog dlg;
        filterDialogWrite(dlg,item);
        if(dlg.exec())
        {
            FilterItem* newitem = new FilterItem(0);
            project.filter->addTopLevelItem(newitem);
            filterDialogRead(dlg,newitem);
        }
    }
    else {
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No Filter selected!"));
    }
}

void MainWindow::on_action_menuFilter_Edit_triggered() {
    QTreeWidget *widget;

    /* get currently visible filter list in user interface */
    if(ui->tabPFilter->isVisible()) {
        widget = project.filter;
    }
    else
        return;

    /* get selected filter form list */
    QList<QTreeWidgetItem *> list = widget->selectedItems();
    if((list.count() == 1) ) {
        FilterItem* item = (FilterItem*) list.at(0);

        /* show filter dialog */
        FilterDialog dlg;
        filterDialogWrite(dlg,item);
        if(dlg.exec())
        {
            filterDialogRead(dlg,item);
        }
    }
    else {
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No Filter selected!"));
    }
}

void MainWindow::on_action_menuFilter_Delete_triggered() {
    QTreeWidget *widget;

    /* get currently visible filter list in user interface */
    if(ui->tabPFilter->isVisible()) {
        widget = project.filter;
    }
    else
        return;

    /* get selected filter from list */
    QList<QTreeWidgetItem *> list = widget->selectedItems();
    if((list.count() == 1) ) {
        /* delete filter */
        delete widget->takeTopLevelItem(widget->indexOfTopLevelItem(list.at(0)));

        ui->filterButton->setIcon(QIcon(":/toolbar/png/weather-storm.png"));
        ui->filterStatus->setText("Filters changed. Please enable filtering.");

        ui->filterButton->setChecked(Qt::Unchecked);
        ui->filterButton->setText("Enable filters");

        /* update filter list in DLT log file */
        //filterUpdate();
    }
    else {
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No Filter selected!"));
    }

    on_filterWidget_itemSelectionChanged();
}

void MainWindow::on_action_menuFilter_Clear_all_triggered() {
    /* delete complete filter list */
    project.filter->clear();

    ui->filterButton->setIcon(QIcon(":/toolbar/png/weather-storm.png"));
    ui->filterStatus->setText("Filters changed. Please enable filtering.");

    ui->filterButton->setChecked(Qt::Unchecked);
    ui->filterButton->setText("Enable filters");

}

void MainWindow::filterUpdate() {
    QDltFilter afilter;

    /* update all filters from filter configuration to DLT filter list */

    /* clear old filter list */
    qfile.clearFilter();

    /* iterate through all positive filters */
    for(int num = 0; num < project.filter->topLevelItemCount (); num++)
    {
        FilterItem *item = (FilterItem*)project.filter->topLevelItem(num);

        afilter.ecuid = item->ecuId;
        afilter.apid = item->applicationId;
        afilter.ctid = item->contextId;
        afilter.header = item->headerText;
        afilter.payload = item->payloadText;

        afilter.enableFilter = item->enableFilter;
        afilter.enableEcuid = item->enableEcuId;
        afilter.enableApid = item->enableApplicationId;
        afilter.enableCtid = item->enableContextId;
        afilter.enableHeader = item->enableHeaderText;
        afilter.enablePayload = item->enablePayloadText;
        afilter.enableCtrlMsgs = item->enableCtrlMsgs;
        afilter.enableLogLevelMax = item->enableLogLevelMax;
        afilter.enableLogLevelMin = item->enableLogLevelMin;

        afilter.logLevelMax = item->logLevelMax;
        afilter.logLevelMin = item->logLevelMin;

        afilter.filterColour = item->filterColour;
        item->setBackground(0,item->filterColour);
        item->setBackground(1,item->filterColour);

        switch(item->type)
        {
        case FilterItem::positive:
            qfile.addPFilter(afilter);
            break;
        case FilterItem::negative:
            qfile.addNFilter(afilter);
            break;
        case FilterItem::marker:
            qfile.addMarker(afilter);
            break;
        }

    }
}

void MainWindow::on_filterButton_clicked(bool checked)
{
    filterUpdate();

    /* enable/disable filter */
    qfile.enableFilter(checked);
    qfile.clearFilterIndex();

    if(checked)
    {
        ui->filterButton->setText("Filters enabled");
        ui->filterButton->setIcon(QIcon(":/toolbar/png/weather-clear.png"));
        ui->filterStatus->setText("");

        PluginItem *item;
        QList<PluginItem*> activeDecoderPlugins;

        QProgressDialog filterprogress("Applying filters for message 0/0", "Cancel", 0, qfile.size(), this);
        filterprogress.setWindowTitle("DLT Viewer");
        filterprogress.setWindowModality(Qt::WindowModal);
        filterprogress.show();

        for(int i = 0; i < project.plugin->topLevelItemCount(); i++)
        {
            item = (PluginItem*)project.plugin->topLevelItem(i);

            if(item->getMode() != PluginItem::ModeDisable &&item->plugindecoderinterface )
            {

                    activeDecoderPlugins.append(item);
            }
        }

        ThreadFilter thread;
        thread.setQDltFile(&qfile);
        thread.setActiveDecoderPlugins(&activeDecoderPlugins);
        thread.setStartIndex(0);
        thread.setStopIndex( qfile.size());

        connect(&thread, SIGNAL(percentageComplete(int)), &filterprogress, SLOT(setValue(int)));
        connect(&thread, SIGNAL(updateProgressText(QString)), &filterprogress, SLOT(setLabelText(QString)));
        connect(&thread, SIGNAL(finished()), this, SLOT(threadpluginFinished()));
        connect(&filterprogress, SIGNAL(canceled()), &thread, SLOT(stopProcessMsg()));

        threadIsRunnging = true;

        thread.start();
        thread.setPriority(QThread::HighestPriority);

        while(threadIsRunnging){
            QApplication::processEvents();
        }

        if(filterprogress.wasCanceled())
        {
            QMessageBox::warning(this, tr("DLT Viewer"), tr("You canceled the filter progress. Not all messages could be processed by the activated filters!"), QMessageBox::Ok);
        }

    }
    else
    {
        ui->filterButton->setText("Filters disabled");
        ui->filterButton->setIcon(QIcon(":/toolbar/png/weather-overcast.png"));
        ui->filterStatus->setText("");
    }

    ui->tableView->selectionModel()->clear();
    tableModel->modelChanged();
}


void MainWindow::on_tableView_customContextMenuRequested(QPoint pos)
{
    /* show custom pop menu  for configuration */
    QPoint globalPos = ui->tableView->mapToGlobal(pos);
    QMenu menu(ui->tableView);
    QAction *action;
    QModelIndexList list = ui->tableView->selectionModel()->selection().indexes();

    action = new QAction("&Copy Selection to Clipboard", this);
    connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Copy_to_clipboard_triggered()));
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("&Export ASCII", this);
    if(qfile.sizeFilter() <= 0)
        action->setEnabled(false);
    else
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFile_Export_ASCII_triggered()));

    menu.addAction(action);

    action = new QAction("Export Selection", this);
    if(list.count() <= 0)
        action->setEnabled(false);
    else
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFile_Export_Selection_triggered()));
    menu.addAction(action);

    action = new QAction("Export Selection ASCII", this);
    if(list.count() <= 0)
        action->setEnabled(false);
    else
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFile_Export_Selection_ASCII_triggered()));
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("&Filter Add", this);
    connect(action, SIGNAL(triggered()), this, SLOT(filterAddTable()));
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("Load Filter(s)...", this);
    connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Load_triggered()));
    menu.addAction(action);

    /* show popup menu */
    menu.exec(globalPos);
}

void MainWindow::keyPressEvent ( QKeyEvent * event )
{
    if(event->matches(QKeySequence::Copy))
    {
        exportSelection(true,false);
    }
    if(event->matches(QKeySequence::Paste))
    {
        QMessageBox::warning(this, QString("Paste"),
                             QString("pressed"));
    }
    if(event->matches(QKeySequence::Cut))
    {
        QMessageBox::warning(this, QString("Cut"),
                             QString("pressed"));
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QString filename;
    if (event->mimeData()->hasUrls())
    {
        QUrl url = event->mimeData()->urls()[0];
        filename = url.toLocalFile();

        if(filename.endsWith(".dlt", Qt::CaseInsensitive))
        {
            /* DLT log file dropped */
            openDltFile(filename);
        }
        else if(filename.endsWith(".dlp", Qt::CaseInsensitive))
        {
            /* Project file dropped */
            openDlpFile(filename);
        }
        else
        {
            QMessageBox::warning(this, QString("Drag&Drop"),
                                 QString("No DLT log file or project file dropped!\n")+filename);
        }
    }
    else
    {
        QMessageBox::warning(this, QString("Drag&Drop"),
                             QString("No DLT log file or project file dropped!\n")+filename);
    }
}

void MainWindow::sectionInTableDoubleClicked(int logicalIndex){
    ui->tableView->resizeColumnToContents(logicalIndex);
}

void MainWindow::on_pluginWidget_itemExpanded(QTreeWidgetItem* item)
{
    PluginItem *plugin = (PluginItem*)item;
    plugin->takeChildren();
    QStringList list = plugin->plugininterface->infoConfig();
    for(int num=0;num<list.size();num++) {
        plugin->addChild(new QTreeWidgetItem(QStringList(list.at(num))));
    }
}

void MainWindow::on_filterWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    on_filterWidget_itemSelectionChanged();

    if(column == 0)
    {
        FilterItem *tmp = (FilterItem*)item;
        if(tmp->checkState(column) == Qt::Unchecked)
        {
            tmp->enableFilter = false;
        }
        else
        {
            tmp->enableFilter = true;
        }

        ui->filterButton->setIcon(QIcon(":/toolbar/png/weather-storm.png"));
        ui->filterStatus->setText("Filters changed. Please enable filtering.");

        ui->filterButton->setChecked(Qt::Unchecked);
        ui->filterButton->setText("Enable filters");

        //filterUpdate();
    }
}


void MainWindow::iterateDecodersForMsg(QDltMsg &msg, int triggeredByUser)
{
    for(int i = 0; i < project.plugin->topLevelItemCount (); i++)
    {
        PluginItem *item = (PluginItem*)project.plugin->topLevelItem(i);

        if(item->getMode() != item->ModeDisable &&
                item->plugindecoderinterface &&
                item->plugindecoderinterface->isMsg(msg,triggeredByUser))
        {
            item->plugindecoderinterface->decodeMsg(msg,triggeredByUser);
            break;
        }
    }
}

void MainWindow::on_action_menuConfig_Collapse_All_ECUs_triggered()
{
    ui->configWidget->collapseAll();
}

void MainWindow::on_action_menuConfig_Expand_All_ECUs_triggered()
{
    ui->configWidget->expandAll();
}



void MainWindow::on_action_menuConfig_Copy_to_clipboard_triggered()
{
    exportSelection(true,false);
}

void MainWindow::on_action_menuFilter_Append_Filters_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Load DLT Filter file"), workingDirectory, tr("DLT Filter Files (*.dlf);;All files (*.*)"));

    if(!fileName.isEmpty() && project.LoadFilter(fileName,false))
    {
        ui->filterButton->setIcon(QIcon(":/toolbar/png/weather-storm.png"));
        ui->filterStatus->setText("Filters changed. Please enable filtering.");

        ui->filterButton->setChecked(Qt::Unchecked);
        ui->filterButton->setText("Enable filters");

        setCurrentFilters(fileName);

    }

    on_filterWidget_itemSelectionChanged();

}
