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

#include <iostream>
#include <QMimeData>
#include <QTreeView>
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
#include <QDateTime>
#include <QLabel>

/**
 * From QDlt.
 * Must be a "C" include to interpret the imports correctly
 * for MSVC compilers.
 **/
extern "C" {
    #include "dlt_common.h"
    #include "dlt_user.h"
}

#include "mainwindow.h"

#include "ecudialog.h"
#include "applicationdialog.h"
#include "contextdialog.h"
#include "multiplecontextdialog.h"
#include "plugindialog.h"
#include "settingsdialog.h"
#include "injectiondialog.h"
#include "qextserialenumerator.h"
#include "version.h"
#include "dltfileutils.h"
#include "dltuiutils.h"
#include "dltexporter.h"
#include "jumptodialog.h"
#include "fieldnames.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer(this),
    qcontrol(this),
    pulseButtonColor(255, 40, 40)
{
    ui->setupUi(this);
    ui->enableConfigFrame->setVisible(false);
    setAcceptDrops(true);

    initState();



    /* Apply loaded settings */
    applySettings();

    initSearchTable();

    initView();

    initSignalConnections();

    initFileHandling();




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





    restoreGeometry(DltSettingsManager::getInstance()->value("geometry").toByteArray());
    restoreState(DltSettingsManager::getInstance()->value("windowState").toByteArray());
}

MainWindow::~MainWindow()
{
    DltSettingsManager::close();
    /**
     * All plugin dockwidgets must be removed from the layout manually and
     * then deleted. This has to be done here, because they contain
     * UI components owned by the plugins. The plugins will destroy their
     * own UI components. If the dockwidget is not manually removed, the
     * parent destructor of MainWindow will try to automatically delete
     * the dockWidgets subcomponents, which are already destroyed
     * when unloading plugins.
     **/
    for(int i=0;i<project.plugin->topLevelItemCount();i++)
    {
        PluginItem *item = (PluginItem *) project.plugin->topLevelItem(i);
        if(item->dockWidget != NULL)
        {
            removeDockWidget(item->dockWidget);
            delete item->dockWidget;
        }
    }

    delete ui;
    delete tableModel;
    delete searchDlg;
    delete dltIndexer;
    delete m_shortcut_searchnext;
    delete m_shortcut_searchprev;
}

void MainWindow::initState()
{

    /* Settings */
    settings = new SettingsDialog(&qfile,this);
    settings->assertSettingsVersion();
    settings->readSettings();
    recentFiles = settings->getRecentFiles();
    recentProjects = settings->getRecentProjects();
    recentFilters = settings->getRecentFilters();

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


    /* initialize injection */
    injectionAplicationId.clear();
    injectionContextId.clear();
    injectionServiceId.clear();
    injectionData.clear();
}

void MainWindow::initView()
{

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

    /* Start pulsing the apply changes button, when filters draged&dropped */
    connect(ui->filterWidget, SIGNAL(filterItemDropped()), this, SLOT(filterOrderChanged()));

    /* initialise statusbar */
    totalBytesRcvd = 0;
    totalByteErrorsRcvd = 0;
    totalSyncFoundRcvd = 0;
    statusFilename = new QLabel("no log file loaded");
    statusBytesReceived = new QLabel("Recv: 0");
    statusByteErrorsReceived = new QLabel("Recv Errors: 0");
    statusSyncFoundReceived = new QLabel("Sync found: 0");
    statusBar()->addWidget(statusFilename);
    statusBar()->addWidget(statusBytesReceived);
    statusBar()->addWidget(statusByteErrorsReceived);
    statusBar()->addWidget(statusSyncFoundReceived);

    /* Create search text box */
    searchTextbox = new QLineEdit();
    searchDlg->appendLineEdit(searchTextbox);
    connect(searchTextbox, SIGNAL(textEdited(QString)),searchDlg,SLOT(textEditedFromToolbar(QString)));
    connect(searchTextbox, SIGNAL(returnPressed()),searchDlg,SLOT(findNextClicked()));

    /* Initialize toolbars. Most of the construction and connection is done via the
     * UI file. See mainwindow.ui, ActionEditor and Signal & Slots editor */
    QList<QAction *> mainActions = ui->mainToolBar->actions();
    m_searchActions = ui->searchToolbar->actions();

    /* Point scroll toggle button to right place */
    scrollButton = mainActions.at(ToolbarPosition::AutoScroll);

    /* Update the scrollbutton status */
    updateScrollButton();
}

void MainWindow::initSignalConnections()
{
    /* Connect RegExp settings from and to search dialog */
    connect(m_searchActions.at(ToolbarPosition::Regexp), SIGNAL(toggled(bool)), searchDlg->regexpCheckBox, SLOT(setChecked(bool)));
    connect(searchDlg->regexpCheckBox, SIGNAL(toggled(bool)), m_searchActions.at(ToolbarPosition::Regexp), SLOT(setChecked(bool)));

    /* Connect previous and next buttons to search dialog slots */
    connect(m_searchActions.at(ToolbarPosition::FindPrevious), SIGNAL(triggered()), searchDlg, SLOT(findPreviousClicked()));
    connect(m_searchActions.at(ToolbarPosition::FindNext), SIGNAL(triggered()), searchDlg, SLOT(findNextClicked()));

    connect(searchDlg->CheckBoxSearchtoList,SIGNAL(toggled(bool)),ui->actionSearchList,SLOT(setChecked(bool)));
    connect(ui->actionSearchList,SIGNAL(toggled(bool)),searchDlg->CheckBoxSearchtoList,SLOT(setChecked(bool)));
    ui->actionSearchList->setChecked(searchDlg->searchtoIndex());


    /* Insert search text box to search toolbar, before previous button */
    QAction *before = m_searchActions.at(ToolbarPosition::FindPrevious);
    ui->searchToolbar->insertWidget(before, searchTextbox);

    /* adding shortcuts - regard: in the search window, the signal is caught by another way, this here only catches the keys when main window is active */
    m_shortcut_searchnext = new QShortcut(QKeySequence("F3"), this);
    connect(m_shortcut_searchnext, SIGNAL(activated()), searchDlg, SLOT( on_pushButtonNext_clicked() ) );
    m_shortcut_searchprev = new QShortcut(QKeySequence("F2"), this);
    connect(m_shortcut_searchprev, SIGNAL(activated()), searchDlg, SLOT( on_pushButtonPrevious_clicked() ) );

    connect((QObject*)(ui->tableView->verticalScrollBar()), SIGNAL(valueChanged(int)), this, SLOT(tableViewValueChanged(int)));
    connect(ui->tableView->horizontalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(sectionInTableDoubleClicked(int)));

    //for search result table
    connect(searchDlg, SIGNAL(refreshedSearchIndex()), this, SLOT(searchTableRenewed()));
    connect( m_searchresultsTable, SIGNAL( doubleClicked (QModelIndex) ), this, SLOT( searchtable_cellSelected( QModelIndex ) ) );

}

void MainWindow::initSearchTable()
{

    //init search Dialog
    searchDlg = new SearchDialog(this);
    searchDlg->file = &qfile;
    searchDlg->table = ui->tableView;
    searchDlg->plugin = project.plugin;

    /* initialise DLT Search handling */
    m_searchtableModel = new SearchTableModel("Search Index Mainwindow");
    m_searchtableModel->qfile = &qfile;
    m_searchtableModel->project = &project;

    searchDlg->registerSearchTableModel(m_searchtableModel);

    m_searchresultsTable = ui->tableView_SearchIndex;
    m_searchresultsTable->setModel(m_searchtableModel);

    m_searchresultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);


    m_searchresultsTable->verticalHeader()->setVisible(false);    
    m_searchresultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //Removing lines which are unlinkely to be necessary for a search. Maybe make configurable.
    //Ideally possible with right-click
    m_searchresultsTable->setColumnHidden(FieldNames::Counter, true);
    //m_searchresultsTable->setColumnHidden(FieldNames::EcuId, true);
    m_searchresultsTable->setColumnHidden(FieldNames::Type, true);
    m_searchresultsTable->setColumnHidden(FieldNames::Subtype, true);
    m_searchresultsTable->setColumnHidden(FieldNames::Mode, true);
    m_searchresultsTable->setColumnHidden(FieldNames::ArgCount, true);

    QFont searchtableViewFont = m_searchresultsTable->font();
    searchtableViewFont.setPointSize(settings->fontSize);
    m_searchresultsTable->setFont(searchtableViewFont);

    // Rescale the height of a row to choosen font size + 8 pixels
    m_searchresultsTable->verticalHeader()->setDefaultSectionSize(settings->fontSize+8);

    /* set table size and en */
    m_searchresultsTable->setColumnWidth(FieldNames::Index,50);
    m_searchresultsTable->setColumnWidth(FieldNames::Time,150);
    m_searchresultsTable->setColumnWidth(FieldNames::TimeStamp,70);
    m_searchresultsTable->setColumnWidth(3,40);
    m_searchresultsTable->setColumnWidth(FieldNames::EcuId,40);
    m_searchresultsTable->setColumnWidth(5,40);
    m_searchresultsTable->setColumnWidth(6,40);
    m_searchresultsTable->setColumnWidth(7,50);
    m_searchresultsTable->setColumnWidth(8,50);
    m_searchresultsTable->setColumnWidth(9,40);
    m_searchresultsTable->setColumnWidth(10,40);
    m_searchresultsTable->setColumnWidth(FieldNames::Payload,1000);

    ui->dockWidgetSearchIndex->hide();    

}

void MainWindow::initFileHandling()
{
    /* Initialize dlt-file indexer  */
    dltIndexer = new DltFileIndexer(&qfile, this);

    /* Plugins/Filters enabled checkboxes */
    ui->pluginsEnabled->setChecked(DltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool());
    ui->filtersEnabled->setChecked(DltSettingsManager::getInstance()->value("startup/filtersEnabled", true).toBool());

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
    outputfileIsFromCLI = false;
    outputfileIsTemporary = false;
    if(OptManager::getInstance()->isLogFile())
    {
        openDltFile(OptManager::getInstance()->getLogFile());
        /* Command line file is treated as temp file */
        outputfileIsTemporary = true;
        outputfileIsFromCLI = true;
    }
    else
    {
        /* load default log file */
        statusFilename->setText("no log file loaded");
        if(settings->defaultLogFile)
        {
            openDltFile(settings->defaultLogFileName);
            outputfileIsFromCLI = false;
            outputfileIsTemporary = false;
        }
        else
        {
            /* Create temp file */
            QString fn = DltFileUtils::createTempFile(DltFileUtils::getTempPath(settings));
            outputfile.setFileName(fn);
            outputfileIsTemporary = true;
            outputfileIsFromCLI = false;
            if(outputfile.open(QIODevice::WriteOnly|QIODevice::Truncate))
                reloadLogFile();
            else
                QMessageBox::critical(0, QString("DLT Viewer"),
                                      QString("Cannot load temporary log file \"%1\"\n%2")
                                      .arg(outputfile.fileName())
                                      .arg(outputfile.errorString()));
        }
    }

    if(OptManager::getInstance()->isFilterFile()){
        if(project.LoadFilter(OptManager::getInstance()->getFilterFile(),false))
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


	draw_timer.setSingleShot (true);
    connect(&draw_timer, SIGNAL(timeout()), this, SLOT(draw_timeout()));


    DltSettingsManager *settingsmanager = DltSettingsManager::getInstance();
    bool startup_minimized = settingsmanager->value("StartUpMinimized",false).toBool();
    if (startup_minimized)
        this->setWindowState(Qt::WindowMinimized);
}

void MainWindow::commandLineConvertToASCII(){
    QByteArray data;
    QDltMsg msg;
    QString text;

    qfile.enableFilter(true);
    openDltFile(OptManager::getInstance()->getConvertSourceFile());
    outputfileIsFromCLI = false;
    outputfileIsTemporary = false;

    QFile asciiFile(OptManager::getInstance()->getConvertDestFile());
    if(!asciiFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //Error output
        exit(0);
    }

    for(int num = 0;num< qfile.sizeFilter();num++)
    {

        /* get message from log file */
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
        asciiFile.write(text.toLatin1().constData());

    }

    asciiFile.close();
}

void MainWindow::ErrorMessage(QMessageBox::Icon level, QString title, QString message){

  if (OptManager::getInstance()->issilentMode())
    {
      qDebug()<<message;
    }
  else
    {
      if (level == QMessageBox::Critical)
        QMessageBox::critical(this, title, message);
      else if (level == QMessageBox::Warning)
        QMessageBox::warning(this, title, message);
      else if (level == QMessageBox::Information)
        QMessageBox::information(this, title, message);
      else
        QMessageBox::critical(this, "ErrorMessage problem", "unhandled case");
    }

}

void MainWindow::commandLineExecutePlugin(QString plugin, QString cmd, QStringList params)
{
    bool plugin_found = false;
    for(int i = 0;i < project.plugin->topLevelItemCount();i++)
    {
        PluginItem *item = (PluginItem*)project.plugin->topLevelItem(i);
        if(item->getName().compare(plugin, Qt::CaseInsensitive) == 0)
        {
            plugin_found = true;
            /* Check that this is a command plugin */
            QDltPluginCommandInterface *cmdif = item->plugincommandinterface;
            if(cmdif == NULL)
            {
                QString msg("Error: ");
                msg = msg+plugin+" is not a command plugin.";
                ErrorMessage(QMessageBox::Critical, plugin, msg);
                exit(-1);
            }
            if(!cmdif->command(cmd, params))
            {
                if(item->plugininterface)
                {
                    QString msg("Error: ");
                    msg.append(plugin);
                    msg.append(item->plugininterface->error());
                    ErrorMessage(QMessageBox::Warning,plugin, msg);
                }
                else
                {
                    QString msg("Error: unhandled case in: ");
                    msg.append(__func__);
                    ErrorMessage(QMessageBox::Critical,"commandLineExecutePlugin", msg);
                }
                exit(-1);
            }
            else
            {
                exit(0);
            }
        }
    }

    if(!plugin_found)
    {
        qDebug() << "Plugin not found " << plugin;
    }
}

void MainWindow::deleteactualFile(){
    if(outputfileIsTemporary && !outputfileIsFromCLI)
    {
        // Delete created temp file
        qfile.close();
        outputfile.close();
        if(outputfile.exists() && !outputfile.remove())
        {
            QMessageBox::critical(0, QString("DLT Viewer"),
                                  QString("Cannot delete temporary log file \"%1\"\n%2")
                                  .arg(outputfile.fileName())
                                  .arg(outputfile.errorString()));
        }
    }
}


void MainWindow::closeEvent(QCloseEvent *event)
{

    settings->writeSettings(this);
    if(settings->tempCloseWithoutAsking || outputfile.size() == 0)
    {

        deleteactualFile();

        QMainWindow::closeEvent(event);
    }
    else if(outputfileIsTemporary && !outputfileIsFromCLI)
    {
        if(QMessageBox::information(this, "DLT Viewer",
           "You still have an unsaved temporary file open. Exit anyway?",
           QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            deleteactualFile();

            QMainWindow::closeEvent(event);
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        QMainWindow::closeEvent(event);
    }
}

void MainWindow::on_action_menuFile_New_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("New DLT Log file"), workingDirectory.getDltDirectory(), tr("DLT Files (*.dlt);;All files (*.*)"));

    if(fileName.isEmpty())
    {
        return;
    }

    /* change DLT file working directory */
    workingDirectory.setDltDirectory(QFileInfo(fileName).absolutePath());

    /* close existing file */
    if(outputfile.isOpen())
    {
        if (outputfile.size() == 0)
        {
            deleteactualFile();
        }
        else
        {
            outputfile.close();
        }
    }

    /* create new file; truncate if already exist */
    outputfile.setFileName(fileName);
    outputfileIsTemporary = false;
    outputfileIsFromCLI = false;
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
        tr("Open DLT Log file"), workingDirectory.getDltDirectory(), tr("DLT Files (*.dlt);;All files (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change DLT file working directory */
    workingDirectory.setDltDirectory(QFileInfo(fileName).absolutePath());

    openDltFile(fileName);
    outputfileIsFromCLI = false;
    outputfileIsTemporary = false;

    searchDlg->setMatch(false);
    searchDlg->setOnceClicked(false);
    searchDlg->setStartLine(-1);
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
        workingDirectory.setDltDirectory(QFileInfo(fileName).absolutePath());

         /* open existing file and append new data */
        if (true == openDltFile(fileName))
          {
            outputfileIsTemporary = false;
            outputfileIsFromCLI = false;
          }
        else
          {
            removeCurrentFile(fileName);
          }
    }
}

bool MainWindow::openDltFile(QString fileName)
{
    /* close existing file */
    bool ret = false;
    if(outputfile.isOpen())
    {
        if (outputfile.size() == 0)
        {
            deleteactualFile();
        }
        else
        {
            outputfile.close();
        }
    }

    /* open existing file and append new data */
    outputfile.setFileName(fileName);
    setCurrentFile(fileName);
    if(outputfile.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        reloadLogFile();
        ret = true;
    }
    else
    {
        QMessageBox::critical(0, QString("DLT Viewer"),
                              QString("Cannot open log file \"%1\"\n%2")
                              .arg(fileName)
                              .arg(outputfile.errorString()));
        ret = false;

    }

    return ret;
}

void MainWindow::on_action_menuFile_Import_DLT_Stream_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Import DLT Stream"), workingDirectory.getDltDirectory(), tr("DLT Stream file (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change DLT file working directory */
    workingDirectory.setDltDirectory(QFileInfo(fileName).absolutePath());

    if(!outputfile.isOpen())
        return;

    DltFile importfile;

    dlt_file_init(&importfile,0);

    /* open DLT stream file */
    dlt_file_open(&importfile,fileName.toLatin1(),0);

    /* parse and build index of complete log file and show progress */
    while (dlt_file_read_raw(&importfile,false,0)>=0)
    {
        // https://bugreports.qt-project.org/browse/QTBUG-26069
        outputfile.seek(outputfile.size());
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
        tr("Import DLT Stream with serial header"), workingDirectory.getDltDirectory(), tr("DLT Stream file (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change DLT file working directory */
    workingDirectory.setDltDirectory(QFileInfo(fileName).absolutePath());

    if(!outputfile.isOpen())
        return;

    DltFile importfile;

    dlt_file_init(&importfile,0);

    /* open DLT stream file */
    dlt_file_open(&importfile,fileName.toLatin1(),0);

    /* parse and build index of complete log file and show progress */
    while (dlt_file_read_raw(&importfile,true,0)>=0)
    {
        // https://bugreports.qt-project.org/browse/QTBUG-26069
        outputfile.seek(outputfile.size());
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
        tr("Append DLT File"), workingDirectory.getDltDirectory(), tr("DLT File (*.dlt)"));

    if(fileName.isEmpty())
        return;

    /* change DLT file working directory */
    workingDirectory.setDltDirectory(QFileInfo(fileName).absolutePath());

    if(!outputfile.isOpen())
        return;

    DltFile importfile;

    dlt_file_init(&importfile,0);

    QProgressDialog progress("Append log file", "Cancel Loading", 0, 100, this);
    progress.setModal(true);
    int num = 0;

    /* open DLT log file with same filename as output file */
    if (dlt_file_open(&importfile,fileName.toLatin1() ,0)<0)
    {
        return;
    }

    /* get number of files in DLT log file */
    while (dlt_file_read(&importfile,0)>=0)
    {
        num++;
        if ( 0 == (num%1000))
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
        if ( 0 == (pos%1000))
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
        tr("Export to ASCII"), workingDirectory.getExportDirectory(), tr("ASCII Files (*.txt);;All files (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change last export directory */
    workingDirectory.setExportDirectory(QFileInfo(fileName).absolutePath());

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
        if( 0 == (num%1000))
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
        outfile.write(text.toLatin1().constData());
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
    qSort(list.begin(), list.end());

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
                tr("Export Selection"), workingDirectory.getExportDirectory(), tr("Text Files (*.txt)"));
        }
        else
        {
            fileName = QFileDialog::getSaveFileName(this,
                tr("Export Selection"), workingDirectory.getExportDirectory(), tr("DLT Files (*.dlt)"));
        }
        if(fileName.isEmpty())
        {
            return;
        }
        else
        {
            /* change last export directory */
            workingDirectory.setExportDirectory(QFileInfo(fileName).absolutePath());
        }
    }


    QFile outfile(fileName);
    if(file)
    {
        if(!outfile.open(QIODevice::WriteOnly | QIODevice::Text))
            return;
    }


    QProgressDialog fileprogress("Export...", "Cancel", 0, list.count(), this);
    fileprogress.setWindowTitle("DLT Viewer");
    fileprogress.setWindowModality(Qt::WindowModal);
    fileprogress.show();
    for(int num=0; num < list.count();num++)
    {
        if(0 == num%1000)
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
                    outfile.write(text.toLatin1().constData());
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

void MainWindow::on_action_menuFile_Export_CSV_triggered()
{
    if(qfile.sizeFilter() <= 0)
    {
        QMessageBox::critical(this, QString("DLT Viewer"),
                              QString("Nothing to export. Make sure you have a DLT file open and that not everything is filtered."));
        return;
    }

    QFileDialog dialog(this);
    QStringList filters;
    filters << "CSV Files (*.csv)" <<"All files (*.*)";
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("csv");
    dialog.setDirectory(workingDirectory.getExportDirectory());
    dialog.setNameFilters(filters);
    dialog.setWindowTitle("Export to CSV file");
    dialog.exec();
    if(dialog.result() != QFileDialog::Accepted ||
        dialog.selectedFiles().count() < 1)
    {
        return;
    }




    QString fileName = dialog.selectedFiles()[0];

    if(fileName.isEmpty())
        return;

    /* change last export directory */
    workingDirectory.setExportDirectory(QFileInfo(fileName).absolutePath());
    DltExporter exporter(this);
    QFile outfile(fileName);
    exporter.exportCSV(&qfile, &outfile, project.plugin);
}

void MainWindow::on_action_menuFile_Export_Selection_CSV_triggered()
{
    QModelIndexList list = ui->tableView->selectionModel()->selection().indexes();
    if(qfile.sizeFilter() <= 0)
    {
        QMessageBox::critical(this, QString("DLT Viewer"),
                              QString("Nothing to export. Make sure you have a DLT file open and that not everything is filtered."));
        return;
    }

    if(list.count() <= 0)
    {
        QMessageBox::critical(this, QString("DLT Viewer"),
                              QString("No messages selected. Select something from the main view."));
        return;
    }

    QFileDialog dialog(this);
    QStringList filters;
    filters << "CSV Files (*.csv)" <<"All files (*.*)";
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("csv");
    dialog.setDirectory(workingDirectory.getExportDirectory());
    dialog.setNameFilters(filters);
    dialog.setWindowTitle("Export to CSV file");
    dialog.exec();
    if(dialog.result() != QFileDialog::Accepted ||
        dialog.selectedFiles().count() < 1)
    {
        return;
    }






    QString fileName = dialog.selectedFiles()[0];

    if(fileName.isEmpty())
    {
        return;
    }

    /* change last export directory */
    workingDirectory.setExportDirectory(QFileInfo(fileName).absolutePath());

    QFile outfile(fileName);
    DltExporter exporter(this);
    exporter.exportCSV(&qfile, &outfile, project.plugin, &list);
}


void MainWindow::on_action_menuFile_SaveAs_triggered()
{

    QFileDialog dialog(this);
    QStringList filters;
    filters << "DLT Files (*.dlt)" <<"All files (*.*)";
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("dlt");
    dialog.setDirectory(workingDirectory.getDltDirectory());
    dialog.setNameFilters(filters);
    dialog.setWindowTitle("Save DLT Log file");
    dialog.exec();
    if(dialog.result() != QFileDialog::Accepted ||
        dialog.selectedFiles().count() < 1)
    {
        return;
    }



    QString fileName = dialog.selectedFiles()[0];

    if(fileName.isEmpty() || dialog.result() == QDialog::Rejected)
    {
        return;
    }

    /* check if filename is the same as already open */
    if(outputfile.fileName()==fileName)
    {
        QMessageBox::critical(0, QString("DLT Viewer"),
                              QString("File is already open!"));
        return;
    }

    /* change DLT file working directory */
    workingDirectory.setDltDirectory(QFileInfo(fileName).absolutePath());

    qfile.close();
    outputfile.close();

    QFile sourceFile( outputfile.fileName() );
    QFile destFile( fileName );

    /* Dialog will ask if you want to replace */
    if(destFile.exists())
    {
        if(!destFile.remove())
        {
            QMessageBox::critical(0, QString("DLT Viewer"),
                                  QString("Save as failed! Could not delete old file."));
            return;
        }
    }


    if(!sourceFile.copy(destFile.fileName()))
    {
        QMessageBox::critical(0, QString("DLT Viewer"),
                              QString("Save as failed! Could not move to new destination."));
        return;
    }

    outputfile.setFileName(fileName);
    outputfileIsTemporary = false;
    outputfileIsFromCLI = false;
    setCurrentFile(fileName);
    if(outputfile.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        reloadLogFile();
    }
    else
        QMessageBox::critical(0, QString("DLT Viewer"),
                              QString("Cannot rename log file \"%1\"\n%2")
                              .arg(fileName)
                              .arg(outputfile.errorString()));
}

void MainWindow::on_action_menuFile_Clear_triggered()
{
    QString fn = DltFileUtils::createTempFile(DltFileUtils::getTempPath(settings));
    if(!fn.length())
    {
        /* Something went horribly wrong with file name creation
         * There's nothing we can do at this point */
        return;
    }

    QString oldfn = outputfile.fileName();

    if(outputfile.isOpen())
    {
        if (outputfile.size() == 0)
        {
            deleteactualFile();
        }
        else
        {
            outputfile.close();
        }
    }

    outputfile.setFileName(fn);

    if(outputfile.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        reloadLogFile();
    }
    else
    {
        QMessageBox::critical(0, QString("DLT Viewer"),
                              QString("Cannot open log file \"%1\"\n%2")
                              .arg(fn)
                              .arg(outputfile.errorString()));
    }

    if(outputfileIsTemporary && !settings->tempSaveOnClear && !outputfileIsFromCLI)
    {
        QFile dfile(oldfn);
        if(!dfile.remove())
        {
            QMessageBox::critical(0, QString("DLT Viewer"),
                                  QString("Cannot delete log file \"%1\"\n%2")
                                  .arg(oldfn)
                                  .arg(dfile.errorString()));
        }
    }
    outputfileIsTemporary = true;
    outputfileIsFromCLI = false;
    return;
}
void MainWindow::applyPlugins(QList<PluginItem*> activeViewerPlugins, QList<PluginItem*>activeDecoderPlugins)
{
    QDltMsg msg;
    PluginItem *item;
    QProgressDialog fileprogress("Applying plugins.", "Cancel", 0, qfile.size(), this);
    fileprogress.setWindowModality(Qt::WindowModal);
    qfile.enableFilter(DltSettingsManager::getInstance()->value("startup/filtersEnabled", true).toBool());

    for(int ix=0;ix<qfile.size();ix++)
    {
        /* Fill message from file */
        if(!qfile.getMsg(ix, msg))
        {
            /* Skip broken messages */
            continue;
        }

        /* Process all viewer plugins */
        for(int ivp=0;ivp < activeViewerPlugins.size();ivp++)
        {
            item = (PluginItem*)activeViewerPlugins.at(ivp);
            item->pluginviewerinterface->initMsg(ix, msg);
        }

        /* Process all decoderplugins */
        for(int idp = 0; idp < activeDecoderPlugins.size();idp++)
        {
            item = (PluginItem*)activeDecoderPlugins.at(idp);
            if(item->plugindecoderinterface->isMsg(msg, 0))
            {
                item->plugindecoderinterface->decodeMsg(msg, 0);
                /* TODO: Do we, or do we not want to break here?
                 * Perhaps user wants to stack multiple plugins */
                break;
            }
        }

        /* Add to filterindex if matches */
        if(qfile.checkFilter(msg))
        {
            qfile.addFilterIndex(ix);
        }

        /* Offer messages again to viewer plugins after decode */
        for(int ivp=0;ivp<activeViewerPlugins.size();ivp++)
        {
            item = (PluginItem *)activeViewerPlugins.at(ivp);
            item->pluginviewerinterface->initMsgDecoded(ix, msg);
        }

        /* Update progress every 0.5% */
        if( 0 == (ix%1000))
        {
            fileprogress.setValue(ix);
            fileprogress.setLabelText(
                        QString("Applying Plugins for Message %1/%2")
                        .arg(ix).arg(qfile.size()));
            if(fileprogress.wasCanceled())
            {
                break;
            }
            QApplication::processEvents();
        }
    }

    for(int i = 0; i < activeViewerPlugins.size(); i++){
        item = (PluginItem*)activeViewerPlugins.at(i);
        item->pluginviewerinterface->initFileFinish();

    }

    if(fileprogress.wasCanceled())
    {
        QMessageBox::warning(this, tr("DLT Viewer"), tr("You canceled the initialisation progress. Not all messages could be processed by the enabled Plugins!"), QMessageBox::Ok);
    }
}

void MainWindow::reloadLogFile()
{
    PluginItem *item = 0;
    QList<PluginItem*> activeViewerPlugins;
    QList<PluginItem*> activeDecoderPlugins;

    ui->tableView->selectionModel()->clear();
    m_searchtableModel->clear_SearchResults();
    ui->dockWidgetSearchIndex->hide();

    qfile.open(outputfile.fileName());

    /* Create the main index */
    ui->tableView->lock();
    dltIndexer->index();
    ui->tableView->unlock();

    /* Collect all plugins */
    if(DltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool())
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
                if(item->pluginviewerinterface)
                {

                    item->pluginviewerinterface->initFileStart(&qfile);
                    activeViewerPlugins.append(item);
                }
            }
        }
    }

    dltIndexer->lock();
    /* Apply collected plugins.
     * Please note that filterIndex is created as a side effect */
    applyPlugins(activeViewerPlugins, activeDecoderPlugins);
    dltIndexer->unlock();

    tableModel->modelChanged();
    m_searchtableModel->modelChanged();

    /* set name of opened log file in status bar */
    statusFilename->setText(outputfile.fileName());



    /* We might have had readyRead events, which we missed */
    readyRead();
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

    DltSettingsManager *settingsmanager = DltSettingsManager::getInstance();

    int refreshRate = settingsmanager->value("RefreshRate",DEFAULT_REFRESH_RATE).toInt();
    if ( refreshRate )
        draw_interval = 1000 / refreshRate;
	else
		draw_interval = 1000 / DEFAULT_REFRESH_RATE;	
}

void MainWindow::on_action_menuFile_Settings_triggered()
{
    /* show settings dialog */
    settings->writeDlg();

    if(settings->exec()==1)
    {
        /* change settings and store settings persistently */
        settings->readDlg();
        settings->writeSettings(this);

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
        tr("Open DLT Project file"), workingDirectory.getDlpDirectory(), tr("DLT Project Files (*.dlp);;All files (*.*)"));

    /* open existing project */
    if(!fileName.isEmpty())
    {
        /* change Project file working directory */
        workingDirectory.setDlpDirectory(QFileInfo(fileName).absolutePath());

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

    QFileDialog dialog(this);
    QStringList filters;
    filters << "DLT Project Files (*.dlp)" <<"All files (*.*)";
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("dlp");
    dialog.setDirectory(workingDirectory.getDlpDirectory());
    dialog.setNameFilters(filters);
    dialog.setWindowTitle("Save DLT Project file");
    dialog.exec();
    if(dialog.result() != QFileDialog::Accepted ||
        dialog.selectedFiles().count() < 1)
    {
        return;
    }




    QString fileName = dialog.selectedFiles()[0];


    /* save project */
    if(fileName.isEmpty() || dialog.result() == QDialog::Rejected)
    {
        //return;
    }
    else if( project.Save(fileName))
    {
        /* change Project file working directory */
        workingDirectory.setDlpDirectory(QFileInfo(fileName).absolutePath());

        this->setWindowTitle(QString("DLT Viewer - "+fileName+" - Version : %1 %2").arg(PACKAGE_VERSION).arg(PACKAGE_VERSION_STATE));

        setCurrentProject(fileName);
    }
}

QStringList MainWindow::getSerialPortsWithQextEnumerator(){

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

    QStringList portListPreset = getSerialPortsWithQextEnumerator();

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
        setCurrentHostname(ecuitem->getHostname());
        setCurrentPort(ecuitem->getPort());

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

        QStringList portListPreset = getSerialPortsWithQextEnumerator();

        EcuItem* ecuitem = (EcuItem*) list.at(0);

        /* show ECU configuration dialog */
        EcuDialog dlg(ecuitem->id,ecuitem->description,ecuitem->interfacetype,ecuitem->getHostname(),ecuitem->getTcpport(),ecuitem->getPort(),ecuitem->getBaudrate(),
                      ecuitem->loglevel,ecuitem->tracestatus,ecuitem->verbosemode,ecuitem->getSendSerialHeaderTcp(),ecuitem->getSendSerialHeaderSerial(),ecuitem->getSyncSerialHeaderTcp(),ecuitem->getSyncSerialHeaderSerial(),
                      ecuitem->timingPackets,ecuitem->sendGetLogInfo,ecuitem->updateDataIfOnline,ecuitem->autoReconnect,ecuitem->autoReconnectTimeout);

        /* Read settings for recent hostnames and ports */
        recentHostnames = DltSettingsManager::getInstance()->value("other/recentHostnameList",hostnameListPreset).toStringList();
        recentPorts = DltSettingsManager::getInstance()->value("other/recentPortList",portListPreset).toStringList();

        setCurrentHostname(ecuitem->getHostname());

        //serial Port
        setCurrentPort(ecuitem->getPort());

        dlg.setHostnameList(recentHostnames);
        dlg.setPortList(recentPorts);

        if(dlg.exec())
        {
            bool interfaceChanged = false;
            if((ecuitem->interfacetype != dlg.interfacetype() ||
                ecuitem->getHostname() != dlg.hostname() ||
                ecuitem->getTcpport() != dlg.tcpport() ||
                ecuitem->getPort() != dlg.port() ||
                ecuitem->getBaudrate() != dlg.baudrate()) &&
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
            setCurrentHostname(ecuitem->getHostname());
            setCurrentPort(ecuitem->getPort());

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


    action = new QAction("Save Filter...", this);
    if(project.filter->topLevelItemCount() <= 0)
        action->setEnabled(false);
    else
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Save_As_triggered()));
    menu.addAction(action);

    action = new QAction("Load Filter...", this);
    connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Load_triggered()));
    menu.addAction(action);

    action = new QAction("Append Filter...", this);
    connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Append_Filters_triggered()));
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

    action = new QAction("Filter Duplicate...", this);
    if(list.size() != 1)
        action->setEnabled(false);
    else
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Duplicate_triggered()));
    menu.addAction(action);

    action = new QAction("Filter Delete", this);
    if(list.size() != 1)
        action->setEnabled(false);
    else
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Delete_triggered()));
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
        menu.addSeparator();

        if(item->pluginviewerinterface)
        {
            /* If a viewer plugin is disabled, or enabled but not shown,
             * add 'show' action. Else add 'hide' action */
            if(item->getMode() != PluginItem::ModeShow)
            {
                action = new QAction("Plugin Show", this);
                connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuPlugin_Show_triggered()));
                menu.addAction(action);
            }
            else
            {
                action = new QAction("Plugin Hide", this);
                connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuPlugin_Hide_triggered()));
                menu.addAction(action);
            }
        }

        /* If the plugin is shown or enabled, present the 'disable' option.
         * Else, present the 'enable' option */
        if(item->getMode() != PluginItem::ModeDisable)
        {
            action = new QAction("Plugin Disable", this);
            connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuPlugin_Disable_triggered()));
            menu.addAction(action);
        }
        else
        {
            action = new QAction("Plugin Enable", this);
            connect(action, SIGNAL(triggered()), this, SLOT(action_menuPlugin_Enable_triggered()));
            menu.addAction(action);
        }
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
            ecuitem->m_serialport->close();
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
        ecuitem->tcpcon.clear();
        ecuitem->serialcon.clear();

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

                //disconnect(&ecuitem->socket,0,0,0);
                ecuitem->socket.connectToHost(ecuitem->getHostname(),ecuitem->getTcpport());
            }
        }
        else
        {
            /* Serial */
            if(!ecuitem->m_serialport)
            {
                PortSettings settings = {ecuitem->getBaudrate(), DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10}; //Before timeout was 1
                ecuitem->m_serialport = new QextSerialPort(ecuitem->getPort(),settings);
                connect(ecuitem->m_serialport, SIGNAL(readyRead()), this, SLOT(readyRead()));
                connect(ecuitem->m_serialport,SIGNAL(dsrChanged(bool)),this,SLOT(stateChangedSerial(bool)));
            }
            else{

                //to keep things consistent: delete old member, create new one
                //alternatively we could just close the port, and set the new settings.
                ecuitem->m_serialport->close();
                //delete(ecuitem->m_serialport);
                ecuitem->m_serialport->setBaudRate(ecuitem->getBaudrate());
                //ecuitem->m_serialport->setDataBits(settings.DataBits);
                //ecuitem->m_serialport->setFlowControl(settings.FlowControl);
                //ecuitem->m_serialport->setStopBits(settings.StopBits);
                //ecuitem->m_serialport->setParity(settings.Parity);
                //ecuitem->m_serialport->setTimeout(settings.Timeout_Millisec);
                ecuitem->m_serialport->setPortName(ecuitem->getPort());
            }

            if(ecuitem->m_serialport->isOpen())
            {
                ecuitem->m_serialport->close();
                ecuitem->m_serialport->setBaudRate(ecuitem->getBaudrate());
            }

            ecuitem->m_serialport->open(QIODevice::ReadWrite);

            if(ecuitem->m_serialport->isOpen())
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
            ecuitem->tcpcon.clear();
            ecuitem->serialcon.clear();
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

            /* Try to reconnect if the ecuitem has not received
             * new data for long enough time.
             * If the indexer is busy indexing,
             * do not disconnect yet. Wait for future timeouts until
             * indexer is free. */
            if(ecuitem->isAutoReconnectTimeoutPassed() &&
               dltIndexer->tryLock())
            {
                if(ecuitem->interfacetype == 0 && ecuitem->autoReconnect && ecuitem->connected == true && ecuitem->totalBytesRcvd == ecuitem->totalBytesRcvdLastTimeout)
                {
                    disconnectECU(ecuitem);
                    ecuitem->tryToConnect = true;
                }
                ecuitem->totalBytesRcvdLastTimeout = ecuitem->totalBytesRcvd;
                dltIndexer->unlock();
            }

            if( ecuitem->tryToConnect && !ecuitem->connected)
            {
                connectECU(ecuitem,true);
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

            on_configWidget_itemSelectionChanged();
        }

    }
}

void MainWindow::readyRead()
{
    /* signal emited when socket received data */

    /* Delay reading, if indexer is working on the dlt file */
    if(dltIndexer->tryLock())
    {
        /* find socket which emited signal */
        for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
        {
            EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
            if( (&(ecuitem->socket) == sender()) || (ecuitem->m_serialport == sender()))
            {
                read(ecuitem);
            }
        }
        dltIndexer->unlock();
    }
}

void MainWindow::read(EcuItem* ecuitem)
{
    int32_t bytesRcvd = 0;
    QDltMsg qmsg;
    PluginItem *item = 0;
    QList<PluginItem*> activeViewerPlugins;
    QList<PluginItem*> activeDecoderPlugins;

    if (!ecuitem)
        return;

    QByteArray data;
    if(ecuitem->interfacetype == 0)
    {
        /* TCP */
        bytesRcvd = ecuitem->socket.bytesAvailable();
        data = ecuitem->socket.readAll();
        bytesRcvd = data.size();
        ecuitem->tcpcon.add(data);
     }
    else if(ecuitem->m_serialport)
    {
        /* serial */
        bytesRcvd = ecuitem->m_serialport->bytesAvailable();
        data = ecuitem->m_serialport->readAll();
        bytesRcvd = data.size();
        ecuitem->serialcon.add(data);
    }

    /* reading data; new data is added to the current buffer */
    if (bytesRcvd>0)
    {

        ecuitem->totalBytesRcvd += bytesRcvd;

        while((ecuitem->interfacetype == 0 && ecuitem->tcpcon.parse(qmsg)) ||
              (ecuitem->interfacetype == 1 && ecuitem->serialcon.parse(qmsg)))
        {
            DltStorageHeader str;
            str.pattern[0]='D';
            str.pattern[1]='L';
            str.pattern[2]='T';
            str.pattern[3]=0x01;
            QDateTime time = QDateTime::currentDateTime();
            str.seconds = (time_t)time.toTime_t(); /* value is long */
            str.microseconds = (int32_t)time.time().msec(); /* value is long */
            str.ecu[0]=0;
            str.ecu[1]=0;
            str.ecu[2]=0;
            str.ecu[3]=0;
            /* prepare storage header */
            if (!qmsg.getEcuid().isEmpty())
               dlt_set_id(str.ecu,qmsg.getEcuid().toLatin1());
            else
                dlt_set_id(str.ecu,ecuitem->id.toLatin1());

            /* check if message is matching the filter */
            if (outputfile.isOpen())
            {

                if ((settings->writeControl && (qmsg.getType()==QDltMsg::DltTypeControl)) || (!(qmsg.getType()==QDltMsg::DltTypeControl)))
                {
                    // https://bugreports.qt-project.org/browse/QTBUG-26069
                    outputfile.seek(outputfile.size());
                    outputfile.write((char*)&str,sizeof(DltStorageHeader));
                    QByteArray buffer = qmsg.getHeader();
                    outputfile.write(buffer);
                    buffer = qmsg.getPayload();
                    outputfile.write(buffer);
                    outputfile.flush();
                }
            }

            /* analyse received message, check if DLT control message response */
            if ( (qmsg.getType()==QDltMsg::DltTypeControl) && (qmsg.getSubtype()==QDltMsg::DltControlResponse))
            {
                controlMessage_ReceiveControlMessage(ecuitem,qmsg);
            }
        }

        if(ecuitem->interfacetype == 0)
        {
            /* TCP */
            totalByteErrorsRcvd+=ecuitem->tcpcon.bytesError;
            ecuitem->tcpcon.bytesError = 0;
            totalBytesRcvd+=ecuitem->tcpcon.bytesReceived;
            ecuitem->tcpcon.bytesReceived = 0;
            totalSyncFoundRcvd+=ecuitem->tcpcon.syncFound;
            ecuitem->tcpcon.syncFound = 0;
         }
        else if(ecuitem->m_serialport)
        {
            /* serial */
            totalByteErrorsRcvd+=ecuitem->serialcon.bytesError;
            ecuitem->serialcon.bytesError = 0;
            totalBytesRcvd+=ecuitem->serialcon.bytesReceived;
            ecuitem->serialcon.bytesReceived = 0;
            totalSyncFoundRcvd+=ecuitem->serialcon.syncFound;
            ecuitem->serialcon.syncFound = 0;
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
                    if(item->pluginviewerinterface)
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

            if (!draw_timer.isActive())
                draw_timer.start(draw_interval);

            for(int i = 0; i < activeViewerPlugins.size(); i++){
                item = (PluginItem*)activeViewerPlugins.at(i);
                item->pluginviewerinterface->updateFileFinish();
            }
        }
    }
}



void MainWindow::draw_timeout()
{
    drawUpdatedView();
}


void MainWindow::drawUpdatedView()
{

    statusByteErrorsReceived->setText(QString("Recv Errors: %1").arg(totalByteErrorsRcvd));
    statusBytesReceived->setText(QString("Recv: %1").arg(totalBytesRcvd));
    statusSyncFoundReceived->setText(QString("Sync found: %1").arg(totalSyncFoundRcvd));

    tableModel->modelChanged();

    //Line below would resize the payload column automatically so that the whole content is readable
    //ui->tableView->resizeColumnToContents(11); //Column 11 is the payload column
    if(settings->autoScroll) {
        ui->tableView->scrollToBottom();
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


    for(int i = 0; i < activeDecoderPlugins.size(); i++)
    {
        item = (PluginItem*)activeDecoderPlugins.at(i);

        if(item->plugindecoderinterface->isMsg(msg,0))
        {
            item->plugindecoderinterface->decodeMsg(msg,0);
            break;
        }
    }


    for(int i = 0; i < activeViewerPlugins.size(); i++){
        item = (PluginItem*)activeViewerPlugins.at(i);
        item->pluginviewerinterface->selectedIdxMsgDecoded(msgIndex,msg);
    }

}

void MainWindow::controlMessage_ReceiveControlMessage(EcuItem *ecuitem, QDltMsg &msg)
{
    const char *ptr;
    int32_t length;

    QByteArray payload = msg.getPayload();
    ptr = payload.constData();
    length = payload.size();

    /* control message was received */
    uint32_t service_id=0, service_id_tmp=0;
    DLT_MSG_READ_VALUE(service_id_tmp,ptr,length,uint32_t);
    service_id=DLT_ENDIAN_GET_32( ((msg.getEndianness()==QDltMsg::DltEndiannessBigEndian)?DLT_HTYP_MSBF:0), service_id_tmp);

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
            count_app_ids=DLT_ENDIAN_GET_16(((msg.getEndianness()==QDltMsg::DltEndiannessBigEndian)?DLT_HTYP_MSBF:0), count_app_ids_tmp);

            for (int32_t num=0;num<count_app_ids;num++)
            {
                char apid[DLT_ID_SIZE+1];
                apid[DLT_ID_SIZE] = 0;

                DLT_MSG_READ_ID(apid,ptr,length);

                uint16_t count_context_ids=0,count_context_ids_tmp=0;
                DLT_MSG_READ_VALUE(count_context_ids_tmp,ptr,length,uint16_t);
                count_context_ids=DLT_ENDIAN_GET_16(((msg.getEndianness()==QDltMsg::DltEndiannessBigEndian)?DLT_HTYP_MSBF:0), count_context_ids_tmp);

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
                        context_description_length=DLT_ENDIAN_GET_16(((msg.getEndianness()==QDltMsg::DltEndiannessBigEndian)?DLT_HTYP_MSBF:0),context_description_length_tmp);

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
                    application_description_length=DLT_ENDIAN_GET_16(((msg.getEndianness()==QDltMsg::DltEndiannessBigEndian)?DLT_HTYP_MSBF:0),application_description_length_tmp);
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
    dlt_set_storageheader(msg.storageheader,ecuitem->id.toLatin1());

    /* prepare standard header */
    msg.standardheader = (DltStandardHeader*)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_WEID | DLT_HTYP_WTMS | DLT_HTYP_UEH | DLT_HTYP_PROTOCOL_VERSION1 ;

#if (BYTE_ORDER==BIG_ENDIAN)
    msg.standardheader->htyp = (msg.standardheader->htyp | DLT_HTYP_MSBF);
#endif

    msg.standardheader->mcnt = 0;

    /* Set header extra parameters */
    dlt_set_id(msg.headerextra.ecu,ecuitem->id.toLatin1());
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
        dlt_set_id(msg.extendedheader->apid, appid.toLatin1());
    }
    if (contid.isEmpty())
    {
        dlt_set_id(msg.extendedheader->ctid,"CON");       /* context id */
    }
    else
    {
        dlt_set_id(msg.extendedheader->ctid, contid.toLatin1());
    }

    /* prepare length information */
    msg.headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + sizeof(DltExtendedHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp);
    msg.standardheader->len = DLT_HTOBE_16(msg.headersize - sizeof(DltStorageHeader) + msg.datasize);

    /* send message to daemon */
    if (ecuitem->interfacetype == 0 && ecuitem->socket.isOpen())
    {
        /* Optional: Send serial header, if requested */
        if (ecuitem->getSendSerialHeaderTcp())
            ecuitem->socket.write((const char*)dltSerialHeader,sizeof(dltSerialHeader));

        /* Send data */
        ecuitem->socket.write((const char*)msg.headerbuffer+sizeof(DltStorageHeader),msg.headersize-sizeof(DltStorageHeader));
        ecuitem->socket.write((const char*)msg.databuffer,msg.datasize);
    }
    else if (ecuitem->interfacetype == 1 && ecuitem->m_serialport && ecuitem->m_serialport->isOpen())
    {
        /* Optional: Send serial header, if requested */
        if (ecuitem->getSendSerialHeaderSerial())
            ecuitem->m_serialport->write((const char*)dltSerialHeader,sizeof(dltSerialHeader));

        /* Send data */
        ecuitem->m_serialport->write((const char*)msg.headerbuffer+sizeof(DltStorageHeader),msg.headersize-sizeof(DltStorageHeader));
        ecuitem->m_serialport->write((const char*)msg.databuffer,msg.datasize);
    }
    else
    {
        /* ECU is not connected */
        return;
    }

    /* Skip the file handling, if indexer is working on the file */
    if(dltIndexer->tryLock())
    {
        /* store ctrl message in log file */
        if (outputfile.isOpen())
        {
            if (settings->writeControl)
            {
                // https://bugreports.qt-project.org/browse/QTBUG-26069
                outputfile.seek(outputfile.size());
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
        dltIndexer->unlock();
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
    dlt_set_id(req->apid,app.toLatin1());
    dlt_set_id(req->ctid,con.toLatin1());
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
    dlt_set_id(req->apid,app.toLatin1());
    dlt_set_id(req->ctid,con.toLatin1());
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

void MainWindow::on_action_menuHelp_Support_triggered()
{
  QMessageBox msgBox(this);
  msgBox.setWindowTitle("Mail-Support DLT");
  msgBox.setTextFormat(Qt::RichText); //this is what makes the links clickable
  QString text = "<a href='mailto:";
  text.append(DLT_SUPPORT_MAIL_ADDRESS);
  text.append("?Subject=DLT Question: [please add subject] ");//subject
  text.append("&body=Please keep version information in mail:%0D%0ADLT Version: ").append(PACKAGE_VERSION).append("-");//body start
  text.append(PACKAGE_VERSION_STATE);
  text.append("%0D%0ABuild Date: ");
  text.append(__DATE__);
  text.append("-");
  text.append(__TIME__).append("\nQt Version: ").append(QT_VERSION_STR);
  text.append("'");//end body
  text.append("><center>Mailto ").append(DLT_SUPPORT_NAME).append(" DLT-Viewer-Support:<br>");
  text.append(DLT_SUPPORT_MAIL_ADDRESS).append("</center></a>");
  msgBox.setText(text);
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.exec();
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
                             QString(" -s or --silent \t\tEnable silent mode without warning message boxes\n")+
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
        //ui->action_menuFilter_Save_As->setEnabled(true);
        ui->action_menuFilter_Clear_all->setEnabled(true);
    }else{
        //ui->action_menuFilter_Save_As->setEnabled(false);
        ui->action_menuFilter_Clear_all->setEnabled(false);
    }

    if((project.filter->selectedItems().count() >= 1) ) {
        ui->action_menuFilter_Delete->setEnabled(true);
        ui->action_menuFilter_Edit->setEnabled(true);        
        ui->action_menuFilter_Duplicate->setEnabled(true);
    }else{
        ui->action_menuFilter_Delete->setEnabled(false);
        ui->action_menuFilter_Edit->setEnabled(false);        
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

void MainWindow::updateScrollButton()
{
    // Mapping: variable to button
    if (settings->autoScroll == Qt::Unchecked )
        scrollButton->setChecked(false);
    else
        scrollButton->setChecked(true);
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

            /* Change Project file working directory */
            workingDirectory.setDlpDirectory(QFileInfo(projectName).absolutePath());

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
            workingDirectory.setDlfDirectory(QFileInfo(fileName).absolutePath());
            setCurrentFilters(fileName);
            ui->applyConfig->startPulsing(pulseButtonColor);
            ui->applyConfig->setEnabled(true);
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
            on_actionAutoScroll_triggered(Qt::Checked);
            updateScrollButton();
        }
    }
    else
    {
        /* Only disable, if enabled */
        if (settings->autoScroll==Qt::Checked)
        {
            /* disable scrolling */
            on_actionAutoScroll_triggered(Qt::Unchecked);
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
        if( ecuitem->m_serialport == sender())
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
    /* set filter for plugin files */
    QStringList filters;
    filters << "*.dll" << "*.so";
    dir.setNameFilters(filters);

    /* iterate through all plugins */
    foreach (QString fileName, dir.entryList(QDir::Files))
    {
        QPluginLoader *pluginLoader = new QPluginLoader(dir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader->instance();
        if (plugin)
        {
            QDLTPluginInterface *plugininterface = qobject_cast<QDLTPluginInterface *>(plugin);
            if (plugininterface)
            {
                if(QString::compare( plugininterface->pluginInterfaceVersion(),PLUGIN_INTERFACE_VERSION, Qt::CaseSensitive) == 0){

                    PluginItem* item = new PluginItem(0);
                    item->loader = pluginLoader;
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
        else {
            QMessageBox::warning(0, QString("DLT Viewer"),QString("The plugin %1 cannot be loaded.\n\nError: %2").arg(dir.absoluteFilePath(fileName)).arg(pluginLoader->errorString()));
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

    bool ret = item->plugininterface->loadConfig(item->getFilename());
    if ( false == ret )
      {
        if (item->getMode() != PluginItem::ModeDisable)
          {
            QString err_header = "Plugin: ";
            err_header.append(item->plugininterface->name());
            err_header.append(" returned error: ");
            QString err_text = item->plugininterface->error();
            ErrorMessage(QMessageBox::Warning,err_header,err_text);
          }
      }

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
        if(dlg.exec()) {
            /* Check if there was a change that requires a refresh */
            if(item->getMode() != dlg.getMode())
                callInitFile = true;
            if(item->getMode() == PluginItem::ModeShow && dlg.getMode() != PluginItem::ModeDisable)
                callInitFile = false;
            if(dlg.getMode() == PluginItem::ModeShow && item->getMode() != PluginItem::ModeDisable)
                callInitFile = false;
            if(item->getFilename() != dlg.getFilename())
                callInitFile = true;

            item->setFilename( dlg.getFilename() );
            item->setMode( dlg.getMode() );
            item->setType( dlg.getType() );

            /* update plugin item */
            updatePlugin(item);
            item->savePluginModeToSettings();
        }
        if(callInitFile)
        {
            ui->applyConfig->startPulsing(pulseButtonColor);
            ui->applyConfig->setEnabled(true);
        }
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No Plugin selected!"));

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
                ui->applyConfig->startPulsing(pulseButtonColor);
                ui->applyConfig->setEnabled(true);
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

void MainWindow::action_menuPlugin_Enable_triggered()
{
    /* get selected plugin */
    QList<QTreeWidgetItem *> list = project.plugin->selectedItems();
    if((list.count() == 1) ) {
        PluginItem* item = (PluginItem*) list.at(0);

        if(item->getMode() == PluginItem::ModeDisable){
            item->setMode( PluginItem::ModeEnable );
            item->savePluginModeToSettings();
            updatePlugin(item);
            ui->applyConfig->startPulsing(pulseButtonColor);
            ui->applyConfig->setEnabled(true);
        }else{
            QMessageBox::warning(0, QString("DLT Viewer"),
                                 QString("The selected Plugin is already deactivated."));
        }
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No Plugin selected!"));
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
            ui->applyConfig->startPulsing(pulseButtonColor);
            ui->applyConfig->setEnabled(true);
        }else{
            QMessageBox::warning(0, QString("DLT Viewer"),
                                 QString("The selected Plugin is already deactivated."));
        }
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No Plugin selected!"));
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
        tr("Save DLT Filters"), workingDirectory.getDlfDirectory(), tr("DLT Filter File (*.dlf);;All files (*.*)"));

    if(!fileName.isEmpty())
    {
        workingDirectory.setDlfDirectory(QFileInfo(fileName).absolutePath());
        project.SaveFilter(fileName);
        setCurrentFilters(fileName);
    }
}


void MainWindow::on_action_menuFilter_Load_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Load DLT Filter file"), workingDirectory.getDlfDirectory(), tr("DLT Filter Files (*.dlf);;All files (*.*)"));

    if(!fileName.isEmpty() && project.LoadFilter(fileName,true))
    {
        workingDirectory.setDlfDirectory(QFileInfo(fileName).absolutePath());
        setCurrentFilters(fileName);
        ui->applyConfig->startPulsing(pulseButtonColor);
        ui->applyConfig->setEnabled(true);
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
    dlg.setEnableRegexp(item->enableRegexp);
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
    item->enableRegexp = dlg.getEnableRegexp();
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
    on_filterWidget_itemSelectionChanged();

    /* Update filters in qfile and either update
     * view or pulse the button depending on if it is a filter or
     * marker. */
    filterUpdate();
    if(item->type == FilterItem::marker)
    {
        tableModel->modelChanged();
    }
    else
    {
        ui->applyConfig->startPulsing(pulseButtonColor);
        ui->applyConfig->setEnabled(true);
    }
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
        FilterItem *item = (FilterItem *)widget->takeTopLevelItem(widget->indexOfTopLevelItem(list.at(0)));
        filterUpdate();
        if(item->type == FilterItem::marker)
        {
            tableModel->modelChanged();
        }
        else
        {
            ui->applyConfig->startPulsing(pulseButtonColor);
            ui->applyConfig->setEnabled(true);
        }
        delete widget->takeTopLevelItem(widget->indexOfTopLevelItem(list.at(0)));
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
    ui->applyConfig->startPulsing(pulseButtonColor);
    ui->applyConfig->setEnabled(true);
}

void MainWindow::filterUpdate() {
    QDltFilter afilter;

    /* update all filters from filter configuration to DLT filter list */

    /* clear old filter list */
    qfile.clearFilter();

    /* iterate through all filters */
    for(int num = 0; num < project.filter->topLevelItemCount (); num++)
    {
        FilterItem *item = (FilterItem*)project.filter->topLevelItem(num);

        afilter.ecuid = item->ecuId;
        afilter.apid = item->applicationId;
        afilter.ctid = item->contextId;
        afilter.header = item->headerText;
        afilter.payload = item->payloadText;

        afilter.enableRegexp = item->enableRegexp;
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
        item->setForeground(0,DltUiUtils::optimalTextColor(item->filterColour));
        item->setForeground(1,DltUiUtils::optimalTextColor(item->filterColour));

        if(afilter.enableRegexp)
        {
            if(!afilter.compileRegexps())
            {
                // This is also validated in the UI part
                qDebug() << "Error compiling a regexp" << endl;
            }
        }

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
            outputfileIsTemporary = false;
            outputfileIsFromCLI   = false;
            workingDirectory.setDltDirectory(QFileInfo(filename).absolutePath());
        }
        else if(filename.endsWith(".dlp", Qt::CaseInsensitive))
        {
            /* Project file dropped */
            openDlpFile(filename);
            workingDirectory.setDlpDirectory(QFileInfo(filename).absolutePath());
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
        ui->applyConfig->startPulsing(pulseButtonColor);
        ui->applyConfig->setEnabled(true);
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
        tr("Load DLT Filter file"), workingDirectory.getDlfDirectory(), tr("DLT Filter Files (*.dlf);;All files (*.*)"));

    if(!fileName.isEmpty() && project.LoadFilter(fileName,false))
    {
        workingDirectory.setDlfDirectory(QFileInfo(fileName).absolutePath());
        setCurrentFilters(fileName);
        ui->applyConfig->startPulsing(pulseButtonColor);
        ui->applyConfig->setEnabled(true);
    }
    on_filterWidget_itemSelectionChanged();
}

int MainWindow::nearest_line(int line){

    if (line < 0 || line > qfile.size()-1){
        return -1;
    }

    // If filters are off, just go directly to the row
    int row = 0;
    if(!qfile.isFilter())
    {
        row = line;
    }
    else
    {
        /* Iterate through filter index, trying to find
         * matching index. If it cannot be found, just settle
         * for the last one that we saw before going over */
        int lastFound = 0, i;
        for(i=0;i<qfile.sizeFilter();i++)
        {
            if(qfile.getMsgFilterPos(i) < line)
            {
                // Track previous smaller
                lastFound = i;
            }
            else if(qfile.getMsgFilterPos(i) == line)
            {
                // It's actually visible
                lastFound = i;
                break;
            }
            else if(qfile.getMsgFilterPos(i) > line)
            {
                // Ok, we went past it, use the last one.
                break;
            }
        }
        row = lastFound;
    }
    return row;
}

bool MainWindow::jump_to_line(int line)
{

    int row = nearest_line(line);
    if (0 > row)
        return false;

    ui->tableView->selectionModel()->clear();
    QModelIndex idx = tableModel->index(row, 0, QModelIndex());
    ui->tableView->scrollTo(idx, QAbstractItemView::PositionAtTop);
    ui->tableView->selectionModel()->select(idx, QItemSelectionModel::Select|QItemSelectionModel::Rows);
    ui->tableView->setFocus();

    return true;
}

void MainWindow::on_actionJump_To_triggered()
{
    JumpToDialog dlg(this);
    int min = 0;
    int max = qfile.size()-1;
    dlg.setLimits(min, max);

    int result = dlg.exec();

    if(result != QDialog::Accepted)
    {
        return;
    }

    jump_to_line(dlg.getIndex());


}

void MainWindow::on_actionAutoScroll_triggered(bool checked)
{
    int autoScrollOld = settings->autoScroll;

    // Mapping: button to variable
    settings->autoScroll = (checked?Qt::Checked:Qt::Unchecked);

    if (autoScrollOld!=settings->autoScroll)
        settings->writeSettings(this);
}

void MainWindow::on_actionConnectAll_triggered()
{
    connectAll();
}

void MainWindow::on_actionDisconnectAll_triggered()
{
    disconnectAll();
}

void MainWindow::on_pluginsEnabled_clicked(bool checked)
{
    DltSettingsManager::getInstance()->setValue("startup/pluginsEnabled", checked);
    ui->applyConfig->startPulsing(pulseButtonColor);
    ui->applyConfig->setEnabled(true);
}

void MainWindow::on_filtersEnabled_clicked(bool checked)
{
    DltSettingsManager::getInstance()->setValue("startup/filtersEnabled", checked);
    ui->applyConfig->startPulsing(pulseButtonColor);
    ui->applyConfig->setEnabled(true);
}

void MainWindow::on_applyConfig_clicked()
{
    ui->applyConfig->stopPulsing();
    ui->applyConfig->setEnabled(false);
    saveSelection();
    filterUpdate();
    reloadLogFile();
    restoreSelection();
}

void MainWindow::saveSelection()
{
    previousSelection.clear();
    /* Store old selections */
    QModelIndexList rows = ui->tableView->selectionModel()->selectedRows();

    for(int i=0;i<rows.count();i++)
    {
        int sr = rows.at(i).row();
        previousSelection.append(qfile.getMsgFilterPos(sr));
    }
}

void MainWindow::restoreSelection()
{
    int firstSelection = 0;
    QModelIndex scrollToTarget = tableModel->index(0, 0);

    /* Try to re-select old indices */
    QItemSelection newSelection;
    for(int j=0;j<previousSelection.count();j++)
    {
        if(j == 0)
        {
            firstSelection = nearest_line(previousSelection.at(j));
        }
        int nearest = nearest_line(previousSelection.at(j));
        QModelIndex idx = tableModel->index(nearest, 0);
        newSelection.select(idx, idx);
    }
    ui->tableView->selectionModel()->select(newSelection, QItemSelectionModel::Select|QItemSelectionModel::Rows);
    scrollToTarget = tableModel->index(firstSelection, 0);
    ui->tableView->scrollTo(scrollToTarget, QTableView::PositionAtTop);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if(index > 0)
    {
        ui->enableConfigFrame->setVisible(true);
    }
    else
    {
        ui->enableConfigFrame->setVisible(false);
    }
}

void MainWindow::filterOrderChanged()
{
    filterUpdate();
    tableModel->modelChanged();
}

void MainWindow::searchTableRenewed()
{
    if ( 0 < m_searchtableModel->get_SearchResultListSize())
        ui->dockWidgetSearchIndex->show();

    m_searchtableModel->modelChanged(); 
}


void MainWindow::searchtable_cellSelected( QModelIndex index)
{

    int position = index.row();
    unsigned long entry;

    if (! m_searchtableModel->get_SearchResultEntry(position, entry) )
        return;

    tableModel->setLastSearchIndex(entry);
    jump_to_line(entry);

}


