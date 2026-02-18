/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
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
 * \file mainwindow.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "filtergrouplogs.h"
#include <algorithm>
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
#include <QFileSystemModel>
#include <QLineEdit>
#include <QUrl>
#include <QDateTime>
#include <QLabel>
#include <QInputDialog>
#include <QByteArray>
#include <QSysInfo>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QNetworkProxyFactory>
#include <QNetworkInterface>
#include <QSortFilterProxyModel>
#include <QDesktopServices>
#include <QProcess>
#include <QStyleFactory>
#include <QTextStream>
#include <QTemporaryFile>
#include <QtEndian>
#include <QDir>
#include <QDirIterator>

#if defined(_MSC_VER)
#include <io.h>
#include <WinSock.h>
#endif

#include "mainwindow.h"
#include "ecudialog.h"
#include "applicationdialog.h"
#include "contextdialog.h"
#include "multiplecontextdialog.h"
#include "plugindialog.h"
#include "settingsdialog.h"
#include "injectiondialog.h"
#include "version.h"
#include "dltfileutils.h"
#include "dltuiutils.h"
#include "qdltexporter.h"
#include "qdltimporter.h"
#include "jumptodialog.h"
#include "fieldnames.h"
#include "tablemodel.h"
#include "qdltoptmanager.h"
#include "qdltctrlmsg.h"
#include <qdltmsgwrapper.h>
#include "ecutree.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer(this),
    qcontrol(this),
    pulseButtonColor(255, 40, 40),
    isSearchOngoing(false)
{
    dltIndexer = NULL;
    settings = QDltSettingsManager::getInstance();
    ui->setupUi(this);
    ui->enableConfigFrame->setVisible(false);
    setAcceptDrops(true);

    target_version_string = "";

    searchDlg->loadSearchHistoryList(searchHistory);
    filterIsChanged = false;

    initState();

    /* Apply loaded settings */
    initSearchTable();

    initView();

    applySettings();

    initSignalConnections();

    initFileHandling();


    /* Commands plugin after loading log file */
    qDebug() << "### Plugin commands after loading log file";
    if(!QDltOptManager::getInstance()->getPostPluginCommands().isEmpty())
    {
        QStringList commands = QDltOptManager::getInstance()->getPostPluginCommands();

        for(int num = 0; num< commands.size();num++)
        {
            qDebug() << "Command:" << commands[num];
            QStringList args = commands[num].split("|");
            if(args.size() > 1)
             {
                QString pluginName = args.at(0);
                QString commandName = args.at(1);
                args.removeAt(0);
                args.removeAt(0);
                QStringList commandParams = args;
                commandLineExecutePlugin(pluginName,commandName,commandParams);
             }
        }

    }

    filterUpdate(); // update filters of qfile before starting Exporting for RegEx operation

    if(!QDltOptManager::getInstance()->getConvertDestFile().isEmpty())
    {
        switch ( QDltOptManager::getInstance()->get_convertionmode() )
        {
        case e_UTF8:
             commandLineConvertToUTF8();
            break;
        case e_DLT:
             commandLineConvertToDLT();
            break;
        case e_ASCI:
             commandLineConvertToASCII();
            break;
        case e_CSV:
             commandLineConvertToCSV();
            break;
        case e_DDLT:
             commandLineConvertToDLTDecoded();
            break;
        default:
             commandLineConvertToASCII();
            break;
        }
    }

    if(QDltOptManager::getInstance()->isTerminate())
    {
        qDebug() << "### Terminate DLT Viewer by option -t";
        exit(0);
    }

    /* auto connect */
    if( (settings->autoConnect != 0) ) // in convertion mode we do not need any connection ...)
    {
        connectAll();
    }

    /* start timer for autoconnect */
    connect(&timer, SIGNAL(timeout()), this, SLOT(timeout())); // we want to start the timer only when an ECU connection is active

    restoreGeometry(settings->geometry);
    restoreState(settings->windowState);

    /* update plugins again to hide plugins shown before after restoreState */
    updatePlugins();

    /*sync checkboxes with action toolbar*/
    ui->actionToggle_FiltersEnabled->setChecked(ui->filtersEnabled->isChecked());
    ui->actionToggle_PluginsEnabled->setChecked(ui->pluginsEnabled->isChecked());
    ui->actionToggle_SortByTimeEnabled->setChecked(ui->checkBoxSortByTime->isChecked());
    ui->actionSort_By_Timestamp->setChecked(ui->checkBoxSortByTimestamp->isChecked());
    ui->actionProject->setChecked(ui->dockWidgetContents->isVisible());
    ui->actionSearch_Results->setChecked(ui->dockWidgetSearchIndex->isVisible());

    if ( true == (bool) settings->StartupMinimized )
    {
        qDebug() << "Start minimzed as defined in the settings";
        this->setWindowState(Qt::WindowMinimized);
    }
}

MainWindow::~MainWindow()
{
    timer.stop(); // stop the receive timeout timer in case it is running
    dltIndexer->stop(); // in case a thread is running we want to stop it
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


    if(( settings->appendDateTime == 1) && (outputfile.size() != 0))
    {
        // get new filename
        QFileInfo info(outputfile.fileName());
        QString newFilename = info.baseName()+
                (startLoggingDateTime.toString("__yyyyMMdd_hhmmss"))+
                (QDateTime::currentDateTime().toString("__yyyyMMdd_hhmmss"))+
                QString(".dlt");
        QFileInfo infoNew(info.absolutePath(),newFilename);

        // rename old file
        qfile.close();
        outputfile.flush();
        outputfile.close();
        bool result = outputfile.rename(info.absoluteFilePath(), infoNew.absoluteFilePath());
        if ( false == result )
        {
        qDebug() << "ERROR renaming" <<  info.absoluteFilePath() << "to" << infoNew.absoluteFilePath();
        }
        else
        {
        qDebug() << "Renaming " <<  info.absoluteFilePath() << "to" << infoNew.absoluteFilePath();
        }
    }

    // deleting search history
    for (int i= 0; i < MaxSearchHistory; i++)
    {
        if (NULL != searchHistoryActs[i])
        {
            delete searchHistoryActs[i];
        }
    }

    QDltSettingsManager::close();
    delete ui;
    delete tableModel;
    delete searchDlg;
    delete dltIndexer;
    delete m_shortcut_searchnext;
    delete m_shortcut_searchprev;
}

void MainWindow::initState()
{
    /* Shortcut for Copy Selection Payload to Clipboard */
    copyPayloadShortcut = new QShortcut(QKeySequence("Ctrl+P"), this);
    connect(copyPayloadShortcut, &QShortcut::activated, this, &MainWindow::onActionMenuConfigCopyPayloadToClipboardTriggered);
  
    /* Shortcut for Mark/Unmark lines */
    markShortcut = new QShortcut(QKeySequence("Ctrl+M"), this);
    connect(markShortcut, &QShortcut::activated, this, &MainWindow::mark_unmark_lines);

    /* Settings */
    settingsDlg = new SettingsDialog(&qfile,this);
    settingsDlg->assertSettingsVersion();
    settingsDlg->readSettings();

    if (QDltSettingsManager::UI_Colour::UI_Dark == QDltSettingsManager::getInstance()->uiColour)
    {
        qApp->setStyle(QStyleFactory::create("Fusion"));
        QPalette darkMode;
        QColor foregroundColor = QColor(49,50,53);
        QColor backgroundColor = QColor(31,31,31);
        QColor disabledColor = QColor(127,127,127);
        QColor brightColor = QColor(253,253,255);
        QColor brighterColor = QColor(Qt::white);
        QColor darkColor = QColor(Qt::black);
        QColor highlightColor = QColor(51,144,255);
        darkMode.setColor(QPalette::AlternateBase, foregroundColor);
        darkMode.setColor(QPalette::Base, backgroundColor);
        darkMode.setColor(QPalette::BrightText, brighterColor);
        darkMode.setColor(QPalette::Disabled, QPalette::BrightText, disabledColor);
        darkMode.setColor(QPalette::Button, foregroundColor);
        darkMode.setColor(QPalette::ButtonText, brightColor);
        darkMode.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
        darkMode.setColor(QPalette::Highlight, highlightColor);
        darkMode.setColor(QPalette::HighlightedText, darkColor);
        darkMode.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);
        darkMode.setColor(QPalette::Link, highlightColor);
        darkMode.setColor(QPalette::Text, brightColor);
        darkMode.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
        darkMode.setColor(QPalette::ToolTipBase, foregroundColor);
        darkMode.setColor(QPalette::ToolTipText, brighterColor);
        darkMode.setColor(QPalette::Disabled, QPalette::ToolTipText, disabledColor);
        darkMode.setColor(QPalette::PlaceholderText, brightColor);
        darkMode.setColor(QPalette::Disabled, QPalette::PlaceholderText, disabledColor);
        darkMode.setColor(QPalette::Window, foregroundColor);
        darkMode.setColor(QPalette::WindowText, brightColor);
        darkMode.setColor(QPalette::Disabled, QPalette::WindowText, disabledColor);
        darkMode.setColor(QPalette::Light, disabledColor);
        darkMode.setColor(QPalette::Midlight, disabledColor);
        darkMode.setColor(QPalette::Dark, foregroundColor);
        darkMode.setColor(QPalette::Mid, backgroundColor);
        darkMode.setColor(QPalette::Shadow, darkColor);
        qApp->setPalette(darkMode);
    }

    recentFiles = settingsDlg->getRecentFiles();
    recentProjects = settingsDlg->getRecentProjects();
    recentFilters = settingsDlg->getRecentFilters();

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
    tableModel->pluginManager = &pluginManager;

    /* initialise project configuration */
    project.ecu = ui->configWidget;
    project.filter = ui->filterWidget;
    project.plugin = ui->pluginWidget;

    connect(ui->pluginWidget, SIGNAL(pluginOrderChanged(QString, int)), this, SLOT(onPluginWidgetPluginPriorityChanged(QString, int)));

    //project.settings = settings;
    project.settings = QDltSettingsManager::getInstance();

    /* Load Plugins before loading default project */
    qDebug() << "### Load Plugins";
    loadPlugins();
    pluginManager.autoscrollStateChanged(settings->autoScroll);

    /* initialize injection */
    injectionAplicationId.clear();
    injectionContextId.clear();
    injectionServiceId.clear();
    injectionData.clear();
    injectionDataBinary = false;
}

void MainWindow::initView()
{
    int maxWidth = 0;
    // With QT5.8 we have a bug with the system proxy configuration
    // which we want to avoid, so we disable it
    QNetworkProxyFactory::setUseSystemConfiguration(false);

    /* make focus on elements visible */
    project.ecu->setStyleSheet("QTreeWidget:focus { border-color:lightgray; border-style:solid; border-width:1px; }");
    ui->tableView->setStyleSheet("QTableView:focus { border-color:lightgray; border-style:solid; border-width:1px; }");
    ui->tableView_SearchIndex->setStyleSheet("QTableView:focus { border-color:lightgray; border-style:solid; border-width:1px; }");

    if (QDltSettingsManager::UI_Colour::UI_Dark == QDltSettingsManager::getInstance()->uiColour)
    {
        project.ecu->setStyleSheet("QTreeWidget:focus { border-color:#7f7f7f; border-style:solid; border-width:1px; }");
        ui->tableView->setStyleSheet("QTableView:focus { border-color:#7f7f7f; border-style:solid; border-width:1px; }");
        ui->tableView_SearchIndex->setStyleSheet("QTableView:focus { border-color:#7f7f7f; border-style:solid; border-width:1px; }");
    }

    /* update default filter selection */
    on_actionDefault_Filter_Reload_triggered();

    /* set table size and en */
    ui->tableView->setModel(tableModel);

    QHeaderView *header = ui->tableView->horizontalHeader();
    header->installEventFilter(tableModel);

    /* For future use enable HTML View in Table */
    //HtmlDelegate* delegate = new HtmlDelegate();
    //ui->tableView->setItemDelegate(delegate);
    //ui->tableView->setItemDelegateForColumn(FieldNames::Payload,delegate);

    /* preset the witdth of the columns somwhow */
    for  (int col=0;col <= ui->tableView->model()->columnCount();col++)
    {
      ui->tableView->setColumnWidth(col,FieldNames::getColumnWidth((FieldNames::Fields)col,settings));
    }

    // Some decoder-plugins can create very long payloads, which in turn severly impact performance
    // So set some limit on what is displayed in the tableview. All details are always available
    // using the message viewer-plugin
    ui->tableView->horizontalHeader()->setMaximumSectionSize(5000);

    // set initial file explorer view
    if (!recentFiles.empty()) {
        ui->tabExplore->setCurrentFile(recentFiles[0]);
    }

    connect(ui->tabExplore, &FileExplorerTab::fileActivated, this, [this](const QString& path){
        openSupportedFile(path);
    });

    /* Enable column sorting of config widget */
    ui->configWidget->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
    ui->configWidget->setSortingEnabled(true);             // should cause sort on add
    ui->configWidget->setHeaderHidden(false);
    ui->filterWidget->setHeaderHidden(false);
    ui->pluginWidget->setHeaderHidden(false);

    /* Start pulsing the apply changes button, when filters draged&dropped */
    connect(ui->filterWidget, SIGNAL(filterItemDropped()), this, SLOT(filterOrderChanged()));
    connect(ui->filterWidget, SIGNAL(filterCountChanged()), this, SLOT(filterCountChanged()));

    /* initialise statusbar */
    totalBytesRcvd = 0;
    totalByteErrorsRcvd = 0;
    totalSyncFoundRcvd = 0;

    /* filename string */
    statusFilename = new QLabel("No log file loaded");
    statusFilename->setMinimumWidth(statusFilename->width());
    //statusFilename->setMaximumWidth(statusFilename->width());
    statusFilename->setMaximumWidth(1240);
                                          // 640 is the initial width of the label
                                          // but for some reason we need this for the very
                                          // first call when setting the tempfile string
                                          // unless this there are is displayed "..."
                                          // more propper solution appreciated ...

    statusFilename->setWordWrap(true);

    /* version string */
    statusFileVersion = new QLabel("Version: <n.a.>");
    maxWidth = QFontMetrics(statusFileVersion->font()).averageCharWidth() * 70;

    statusFileVersion->setMaximumWidth(maxWidth);
    statusFileVersion->setMinimumWidth(1);

    statusFileError = new QLabel("FileErr: 0");
    statusFileError->setText(QString("FileErr: %L1").arg(0));

    statusBytesReceived = new QLabel("Recv: 0");
    statusByteErrorsReceived = new QLabel("Recv Errors: 0");
    statusSyncFoundReceived = new QLabel("Sync found: 0");
    statusProgressBar = new QProgressBar();

    statusBar()->addWidget(statusFilename,1);
    statusBar()->addWidget(statusFileVersion, 1);
    statusBar()->addWidget(statusFileError, 0);
    statusBar()->addWidget(statusBytesReceived, 0);
    statusBar()->addWidget(statusByteErrorsReceived);
    statusBar()->addWidget(statusSyncFoundReceived);
    statusBar()->addWidget(statusProgressBar);

    /* Create search text box */
    searchInput = new SearchForm;
    connect(searchInput, &SearchForm::abortSearch, searchDlg, &SearchDialog::abortSearch);
    searchDlg->appendLineEdit(searchInput->input());
    searchInput->loadComboBoxSearchHistory();

    connect(searchInput->input(), SIGNAL(textChanged(QString)),searchDlg,SLOT(textEditedFromToolbar(QString)));
    connect(searchInput->input(), SIGNAL(returnPressed()), this, SLOT(on_actionFindNext()));
    connect(searchInput->input(), SIGNAL(returnPressed()),searchDlg,SLOT(findNextClicked()));
    connect(searchDlg, SIGNAL(searchProgressChanged(bool)), this, SLOT(onSearchProgressChanged(bool)));
    connect(searchDlg, &SearchDialog::searchProgressValueChanged, this, [this](int progress){
        searchInput->setProgress(progress);
    });
    connect(settingsDlg, SIGNAL(FilterPathChanged()), this, SLOT(on_actionDefault_Filter_Reload_triggered()));
    connect(settingsDlg, SIGNAL(PluginsAutoloadChanged()), this, SLOT(triggerPluginsAutoload()));

    QAction *focusSearchTextbox = new QAction(this);
    focusSearchTextbox->setShortcut(Qt::Key_L | Qt::CTRL);
    connect(focusSearchTextbox, SIGNAL(triggered()), searchInput->input(), SLOT(setFocus()));
    addAction(focusSearchTextbox);

    /* Initialize toolbars. Most of the construction and connection is done via the
     * UI file. See mainwindow.ui, ActionEditor and Signal & Slots editor */
    QList<QAction *> mainActions = ui->mainToolBar->actions();
    m_searchActions = ui->searchToolbar->actions();

    /* Point scroll toggle button to right place */
    scrollButton = mainActions.at(ToolbarPosition::AutoScroll);

    /* Update the scrollbutton status */
    updateScrollButton();

    /* Add shortcut to apply config */
    QAction *applyConfig = new QAction(this);
    applyConfig->setShortcut((Qt::SHIFT | Qt::CTRL) | Qt::Key_C);
    connect(applyConfig, SIGNAL(triggered()), this, SLOT(on_applyConfig_clicked()));
    addAction(applyConfig);

    /* Add shortcut to add filter */
    QAction *addFilter = new QAction(this);
    addFilter->setShortcut((Qt::SHIFT | Qt::CTRL) | Qt::Key_A);
    connect(addFilter, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Add_triggered()));
    addAction(addFilter);
}

void MainWindow::initSignalConnections()
{
    /* Initialize Search History */
    for (int i= 0; i < MaxSearchHistory; i++)
    {
        searchHistoryActs[i] = new QAction(this);
        searchHistoryActs[i]->setVisible(false);
        connect(searchHistoryActs[i], SIGNAL(triggered()), searchDlg, SLOT(loadSearchHistory()));
        ui->menuHistory->addAction(searchHistoryActs[i]);
    }

    /* Connect RegExp settings from and to search dialog */
    connect(m_searchActions.at(ToolbarPosition::Regexp), SIGNAL(toggled(bool)), searchDlg->regexpCheckBox, SLOT(setChecked(bool)));
    connect(searchDlg->regexpCheckBox, SIGNAL(toggled(bool)), m_searchActions.at(ToolbarPosition::Regexp), SLOT(setChecked(bool)));

    /* Connect previous and next buttons to search dialog slots */
    connect(m_searchActions.at(ToolbarPosition::FindPrevious), SIGNAL(triggered()), searchDlg, SLOT(findPreviousClicked()));
    connect(m_searchActions.at(ToolbarPosition::FindNext), SIGNAL(triggered()), searchDlg, SLOT(findNextClicked()));
    connect(m_searchActions.at(ToolbarPosition::FindNext), SIGNAL(triggered()), this, SLOT(on_actionFindNext()));

    /* Connect Search dialog find to action History */
    connect(searchDlg,SIGNAL(addActionHistory()),this,SLOT(onAddActionToHistory()));

    /* Insert search text box to search toolbar, before previous button */

    QAction *before = m_searchActions.at(ToolbarPosition::FindPrevious);
    ui->searchToolbar->insertWidget(before, searchInput);

    /* adding shortcuts - regard: in the search window, the signal is caught by another way, this here only catches the keys when main window is active */
    m_shortcut_searchnext = new QShortcut(QKeySequence("F3"), this);
    connect(m_shortcut_searchnext, &QShortcut::activated, searchDlg, &SearchDialog::findNextClicked);
    m_shortcut_searchprev = new QShortcut(QKeySequence("F2"), this);
    connect(m_shortcut_searchprev, &QShortcut::activated, searchDlg, &SearchDialog::findPreviousClicked);

    connect(ui->tableView->horizontalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(sectionInTableDoubleClicked(int)));

    //for search result table
    connect(searchDlg, SIGNAL(refreshedSearchIndex()), this, SLOT(searchTableRenewed()));
    connect( m_searchresultsTable, SIGNAL( doubleClicked (QModelIndex) ), this, SLOT( searchtable_cellSelected( QModelIndex ) ) );
    connect( m_searchresultsTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onSearchresultsTableSelectionChanged );

    // connect tableView selection model change to handler in mainwindow
    connect(ui->tableView->selectionModel(),  &QItemSelectionModel::selectionChanged, this, &MainWindow::onTableViewSelectionChanged);

    // connect file loaded signal with  explorerView
    connect(this, &MainWindow::dltFileLoaded, this, [this](){
        ui->tabExplore->setCurrentFile(recentFiles[0]);
    });

    connect(ui->tableView, &DltTableView::changeFontSize, this, [this](int direction){
        QFont font;
        font.fromString(settings->fontName);
        int fontSize = font.pointSize() + direction;
        font.setPointSize(fontSize);
        settings->fontName = font.toString();
        ui->tableView->setFont(font);
    });
}

void MainWindow::initSearchTable()
{

    //init search Dialog
    searchDlg = new SearchDialog(this);
    searchDlg->file = &qfile;
    searchDlg->table = ui->tableView;
    searchDlg->pluginManager = &pluginManager;

    /* initialise DLT Search handling */
    m_searchtableModel = new SearchTableModel("Search Index Mainwindow");
    m_searchtableModel->qfile = &qfile;
    m_searchtableModel->project = &project;
    m_searchtableModel->pluginManager = &pluginManager;

    searchDlg->registerSearchTableModel(m_searchtableModel);

    m_searchresultsTable = ui->tableView_SearchIndex;
    m_searchresultsTable->setModel(m_searchtableModel);

    m_searchresultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    /* With Autoscroll= false the tableview doesn't jump to the right edge,
        for example, if the payload column is stretched to full size */
    m_searchresultsTable->setAutoScroll(false);

    m_searchresultsTable->verticalHeader()->setVisible(false);
    m_searchresultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);


    /* set table size and en */
   for  (int col=0;col <= m_searchresultsTable->model()->columnCount();col++)
   {
     m_searchresultsTable->setColumnWidth(col,FieldNames::getColumnWidth((FieldNames::Fields)col,settings));
   }

}

void MainWindow::initFileHandling()
{
    /* Initialize dlt-file indexer  */
    dltIndexer = new DltFileIndexer(&qfile,&pluginManager,&defaultFilter, this);

    /* connect signals */
    connect(dltIndexer, SIGNAL(progressMax(int)), this, SLOT(reloadLogFileProgressMax(int)));
    connect(dltIndexer, SIGNAL(progress(int)), this, SLOT(reloadLogFileProgress(int)));
    connect(dltIndexer, SIGNAL(progressText(QString)), this, SLOT(reloadLogFileProgressText(QString)));
    connect(dltIndexer, SIGNAL(versionString(QString,QString)), this, SLOT(reloadLogFileVersionString(QString,QString)));
    connect(dltIndexer, SIGNAL(finishIndex()), this, SLOT(reloadLogFileFinishIndex()));
    connect(dltIndexer, SIGNAL(finishFilter()), this, SLOT(reloadLogFileFinishFilter()));
    connect(dltIndexer, SIGNAL(finishDefaultFilter()), this, SLOT(reloadLogFileFinishDefaultFilter()));
    connect(dltIndexer, SIGNAL(timezone(int,unsigned char)), this, SLOT(controlMessage_Timezone(int,unsigned char)));
    connect(dltIndexer, SIGNAL(unregisterContext(QString,QString,QString)), this, SLOT(controlMessage_UnregisterContext(QString,QString,QString)));
    connect(dltIndexer, SIGNAL(finished()), this, SLOT(indexDone()));
    connect(dltIndexer, SIGNAL(started()), this, SLOT(indexStart()));

    /* Plugins/Filters enabled checkboxes */
    pluginsEnabled = QDltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool();
    dltIndexer->setPluginsEnabled(pluginsEnabled);
    ui->pluginsEnabled->setChecked(pluginsEnabled);

    ui->filtersEnabled->setChecked(QDltSettingsManager::getInstance()->value("startup/filtersEnabled", true).toBool());
    ui->checkBoxSortByTime->setEnabled(ui->filtersEnabled->isChecked());
    ui->checkBoxSortByTime->setChecked(QDltSettingsManager::getInstance()->value("startup/sortByTimeEnabled", false).toBool());
    ui->checkBoxSortByTimestamp->setEnabled(ui->filtersEnabled->isChecked());
    ui->checkBoxSortByTimestamp->setChecked(QDltSettingsManager::getInstance()->value("startup/sortByTimestampEnabled", false).toBool());
    ui->checkBoxFilterRange->setEnabled(ui->filtersEnabled->isChecked());
    ui->lineEditFilterStart->setEnabled(ui->checkBoxFilterRange->isChecked() && ui->filtersEnabled->isChecked());
    ui->lineEditFilterEnd->setEnabled(ui->checkBoxFilterRange->isChecked() && ui->filtersEnabled->isChecked());

    /* Process Project */
    if(QDltOptManager::getInstance()->isProjectFile())
    {
        openDlpFile(QDltOptManager::getInstance()->getProjectFile());
    }
    else
    {
        /* Load default project file */
        this->setWindowTitle(QString("DLT Viewer - unnamed project - Version : %1 %2").arg(PACKAGE_VERSION).arg(PACKAGE_VERSION_STATE));
        if(settings->defaultProjectFile)
        {
            qDebug() << QString("Loading default project %1").arg(settings->defaultProjectFileName);
            if(!openDlpFile(settings->defaultProjectFileName))
            {
             if (QDltOptManager::getInstance()->issilentMode())
              {
               qDebug() << QString("Cannot load default project %1").arg(settings->defaultProjectFileName);
              }
             else
              {
               QMessageBox::critical(0, QString("DLT Viewer"), QString("Cannot load default project \"%1\"").arg(settings->defaultProjectFileName));
              }

            }
        }
    }

    /* Commands plugin before loading log file */
    qDebug() << "### Plugin commands before loading log file";
    if(!QDltOptManager::getInstance()->getPrePluginCommands().isEmpty())
    {
        QStringList commands = QDltOptManager::getInstance()->getPrePluginCommands();

        // Enable plugins, if they are not enabled
        if(!pluginsEnabled)
        {
            qDebug() << "Enable plugins, because they were disabled!";
            pluginsEnabled = true;
            dltIndexer->setPluginsEnabled(pluginsEnabled);
        }

        for(int num = 0; num< commands.size();num++)
        {
            qDebug() << "Command:" << commands[num];
            QStringList args = commands[num].split("|");
            if(args.size() > 1)
             {
                QString pluginName = args.at(0);
                QString commandName = args.at(1);
                args.removeAt(0);
                args.removeAt(0);
                QStringList commandParams = args;
                commandLineExecutePlugin(pluginName,commandName,commandParams);
             }
        }
    }

    /* load filters by command line */
    if(!QDltOptManager::getInstance()->getFilterFiles().isEmpty())
    {
        qDebug() << "### Load filter";

        // enable filters if they are not enabled
        if(QDltSettingsManager::getInstance()->value("startup/filtersEnabled", true).toBool()==false)
        {
            qDebug("Enable filters, as they were disabled and at least one filter is provided by the commandline!");
            QDltSettingsManager::getInstance()->setValue("startup/filtersEnabled", true);
        }

        for ( const auto& filter : QDltOptManager::getInstance()->getFilterFiles() )
        {
            qDebug() << "Load filter:" << filter;
            if(project.LoadFilter(filter,false))
            {
                // qDebug() << QString("Loading default filter %1").arg(settings->defaultFilterPath);
                filterUpdate();
                setCurrentFilters(filter);
            }
            else
            {
               if (QDltOptManager::getInstance()->issilentMode())
                {
                    qDebug() << "Loading DLT Filter file failed!";
                }
                else
                {
                    QMessageBox::critical(0, QString("DLT Viewer"),QString("Loading DLT Filter file failed!"));
                }
            }
        }
    }

    /* Process Logfile */
    outputfileIsFromCLI = false;
    outputfileIsTemporary = false;
    if(!QDltOptManager::getInstance()->getLogFiles().isEmpty())
    {
        qDebug() << "### Load DLT files";
        QStringList logFiles = QDltOptManager::getInstance()->getLogFiles();
        logFiles.sort();
        openDltFile(logFiles);
       /* Command line file is treated as temp file */
        outputfileIsTemporary = true;
        outputfileIsFromCLI = true;
    }
    else
    {
        /* load default log file */
        if(settings->defaultLogFile)
        {
            openDltFile(QStringList(settings->defaultLogFileName));
            qDebug() << QString("Open default log file ") << QStringList(settings->defaultLogFileName);
            outputfileIsFromCLI = false;
            outputfileIsTemporary = false;
        }
        else
        {
            /* Create temp file */
            QString fn = DltFileUtils::createTempFile(DltFileUtils::getTempPath(QDltOptManager::getInstance()->issilentMode()), QDltOptManager::getInstance()->issilentMode());
            outputfile.setFileName(fn);
            outputfileIsTemporary = true;
            outputfileIsFromCLI = false;

            if(true == outputfile.open(QIODevice::WriteOnly|QIODevice::Truncate))
            {
                openFileNames = QStringList(fn);
                isDltFileReadOnly = false;
                if(QDltOptManager::getInstance()->isCommandlineMode())
                    // if dlt viewer started as converter or with plugin option load file non multithreaded
                    reloadLogFile(false,false);
                else
                    // normally load log file mutithreaded
                    reloadLogFile();
                outputfile.close(); // open later again when writing
            }
            else
            {
               if (QDltOptManager::getInstance()->issilentMode())
                {
                qDebug() << QString("Cannot load temporary log file %1 %2").arg(outputfile.fileName()).arg(outputfile.errorString());
                }
               else
                {
                 QMessageBox::critical(0, QString("DLT Viewer"), QString("Cannot load temporary log file \"%1\"\n%2").arg(outputfile.fileName()).arg(outputfile.errorString()));
                }
            }
        }
    }

    // Import PCAP files from commandline
    if(!QDltOptManager::getInstance()->getPcapFiles().isEmpty())
    {
        qDebug() << "### Import PCAP files";
        for ( const auto& filename : QDltOptManager::getInstance()->getPcapFiles() )
        {
            QDltImporter importer(&outputfile);
            importer.setPcapPorts(settings->importerPcapPorts);
            importer.dltIpcFromPCAP(filename);
        }
        if(QDltOptManager::getInstance()->isCommandlineMode())
            // if dlt viewer started as converter or with plugin option load file non multithreaded
            reloadLogFile(false,false);
        else
            // normally load log file mutithreaded
            reloadLogFile();
    }

    // Import mf4 files from commandline
    if(!QDltOptManager::getInstance()->getMf4Files().isEmpty())
    {
        qDebug() << "### Import MF4 files";
        for ( const auto& filename : QDltOptManager::getInstance()->getMf4Files() )
        {
            QDltImporter importer(&outputfile);
            importer.dltIpcFromMF4(filename);
        }
        if(QDltOptManager::getInstance()->isCommandlineMode())
            // if dlt viewer started as converter or with plugin option load file non multithreaded
            reloadLogFile(false,false);
        else
            // normally load log file mutithreaded
            reloadLogFile();
    }
}


void MainWindow::commandLineConvertToDLT()
{
    qDebug() << "### Convert to DLT";

    /* start exporter */
    QDltExporter exporter(&qfile,QDltOptManager::getInstance()->getConvertDestFile(),&pluginManager,QDltExporter::FormatDlt,QDltExporter::SelectionFiltered,0,project.settings->automaticTimeSettings,project.settings->utcOffset,project.settings->dst,QDltOptManager::getInstance()->getDelimiter(),QDltOptManager::getInstance()->getSignature());
    qDebug() << "Commandline DLT convert to " << QDltOptManager::getInstance()->getConvertDestFile();
    exporter.exportMessages();
    qDebug() << "DLT export to DLT file format done";
}


void MainWindow::commandLineConvertToASCII()
{
    qDebug() << "### Convert to ASCII";

    /* start exporter */
    QDltExporter exporter(&qfile,QDltOptManager::getInstance()->getConvertDestFile(),&pluginManager,QDltExporter::FormatAscii,QDltExporter::SelectionFiltered,0,project.settings->automaticTimeSettings,project.settings->utcOffset,project.settings->dst,QDltOptManager::getInstance()->getDelimiter(),QDltOptManager::getInstance()->getSignature());
    qDebug() << "Commandline ASCII convert to " << QDltOptManager::getInstance()->getConvertDestFile();
    exporter.exportMessages();
    qDebug() << "DLT export ASCII done";
}

void MainWindow::commandLineConvertToCSV()
{
    qDebug() << "### Convert to CSV";

    /* start exporter */
    QDltExporter exporter(&qfile,QDltOptManager::getInstance()->getConvertDestFile(),&pluginManager,QDltExporter::FormatCsv,QDltExporter::SelectionFiltered,0,project.settings->automaticTimeSettings,project.settings->utcOffset,project.settings->dst,QDltOptManager::getInstance()->getDelimiter(),QDltOptManager::getInstance()->getSignature());
    qDebug() << "Commandline ASCII convert to " << QDltOptManager::getInstance()->getConvertDestFile();
    exporter.exportMessages();
    qDebug() << "DLT export CSV done";
}


void MainWindow::commandLineConvertToUTF8()
{
    /* start exporter */
    qDebug() << "### Convert to UTF8";

    QDltExporter exporter(&qfile,QDltOptManager::getInstance()->getConvertDestFile(),&pluginManager,QDltExporter::FormatUTF8,QDltExporter::SelectionFiltered,0,project.settings->automaticTimeSettings,project.settings->utcOffset,project.settings->dst,QDltOptManager::getInstance()->getDelimiter(),QDltOptManager::getInstance()->getSignature());
    qDebug() << "Commandline UTF8 convert to " << QDltOptManager::getInstance()->getConvertDestFile();
    exporter.exportMessages();
    qDebug() << "DLT export UTF8 done";
}

void MainWindow::commandLineConvertToDLTDecoded()
{
    qDebug() << "### Convert to DLT Decoded";

    /* start exporter */
    QDltExporter exporter(&qfile,QDltOptManager::getInstance()->getConvertDestFile(),&pluginManager,QDltExporter::FormatDltDecoded,QDltExporter::SelectionFiltered,0,project.settings->automaticTimeSettings,project.settings->utcOffset,project.settings->dst,QDltOptManager::getInstance()->getDelimiter(),QDltOptManager::getInstance()->getSignature());
    qDebug() << "Commandline decoding to dlt formated file" << QDltOptManager::getInstance()->getConvertDestFile();
    exporter.exportMessages();
    qDebug() << "DLT export DLT decoded done";
}


void MainWindow::ErrorMessage(QMessageBox::Icon level, QString title, QString message){

  if (QDltOptManager::getInstance()->issilentMode())
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

void MainWindow::commandLineExecutePlugin(QString name, QString cmd, QStringList params)
{
    QDltPlugin *plugin = pluginManager.findPlugin(name);

    if(!plugin)
    {
        qDebug() << "Plugin not found or out of memory" << name << "in" << __LINE__ << __FILE__;
        exit(-1);
    }

    /* Check that this is a command plugin */
    if(!plugin->isCommand())
    {
        QString msg("Error: ");
        msg = msg+name+" is not a command plugin.";
        ErrorMessage(QMessageBox::Critical, name, msg);
        exit(-1);
    }
    if(!plugin->command(cmd, params))
    {
        QString msg("Error: ");
        msg.append(name);
        msg.append(plugin->error());
        ErrorMessage(QMessageBox::Warning,name, msg);

        exit(-1);
    }
}

void MainWindow::deleteactualFile()
{
    if(outputfileIsTemporary && !outputfileIsFromCLI)
    {
        // Delete created temp file
        qfile.close();
        outputfile.close();
        if(outputfile.exists() && !outputfile.remove())
        {
         if ( QDltOptManager::getInstance()->issilentMode() == true )
          {
            qDebug() << "Can not delete temporary log file" << outputfile.fileName() << outputfile.errorString();
          }
        else
         {
            QMessageBox::critical(0, QString("DLT Viewer"),
                                  QString("Can not delete temporary log file \"%1\"\n%2")
                                  .arg(outputfile.fileName())
                                  .arg(outputfile.errorString()));
         }
	   }
    }
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    // Shall we save the updated plugin execution priorities??

    settingsDlg->writeSettings(this);
    if(filterIsChanged)
    {
        if(QMessageBox::information(this, "DLT Viewer",
           "You have changed the filter. Do you want to save the filter configuration?",
           QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            on_action_menuFilter_Save_As_triggered();
        }
    }
    if(true == isSearchOngoing)
    {
        event->ignore();
    }
    else if(settings->tempCloseWithoutAsking || outputfile.size() == 0)
    {
        if(!settings->tempSaveOnExit)
            deleteactualFile();

        QMainWindow::closeEvent(event);
    }
    else if(outputfileIsTemporary && !outputfileIsFromCLI)
    {
        if(QMessageBox::information(this, "DLT Viewer",
           "You still have an unsaved temporary file open. Exit anyway?",
           QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            if(!settings->tempSaveOnExit)
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
    if(searchDlg){
            searchDlg->saveSearchHistory(searchHistory);
    }
    if(searchInput){
                searchInput->saveComboBoxSearchHistory();
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

    onNewTriggered(fileName);
}

void MainWindow::onNewTriggered(QString fileName)
{
    //qDebug() << "MainWindow::onNewTriggered" << fileName << __FILE__ << __LINE__;
    // change DLT file working directory
    workingDirectory.setDltDirectory(QFileInfo(fileName).absolutePath());

    // close existing file
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

    // create new file; truncate if already exist
    outputfile.setFileName(fileName);
    outputfileIsTemporary = false;
    outputfileIsFromCLI = false;
    setCurrentFile(fileName);
    if(true == outputfile.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        //qDebug() << "Opening file(s)" << outputfile.fileName() << __FILE__ << __LINE__;
        openFileNames = QStringList(fileName);
        isDltFileReadOnly = false;
        //reloadLogFile(false,false); // avoid "CORRUPT MESSAGE" - non threading !
        reloadLogFile(); // avoid "CORRUPT MESSAGE" - non threading !
        outputfile.close(); // open later again when writing
    }
    else
     {
        if (QDltOptManager::getInstance()->issilentMode())
         {
         qDebug() <<  QString("Cannot create new log file ") << outputfile.fileName() << fileName << outputfile.errorString();
         }
         else
         {
          QMessageBox::critical(0, QString("DLT Viewer"),
                               QString("Cannot create new log file \"%1\"\n%2")
                               .arg(fileName)
                               .arg(outputfile.errorString()));
         }
    }
}



void MainWindow::on_action_menuFile_Open_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
        tr("Open DLT/PCAP/MF4 files"), workingDirectory.getDltDirectory(), tr("DLT/PCAP/MF4 files (*.dlt *.DLT *.pcap *.PCAP *.mf4 *.MF4);;DLT files (*.dlt *.DLT);;PCAP files (*.pcap *.PCAP);;MF4 files (*.mf4 *.MF4)"));

    if(fileNames.isEmpty())
        return;

    /* change DLT file working directory */
    workingDirectory.setDltDirectory(QFileInfo(fileNames[0]).absolutePath());

    QStringList dltFileNames,pcapFileNames,mf4FileNames;

    for ( const auto& i : fileNames )
    {
        if(i.endsWith(".dlt",Qt::CaseInsensitive))
            dltFileNames+=i;
        else if(i.endsWith(".pcap",Qt::CaseInsensitive))
            pcapFileNames+=i;
        else if(i.endsWith(".mf4",Qt::CaseInsensitive))
            mf4FileNames+=i;
    }

    if(!dltFileNames.isEmpty()&&pcapFileNames.isEmpty()&&mf4FileNames.isEmpty())
    {
        onOpenTriggered(dltFileNames);
    }
    else if(dltFileNames.isEmpty()&&!pcapFileNames.isEmpty()&&mf4FileNames.isEmpty())
    {
        on_action_menuFile_Clear_triggered();
        QDltImporter *importerThread = new QDltImporter(&outputfile,pcapFileNames);
        importerThread->setPcapPorts(settings->importerPcapPorts);
        connect(importerThread, &QDltImporter::progress,    this, &MainWindow::progress);
        connect(importerThread, &QDltImporter::resultReady, this, &MainWindow::handleImportResults);
        connect(importerThread, &QDltImporter::finished,    importerThread, &QObject::deleteLater);
        statusProgressBar->show();
        importerThread->start();
    }
    else if(dltFileNames.isEmpty()&&pcapFileNames.isEmpty()&&!mf4FileNames.isEmpty())
    {
        on_action_menuFile_Clear_triggered();
        QDltImporter *importerThread = new QDltImporter(&outputfile,mf4FileNames);
        importerThread->setPcapPorts(settings->importerPcapPorts);
        connect(importerThread, &QDltImporter::progress,    this, &MainWindow::progress);
        connect(importerThread, &QDltImporter::resultReady, this, &MainWindow::handleImportResults);
        connect(importerThread, &QDltImporter::finished,    importerThread, &QObject::deleteLater);
        statusProgressBar->show();
        importerThread->start();
    }
    else
    {
        QMessageBox msgBox(QMessageBox::Warning, "Open DLT/PCAP/MF4 files", "Mixing opening different file types not allowed!", QMessageBox::Close);
        qDebug() << "ERROR: Mixing opening different file types not allowed!";
    }

}


void MainWindow::onOpenTriggered(QStringList filenames)
{
    /* change DLT file working directory */
    workingDirectory.setDltDirectory(QFileInfo(filenames[0]).absolutePath());
    openDltFile(filenames);
    outputfileIsFromCLI = false;
    outputfileIsTemporary = false;

    searchDlg->setMatch(false);
    searchDlg->focusRow(-1);
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
        if (true == openDltFile(QStringList(fileName)))
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

bool MainWindow::openDltFile(QStringList fileNames)
{
    /* close existing file */
    bool ret = false;

    if(fileNames.size()==0)
    {
        qDebug() << "Open filename error in " << __FILE__ << __LINE__;
        return false;
    }
    /* Color of the scrollbar when dark mode is enabled */
    if (QDltSettingsManager::UI_Colour::UI_Dark == QDltSettingsManager::getInstance()->uiColour)
    {
        ui->tableView->setStyleSheet("QTableView QScrollBar::vertical { background-color :#646568; }"
                                            "QScrollBar::horizontal  { background-color :#646568; }");
    }
    //clear search history list
    //searchHistory.clear();
    //clear all the action buttons from history
    for (int i = 0; i < MaxSearchHistory; i++)
    {
        searchHistoryActs[i]->setVisible(false);
    }

    // clear the cache stored for the history
    searchDlg->clearCacheHistory();
    onAddActionToHistory();
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

    /* if the input files are not in DLT format, convert them first */
    if (QDltOptManager::getInstance()->get_inputmode() == e_inputmode::STREAM){
        /* tempfile must be static to prevent deletion before application finishes */
        static QTemporaryFile tempfile; 
        DltFile importfile;

        dlt_file_init(&importfile,0);

        /* open DLT stream file */
        dlt_file_open(&importfile,fileNames.last().toLatin1(),0);
        if(!tempfile.open())
        {
            qDebug() << "Failed opening WriteOnly" << outputfile.fileName();
            return false;
        }
        while (dlt_file_read_raw(&importfile,false,0)>=0)
        {
            tempfile.write((char*)importfile.msg.headerbuffer,importfile.msg.headersize);
            tempfile.write((char*)importfile.msg.databuffer,importfile.msg.datasize);
        }
        tempfile.flush();
        tempfile.close();
        fileNames.append(tempfile.fileName());
    }

    /* open existing file and append new data */
    outputfile.setFileName(fileNames.last());
    setCurrentFile(fileNames.last());
    if( true == outputfile.open(QIODevice::WriteOnly|QIODevice::Append) )
    {
        openFileNames = fileNames;
        isDltFileReadOnly = false;
        //qDebug() << "Opening file(s) wo" << outputfile.fileName() << __FILE__ << __LINE__;
        if(QDltOptManager::getInstance()->isCommandlineMode())
            // if dlt viewer started as converter or with plugin option load file non multithreaded
            reloadLogFile(false,false);
        else
            // normally load log file mutithreaded
            reloadLogFile();
        outputfile.close(); // open later again when writing
        ret = true;
    }
    else
    {
        /* try opening read only */
        if(outputfile.open(QIODevice::ReadOnly))
        {
            openFileNames = fileNames;
            isDltFileReadOnly = true;
            if(QDltOptManager::getInstance()->isCommandlineMode())
                // if dlt viewer started as converter or with plugin option load file non multithreaded
                reloadLogFile(false,false);
            else
                // normally load log file mutithreaded
                reloadLogFile();
            outputfile.close(); // open later again when writing
            ret = true;
            //qDebug() << "Loading file" << fileNames.last() << outputfile.errorString();
        }
        else
        {
            if (QDltOptManager::getInstance()->issilentMode())
              {
                qDebug() << "Accessing logfile error" << fileNames.last() << outputfile.errorString();
              }
            else
              {
                QMessageBox::critical(0, QString("DLT Viewer"), QString("Cannot open log file \"%1\"\n%2").arg(fileNames.last()).arg(outputfile.errorString()));
              }
            ret = false;
        }
    }

    // clear index filter
    ui->checkBoxFilterRange->setChecked(false);
    //ui->lineEditFilterStart->setText(QString("0"));
    //ui->lineEditFilterEnd->setText(QString("%1").arg(qfile.size()));

    if (ret)
        emit dltFileLoaded();

    //qDebug() << "Open files done" << __FILE__ << __LINE__;
    return ret;
}

void MainWindow::appendDltFile(const QString &fileName)
{
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

    if (importfile.file_length <= 0) // This can happen
    {
        dlt_file_free(&importfile, 0);
        return;
    }

    /* get number of files in DLT log file */
    while (dlt_file_read(&importfile,0)>=0)
    {
        num++;
        if ( 0 == (num%1000))
        {
            progress.setValue(
                    static_cast<int>(static_cast<float>(importfile.file_position) * 100.0f
                            / static_cast<float>(importfile.file_length)));
        }
        if (progress.wasCanceled())
        {
            dlt_file_free(&importfile,0);
            return;
        }
    }

    /* read DLT messages and append to current output file */
    if(!outputfile.isOpen() && !outputfile.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        qDebug() << "Failed opening WriteOnly" << outputfile.fileName();
        return;
    }
    for(int pos = 0 ; pos<num ; pos++)
    {
        if ( 0 == (pos % 1000))
        {
            progress.setValue(
                    static_cast<int>(static_cast<float>(pos) * 100.0f
                                     / static_cast<float>(num)));
        }
        if (progress.wasCanceled())
        {
            dlt_file_free(&importfile,0);
            reloadLogFile();
            outputfile.close();
            return;
        }
        dlt_file_message(&importfile,pos,0);
        outputfile.write((char*)importfile.msg.headerbuffer,importfile.msg.headersize);
        outputfile.write((char*)importfile.msg.databuffer,importfile.msg.datasize);
    }
    outputfile.flush();
    outputfile.close();

    dlt_file_free(&importfile,0);

    /* reload log file */
    reloadLogFile();
}

void MainWindow::on_action_menuFile_Import_DLT_Stream_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Import DLT Stream"), workingDirectory.getDltDirectory(), tr("DLT Stream file (*.*)"));
   
    if(fileName.isEmpty())
        return;

    /* change DLT file working directory */
    workingDirectory.setDltDirectory(QFileInfo(fileName).absolutePath());

    DltFile importfile;

    dlt_file_init(&importfile,0);

    /* open DLT stream file */
    dlt_file_open(&importfile,fileName.toLatin1(),0);

    /* parse and build index of complete log file and show progress */
    if(!outputfile.isOpen() && !outputfile.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        qDebug() << "Failed opening WriteOnly" << outputfile.fileName();
        return;
    }
    int version = (dlt_file_check_version(&importfile,0)&0xe0) >>5;
    qDebug() << "DLT file version " << version;
    auto dltReadFunc = (version == 2)  ? dltv2_file_read_raw : dlt_file_read_raw;
	while (dltReadFunc(&importfile,false,0)>=0)
	        {   
	            outputfile.write((char*)importfile.msg.headerbuffer,importfile.msg.headersize);
	            outputfile.write((char*)importfile.msg.databuffer,importfile.msg.datasize);
	        }
    outputfile.flush();
    outputfile.close();

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

    DltFile importfile;

    dlt_file_init(&importfile,0);

    /* open DLT stream file */
    dlt_file_open(&importfile,fileName.toLatin1(),0);

    /* parse and build index of complete log file and show progress */
    if(!outputfile.isOpen() && !outputfile.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        qDebug() << "Failed opening WriteOnly" << outputfile.fileName();
        return;
    }
    while (dlt_file_read_raw(&importfile,true,0)>=0)
    {
        outputfile.write((char*)importfile.msg.headerbuffer,importfile.msg.headersize);
        outputfile.write((char*)importfile.msg.databuffer,importfile.msg.datasize);
    }
    outputfile.flush();
    outputfile.close();

    dlt_file_free(&importfile,0);

    if(importfile.error_messages>0)
    {
        QMessageBox::warning(this, QString("Import DLT Stream with serial header"),
                             QString("%1 corrupted messages during import found!").arg(importfile.error_messages));
    }

    reloadLogFile();
}

void MainWindow::on_actionAppend_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
        tr("Append DLT/PCAP/MF4 files"), workingDirectory.getDltDirectory(), tr("DLT/PCAP/MF4 files (*.dlt *.DLT *.pcap *.PCAP *.mf4 *.MF4);;DLT files (*.dlt *.DLT);;PCAP files (*.pcap *.PCAP);;MF4 files (*.mf4 *.MF4)"));

    if(fileNames.isEmpty())
        return;

    /* change DLT file working directory */
    workingDirectory.setDltDirectory(QFileInfo(fileNames[0]).absolutePath());

    QStringList importFilenames;
    for ( const auto& i : fileNames )
    {
        if(i.endsWith(".dlt",Qt::CaseInsensitive))
            appendDltFile(i);
        else if(i.endsWith(".pcap",Qt::CaseInsensitive))
            importFilenames.append(i);
        else if(i.endsWith(".mf4",Qt::CaseInsensitive))
            importFilenames.append(i);
    }
    if(!importFilenames.isEmpty())
    {
        QDltImporter *importerThread = new QDltImporter(&outputfile,importFilenames);
        importerThread->setPcapPorts(settings->importerPcapPorts);
        connect(importerThread, &QDltImporter::progress,    this, &MainWindow::progress);
        connect(importerThread, &QDltImporter::resultReady, this, &MainWindow::handleImportResults);
        connect(importerThread, &QDltImporter::finished,    importerThread, &QObject::deleteLater);
        statusProgressBar->show();
        importerThread->start();
    }
}

void MainWindow::mark_unmark_lines()
{
    TableModel *model = qobject_cast<TableModel *>(ui->tableView->model());
    QModelIndexList list = ui->tableView->selectionModel()->selection().indexes();
    unsigned long int amount = ui->tableView->selectionModel()->selectedRows().count();
    unsigned long int line;

    for ( unsigned long int i = 0; i < amount; i++ )
    {

     line = ui->tableView->selectionModel()->selectedRows().at(i).row();
     line = qfile.getMsgFilterPos(line);
     if ( true == selectedMarkerRows.contains(line) )// so we remove it
      {
       //qDebug() << "Remove selected line" << line;
       selectedMarkerRows.removeAt(selectedMarkerRows.indexOf(line));
      }
     else // we add it
      {
       //qDebug() << "Add selected line" << line;
       selectedMarkerRows.append(line);
      }
    }
    //qDebug() << selectedMarkerRows;
    model->setManualMarker(selectedMarkerRows, QColor(settings->markercolorRed,settings->markercolorGreen,settings->markercolorBlue)); //used in mainwindow
}

void MainWindow::unmark_all_lines()
{
    TableModel *model = qobject_cast<TableModel *>(ui->tableView->model());
    selectedMarkerRows.clear();
    model->setManualMarker(selectedMarkerRows, QColor(settings->markercolorRed,settings->markercolorGreen,settings->markercolorBlue)); //used in mainwindow
}


void MainWindow::exportSelection(bool ascii = true,bool file = false,QDltExporter::DltExportFormat format = QDltExporter::FormatClipboard)
{
    Q_UNUSED(ascii);
    Q_UNUSED(file);

    QModelIndexList list = ui->tableView->selectionModel()->selection().indexes();

    filterUpdate(); // update filters of qfile before starting Exporting for RegEx operation

    QDltExporter exporter(&qfile,"",&pluginManager,format,QDltExporter::SelectionSelected,&list,project.settings->automaticTimeSettings,project.settings->utcOffset,project.settings->dst,QDltOptManager::getInstance()->getDelimiter(),QDltOptManager::getInstance()->getSignature());
    connect(&exporter,SIGNAL(clipboard(QString)),this,SLOT(clipboard(QString)));
    exporter.exportMessages();
    disconnect(&exporter,SIGNAL(clipboard(QString)),this,SLOT(clipboard(QString)));
}

void MainWindow::exportSelection_searchTable(QDltExporter::DltExportFormat format = QDltExporter::FormatClipboard, const QString& fileName)
{
    const QModelIndexList list = ui->tableView_SearchIndex->selectionModel()->selectedRows();
    QModelIndexList allRows;
    for (int row = 0; row < ui->tableView_SearchIndex->model()->rowCount(); ++row) {
        QModelIndex idx = ui->tableView_SearchIndex->model()->index(row, 0);
        allRows.append(idx);
    }

    // Clear the selection from main table.
    ui->tableView->selectionModel()->clear();

    // Determine which rows to process based on operation type and selection
    QModelIndexList rowsToProcess;
    
    if (!fileName.trimmed().isEmpty()) {
        // File export operation - always export all rows
        rowsToProcess = allRows;
    } else {
        // Clipboard operation - use selected rows only
        rowsToProcess = list;
    }

    // Convert the index from search table to main table entry...
    foreach(QModelIndex index, rowsToProcess)
    {
        int position = index.row();
        unsigned long entry;

        if (! m_searchtableModel->get_SearchResultEntry(position, entry) )
            return;

        //jump_to_line
        int row = nearest_line(entry);
        if (0 > row)
            return;

        QModelIndex newIndex = tableModel->index(row, 0, QModelIndex());
        // Select the row in main table mapping to the search table row
        ui->tableView->blockSignals(true);
        ui->tableView->selectionModel()->select(newIndex, QItemSelectionModel::Select|QItemSelectionModel::Rows);
        ui->tableView->blockSignals(false);
    }

    QModelIndexList finallist = ui->tableView->selectionModel()->selection().indexes();

    filterUpdate(); // update filters of qfile before starting Exporting for RegEx operation

    QString exportFile = fileName.trimmed();
    QDltExporter exporter(&qfile,exportFile,&pluginManager,format,QDltExporter::SelectionSelected,&finallist,project.settings->automaticTimeSettings,project.settings->utcOffset,project.settings->dst,QDltOptManager::getInstance()->getDelimiter(),QDltOptManager::getInstance()->getSignature());
    connect(&exporter,SIGNAL(clipboard(QString)),this,SLOT(clipboard(QString)));
    exporter.exportMessages();
    disconnect(&exporter,SIGNAL(clipboard(QString)),this,SLOT(clipboard(QString)));
}

void MainWindow::on_actionExport_triggered()
{
    /* export dialog */
    exporterDialog.setRange(0,qfile.size());
    exporterDialog.exec();

    if(exporterDialog.result() != QDialog::Accepted)
        return;

    QDltExporter::DltExportFormat exportFormat = exporterDialog.getFormat();
    QDltExporter::DltExportSelection exportSelection = exporterDialog.getSelection();
    QModelIndexList list = ui->tableView->selectionModel()->selection().indexes();

    /* check plausibility */
    if(exportSelection == QDltExporter::SelectionAll)
    {
        qDebug() << "DLT Export of all" << qfile.size() << "messages";
        if(qfile.size() <= 0)
        {
            QMessageBox::critical(this, QString("DLT Viewer"),
                                  QString("Nothing to export. Make sure you have a DLT file open."));
            return;
        }
    }
    else if(exportSelection == QDltExporter::SelectionFiltered)
    {
        qDebug() << "DLT Export of filterd" << qfile.sizeFilter() << "messages";
        if(qfile.sizeFilter() <= 0)
        {
            QMessageBox::critical(this, QString("DLT Viewer"),
                                  QString("Nothing to export. Make sure you have a DLT file open and that not everything is filtered."));
            return;
        }
    }
    else if(exportSelection == QDltExporter::SelectionSelected)
    {
        qDebug() << "DLT Export of selected" << list.count() << "messages";
        if(list.count() <= 0)
        {
            QMessageBox::critical(this, QString("DLT Viewer"),
                                  QString("No messages selected. Select something from the main view."));
            return;
        }
    }

    /* ask for filename */
    QFileDialog dialog(this);
    QStringList filters;

    if((exportFormat == QDltExporter::FormatDlt)||(exportFormat == QDltExporter::FormatDltDecoded))
    {
        filters << "DLT Files (*.dlt)" <<"All files (*.*)";
        dialog.setDefaultSuffix("dlt");
        dialog.setWindowTitle("Export to DLT file");
        qDebug() << "DLT Export to Dlt";
    }
    else if(exportFormat == QDltExporter::FormatAscii)
    {
        filters << "Ascii Files (*.txt)" <<"All files (*.*)";
        dialog.setDefaultSuffix("txt");
        dialog.setWindowTitle("Export to Ascii file");
        qDebug() << "DLT Export to Ascii";
    }
    else if(exportFormat == QDltExporter::FormatUTF8)
    {
        filters << "UTF8 Text Files (*.txt)" <<"All files (*.*)";
        dialog.setDefaultSuffix("txt");
        dialog.setWindowTitle("Export to UTF8 file");
        qDebug() << "DLT Export to UTF8";
    }
    else if(exportFormat == QDltExporter::FormatCsv)
    {
        filters << "CSV Files (*.csv)" <<"All files (*.*)";
        dialog.setDefaultSuffix("csv");
        dialog.setWindowTitle("Export to CSV file");
        qDebug() << "DLT Export to CSV";
    }

    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDirectory(workingDirectory.getExportDirectory());
    dialog.setNameFilters(filters);
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
    QDltExporter *exporterThread;

    unsigned long int startix, stopix;
    exporterDialog.getRange(&startix,&stopix);

    filterUpdate(); // update filters of qfile before starting Exporting for RegEx operation

    if(exportSelection == QDltExporter::SelectionSelected) // marked messages
    {
        exporterThread = new QDltExporter(&qfile, fileName, &pluginManager,exportFormat,exportSelection,&list,project.settings->automaticTimeSettings,project.settings->utcOffset,project.settings->dst,QDltOptManager::getInstance()->getDelimiter(),QDltOptManager::getInstance()->getSignature(),this);
    }
    else
    {
        exporterThread = new QDltExporter(&qfile, fileName, &pluginManager,exportFormat,exportSelection,0,project.settings->automaticTimeSettings,project.settings->utcOffset,project.settings->dst,QDltOptManager::getInstance()->getDelimiter(),QDltOptManager::getInstance()->getSignature(),this);
        exporterThread->exportMessageRange(startix,stopix);
    }
    connect(exporterThread, &QDltExporter::progress,    this, &MainWindow::progress);
    connect(exporterThread, &QDltExporter::resultReady, this, &MainWindow::handleExportResults);
    connect(exporterThread, &QDltExporter::finished,    exporterThread, &QObject::deleteLater);
    statusProgressBar->show();
    exporterThread->start();
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

    onSaveAsTriggered(fileName);
}

void MainWindow::onSaveAsTriggered(QString fileName)
{
    //qDebug() << "MainWindow::onSaveAsTriggered" << fileName << __FILE__ << __LINE__;
    /* check if filename is the same as already open */
    if(outputfile.fileName()==fileName)
    {
        QMessageBox::critical(0, QString("DLT Viewer"),
                              QString("File is already open:\n")+fileName);
        return;
    }

    /* check if file is writable */
    QFileInfo fileinfo(QFileInfo(fileName).absolutePath());
    if(!fileinfo.isDir() || !fileinfo.isWritable())
    {
        QMessageBox::critical(0, QString("DLT Viewer"),
                              QString("Cannot write new file:\n")+fileName);
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
        openFileNames = QStringList(fileName);
        isDltFileReadOnly = false;
        reloadLogFile();
        outputfile.close(); // open later again when writing
    }
    else
        QMessageBox::critical(0, QString("DLT Viewer"),
                              QString("Cannot rename log file \"%1\"\n%2")
                              .arg(fileName)
                              .arg(outputfile.errorString()));
}

/* this one is called when clicking at the broom sign */
void MainWindow::on_action_menuFile_Clear_triggered()
{
    //qDebug() << "MainWindow::on_action_menuFile_Clear_triggered()" << outputfile.fileName() << __FILE__ <<  __LINE__;
    dltIndexer->stop(); // in case an indexer thread is running right now we need to stop it

    QString fn = DltFileUtils::createTempFile(DltFileUtils::getTempPath(QDltOptManager::getInstance()->issilentMode()), QDltOptManager::getInstance()->issilentMode());
    if(!fn.length())
    {
        /* Something went horribly wrong with file name creation
         * There's nothing we can do at this point */
        qDebug() << "Error creating new tmp file !" << __LINE__ << __FILE__;
        return;
    }

    // reset / clear file indexes
    dltIndexer->clearindex();

    //clear all the action buttons from history
    for (int i = 0; i < MaxSearchHistory; i++)
    {
        searchHistoryActs[i]->setVisible(false);
    }
    // clear the cache stored for the history
    searchDlg->clearCacheHistory();

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
    totalBytesRcvd = 0; // reset receive counter too
    totalSyncFoundRcvd = 0; // reset sync counter too
    totalByteErrorsRcvd = 0; // reset receive byte error too
    target_version_string.clear();
    autoloadPluginsVersionEcus.clear();
    autoloadPluginsVersionStrings.clear();

    if(true == outputfile.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        openFileNames = QStringList(fn);
        isDltFileReadOnly = false;
        statusFilename->setMinimumWidth(statusFilename->width()); // just works to show default tmp file + location in status line
        reloadLogFile(false,false);
        outputfile.close(); // open later again when writing
    }
    else
    {
        QMessageBox::critical(0, QString("DLT Viewer"),
                              QString("Cannot open log file \"%1\"\n%2")
                              .arg(fn)
                              .arg(outputfile.errorString()));
    }

    if(( true == outputfileIsTemporary ) && !settings->tempSaveOnClear && (false == outputfileIsFromCLI))
    {
        QFile dfile(oldfn);
        if(!dfile.remove())
        {
            QMessageBox::critical(0, QString("DLT Viewer"),
                                  QString("Cannot delete log file \"%1\"\n%2")
                                  .arg(oldfn)
                                  .arg(dfile.errorString()));
          qDebug() <<   QString("Cannot delete log file %1").arg(oldfn) << "in line" <<__LINE__<< "of" << __FILE__;
        }
    }
    outputfileIsTemporary = true;
    outputfileIsFromCLI = false;
    return;
}

void MainWindow::populateEcusTree(EcuTree&& ecuTree)
{
    QList<QTreeWidgetItem*> ecus;
    // populate ECUs tree view
    for (auto& [ecuid, apps] : ecuTree.ecus) {
        EcuItem* ecuItem = new EcuItem(nullptr);

        ecuItem->id = ecuid;

        QList<QTreeWidgetItem*> appsItems;
        for(const auto& [appid, appdata] : apps) {
            ApplicationItem* appItem = new ApplicationItem(ecuItem);
            appItem->id = appid;
            appItem->description = appdata.description;
            appItem->update();

            QList<QTreeWidgetItem*> contextsItems;
            for(const auto& [ctxid, ctxdata] : appdata.contexts) {
                ContextItem* conItem = new ContextItem(appItem);
                conItem->id = ctxid;
                conItem->loglevel = ctxdata.logLevel;
                conItem->tracestatus = ctxdata.traceStatus;
                conItem->description = ctxdata.description;
                conItem->status = ContextItem::valid;
                conItem->update();

                contextsItems.append(conItem);
            }

            appItem->addChildren(contextsItems);
            appsItems.append(appItem);
        }

        ecuItem->addChildren(appsItems);
        ecuItem->update();

        pluginManager.stateChanged(ecus.size(), QDltConnection::QDltConnectionOffline,
                                   ecuItem->getHostname());

        ecus.append(ecuItem);
    }

    project.ecu->addTopLevelItems(ecus);

    /* Update the ECU list in control plugins */
    updatePluginsECUList();
}

void MainWindow::reloadLogFileProgressMax(int num)
{
    statusProgressBar->setRange(0,num);
}

/* triggered by signal "progress" */
void MainWindow::reloadLogFileProgress(int num)
{
    statusProgressBar->setValue(num);
}

void MainWindow::reloadLogFileProgressText(QString text)
{
    statusProgressBar->setFormat(QString("%1 %p%").arg(text));
}

void MainWindow::progress(QString text,int status,int progress)
{
    switch(status)
    {
    case 1:
        statusProgressBar->setRange(0,100);
        statusProgressBar->setValue(0);
        statusProgressBar->setFormat(QString("%1 %p%").arg(text));
        break;
    case 2:
        statusProgressBar->setValue(progress);
        statusProgressBar->setFormat(QString("%1 %p%").arg(text));
        //qDebug().noquote() << "Progress" << text << progress << "%";
        //statusProgressBar->update();
        //statusProgressBar->repaint();
        //QApplication::processEvents();
        break;
    case 3:
        statusProgressBar->setValue(100);
        statusProgressBar->setFormat(QString("%1 %p%").arg(text));
        break;
    }
}

void MainWindow::clipboard(QString text)
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void MainWindow::reloadLogFileVersionString(QString ecuId, QString version)
{
    // version message found in loading file, only called when loading a logfile !

    if(false == autoloadPluginsVersionEcus.contains(ecuId))
    {
      autoloadPluginsVersionStrings.append(version);
      autoloadPluginsVersionEcus.append(ecuId);

      QFontMetrics fm = QFontMetrics(statusFileVersion->font());
      QString versionString = "Version: " + autoloadPluginsVersionStrings.join("\r\n");
      target_version_string = version;
      statusFileVersion->setText(fm.elidedText(versionString.simplified(), Qt::ElideRight, statusFileVersion->width()));
      statusFileVersion->setToolTip(versionString);
      if( (settings->pluginsAutoloadPath != 0 ) && ( pluginsEnabled == true ))
       {
          qDebug() << "Trigger plugin autoload for ECU" << ecuId << "with version" << version;
          pluginsAutoload(version);
       }
    }
}

void MainWindow::reloadLogFileFinishIndex()
{
    // show already unfiltered messages
    tableModel->setForceEmpty(false);
    tableModel->modelChanged();
    this->update(); // force update
    restoreSelection();

    if(( dltIndexer->getMode() == DltFileIndexer::modeIndex))
    {
        // hide progress bar when finished
        statusProgressBar->reset();
        statusProgressBar->hide();
    }

    ui->lineEditFilterStart->setText(QString("0"));
    if(qfile.size()>0)
        ui->lineEditFilterEnd->setText(QString("%1").arg(qfile.size()-1));
    else
        ui->lineEditFilterEnd->setText(QString("0"));

    if (settings->autoScroll) {
        ui->tableView->scrollToBottom();
    }
}

void MainWindow::reloadLogFileFinishFilter()
{
    // unlock table view
    //ui->tableView->unlock();

    // run through all viewer plugins
    // must be run in the UI thread, if some gui actions are performed
   // if((dltIndexer->getMode() == DltFileIndexer::modeIndexAndFilter) && dltIndexer->getPluginsEnabled())
    if((dltIndexer->getMode() == DltFileIndexer::modeIndexAndFilter) && ( pluginsEnabled == true ))
    {
        QList<QDltPlugin*> activeViewerPlugins;
        activeViewerPlugins = pluginManager.getViewerPlugins();
        for(int i = 0; i < activeViewerPlugins.size(); i++){
            QDltPlugin *item = (QDltPlugin*)activeViewerPlugins.at(i);
            item->initFileFinish();
        }
    }

    // enable filter if requested
    qfile.enableFilter(QDltSettingsManager::getInstance()->value("startup/filtersEnabled", true).toBool());
    qfile.enableSortByTime(QDltSettingsManager::getInstance()->value("startup/sortByTimeEnabled", false).toBool());
    qfile.enableSortByTimestamp(QDltSettingsManager::getInstance()->value("startup/sortByTimestampEnabled", false).toBool());

    // updateIndex, if messages are received in between
    updateIndex();

    // update table
    tableModel->setForceEmpty(false);
    tableModel->modelChanged();
    this->update(); // force update
    restoreSelection();
    m_searchtableModel->modelChanged();

    // process getLogInfoMessages
    if ((dltIndexer->getMode() == DltFileIndexer::modeIndexAndFilter) &&
            settings->updateContextLoadingFile) {
        const QList<int> &msgIndexList = dltIndexer->getGetLogInfoList();

        QDltMsg msg;
        EcuTree ecuTree;
        for (const auto msgIndex : msgIndexList) {
            if (qfile.getMsg(msgIndex, msg)) {
                auto ctrlMsg = qdlt::msg::payload::parse(msg.getPayload(), msg.getEndianness() == QDlt::DltEndiannessBigEndian);
                std::visit([&ecuTree, ecuId = msg.getEcuid()](auto&& payload) {
                    using T = std::decay_t<decltype(payload)>;
                    if constexpr (std::is_same_v<T, qdlt::msg::payload::GetLogInfo>) {
                        ecuTree.add(ecuId, payload);
                    }
                }, ctrlMsg);
            }
        }

        populateEcusTree(std::move(ecuTree));
    }

    // We might have had readyRead events, which we missed
    readyRead();

    // hide progress bar when finished
    statusProgressBar->reset();
    statusProgressBar->hide();
}

void MainWindow::reloadLogFileFinishDefaultFilter()
{
    // hide progress bar when finished
    statusProgressBar->reset();
    statusProgressBar->hide();
}

void MainWindow::reloadLogFile(bool update, bool multithreaded)
{
    qint64 fileerrors = 0;
    /* check if in logging only mode, then do not create index */
    tableModel->setLoggingOnlyMode(settings->loggingOnlyMode);
    tableModel->modelChanged();
    
    if( 0 != settings->loggingOnlyMode )
    {
        qDebug() << "Logging only mode !";
       // return;
    }

    /* clear autoload plugins ecu list */
    if( false == update )
    {
        autoloadPluginsVersionEcus.clear();
        autoloadPluginsVersionStrings.clear();
        statusFileVersion->setText("Version: <n.a.>");
    }

    // update indexFilter only if index already generated
    if( true == update )
    {
        if(QDltSettingsManager::getInstance()->value("startup/filtersEnabled", true).toBool())
        {
            //qDebug() << "indexer with filter" << __LINE__;
            dltIndexer->setMode(DltFileIndexer::modeFilter);
        }
        else
        {
            //qDebug() << "indexer without filter" << __LINE__;
            dltIndexer->setMode(DltFileIndexer::modeNone);
        }

        saveSelection();
    }
    else // no update
    {
        if(QDltSettingsManager::getInstance()->value("startup/filtersEnabled", true).toBool() || pluginsEnabled == true)
        {
            //qDebug() << "indexer with filter" << __LINE__;
            dltIndexer->setMode(DltFileIndexer::modeIndexAndFilter);
        }
        else
        {
            dltIndexer->setMode(DltFileIndexer::modeIndex);
        }

        clearSelection();
    }

    // set index filter range
    if(ui->checkBoxFilterRange->isChecked())
    {
        dltIndexer->setFilterIndexEnabled(true);
        dltIndexer->setFilterIndexStart(ui->lineEditFilterStart->text().toULong());
        dltIndexer->setFilterIndexEnd(ui->lineEditFilterEnd->text().toULong());
    }
    else
    {
        dltIndexer->setFilterIndexEnabled(false);
        dltIndexer->setFilterIndexStart(0);
        dltIndexer->setFilterIndexEnd(0);
    }

    // clear all tables
    ui->tableView->selectionModel()->clear();
    m_searchtableModel->clear_SearchResults();

    QString title = "Search Results";
    ui->dockWidgetSearchIndex->setWindowTitle(title);

    // force empty table
    tableModel->setForceEmpty(true);
    tableModel->modelChanged();

    // stop last indexing process, if any
    dltIndexer->stop();

    // open qfile
    if( false == update)
    {
        for(int num=0;num<openFileNames.size();num++)
        {
            bool back = qfile.open(openFileNames[num],num!=0);

            if ( false == back )
            {
              qDebug() << "ERROR opening file (s)" << openFileNames[num] << __FILE__ << __LINE__;
            }
        }
    }
    //qfile.enableFilter(QDltSettingsManager::getInstance()->value("startup/filtersEnabled", true).toBool());
    qfile.enableFilter(false);

    // lock table view
    //ui->tableView->lock();

    // initialise progress bar
    statusProgressBar->reset();
    statusProgressBar->show();

    // set name of opened log file in status bar
    QFontMetrics fm = QFontMetrics(statusFilename->font());
    QString name = outputfile.fileName();

    if ( true == isDltFileReadOnly )
    {
	name += " (ReadOnly)";
    }

    statusFilename->setMinimumWidth(1); // this is the rollback of the workaround for first call
                                        // with value "1" the window can be reduced in width
    statusFilename->setText(fm.elidedText(name, Qt::ElideLeft, statusFilename->width()));
    statusFilename->setToolTip(name);

    // enable plugins
    pluginsEnabled = QDltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool();
    dltIndexer->setPluginsEnabled(pluginsEnabled);
    dltIndexer->setFiltersEnabled(QDltSettingsManager::getInstance()->value("startup/filtersEnabled", true).toBool());
    dltIndexer->setSortByTimeEnabled(QDltSettingsManager::getInstance()->value("startup/sortByTimeEnabled", false).toBool());
    dltIndexer->setSortByTimestampEnabled(QDltSettingsManager::getInstance()->value("startup/sortByTimestampEnabled", false).toBool());
    dltIndexer->setMultithreaded(multithreaded);
    dltIndexer->setFilterCacheEnabled(settings->filterCache);

    // run through all viewer plugins
    // must be run in the UI thread, if some gui actions are performed
    if( (dltIndexer->getMode() == DltFileIndexer::modeIndexAndFilter) && (pluginsEnabled == true) )
    {
        QList<QDltPlugin*> activeViewerPlugins;
        activeViewerPlugins = pluginManager.getViewerPlugins();
        for(int i = 0; i < activeViewerPlugins.size(); i++)
        {
            QDltPlugin *item = (QDltPlugin*)activeViewerPlugins.at(i);
            //qDebug() << __LINE__ << __FILE__ << item;
            item->initFileStart(&qfile);
        }
    }

    // start indexing
    if(multithreaded == true)
     {
        //qDebug() << "Run indexer multi thread" << __FILE__ << __LINE__;
        dltIndexer->start();
     }
    else
     {
        //qDebug() << "Run indexer single thread" << __FILE__ << __LINE__;
        dltIndexer->run();
        fileerrors = dltIndexer->getfileerrors();
        statusFileError->setText(QString("FileErr: %L1").arg(fileerrors));
     }
}

void MainWindow::reloadLogFileDefaultFilter()
{
    // stop last indexing process, if any
    on_actionDefault_Filter_Reload_triggered();
    dltIndexer->stop();

    // set indexing mode
    dltIndexer->setMode(DltFileIndexer::modeDefaultFilter);

    // initialise progress bar
    statusProgressBar->reset();
    statusProgressBar->show();

    // enable plugins
    pluginsEnabled = QDltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool();
    dltIndexer->setPluginsEnabled(pluginsEnabled);
    dltIndexer->setFiltersEnabled(QDltSettingsManager::getInstance()->value("startup/filtersEnabled", true).toBool());
    dltIndexer->setSortByTimeEnabled(QDltSettingsManager::getInstance()->value("startup/sortByTimeEnabled", false).toBool());
    dltIndexer->setSortByTimestampEnabled(QDltSettingsManager::getInstance()->value("startup/sortByTimestampEnabled", false).toBool());

    // start indexing
    dltIndexer->start();
}

void MainWindow::applySettings()
{
    QFont font;// = ui->tableView->font();
    font.fromString(settings->fontName);
    ui->tableView->setFont(font);
    ui->tableView->verticalHeader()->setDefaultSectionSize(settings->sectionSize);
    m_searchresultsTable->setFont(font);
    m_searchresultsTable->verticalHeader()->setDefaultSectionSize(settings->sectionSize);

    for  (int col=0;col <= ui->tableView->model()->columnCount();col++)
    {
        /*switch(col)
        {
        //override column visibility here
        //FieldNames::SessionId: ui->tableView->setColumnHidden(col,true);
        }*/
        ui->tableView->setColumnHidden(col, !(FieldNames::getColumnShown((FieldNames::Fields)col,settings)));
    }
    //Removing lines which are unlinkely to be necessary for a search. Maybe make configurable.
    //Ideally possible with right-click
    for  (int col=0;col <= m_searchresultsTable->model()->columnCount();col++)
    {
        switch(col)
        {
        //override column visibility here
        case(FieldNames::SessionId): m_searchresultsTable->setColumnHidden(col, true);break;
        case(FieldNames::Counter):   m_searchresultsTable->setColumnHidden(col, true);break;
        case(FieldNames::Type):      m_searchresultsTable->setColumnHidden(col, true);break;
        case(FieldNames::Subtype):   m_searchresultsTable->setColumnHidden(col, true);break;
        case(FieldNames::Mode):      m_searchresultsTable->setColumnHidden(col, true);break;
        case(FieldNames::ArgCount):  m_searchresultsTable->setColumnHidden(col, true);break;
        default:m_searchresultsTable->setColumnHidden(col, !(FieldNames::getColumnShown((FieldNames::Fields)col,settings)));break;
        }
    }

    // disable or enable filter cache
    if(dltIndexer)
        dltIndexer->setFilterCacheEnabled(settings->filterCache);

    // set DLT message chache size
    qfile.setCacheSize(settings->msgCacheSize);

    // set DLTv2 Support
    qfile.setDLTv2Support(settings->supportDLTv2Decoding);
}

void MainWindow::on_action_menuFile_DLTFilesize_triggered()
{
  quint64 payloadSize = qfile.getTotalPayloadSize();
  quint64 messageSize = qfile.getTotalMessageSize();
  quint64 storageSize = qfile.getTotalStorageSize();

  if (payloadSize == 0 && messageSize == 0 && storageSize == 0) {
    QMessageBox::warning(this, "No Data", "Please open a valid DLT file first.");
    return;
  }

  QString message = QString(
        "Payload Size (octets): %1\n"
        "Message Size (octets): %2\n"
        "Storage Size (octets): %3"
        ).arg(payloadSize).arg(messageSize).arg(storageSize);

    QMessageBox::information(this, "DLT File Size Information", message);
}

void MainWindow::on_action_menuFile_Settings_triggered()
{
    // show settings dialog
    settingsDlg->writeDlg();

    // store old values
    int defaultFilterPath = settings->defaultFilterPath;
    QString defaultFilterPathName = settings->defaultFilterPathName;
    int loggingOnlyMode=settings->loggingOnlyMode;

    if(settingsDlg->exec()==1)
    {
        // change settings and store settings persistently
        settingsDlg->readDlg();
        settingsDlg->writeSettings(this);

        // Apply settings to table
        applySettings();

        // reload multifilter list if changed
        if((defaultFilterPath != settings->defaultFilterPath)||(settings->defaultFilterPath && defaultFilterPathName != settings->defaultFilterPathName))
        {
            on_actionDefault_Filter_Reload_triggered();
        }

        updateScrollButton();

        if(loggingOnlyMode!=settings->loggingOnlyMode)
        {
            tableModel->setLoggingOnlyMode(settings->loggingOnlyMode);
            tableModel->modelChanged();
            /* to remove ?? - in case logging only is disbaled the file is reloaded anyway
            if(false == settings->loggingOnlyMode)
            {
                if ( true == QDltOptManager::getInstance()->issilentMode() ) // inverse logic !!
                  qDebug() << "Logging only mode disabled! Please reload DLT file to view file!)";
                else
                  QMessageBox::information(0, QString("DLT Viewer"), QString("Logging only mode disabled! Please reload DLT file to view file!"));
            }
           */
        }

        // update table, perhaps settings changed table, e.g. number of columns
        tableModel->modelChanged();
    }
}

void MainWindow::on_action_menuFile_Quit_triggered()
{
    /* TODO: Add quit code here */
    this->close();

}

void MainWindow::on_actionFindNext()
{
    searchInput->updateHistory();

    QString title = "Search Results";
    if ( 0 < m_searchtableModel->get_SearchResultListSize())
    {
        title += QStringLiteral(": %L1").arg(m_searchtableModel->get_SearchResultListSize());
    }
    ui->dockWidgetSearchIndex->setWindowTitle(title);
    ui->dockWidgetSearchIndex->show();
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

        openDlpFile(fileName);
    }

}

bool MainWindow::anyFiltersEnabled()
{
    if(!(QDltSettingsManager::getInstance()->value("startup/filtersEnabled", true).toBool()))
    {
        return false;
    }
    bool foundEnabledFilter = false;
    for(int num = 0; num < project.filter->topLevelItemCount (); num++)
    {
        FilterItem *item = (FilterItem*)project.filter->topLevelItem(num);
        if(item->checkState(0) == Qt::Checked)
        {
            foundEnabledFilter = true;
            break;
        }
    }
    return foundEnabledFilter;
}

bool MainWindow::openDlfFile(QString fileName,bool replace)
{
    if(!fileName.isEmpty() && project.LoadFilter(fileName,replace))
    {
        workingDirectory.setDlfDirectory(QFileInfo(fileName).absolutePath());
        setCurrentFilters(fileName);
        applyConfigEnabled(true);
        on_filterWidget_itemSelectionChanged();
        ui->tabWidget->setCurrentWidget(ui->tabPFilter);
    }
    else
    {
        QMessageBox::critical(0, QString("DLT Viewer"),QString("Loading DLT Filter file failed!"));
    }
    return true;
}

bool MainWindow::openDlpFile(QString fileName)
{
    /* stop first all ECU connections, so that DLT Viewer will not crash */
    disconnectAll();

    /* Open existing project */
    if(project.Load(fileName))
    {
        /* Applies project settings and save it to registry */
        applySettings();

        /* change Project file working directory */
        workingDirectory.setDlpDirectory(QFileInfo(fileName).absolutePath());

        this->setWindowTitle(QString("DLT Viewer - "+fileName+" - Version : %1 %2").arg(PACKAGE_VERSION).arg(PACKAGE_VERSION_STATE));

        /* Load the plugins description files after loading project */
        updatePlugins();

        setCurrentProject(fileName);

        /* Update the ECU list in control plugins */
        updatePluginsECUList();

        /* After loading the project file update the filters */
        filterUpdate();
        /* and update UI elements for filters */
        on_filterWidget_itemSelectionChanged();

        /* Finally, enable the 'Apply' button, if needed */
        if((QDltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool()) || anyFiltersEnabled())
        {
            applyConfigEnabled(true);
        }
        // Reload logile to enable filters from project file
        if(QDltOptManager::getInstance()->isCommandlineMode())
            // if dlt viewer started as converter or with plugin option load file non multithreaded
            reloadLogFile(false,false);
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

QStringList MainWindow::getAvailableSerialPorts()
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    QStringList portList;

    for (int i = 0; i < ports.size(); i++)
    {
        portList << ports.at(i).portName();
    }

    return portList;
}


QStringList MainWindow::getAvailableNetworkInterfaces()
{
    // the network interfaces are identified by the assigned IP addresse
    // this is e.g. used to select interface for UDP reception
    QList<QNetworkInterface> mListIfaces = QNetworkInterface::allInterfaces();
    QStringList network_interface_namelist;

    for (int i = 0; i < mListIfaces.length(); i++)
    {
      unsigned int flags = mListIfaces[i].flags();
      bool isRunning = (bool)(flags & QNetworkInterface::IsRunning);
      if (true == isRunning)
       {
        QList< QNetworkAddressEntry > iflistings =  mListIfaces[i].addressEntries();
        foreach (const QNetworkAddressEntry &address, iflistings)
         {
          if ( address.ip().protocol() ==  QAbstractSocket::IPv4Protocol  )
           {
             //qDebug() << mListIfaces.at(i).name() << address.ip();
             network_interface_namelist << address.ip().toString();
           }
         } // foreach
       } // isRunning
    }

    network_interface_namelist << QString("AnyIP"); // at the end assign
    return network_interface_namelist;
}


void MainWindow::on_action_menuConfig_ECU_Add_triggered()
{
    //qDebug() << "on_action_menuConfig_ECU_Add_triggered" << __LINE__ << __FILE__;
    static int autoconnect = 1;
    int okorcancel = 1;
    QStringList hostnameListPreset;
    hostnameListPreset << "localhost";
    QStringList multicastAddressesListPreset;
    multicastAddressesListPreset << "<None>";

    QStringList SerialportListPreset = getAvailableSerialPorts();
    QStringList IPportListPreset = getAvailableIPPorts();
    QStringList UDPportListPreset = getAvailableUDPPorts();

    /* show ECU configuration dialog */
    EcuDialog dlg;
    EcuItem initItem;
    dlg.setSerialPortList();
    dlg.setData(initItem);

    /* Read settings for recent hostnames and ports */
    recentHostnames = QDltSettingsManager::getInstance()->value("other/recentHostnameList",hostnameListPreset).toStringList();
    recentIPPorts = QDltSettingsManager::getInstance()->value("other/recentIPPortList",IPportListPreset).toStringList();
    recentUDPPorts = QDltSettingsManager::getInstance()->value("other/recentUDPPortList",UDPportListPreset).toStringList();
    recentEthIF = QDltSettingsManager::getInstance()->value("other/recentEthernetInterface").toString();
    recent_multicastAddresses = QDltSettingsManager::getInstance()->value("other/recentHostMulticastAddresses",multicastAddressesListPreset).toStringList();

    bool b_mcastpreset = QDltSettingsManager::getInstance()->value("other/multicast").toBool();
    int i_iftypeindex = QDltSettingsManager::getInstance()->value("other/iftypeindex").toInt();

    dlg.setIFpresetindex(i_iftypeindex);
    dlg.setMulticast(b_mcastpreset);
    dlg.setHostnameList(recentHostnames);
    dlg.setIPPortList(recentIPPorts);
    dlg.setUDPPortList(recentUDPPorts);
    dlg.setNetworkIFList(recentEthIF);
    dlg.setMulticastAddresses(recent_multicastAddresses);

    if ( ( 1 == settings->autoConnect ) &&
         ( true == QDltOptManager::getInstance()->issilentMode() &&
         ( 1 == autoconnect ) ) )
    {
      qDebug() << "Autoconnect at start: in slient mode just connect to default or project defined ECU ...";
      autoconnect = 0; // we use this tmp flag because we don not want to alter the setings file !
      // and we need the ECU dialog of course we initiated in the GUI after start !
    }
    else
    {
        okorcancel = dlg.exec();// in non silent mode we want to see a dialog box
    }

    if( okorcancel == 1 )
    {
       /* add new ECU to configuration */
       EcuItem* ecuitem = new EcuItem(0);
       dlg.setDialogToEcuItem(ecuitem);

       /* update ECU item */
       ecuitem->update();

       /* add ECU to configuration */
       project.ecu->addTopLevelItem(ecuitem);

       /* Update settings for recent hostnames and ports */
       setCurrentMCAddress(ecuitem->getmcastIP()); // store it in the settings file
       setCurrentHostname(ecuitem->getHostname());
       setCurrentEthIF(ecuitem->getEthIF());
       setCurrentIPPort(QString("%1").arg(ecuitem->getIpport()));
       setCurrentUDPPort(QString("%1").arg(ecuitem->getUdpport()));
       setMcast(ecuitem->is_multicast);
       setInterfaceTypeSelection(dlg.interfacetypecurrentindex());

       /* Update the ECU list in control plugins */
       updatePluginsECUList();

       pluginManager.stateChanged(project.ecu->indexOfTopLevelItem(ecuitem), QDltConnection::QDltConnectionOffline,ecuitem->getHostname());
    }
    return;
}

void MainWindow::on_action_menuConfig_ECU_Edit_triggered()
{
    /* find selected ECU in configuration */
    //qDebug() << "on_action_menuConfig_ECU_Edit_triggered" << __LINE__ << __FILE__;
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        QStringList hostnameListPreset;
        QStringList multicastAddressesListPreset;
        QStringList SerialportListPreset = getAvailableSerialPorts();
        QStringList IPportListPreset = getAvailableIPPorts();
        QStringList UDPportListPreset = getAvailableUDPPorts();
        QStringList NetworkIFListPreset =  getAvailableNetworkInterfaces();

        EcuItem* ecuitem = (EcuItem*) list.at(0);

        /* show ECU configuration dialog */
        EcuDialog dlg;
        dlg.setSerialPortList();
        dlg.setData(*ecuitem);

        /* Read settings for recent hostnames and ports */
        recentHostnames = QDltSettingsManager::getInstance()->value("other/recentHostnameList",hostnameListPreset).toStringList();
        recentIPPorts= QDltSettingsManager::getInstance()->value("other/recentIPPortList",IPportListPreset).toStringList();
        recentUDPPorts= QDltSettingsManager::getInstance()->value("other/recentUDPPortList",UDPportListPreset).toStringList();
        recentEthIF = QDltSettingsManager::getInstance()->value("other/recentEthernetInterface").toString();
        recent_multicastAddresses = QDltSettingsManager::getInstance()->value("other/recentHostMulticastAddresses",multicastAddressesListPreset).toStringList();

        // Ethernet IF
        setCurrentHostname(ecuitem->getHostname());

        // IP port
        setCurrentIPPort(QString("%1").arg(ecuitem->getIpport()));
        setCurrentUDPPort(QString("%1").arg(ecuitem->getUdpport()));

        // MC Ethernet IF
        setCurrentEthIF(ecuitem->getEthIF());

        dlg.setHostnameList(recentHostnames);
        dlg.setIPPortList(recentIPPorts);
        dlg.setUDPPortList(recentUDPPorts);
        dlg.setNetworkIFList(ecuitem->getEthIF());
        dlg.setMulticastAddresses(recent_multicastAddresses);

        if(dlg.exec())
        {
            bool interfaceChanged = false;
            if((ecuitem->interfacetype != dlg.interfacetype() ||
                ecuitem->getHostname() != dlg.hostname() ||
                ecuitem->getIpport() != dlg.tcpport() ||
                ecuitem->getUdpport() != dlg.udpport() ||
                ecuitem->getEthIF() != dlg.EthInterface() ||
                ecuitem->getmcastIP() != dlg.mcastaddress() ||
                ecuitem->getPort() != dlg.serialPort() ||
                ecuitem->is_multicast != dlg.getMulticast() ||
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
            setCurrentMCAddress(ecuitem->getmcastIP()); // store it in the settings file
//tbd save is muticast
            setCurrentIPPort(QString("%1").arg(ecuitem->getIpport()));
            setCurrentUDPPort(QString("%1").arg(ecuitem->getUdpport()));
            setCurrentEthIF(ecuitem->getEthIF());

            /* Update the ECU list in control plugins */
            updatePluginsECUList();

        }
    }
}

void MainWindow::on_action_menuConfig_ECU_Delete_triggered()
{
    /* find selected ECU in configuration */
    //qDebug() << "Delete ECU item" << __LINE__ << __FILE__;
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
        action = new QAction("&Edit All Log Levels...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Edit_All_Log_Levels_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("DLT &Set Log Levels...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Set_Log_Level_triggered()));
        menu.addAction(action);
    }
    else if((list.count() > 1) && (list.at(0)->type() == ecu_type))
    {
        action = new QAction("&Edit All Log Levels...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Edit_All_Log_Levels_triggered()));
        menu.addAction(action);
    }
    else if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        /* ECU is selected */

        action = new QAction("ECU Add...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_ECU_Add_triggered()));
        menu.addAction(action);

        action = new QAction("ECU Edit...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_ECU_Edit_triggered()));
        menu.addAction(action);

        action = new QAction("ECU Delete", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_ECU_Delete_triggered()));
        menu.addAction(action);

        action = new QAction("&ECU Edit All Log Levels...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Edit_All_Log_Levels_triggered()));
        menu.addAction(action);

        action = new QAction("ECU Delete All Contexts", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Delete_All_Contexts_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("Application Add...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Application_Add_triggered()));
        menu.addAction(action);

        action = new QAction("Save IDs as csv", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(onActionMenuConfigSaveAllECUsTriggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("ECU Connect", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Connect_triggered()));
        menu.addAction(action);

        action = new QAction("ECU Disconnect", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Disconnect_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("Expand All ECUs", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Expand_All_ECUs_triggered()));
        menu.addAction(action);

        action = new QAction("Collapse All ECUs", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Collapse_All_ECUs_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("DLT Get Log Info", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Get_Log_Info_triggered()));
        menu.addAction(action);

        action = new QAction("DLT Set All Log Levels", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Set_All_Log_Levels_triggered()));
        menu.addAction(action);

        action = new QAction("DLT Get Default Log Level", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Get_Default_Log_Level_triggered()));
        menu.addAction(action);

        action = new QAction("DLT Set Default Log Level", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Set_Default_Log_Level_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("Store Config", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Store_Config_triggered()));
        menu.addAction(action);

        action = new QAction("Reset to Factory Default", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Reset_to_Factory_Default_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("Send Injection...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Send_Injection_triggered()));
        menu.addAction(action);

        action = new QAction("Get Software Version", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Get_Software_Version_triggered()));
        menu.addAction(action);

        action = new QAction("Get Local Time", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Get_Local_Time_2_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("&Filter Add", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(filterAdd()));
        menu.addAction(action);
    }
    else if((list.count() > 1) && (list.at(0)->type() == application_type))
    {
        action = new QAction("&Edit All Log Levels...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Edit_All_Log_Levels_triggered()));
        menu.addAction(action);
    }
    else if((list.count() == 1) && (list.at(0)->type() == application_type))
    {
        /* Application is selected */

        action = new QAction("&Application Edit...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Application_Edit_triggered()));
        menu.addAction(action);

        action = new QAction("A&pplication Delete...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Application_Delete_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("&Context Add...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Context_Add_triggered()));
        menu.addAction(action);

        action = new QAction("&Edit All Log Levels...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Edit_All_Log_Levels_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("&Filter Add", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(filterAdd()));
        menu.addAction(action);
    }
    else if((list.count() == 1) && (list.at(0)->type() == context_type))
    {
        /* Context is selected */

        action = new QAction("&Context Edit...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Context_Edit_triggered()));
        menu.addAction(action);

        action = new QAction("C&ontext Delete...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Context_Delete_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("DLT &Set Log Level...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Set_Log_Level_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("&Filter Add", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(filterAdd()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("Send Injection...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuDLT_Send_Injection_triggered()));
        menu.addAction(action);
    }
    else
    {
        /* nothing is selected */
        action = new QAction("ECU Add...", &menu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_ECU_Add_triggered()));
        menu.addAction(action);

    }

    /* show popup menu */
    menu.exec(globalPos);

}

void MainWindow::on_tabExplore_fileOpenRequested(const QString &path)
{
    qDebug() << "on_tabExplore_fileOpenRequested" << path;
    if (path.endsWith(".dlt", Qt::CaseInsensitive)) {
        onOpenTriggered(QStringList() << path);
    } else if (path.endsWith(".pcap", Qt::CaseInsensitive)) {
        on_action_menuFile_Clear_triggered();
        QDltImporter* importerThread = new QDltImporter(&outputfile, path);
        importerThread->setPcapPorts(settings->importerPcapPorts);
        connect(importerThread, &QDltImporter::progress, this, &MainWindow::progress);
        connect(importerThread, &QDltImporter::resultReady, this, &MainWindow::handleImportResults);
        connect(importerThread, &QDltImporter::finished, importerThread, &QObject::deleteLater);
        statusProgressBar->show();
        importerThread->start();
    } else if (path.endsWith(".mf4", Qt::CaseInsensitive)) {
        on_action_menuFile_Clear_triggered();
        QDltImporter* importerThread = new QDltImporter(&outputfile, path);
        importerThread->setPcapPorts(settings->importerPcapPorts);
        connect(importerThread, &QDltImporter::progress, this, &MainWindow::progress);
        connect(importerThread, &QDltImporter::resultReady, this, &MainWindow::handleImportResults);
        connect(importerThread, &QDltImporter::finished, importerThread, &QObject::deleteLater);
        statusProgressBar->show();
        importerThread->start();
    } else if (path.endsWith(".dlf", Qt::CaseInsensitive)) {
        openDlfFile(path, true);
        reloadLogFile();
    }
}

void MainWindow::on_tabExplore_fileAppendRequested(const QString& path) {
    qDebug() << "on_tabExplore_fileAppendRequested" << path;

    if (path.endsWith(".dlt", Qt::CaseInsensitive))
        appendDltFile(path);
    else if (path.endsWith(".pcap", Qt::CaseInsensitive) ||
             path.endsWith(".mf4", Qt::CaseInsensitive)) {
        QDltImporter* importerThread = new QDltImporter(&outputfile, path);
        importerThread->setPcapPorts(settings->importerPcapPorts);
        connect(importerThread, &QDltImporter::progress, this, &MainWindow::progress);
        connect(importerThread, &QDltImporter::resultReady, this, &MainWindow::handleImportResults);
        connect(importerThread, &QDltImporter::finished, importerThread, &QObject::deleteLater);
        statusProgressBar->show();
        importerThread->start();
    } else if (path.endsWith(".dlf", Qt::CaseInsensitive))
        openDlfFile(path, false);
}

void MainWindow::on_tabExplore_filesOpenRequest(const QStringList& dltPaths) {
    openDltFile(dltPaths);
    outputfileIsTemporary = true;
}

void MainWindow::on_tabExplore_filesAppendRequest(const QStringList& mf4AndPcapPaths) {
    QDltImporter* importerThread = new QDltImporter(&outputfile, mf4AndPcapPaths);
    importerThread->setPcapPorts(settings->importerPcapPorts);
    connect(importerThread, &QDltImporter::progress, this, &MainWindow::progress);
    connect(importerThread, &QDltImporter::resultReady, this, &MainWindow::handleImportResults);
    connect(importerThread, &QDltImporter::finished, importerThread, &QObject::deleteLater);
    statusProgressBar->show();
    importerThread->start();
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
    if(list.size() < 1)
        action->setEnabled(false);
    else
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Delete_triggered()));
    menu.addAction(action);

    action = new QAction("Filter Clear all", this);
    if(project.filter->topLevelItemCount()<1)
        action->setEnabled(false);
    else
        connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Clear_all_triggered()));
    menu.addAction(action);

    menu.addSeparator();

    if(list.size()>=1)
        action = new QAction("Set Selected Active", this);
    else
        action = new QAction("Set All Active", this);
    if(!project.filter->topLevelItemCount())
        action->setEnabled(false);
    else
        connect(action, SIGNAL(triggered()), this, SLOT(onactionmenuFilter_SetAllActiveTriggered()));
    menu.addAction(action);

    if(list.size()>=1)
        action = new QAction("Set Selected Inactive", this);
    else
        action = new QAction("Set All Inactive", this);
    if(!project.filter->topLevelItemCount())
        action->setEnabled(false);
    else
        connect(action, SIGNAL(triggered()), this, SLOT(onactionmenuFilter_SetAllInactiveTriggered()));
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
        QTreeWidgetItem *widgetItem = list.at(0);
        PluginItem* item = (PluginItem*) widgetItem;
        if(widgetItem->type()!=1000)
        {
            action = new QAction("Plugin Edit...", this);
            connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuPlugin_Edit_triggered()));
            menu.addAction(action);
            menu.addSeparator();

            if(item->getPlugin()->isViewer())
            {
                /* If a viewer plugin is disabled, or enabled but not shown,
                 * add 'show' action. Else add 'hide' action */
                if(item->getPlugin()->getMode() != QDltPlugin::ModeShow)
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
            if(item->getMode() != QDltPlugin::ModeDisable)
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

    // periodically update table view to account for the new incoming messages
    const int drawInterval = (settings->RefreshRate > 0) ? 1000 / settings->RefreshRate
                                                         : 1000 / DEFAULT_REFRESH_RATE;
    drawTimer.start(drawInterval);
    connect(&drawTimer, &QTimer::timeout, this, &MainWindow::drawUpdatedView);
}

void MainWindow::disconnectAll()
{
    drawTimer.stop();
    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
        disconnectECU(ecuitem);
    }
    checkConnectionState();
}

void MainWindow::disconnectECU(EcuItem *ecuitem)
{
    if( true == ecuitem->tryToConnect )
    {
        /* disconnect from host */
        ecuitem->tryToConnect = false;
        ecuitem->connected = false;
        ecuitem->connectError.clear();
        ecuitem->update();
        on_configWidget_itemSelectionChanged();

        /* update conenction state */
        if(ecuitem->interfacetype == EcuItem::INTERFACETYPE_TCP || ecuitem->interfacetype == EcuItem::INTERFACETYPE_UDP)
        {
            /* TCP or UDP */
            if (ecuitem->socket->state()!=QAbstractSocket::UnconnectedState)
                ecuitem->socket->disconnectFromHost();
        }
        else
        {
            /* Serial */
            qDebug() << "Close serial port" << ecuitem->getPort();
            ecuitem->m_serialport->close();
        }

        ecuitem->InvalidAll();
    }
    checkConnectionState();
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
    //qDebug() << "try to connect" << __LINE__;
    if(false == ecuitem->tryToConnect || true == force)
    {
        ecuitem->tryToConnect = true;
        ecuitem->connected = false;
        ecuitem->update();
        on_configWidget_itemSelectionChanged();

        /* reset receive buffer */
        ecuitem->totalBytesRcvd = 0;
        ecuitem->totalBytesRcvdLastTimeout = 0;
        ecuitem->ipcon.clear();
        ecuitem->serialcon.clear();

        /* start socket connection to host */
        if(ecuitem->interfacetype == EcuItem::INTERFACETYPE_TCP)
        {
            /* TCP */
            qDebug()<< "Try to connect to ECU" << GetConnectionType(ecuitem->interfacetype) << ecuitem->getHostname() << QDateTime::currentDateTime().toString("hh:mm:ss");
            /* connect socket signals with window slots */
            if (ecuitem->socket->state()==QAbstractSocket::UnconnectedState)
            {
                disconnect(ecuitem->socket,0,0,0);
                connect(ecuitem->socket,SIGNAL(connected()),this,SLOT(connected()));
                connect(ecuitem->socket,SIGNAL(disconnected()),this,SLOT(disconnected()));
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                connect(ecuitem->socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
#else
                connect(ecuitem->socket, &QAbstractSocket::errorOccurred, this, &MainWindow::error);
#endif
                connect(ecuitem->socket,SIGNAL(readyRead()),this,SLOT(readyRead()));
                connect(ecuitem->socket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(stateChangedIP(QAbstractSocket::SocketState)));
                ecuitem->socket->connectToHost(ecuitem->getHostname(),ecuitem->getIpport());
            }
        }
        /* start socket connection to host */
       else if(ecuitem->interfacetype == EcuItem::INTERFACETYPE_UDP)
        {
            /* UDP */
            bool bindstate = false;
            QString connectIPaddress = ecuitem->getEthIF();

            if ( ecuitem->getEthIF() == "AnyIP")
            {
                connectIPaddress = "0.0.0.0"; // we need to translate AnyIP to 0.0.0.0 on Linux ...
            }

           /* connect socket signals with window slots */
            if (ecuitem->socket->state() == QAbstractSocket::UnconnectedState)
            {
               disconnect(ecuitem->socket,0,0,0);

               if (  ecuitem->is_multicast == true )
               {
                 qDebug()<< "Try to connect (UDP/MC) on" << ecuitem->getEthIF() << GetConnectionType(ecuitem->interfacetype)  << "on port" << ecuitem->getUdpport() << "at" << QDateTime::currentDateTime().toString("hh:mm:ss");
                 //bindstate = ecuitem->socket->bind(QHostAddress(ecuitem->getmcastIP()), ecuitem->getUdpport(),QUdpSocket::ShareAddress );
                 bindstate = ecuitem->socket->bind(QHostAddress(connectIPaddress), ecuitem->getUdpport(),QUdpSocket::ShareAddress ) ;
               }
               else
               {
                qDebug()<< "Try to connect (UDP) to" << ecuitem->getEthIF() << GetConnectionType(ecuitem->interfacetype)  << "on port" << ecuitem->getUdpport() << "at" << QDateTime::currentDateTime().toString("hh:mm:ss");
                bindstate = ecuitem->socket->bind(QHostAddress(connectIPaddress), ecuitem->getUdpport(),QUdpSocket::ShareAddress ) ;
               }

               if ( true == bindstate )//ecuitem->socket->bind(QHostAddress(ecuitem->getmcastIP()), ecuitem->getUdpport(),QUdpSocket::ShareAddress ))
                { // green - success
                 qDebug() << "Bound to " << ecuitem->getEthIF() << "on port" << ecuitem->getUdpport();
                 ecuitem->udpsocket.setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption,26214400);
                 ecuitem->tryToConnect = true;
                 if (  ecuitem->is_multicast == true )
                 {
                     QList<QNetworkInterface> 	interfaces  = QNetworkInterface::allInterfaces();
                     int num;
                     for(num=0;num<interfaces.length();num++)
                     {
                         if(interfaces[num].humanReadableName()==ecuitem->getEthIF())
                         {
                             QString multicastAddressArray = ecuitem->getmcastIP();
                             QStringList multicastAddress = multicastAddressArray.split(QRegularExpression("\\s+"));
                             for(int i = 0;i<multicastAddress.size();i++)
                             {
                                 if(!multicastAddress[i].isEmpty())
                                 {
                                     if ( true == ecuitem->udpsocket.joinMulticastGroup(QHostAddress(multicastAddress[i]), interfaces[num]) )
                                     {
                                        qDebug() << "Successfully joined multicast group" << multicastAddress[i] << "on interface" << ecuitem->getEthIF();
                                     }
                                     else // setting up multicast failed
                                     {
                                        ecuitem->connected = false; // unicast socket setup was ok
                                        ecuitem->connectError.append("Error joining multicast group");
                                        qDebug() << "Error joining multicast group" << multicastAddress[i] << "on interface" << ecuitem->getEthIF() << ecuitem->socket->errorString();
                                     }
                                 }
                             }
                             break;
                         }
                     }
                     if(num==interfaces.length())
                     {
                         ecuitem->connected = false; // unicast socket setup was ok
                         ecuitem->connectError.append("Interface not found");
                         qDebug() << "Error joining multicast group" << ecuitem->getmcastIP() << "on interface" << ecuitem->getEthIF() << ecuitem->socket->errorString();
                     }
                 } // multicast
                 else // unicast case
                 {
                     qDebug() <<  "UDP unicast configured to" << ecuitem->getEthIF();
                 }

                 connect(ecuitem->socket,SIGNAL(connected()),this,SLOT(connected()));
                 connect(ecuitem->socket,SIGNAL(disconnected()),this,SLOT(disconnected()));
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                connect(ecuitem->socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
#else
                connect(ecuitem->socket, &QAbstractSocket::errorOccurred, this, &MainWindow::error);
#endif
                 connect(ecuitem->socket,SIGNAL(readyRead()),this,SLOT(readyRead()));
                 connect(ecuitem->socket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(stateChangedIP(QAbstractSocket::SocketState)));
                 ecuitem->update();
                }
                else
                {
                  qDebug() << "Error - binding failed with" << ecuitem->socket->errorString();
                  ecuitem->connectError.append("Binding failed");
                  ecuitem->connected = false;
                  ecuitem->update();
                }
            }
        }
        else
        {
            /* Serial */
            if ( NULL == ecuitem->m_serialport )
            {
                qDebug()<< "Try to connect to ECU on serial port" << ecuitem->getPort() << QDateTime::currentDateTime().toString("hh:mm:ss");
                ecuitem->m_serialport = new QSerialPort();
                if ( NULL != ecuitem->m_serialport )
                {
                ecuitem->m_serialport->setBaudRate(ecuitem->getBaudrate(), QSerialPort::AllDirections);
		ecuitem->m_serialport->setPortName(ecuitem->getPort());
                ecuitem->m_serialport->setDataBits(QSerialPort::Data8);
                ecuitem->m_serialport->setParity(QSerialPort::NoParity);
                ecuitem->m_serialport->setStopBits(QSerialPort::OneStop);
                ecuitem->m_serialport->setFlowControl(QSerialPort::NoFlowControl);
                //ecuitem->m_serialport->waitForReadyRead(0); // this lead to immediate crash on Windows and Ubuntu
                QThread::msleep(10);
                connect(ecuitem->m_serialport,SIGNAL(readyRead()), this, SLOT(readyRead()));
                connect(ecuitem->m_serialport,SIGNAL(dataTerminalReadyChanged(bool)),this,SLOT(stateChangedSerial(bool)));
                //ecuitem->m_serialport->waitForReadyRead(10);
                }
             }
            else // serial port object already exists
            {
                //to keep things consistent: delete old member, create new one
                //alternatively we could just close the port, and set the new settings.
                ecuitem->m_serialport->close();
                ecuitem->m_serialport->setBaudRate(ecuitem->getBaudrate());
                ecuitem->m_serialport->setPortName(ecuitem->getPort());
            }

            if(ecuitem->m_serialport->isOpen())
            {
                ecuitem->m_serialport->close();
                ecuitem->m_serialport->setBaudRate(ecuitem->getBaudrate());
            }

            ecuitem->m_serialport->open(QIODevice::ReadWrite);

            if( true == ecuitem->m_serialport->isOpen())
            {
                ecuitem->connected = true;
                ecuitem->update();
                on_configWidget_itemSelectionChanged();
                qDebug() << "Open serial port" << ecuitem->getPort();
                /* send new default log level to ECU, if selected in dlg */
                if (ecuitem->updateDataIfOnline)
                {
                    sendUpdates(ecuitem);
                }

            }
        }

        if(  (settings->showCtId && settings->showCtIdDesc) || (settings->showApId && settings->showApIdDesc) )
        {
            controlMessage_GetLogInfo(ecuitem);
        }
    }
    checkConnectionState();
}

void MainWindow::connected()
{
    /* signal emited when connected to host */
    /* find socket which emited signal */
    //qDebug() << "Connected" << __LINE__ << __FILE__;
    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
        if( ecuitem->socket == sender())
        {
            /* update connection state */
            ecuitem->connected = true;
            ecuitem->connectError.clear();
            ecuitem->update();
            on_configWidget_itemSelectionChanged();

            /* reset receive buffer */
            ecuitem->totalBytesRcvd = 0;
            ecuitem->totalBytesRcvdLastTimeout = 0;
            ecuitem->ipcon.clear();
            ecuitem->serialcon.clear();
            qDebug()<<"Connected to" << ecuitem->getHostname() << "at" << QDateTime::currentDateTime().toString("hh:mm:ss") << GetConnectionType(ecuitem->interfacetype);
        }
    }
checkConnectionState();
}

void MainWindow::checkConnectionState()
{
  bool oneConnected=false;
  bool oneTryConnect=false;

  /* find socket which emited signal */
  for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
     {
       EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
       if (true == ecuitem->connected)
        {
          oneConnected=true;
        }
       if (true == ecuitem->tryToConnect)
        {
          oneTryConnect=true;
        }
      }

   if (true == oneTryConnect)
   {
      if ( false ==  timer.isActive() )
        {
        //qDebug() << "Start Timer";
        timer.start(AUTOCONNECT_DEFAULT_TIME);
        }
   }
   else
   {
    if ( true ==  timer.isActive() )
       {
         //qDebug() << "Stop Timer";
         timer.stop();
       }
   }
   //qDebug() << "oneConnected" <<oneConnected << "oneTryConnect" <<  oneTryConnect;
   if (true == oneConnected)
    {
     // green
     this->ui->actionConnectAll->setIcon(QIcon(":/toolbar/png/network-transmit-receive_connected.png"));
    }
   else
    {
        if (true == oneTryConnect)
        {
            // red
            this->ui->actionConnectAll->setIcon(QIcon(":/toolbar/png/network-transmit-receive_disconnected.png"));
        }
        else
        {
            // yellow
            this->ui->actionConnectAll->setIcon(QIcon(":/toolbar/png/network-transmit-receive.png"));
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
        if( ecuitem &&
            (ecuitem->interfacetype == EcuItem::INTERFACETYPE_TCP || ecuitem->interfacetype == EcuItem::INTERFACETYPE_UDP) &&
            ecuitem->socket == sender())
        {
            switch (ecuitem->interfacetype)
            {
               case EcuItem::INTERFACETYPE_TCP:
                    qDebug() << "Disconnected" << ecuitem->getHostname() << "at" << QDateTime::currentDateTime().toString("hh:mm:ss") << GetConnectionType(ecuitem->interfacetype);
                    break;
               case EcuItem::INTERFACETYPE_UDP:
                    qDebug() << "UDP socket closed on" << ecuitem->getEthIF() << "at" << QDateTime::currentDateTime().toString("hh:mm:ss") << GetConnectionType(ecuitem->interfacetype);
                    break;
               default:
                    break;
            }

            /* update connection state */
            ecuitem->connected = false;
            ecuitem->connectError.clear();
            ecuitem->InvalidAll();
            ecuitem->update();
            on_configWidget_itemSelectionChanged();

            /* disconnect socket signals from window slots */
            disconnect(ecuitem->socket,0,0,0);
        }
    }
    checkConnectionState();
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
            if( true == ecuitem->isAutoReconnectTimeoutPassed() && ( true ==  dltIndexer->tryLock()) )
            {
                if((ecuitem->interfacetype == EcuItem::INTERFACETYPE_TCP)
                        && ecuitem->autoReconnect && ecuitem->connected != 0
                        && ecuitem->totalBytesRcvd == static_cast<unsigned long>(ecuitem->totalBytesRcvdLastTimeout))
                {
                    disconnectECU(ecuitem);
                    ecuitem->tryToConnect = true;
                }
                else if((ecuitem->interfacetype == EcuItem::INTERFACETYPE_UDP)
                        && ecuitem->autoReconnect && ecuitem->connected != 0
                        && ecuitem->totalBytesRcvd == static_cast<unsigned long>(ecuitem->totalBytesRcvdLastTimeout))
                {
                    qDebug() << "UDP timeout passed for" << ecuitem->getHostname();
                    ecuitem->tryToConnect = true;
                    ecuitem->connected = false;
                }

                ecuitem->totalBytesRcvdLastTimeout = ecuitem->totalBytesRcvd;
                dltIndexer->unlock();
            }

            if( true == ecuitem->tryToConnect && false == ecuitem->connected )
            {

                if( ecuitem->interfacetype == EcuItem::INTERFACETYPE_TCP )
                {
                qDebug() << "TCP reconnect timeout for" << ecuitem->getHostname();
                connectECU(ecuitem,true);
                }
                else if( ecuitem->interfacetype == EcuItem::INTERFACETYPE_UDP )
                {
                ecuitem->update();
                }
            }
        } // for ecuItem
        checkConnectionState();
}

void MainWindow::error(QAbstractSocket::SocketError /* socketError */)
{
    /* signal emited when connection to host is not possible */
    //qDebug() << "Socket error" << __LINE__ << __FILE__;
    /* find socket which emited signal */
    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
        if( ecuitem &&
            (ecuitem->interfacetype == EcuItem::INTERFACETYPE_TCP || ecuitem->interfacetype == EcuItem::INTERFACETYPE_UDP) &&
            ecuitem->socket == sender())
        {
            /* save error */
            ecuitem->connectError = ecuitem->socket->errorString();
            qDebug() << "Socket connection error" << ecuitem->socket->errorString() << "for" << ecuitem->getHostname() << "on" << ecuitem->getIpport();// << __LINE__ << __FILE__;
            /* disconnect socket */
            ecuitem->socket->disconnectFromHost();

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
    //qDebug() << "readyRead" << __LINE__ << __FILE__;
    /* Delay reading, if indexer is working on the dlt file */
    if(true == dltIndexer->tryLock())
    {
        /* find socket which emited signal */
        for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
        {
            EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
            if( ecuitem && (ecuitem->socket == sender() || ecuitem->m_serialport == sender() || dltIndexer == sender() ) && ( true == ecuitem->connected || (ecuitem->interfacetype == EcuItem::INTERFACETYPE_UDP ) ) )
            {
                read(ecuitem);
            }
        }
        dltIndexer->unlock();
    }
    /*
    else
    {
      qDebug() << "Fail locking indexer in readyRead" << __LINE__ << __FILE__;
      //tbd: have a look why this one is called in commandline / mode File open
    }
    */

}

void MainWindow::writeDLTMessageToFile(const QByteArray& bufferHeader, std::string_view payload,
                                       const EcuItem* ecuitem) {
    DltStorageHeader str = QDltImporter::makeDltStorageHeader();
    if (ecuitem)
        dlt_set_id(str.ecu, ecuitem->id.toLatin1());

    /* check if message is matching the filter */
    // open the outputfile, if it is not open yet
    if(!outputfile.isOpen() && !outputfile.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        qDebug() << "Failed opening WriteOnly" << outputfile.fileName();
    }
    // set start time when writing first data
    if(startLoggingDateTime.isNull())
    {
        startLoggingDateTime = QDateTime::currentDateTime();
    }

    if( settings->splitlogfile != 0) // only in case the file size limit checking is active ...
     {
     // check if files size limit reached ( see Settings->Project Other->Maximum File Size )
     if( ( ((outputfile.size()+sizeof(DltStorageHeader)+bufferHeader.size()+ payload.size())) > settings->fmaxFileSizeMB *1000*1000) )
      {
        createsplitfile();
      }
    }

    // write data into file
    if(!ecuitem || !ecuitem->getWriteDLTv2StorageHeader())
    {
        // write version 1 storage header
        outputfile.write((char*)&str,sizeof(DltStorageHeader));
    }
    else
    {
        // write version 2 storage header
        outputfile.write((char*)"DLT",3);
        quint8 version = 2;
        outputfile.write((char*)&version,1);
        quint32 nanoseconds = str.microseconds * 1000ul; // not in big endian format
        outputfile.write((char*)&nanoseconds,4);
        quint64 seconds = (quint64) str.seconds; // not in big endian format
        outputfile.write(((char*)&seconds),5);
        quint8 length;
        length = ecuitem->id.length();
        outputfile.write((char*)&length,1);
        outputfile.write(ecuitem->id.toLatin1(),ecuitem->id.length());
    }
    outputfile.write(bufferHeader);
    outputfile.write(payload.data(), payload.size());
    outputfile.flush();
    //outputfile.close();  // This slows down online tracing, keep open while online tracing
}

void MainWindow::read(EcuItem* ecuitem)
{
    int udpMessageCounter = 0;
    long int bytesRcvd = 0;

    if (nullptr == ecuitem)
    {
       qDebug() << "Invalid ECU given in" << __FILE__ << "Line:" << __LINE__;
       return;
    }

    qmsg.clear();
    data.clear();

    switch (ecuitem->interfacetype)
     {
      case EcuItem::INTERFACETYPE_TCP:
          /* TCP */
          data = ecuitem->socket->readAll();
          bytesRcvd = data.size();
          //qDebug() << "bytes received" << bytesRcvd;
          ecuitem->ipcon.add(data);
          break;
      case EcuItem::INTERFACETYPE_UDP:
          while(ecuitem->udpsocket.hasPendingDatagrams() && udpMessageCounter<100)
          {
            data.resize(ecuitem->udpsocket.pendingDatagramSize());
            bytesRcvd = ecuitem->udpsocket.readDatagram( data.data(), data.size() );
            //qDebug() << "bytes received" << bytesRcvd;
            unsigned int dataSize = data.size();
            const char* dataPtr = data.data();
            // Find one or more DLT messages in the UDP message
            while(dataSize>0)
            {
                quint32 sizeMsg = qmsg.checkMsgSize(dataPtr,dataSize,settings->supportDLTv2Decoding);
                if(sizeMsg>0)
                {
                    // DLT message found, write it with storage header
                    QByteArray empty;
                    if(settings->loggingOnlyFilteredMessages)
                    {
                        // write only messages which match filter
                        bool silentMode = !QDltOptManager::getInstance()->issilentMode();
                        QDltMsg qmsg;
                        qmsg.setMsg(QByteArray(dataPtr,sizeMsg),false,settings->supportDLTv2Decoding);
                        if ( true == pluginsEnabled ) // we check the general plugin enabled/disabled switch
                        {
                           pluginManager.decodeMsg(qmsg,silentMode);
                        }
                        if(qfile.checkFilter(qmsg))
                        {
                            writeDLTMessageToFile(empty,{dataPtr,sizeMsg}, ecuitem);
                        }
                    }
                    else
                    {
                        // write all messages
                        writeDLTMessageToFile(empty, {dataPtr,sizeMsg}, ecuitem);
                    }
                    totalBytesRcvd+=sizeMsg;
                    if(sizeMsg<=dataSize)
                    {
                        dataSize -= sizeMsg;
                        dataPtr += sizeMsg;
                    }
                    else
                    {
                        dataSize = 0;
                    }
                }
                else
                {
                    dataSize = 0;
                }
            }
            //ecuitem->ipcon.add(data);
            ecuitem->connected= true;
            ecuitem->tryToConnect = true;
            ecuitem->update();
            udpMessageCounter++;

            /* analyse received message, check if DLT control message response */
            /*if(qmsg.setMsg(data,false))
            {
                if ( (qmsg.getType()==QDltMsg::DltTypeControl) && (qmsg.getSubtype()==QDltMsg::DltControlResponse))
                {
                    controlMessage_ReceiveControlMessage(ecuitem,qmsg);
                }
            }*/
          }
          break;
      case EcuItem::INTERFACETYPE_SERIAL_DLT:
      case EcuItem::INTERFACETYPE_SERIAL_ASCII:
          data = ecuitem->m_serialport->readAll();
          bytesRcvd = data.size();
          ecuitem->serialcon.add(data);
          break;
      default:
         break;
     }


    if (bytesRcvd <= 0 && ecuitem->connected == false)
    {
      qDebug() << "ERROR: bytesRcvd <= 0 in " << __LINE__ << __FILE__;
      return;
    }

    /* reading data; new data is added to the current buffer */
     ecuitem->totalBytesRcvd += bytesRcvd;

     while(((ecuitem->interfacetype == EcuItem::INTERFACETYPE_TCP) && ecuitem->ipcon.parseDlt(qmsg,settings->supportDLTv2Decoding)) ||
            (ecuitem->interfacetype == EcuItem::INTERFACETYPE_SERIAL_DLT && ecuitem->serialcon.parseDlt(qmsg,settings->supportDLTv2Decoding)) ||
            (ecuitem->interfacetype == EcuItem::INTERFACETYPE_SERIAL_ASCII && ecuitem->serialcon.parseAscii(qmsg)) )
        {
            /* analyse received message, check if DLT control message response */
            if ( (qmsg.getType()==QDltMsg::DltTypeControl) && (qmsg.getSubtype()==QDltMsg::DltControlResponse))
            {
                controlMessage_ReceiveControlMessage(ecuitem,qmsg);
            }

            /* write message to file */
            const QByteArray bufferHeader = qmsg.getHeader();
            const QByteArray bufferPayload = qmsg.getPayload();
            if(settings->loggingOnlyFilteredMessages)
            {
                // write only messages which match filter
                bool silentMode = !QDltOptManager::getInstance()->issilentMode();
                if ( true == pluginsEnabled ) // we check the general plugin enabled/disabled switch
                {
                   pluginManager.decodeMsg(qmsg,silentMode);
                }
                if(qfile.checkFilter(qmsg))
                {
                    writeDLTMessageToFile(
                                bufferHeader,
                                {bufferPayload.data(),
                                 static_cast<std::string_view::size_type>(bufferPayload.size())},
                                ecuitem);
                }
            }
            else
            {
                // write all messages
                writeDLTMessageToFile(
                            bufferHeader,
                            {bufferPayload.data(),
                             static_cast<std::string_view::size_type>(bufferPayload.size())},
                            ecuitem);
            }

        } //end while

     if(ecuitem->interfacetype == EcuItem::INTERFACETYPE_TCP)
        {
            /* TCP or UDP */
            totalByteErrorsRcvd+=ecuitem->ipcon.bytesError;
            ecuitem->ipcon.bytesError = 0;
            totalBytesRcvd+=ecuitem->ipcon.bytesReceived;
            //qDebug() << "totalBytesRcvd" << totalBytesRcvd;
            ecuitem->ipcon.bytesReceived = 0;
            totalSyncFoundRcvd+=ecuitem->ipcon.syncFound;
            ecuitem->ipcon.syncFound = 0;
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

     //if(outputfile.isOpen()) //&& ( settings->loggingOnlyMode == 0 )  )
     //   {
            if(false == dltIndexer->isRunning())
            {
                updateIndex();
            }
     //   }
}


void MainWindow::createsplitfile()
{
    // get new filename
    dltIndexer->stop();
    QFileInfo info(outputfile.fileName());

    QString newFilename = info.baseName()+
            (startLoggingDateTime.toString("__yyyyMMdd_hhmmss"))+
            (QDateTime::currentDateTime().toString("__yyyyMMdd_hhmmss"))+
            QString(".dlt");
    QFileInfo infoNew(info.absolutePath(),newFilename);
    qDebug() << "Split to" <<  outputfile.fileName() << "to" << infoNew.absoluteFilePath();

    // rename old file
    outputfile.copy(outputfile.fileName(),infoNew.absoluteFilePath());

    // set new start time
    startLoggingDateTime = QDateTime::currentDateTime();

    SplitTriggered(info.absoluteFilePath());

}


void MainWindow::SplitTriggered(QString fileName)
{
    // change DLT file working directory
    workingDirectory.setDltDirectory(QFileInfo(fileName).absolutePath());

    // close existing file
    if(outputfile.isOpen())
    {
        //qDebug() << "isOpen" << fileName << __FILE__ << __LINE__;
        if (outputfile.size() == 0)
        {
            deleteactualFile();
        }
        else
        {
            outputfile.close();
        }
    }

    // create new file; truncate if already exist
    outputfile.setFileName(fileName);
    setCurrentFile(fileName);

    outputfileIsTemporary = false;
    outputfileIsFromCLI = false;

    if(true == outputfile.open(QIODevice::WriteOnly|QIODevice::Truncate))
     {
        openFileNames = QStringList(fileName);
        isDltFileReadOnly = false;
        reloadLogFile(false,true);
        outputfile.close(); // open later again when writing
     }
    else
     {
        if (QDltOptManager::getInstance()->issilentMode())
         {
         qDebug() <<  QString("Cannot create new log file ") << outputfile.fileName() << fileName << outputfile.errorString();
         }
         else
         {
         QMessageBox::critical(0, QString("DLT Viewer"),
                                  QString("Cannot create new log file \"%1\"\n%2")
                                  .arg(fileName)
                                  .arg(outputfile.errorString()));
         }
     }
 }


void MainWindow::updateIndex()
{
    QList<QDltPlugin*> activeViewerPlugins;
    QList<QDltPlugin*> activeDecoderPlugins;
    QDltPlugin *item = 0;
    QDltMsg qmsg;

    activeDecoderPlugins = pluginManager.getDecoderPlugins();
    activeViewerPlugins = pluginManager.getViewerPlugins();
    pluginsEnabled = dltIndexer->getPluginsEnabled();

    /* read received messages in DLT file parser and update DLT message list view */
    /* update indexes  and table view */
    int oldsize = qfile.size();
    qfile.updateIndex();

    bool silentMode = !QDltOptManager::getInstance()->issilentMode();

    if(oldsize!=qfile.size())
    {
        // only run through viewer plugins, if new messages are added
        for(int i = 0; i < activeViewerPlugins.size(); i++)
        {
            item = activeViewerPlugins[i];
            item->updateFileStart();
        }
    }

    for(int num=oldsize;num<qfile.size();num++)
    {
     qmsg.setMsg(qfile.getMsg(num),true,settings->supportDLTv2Decoding);
     qmsg.setIndex(num);

     if ( true == pluginsEnabled ) // we check the general plugin enabled/disabled switch
     {
     for(int i = 0; i < activeViewerPlugins.size(); i++)
      {
            item = activeViewerPlugins.at(i);
            item->updateMsg(num,qmsg);
      }
     }

     if ( true == pluginsEnabled ) // we check the general plugin enabled/disabled switch
      {
        pluginManager.decodeMsg(qmsg,silentMode);
      }

     if(qfile.checkFilter(qmsg))
      {
            qfile.addFilterIndex(num);
      }

     if ( true == pluginsEnabled ) // we check the general plugin enabled/disabled switch
     {
     for(int i = 0; i < activeViewerPlugins.size(); i++)
      {
            item = activeViewerPlugins[i];
            item->updateMsgDecoded(num,qmsg);
      }
     }
    }

    if(oldsize!=qfile.size())
    {
        // only run through viewer plugins, if new messages are added
        for(int i = 0; i < activeViewerPlugins.size(); i++)
        {
            item = activeViewerPlugins.at(i);
            item->updateFileFinish();
        }
    }
}

void MainWindow::drawUpdatedView()
{
    statusByteErrorsReceived->setText(QString("Recv Errors: %L1").arg(totalByteErrorsRcvd));
    statusBytesReceived->setText(QString("Recv: %L1").arg(totalBytesRcvd));
    statusSyncFoundReceived->setText(QString("Sync found: %L1").arg(totalSyncFoundRcvd));

    tableModel->modelChanged();

    //Line below would resize the payload column automatically so that the whole content is readable
    //ui->tableView->resizeColumnToContents(11); //Column 11 is the payload column
    if(settings->autoScroll) {
        ui->tableView->scrollToBottom();
    }
}

void MainWindow::onTableViewSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    Q_UNUSED(deselected);
    if(selected.size()>0)
    {
        /* With Autoscroll= false the tableview doesn't jump to the right edge,
        for example, if the payload column is stretched to full size */
        if (settings->autoScroll != Qt::Unchecked)
        {
            on_actionAutoScroll_triggered(false);
            scrollButton->setChecked(false);
            ui->tableView->setAutoScroll(false);
        }
        pluginsEnabled = dltIndexer->getPluginsEnabled();
        QModelIndex index =  selected[0].topLeft();
        QDltPlugin *item = 0;
        QList<QDltPlugin*> activeViewerPlugins;
        QList<QDltPlugin*> activeDecoderPlugins;
        QDltMsg msg;
        int msgIndex;

        // we need to find visible column, otherwise scrollTo does not work, e.g. if Index is disabled
        for(int col = 0; col <= ui->tableView->model()->columnCount(); col++)
        {
           if(!ui->tableView->isColumnHidden(col))
           {
                index = index.sibling(index.row(), col);
                break;
           }
        }

        //scroll manually because autoscroll is off
        ui->tableView->scrollTo(index);

        msgIndex = qfile.getMsgFilterPos(index.row());
        msg.setMsg(qfile.getMsgFilter(index.row()),true,settings->supportDLTv2Decoding);
        msg.setIndex(qfile.getMsgFilterPos(index.row()));
        activeViewerPlugins = pluginManager.getViewerPlugins();
        activeDecoderPlugins = pluginManager.getDecoderPlugins();

        //qDebug() << "Message at row" << index.row() << "at index" << msgIndex << "selected.";
        //qDebug() << "Viewer plugins" << activeViewerPlugins.size() << "decoder plugins" << activeDecoderPlugins.size() ;

        if(activeViewerPlugins.isEmpty() && activeDecoderPlugins.isEmpty())
        {
            return;
        }

        // Update plugins
        for(int i = 0; i < activeViewerPlugins.size() ; i++)
        {
            item = (QDltPlugin*)activeViewerPlugins.at(i);
            item->selectedIdxMsg(msgIndex,msg);

        }

        if ( pluginsEnabled == true )
        {
        pluginManager.decodeMsg(msg,!QDltOptManager::getInstance()->issilentMode());
        }

        for(int i = 0; i < activeViewerPlugins.size(); i++){
            item = (QDltPlugin*)activeViewerPlugins.at(i);
            item->selectedIdxMsgDecoded(msgIndex,msg);
        }
    }
}

void MainWindow::onSearchresultsTableSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    Q_UNUSED(deselected);
    if(selected.size() > 0)
    {
        QModelIndex index = selected[0].topLeft();

        // we need to find visible column, otherwise scrollTo does not work, e.g. if Index is disabled
        for(int col = 0; col <= m_searchresultsTable->model()->columnCount(); col++)
        {
            if(!m_searchresultsTable->isColumnHidden(col))
            {
                index = index.sibling(index.row(), col);
                break;
            }
        }

        //scroll manually because autoscroll is off
        m_searchresultsTable->scrollTo(index);
    }
}

void MainWindow::controlMessage_ReceiveControlMessage(EcuItem *ecuitem, const QDltMsg &msg)
{
    try {
        auto ctrlMsg = qdlt::msg::payload::parse(
                    msg.getPayload(), msg.getEndianness() == QDlt::DltEndiannessBigEndian);
        std::visit(
                    overloaded{
                        [&](const qdlt::msg::payload::GetSoftwareVersion&) {
                            // check if plugin autoload enabled and version string not already parsed
                            if (!autoloadPluginsVersionEcus.contains(msg.getEcuid())) {
                                versionString(msg);
                                autoloadPluginsVersionEcus.append(msg.getEcuid());
                            }
                        },
                        [&](const qdlt::msg::payload::GetLogInfo& payload) {
                            if (payload.status == 8) {
                                ecuitem->InvalidAll();
                            }

                            if (payload.status == 6 || payload.status == 7) {
                                for (const auto& app : payload.apps) {
                                    for (const auto& ctx : app.ctxs) {
                                        controlMessage_SetContext(ecuitem, app.id, ctx.id, ctx.description,
                                        ctx.logLevel, ctx.traceStatus);
                                    }
                                    if (payload.status == 7) {
                                        controlMessage_SetApplication(ecuitem, app.id, app.description);
                                    }
                                }
                            }
                        },
                        [&](const qdlt::msg::payload::GetDefaultLogLevel& payload) {
                            switch (payload.status) {
                                case 0: /* OK */
                                ecuitem->loglevel = payload.logLevel;
                                ecuitem->status = EcuItem::valid;
                                break;
                                case 1: /* NOT_SUPPORTED */
                                ecuitem->status = EcuItem::unknown;
                                break;
                                case 2: /* ERROR */
                                ecuitem->status = EcuItem::invalid;
                                break;
                            }
                            ecuitem->update();
                        },
                        [&](const qdlt::msg::payload::Timezone& payload) {
                            controlMessage_Timezone(payload.timezone, payload.isDst);
                        },
                        [&](const qdlt::msg::payload::UnregisterContext& payload) {
                            controlMessage_UnregisterContext(msg.getEcuid(), payload.appid, payload.ctxid);
                        },
                        [](const qdlt::msg::payload::SetLogLevel&) {
                            // nothing to do
                        },
                        [](const qdlt::msg::payload::Uninteresting& payload) {
                            qDebug() << "Received control message with id: " << payload.serviceId;
                        }},
                    ctrlMsg);
    } catch (const std::exception& e) {
        qDebug() << "Error parsing control message: " << e.what();
    }
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
    if ((ecuitem->interfacetype == EcuItem::INTERFACETYPE_TCP || ecuitem->interfacetype == EcuItem::INTERFACETYPE_UDP) && ecuitem->socket->isOpen())
    {
        QByteArray tmpBuf;

        /* Optional: Send serial header, if requested */
        if (ecuitem->getSendSerialHeaderIp())
            tmpBuf.append((const char*)dltSerialHeader, sizeof(dltSerialHeader));

        /* Send data */
        tmpBuf.append((const char*)msg.headerbuffer+sizeof(DltStorageHeader),msg.headersize-sizeof(DltStorageHeader));
        tmpBuf.append((const char*)msg.databuffer,msg.datasize);

        ecuitem->socket->write(tmpBuf);
    }
    else if (ecuitem->interfacetype == EcuItem::INTERFACETYPE_SERIAL_DLT && ecuitem->m_serialport && ecuitem->m_serialport->isOpen())
    {
        /* Optional: Send serial header, if requested */
        if (ecuitem->getSendSerialHeaderSerial())
            ecuitem->m_serialport->write((const char*)dltSerialHeader,sizeof(dltSerialHeader));

        /* Send data */
        ecuitem->m_serialport->write((const char*)msg.headerbuffer+sizeof(DltStorageHeader),msg.headersize-sizeof(DltStorageHeader));
        ecuitem->m_serialport->write((const char*)msg.databuffer,msg.datasize);
    }
    else if (ecuitem->interfacetype == EcuItem::INTERFACETYPE_SERIAL_ASCII && ecuitem->m_serialport && ecuitem->m_serialport->isOpen())
    {
        /* In SERIAL_ASCII mode we send only user input */
        if (appid == "SER" && contid == "CON") {
            ecuitem->m_serialport->write((const char*)(msg.databuffer+8),(msg.datasize-8));
            ecuitem->m_serialport->write("\r\n");
        }
        else
        {
        return;
        }
    }
    else
    {
        /* ECU is not connected */
        qDebug() << "ECU is not connected !!";
        return;
    }

    /* Skip the file handling, if indexer is working on the file */
    if(!outputfile.isOpen() && !outputfile.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        qDebug() << "Failed opening WriteOnly" << outputfile.fileName();
        return;
    }
    if(dltIndexer->tryLock())
    {
        /* store ctrl message in log file */
        outputfile.write((const char*)msg.headerbuffer,msg.headersize);
        outputfile.write((const char*)msg.databuffer,msg.datasize);
        outputfile.flush();

        /* read received messages in DLT file parser and update DLT message list view */
        /* update indexes  and table view */
        if(!dltIndexer->isRunning())
            updateIndex();

        dltIndexer->unlock();
    }
    //outputfile.close();  // This slows down online tracing, keep open while online tracing

}

void MainWindow::controlMessage_WriteControlMessage(DltMessage &msg, QString appid, QString contid)
{
    QByteArray data;
    QDltMsg qmsg;

    /* prepare storage header */
    msg.storageheader = (DltStorageHeader*)msg.headerbuffer;
    dlt_set_storageheader(msg.storageheader,"DLTV");

    /* prepare standard header */
    msg.standardheader = (DltStandardHeader*)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_WEID | DLT_HTYP_WTMS | DLT_HTYP_UEH | DLT_HTYP_PROTOCOL_VERSION1 ;

#if (BYTE_ORDER==BIG_ENDIAN)
    msg.standardheader->htyp = (msg.standardheader->htyp | DLT_HTYP_MSBF);
#endif

    msg.standardheader->mcnt = 0;

    /* Set header extra parameters */
    dlt_set_id(msg.headerextra.ecu,"DLTV");
    msg.headerextra.tmsp = dlt_uptime();

    /* Copy header extra parameters to headerbuffer */
    dlt_message_set_extraparameters(&msg,0);

    /* prepare extended header */
    msg.extendedheader = (DltExtendedHeader*)(msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp) );
    msg.extendedheader->msin = DLT_MSIN_CONTROL_RESPONSE;
    msg.extendedheader->noar = 1; /* number of arguments */
    if (appid.isEmpty())
    {
        dlt_set_id(msg.extendedheader->apid,"DLTV");       /* application id */
    }
    else
    {
        dlt_set_id(msg.extendedheader->apid, appid.toLatin1());
    }
    if (contid.isEmpty())
    {
        dlt_set_id(msg.extendedheader->ctid,"DLTV");       /* context id */
    }
    else
    {
        dlt_set_id(msg.extendedheader->ctid, contid.toLatin1());
    }

    /* prepare length information */
    msg.headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + sizeof(DltExtendedHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp);
    msg.standardheader->len = DLT_HTOBE_16(msg.headersize - sizeof(DltStorageHeader) + msg.datasize);

    /* Skip the file handling, if indexer is working on the file */
    if(!outputfile.isOpen() && !outputfile.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        qDebug() << "Failed opening WriteOnly" << outputfile.fileName();
        return;
    }
    if(dltIndexer->tryLock())
    {
        /* store ctrl message in log file */
        // https://bugreports.qt-project.org/browse/QTBUG-26069
        outputfile.seek(outputfile.size());
        outputfile.write((const char*)msg.headerbuffer,msg.headersize);
        outputfile.write((const char*)msg.databuffer,msg.datasize);
        outputfile.flush();

        /* read received messages in DLT file parser and update DLT message list view */
        /* update indexes  and table view */
        if(!dltIndexer->isRunning())
            updateIndex();

        dltIndexer->unlock();
    }
    //outputfile.close();  // This slows down online tracing, keep open while online tracing
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

void MainWindow::controlMessage_SetLogLevel(EcuItem* ecuitem, QString app, QString con,
                                            int log_level) {
    DltServiceSetLogLevel req;
    req.service_id = DLT_SERVICE_ID_SET_LOG_LEVEL;
    dlt_set_id(req.apid, app.toLatin1());
    dlt_set_id(req.ctid, con.toLatin1());
    req.log_level = log_level;
    dlt_set_id(req.com, "remo");

    QDltMsgWrapper msgWrapper(std::move(req));
    controlMessage_SendControlMessage(ecuitem, msgWrapper.getMessage(), "", "");
}

void MainWindow::controlMessage_SetDefaultLogLevel(EcuItem* ecuitem, int status) {
    DltServiceSetDefaultLogLevel req;
    req.service_id = DLT_SERVICE_ID_SET_DEFAULT_LOG_LEVEL;
    req.log_level = status;
    dlt_set_id(req.com, "remo");

    QDltMsgWrapper msgWrapper(std::move(req));
    controlMessage_SendControlMessage(ecuitem, msgWrapper.getMessage(), "", "");
}

void MainWindow::controlMessage_SetTraceStatus(EcuItem* ecuitem, QString app, QString con,
                                               int status) {
    DltServiceSetLogLevel req;
    req.service_id = DLT_SERVICE_ID_SET_TRACE_STATUS;
    dlt_set_id(req.apid, app.toLatin1());
    dlt_set_id(req.ctid, con.toLatin1());
    req.log_level = status;
    dlt_set_id(req.com, "remo");

    QDltMsgWrapper msgWrapper(std::move(req));
    controlMessage_SendControlMessage(ecuitem, msgWrapper.getMessage(), "", "");
}

void MainWindow::controlMessage_SetDefaultTraceStatus(EcuItem* ecuitem, int status) {
    DltServiceSetDefaultLogLevel req;
    req.service_id = DLT_SERVICE_ID_SET_DEFAULT_TRACE_STATUS;
    req.log_level = status;
    dlt_set_id(req.com, "remo");

    QDltMsgWrapper msgWrapper(std::move(req));
    controlMessage_SendControlMessage(ecuitem, msgWrapper.getMessage(), "", "");
}

void MainWindow::controlMessage_SetVerboseMode(EcuItem* ecuitem, int verbosemode)
{
    DltServiceSetVerboseMode req;
    req.service_id = DLT_SERVICE_ID_SET_VERBOSE_MODE;
    req.new_status = verbosemode;

    QDltMsgWrapper msgWrapper(std::move(req));
    controlMessage_SendControlMessage(ecuitem, msgWrapper.getMessage(), "", "");
}

void MainWindow::controlMessage_SetTimingPackets(EcuItem* ecuitem, bool enable) {
    DltServiceSetVerboseMode req;
    req.service_id = DLT_SERVICE_ID_SET_TIMING_PACKETS;
    req.new_status = (enable ? 1 : 0);

    QDltMsgWrapper msgWrapper(std::move(req));
    controlMessage_SendControlMessage(ecuitem, msgWrapper.getMessage(), "", "");
}

void MainWindow::controlMessage_GetLogInfo(EcuItem* ecuitem) {
    DltServiceGetLogInfoRequest req;
    req.service_id = DLT_SERVICE_ID_GET_LOG_INFO;
    req.options = 7;
    dlt_set_id(req.apid, "");
    dlt_set_id(req.ctid, "");
    dlt_set_id(req.com, "remo");

    QDltMsgWrapper msgWrapper(std::move(req));
    controlMessage_SendControlMessage(ecuitem, msgWrapper.getMessage(), "", "");
}

void MainWindow::controlMessage_GetSoftwareVersion(EcuItem* ecuitem) {
    DltServiceGetSoftwareVersion req;
    req.service_id = DLT_SERVICE_ID_GET_SOFTWARE_VERSION;

    QDltMsgWrapper msgWrapper(std::move(req));
    controlMessage_SendControlMessage(ecuitem, msgWrapper.getMessage(), "", "");
}

void MainWindow::ControlServiceRequest(EcuItem* ecuitem, uint32_t serviceId) {
    QDltMsgWrapper msgWrapper(std::move(serviceId));
    controlMessage_SendControlMessage(ecuitem, msgWrapper.getMessage(), "", "");
}

void MainWindow::controlMessage_Marker() {
    DltServiceMarker resp;
    resp.service_id = DLT_SERVICE_ID_MARKER;
    resp.status = DLT_SERVICE_RESPONSE_OK;

    QDltMsgWrapper msgWrapper(std::move(resp));
    controlMessage_WriteControlMessage(msgWrapper.getMessage(), "", "");
}

void MainWindow::SendInjection(EcuItem* ecuitem)
{
    qDebug() << "DLT SendInjection" << injectionAplicationId << injectionContextId << injectionServiceId << __LINE__;

    if (ecuitem->interfacetype == EcuItem::INTERFACETYPE_SERIAL_ASCII)
    {
        injectionAplicationId = "SER";
        injectionContextId    = "CON";
        injectionServiceId    = "9999";
    }

    if (injectionAplicationId.isEmpty() || injectionContextId.isEmpty() || injectionServiceId.isEmpty() )
    {
        qDebug() << "Error: either APID =" << injectionAplicationId << ", CTID = "<< injectionContextId <<  "or service ID=" << injectionServiceId << "is missing";
        return;
    }

    bool ok = true;
    unsigned int serviceID = (unsigned int)injectionServiceId.toInt(&ok, 0);
    if ((serviceID < DLT_SERVICE_ID_CALLSW_CINJECTION) || (serviceID == 0)) {
        qDebug() << "Wrong range of service id: " << serviceID << ", it has to be > "
                 << DLT_SERVICE_ID_CALLSW_CINJECTION;
        return;
    }

    QByteArray hexData;
    // prepare injection data
    if (injectionDataBinary) {
        hexData = QByteArray::fromHex(injectionData.toLatin1());
    } else {
        hexData = injectionData.toUtf8();
    }
    const std::vector<uint8_t> dataBytes(hexData.begin(), hexData.end());

    QDltMsgWrapper msgWrapper(serviceID, dataBytes);
    controlMessage_SendControlMessage(ecuitem, msgWrapper.getMessage(), injectionAplicationId,
                                      injectionContextId);
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

void MainWindow::on_action_menuDLT_Get_Local_Time_2_triggered()
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

void MainWindow::connectEcuSignal(int index)
{
    EcuItem* ecuitem = (EcuItem*) project.ecu->topLevelItem(index);

    if(ecuitem)
    {
        connectECU(ecuitem);
    }
}

void MainWindow:: disconnectEcuSignal(int index)
{
    EcuItem* ecuitem = (EcuItem*) project.ecu->topLevelItem(index);

    if(ecuitem)
    {
        disconnectECU(ecuitem);
    }
}

void MainWindow::connectAllEcuSignal()
{
    connectAll();
}

void MainWindow:: disconnectAllEcuSignal()
{
    disconnectAll();
}

void MainWindow::sendInjection(int index, QString applicationId, QString contextId, int serviceId,
                               QByteArray data) {
    EcuItem* ecuitem = (EcuItem*)project.ecu->topLevelItem(index);

    injectionAplicationId = applicationId;
    injectionContextId = contextId;

    if (ecuitem) {
        unsigned int serviceID = serviceId;

        if ((DLT_SERVICE_ID_CALLSW_CINJECTION <= serviceID) && (serviceID != 0)) {
            const std::vector<uint8_t> dataBytes(data.begin(), data.end());
            QDltMsgWrapper msgWrapper(serviceID, dataBytes);
            controlMessage_SendControlMessage(ecuitem, msgWrapper.getMessage(),
                                              injectionAplicationId, injectionContextId);
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
        dlg.updateHistory();

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
        dlg.setDataBinary(injectionDataBinary);

        if(dlg.exec())
        {
            injectionAplicationId = dlg.getApplicationId();
            injectionContextId = dlg.getContextId();
            injectionServiceId = dlg.getServiceId();
            injectionData = dlg.getData();
            injectionDataBinary = dlg.getDataBinary();

            dlg.storeHistory();

            SendInjection(ecuitem);
        }
    }
    //else
    //    QMessageBox::warning(0, QString("DLT Viewer"),
    //                         QString("No ECU selected in configuration!"));
}

void MainWindow::controlMessage_SetApplication(EcuItem *ecuitem, QString apid, QString appdescription)
{
    if (auto appitem = ecuitem->find(apid); appitem) {
        appitem->description = appdescription;
        appitem->update();
    } else {
        appitem = new ApplicationItem(ecuitem);
        appitem->id = apid;
        appitem->description = appdescription;
        appitem->update();
        ecuitem->addChild(appitem);
    }
}

void MainWindow::controlMessage_SetContext(EcuItem *ecuitem, QString apid, QString ctid,QString ctdescription,int log_level,int trace_status)
{
    if (auto appitem = ecuitem->find(apid); appitem) {

        ContextItem *conitem = nullptr;
        for (int numcontext = 0; numcontext < appitem->childCount(); numcontext++) {
            ContextItem *currconitem = (ContextItem *)appitem->child(numcontext);
            if (currconitem->id == ctid) {
                conitem = currconitem;
            }
        }

        if (!conitem) {
            conitem = new ContextItem(appitem);
            appitem->addChild(conitem);
        }
        conitem->id = ctid;
        conitem->loglevel = log_level;
        conitem->tracestatus = trace_status;
        conitem->description = ctdescription;
        conitem->status = ContextItem::valid;
        conitem->update();
    } else {
        appitem = new ApplicationItem(ecuitem);
        appitem->id = apid;
        appitem->description = "";
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
}

void MainWindow::controlMessage_Timezone(int timezone, unsigned char dst)
{
    if(!project.settings->automaticTimeSettings && project.settings->automaticTimezoneFromDlt)
    {
        project.settings->utcOffset = timezone;
        project.settings->dst = dst;
    }
}

void MainWindow::controlMessage_UnregisterContext(QString ecuId,QString appId,QString ctId)
{
    if(!project.settings->updateContextsUnregister)
        return;

    /* find ecu item */
    EcuItem *ecuitemFound = 0;
    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
        if(ecuitem->id == ecuId)
        {
            ecuitemFound = ecuitem;
            break;
        }
    }

    if(!ecuitemFound)
        return;

    /* First try to find existing context */
    if(auto appitem = ecuitemFound->find(appId); appitem)
    {
        for(int numcontext = 0; numcontext < appitem->childCount(); numcontext++)
        {
            ContextItem * conitem = (ContextItem *) appitem->child(numcontext);
            if(conitem->id == ctId)
            {
                /* remove context */
                delete conitem->parent()->takeChild(conitem->parent()->indexOfChild(conitem));
                return;
            }
        }
    }
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
  text.append("-");
  text.append(PACKAGE_REVISION);
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
                             QString("Qt Version: %1\n").arg(QT_VERSION_STR)+
                         #if defined(Q_OS_WIN) && !defined(Q_CC_GNU)
                             QString("Compiler: msvc\n")+
                         #else
                             QString("Compiler: gcc\n")+
                         #endif
                         QString("Compiler Version: %1\n\n").arg(
                         #if defined(_MSC_VER)
                                 _MSC_VER
                         #else
                                 __VERSION__
                         #endif
                                 )+
                         QString("Architecture: %1 Bit\n").arg(QSysInfo::WordSize)+
                         #if (BYTE_ORDER==BIG_ENDIAN)
                             QString("Architecture: Big Endian\n\n")+
                         #else
                             QString("Architecture: Little Endian\n\n")+
                         #endif
                             QString("(C) 2016,2025 BMW AG\n"));
}

void MainWindow::on_action_menuHelp_Command_Line_triggered() {
    QMessageBox::information(
                0, "DLT Viewer - Command line usage\t\t\t\t\t", // tabs used to expand message box !
                QDltOptManager::getInstance()->getHelpText());
}

void MainWindow::on_actionShortcuts_List_triggered(){

    QDialog *shortcutDialog = new QDialog(this);
    shortcutDialog->setWindowTitle("Shortcuts List");
    shortcutDialog->resize(600, 400);

    QTableView *table = new QTableView(shortcutDialog);
    table->setObjectName("Shortcuts Summarise Table");
    QStandardItemModel *model = new QStandardItemModel(0, 2, shortcutDialog);

    QFont headerFont;
    headerFont.setBold(true);

    QStandardItem *headerName = new QStandardItem("Action Name");
    headerName->setFont(headerFont);
    model->setHorizontalHeaderItem(0, headerName);

    QStandardItem *headerFeature = new QStandardItem("Shortcuts");
    headerFeature->setFont(headerFont);
    model->setHorizontalHeaderItem(1, headerFeature);

    // Define shortcut variables
    const QString shortcutNew = "Ctrl + N";
    const QString shortcutOpen = "Ctrl + O";
    const QString shortcutSave = "Ctrl + S";
    const QString shortcutClear = "Ctrl + E";
    const QString shortcutimportDLT = "Ctrl + I";
    const QString shortcutimportSerialHeader = "Ctrl + J";
    const QString shortcutFind = "Ctrl + F";
    const QString shortcutJumpTo = "Ctrl + G";
    const QString shortcutNewProject = "Ctrl + Shift + G";
    const QString shortcutOpenProject = "Ctrl + Shift + O";
    const QString shortcutSaveProject = "Ctrl + Shift + S";
    const QString shortcutExpandAllECU = "Ctrl++";
    const QString shortcutCollapseAllECU = "Ctrl+";
    const QString shortcutCopyPayload = "Ctrl + P";
    const QString shortcutInfo = "F1";
    const QString shortcutQuit = "Ctrl +- Q";

    // Store shortcuts dynamically using a list of pairs
    QList<QPair<QString, QString>> shortcutsList = {

        {"New", shortcutNew},
        {"Open", shortcutOpen},
        {"Save As", shortcutSave},
        {"Clear", shortcutClear},
        {"Import DLT Stream", shortcutimportDLT},
        {"Import DLT Stream with serial header", shortcutimportSerialHeader},
        {"Find", shortcutFind},
        {"Jump To", shortcutJumpTo},
        {"New Project", shortcutNewProject},
        {"Open Project", shortcutOpenProject},
        {"Save Project", shortcutSaveProject},
        {"Expand All ECU", shortcutExpandAllECU},
        {"Collapse All ECU", shortcutCollapseAllECU},
        {"Copy Payload", shortcutCopyPayload},
        {"Info", shortcutInfo},
        {"Quit", shortcutQuit},
    };

    for (int i = 0; i < shortcutsList.size(); ++i) {
        model->insertRow(i);
        model->setData(model->index(i, 0), shortcutsList[i].first);
        model->setData(model->index(i, 1), shortcutsList[i].second);

        // Make the items non-editable
        for (int j = 0; j < 2; ++j) {
            QStandardItem *item = model->item(i, j);
            if (item) {
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            }
        }
    }

    // Center-align
    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            QModelIndex index = model->index(row, col);
            model->setData(index, Qt::AlignCenter, Qt::TextAlignmentRole);
        }
    }

    table->setModel(model);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QVBoxLayout *layout = new QVBoxLayout(shortcutDialog);
    layout->addWidget(table);

    shortcutDialog->setLayout(layout);
    shortcutDialog->exec();
    delete shortcutDialog;
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

    ui->action_menuFilter_Delete->setEnabled(project.filter->selectedItems().count() >= 1);
    ui->action_menuFilter_Edit->setEnabled(project.filter->selectedItems().count()==1);
    ui->action_menuFilter_Duplicate->setEnabled(project.filter->selectedItems().count()==1);
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
    ui->action_menuDLT_Get_Local_Time_2->setEnabled(ecuitem && ecuitem->connected && !appitem);
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

void MainWindow::onPluginWidgetPluginPriorityChanged(const QString name, int prio)
{
    pluginManager.setPluginPriority(name, prio);
}

void MainWindow::updateScrollButton()
{
    // Mapping: variable to button
    scrollButton->setChecked(settings->autoScroll);

    // inform plugins about changed autoscroll status
    pluginManager.autoscrollStateChanged(settings->autoScroll);
}


void MainWindow::updateRecentFileActions()
{
    int numRecentFiles = qMin(recentFiles.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i)
    {
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
    QDltSettingsManager::getInstance()->setValue("other/recentFileList",recentFiles);
}

void MainWindow::removeCurrentFile(const QString &fileName)
{
    recentFiles.removeAll(fileName);
    updateRecentFileActions();

    // write settings
    QDltSettingsManager::getInstance()->setValue("other/recentFileList",recentFiles);
}

void MainWindow::openRecentProject()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QString projectName;

    if (action)
    {
        projectName = action->data().toString();

        /* Open existing project */
        if(!projectName.isEmpty() && openDlpFile(projectName))
        {
           //thats it.
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
    QDltSettingsManager::getInstance()->setValue("other/recentProjectList",recentProjects);
}

void MainWindow::removeCurrentProject(const QString &projectName)
{
    recentProjects.removeAll(projectName);
    updateRecentProjectActions();

    // write settings
    QDltSettingsManager::getInstance()->setValue("other/recentProjectList",recentProjects);
}


void MainWindow::openRecentFilters()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QString fileName;

    if (action)
    {
        fileName = action->data().toString();

        openDlfFile(fileName,true);
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
    QDltSettingsManager::getInstance()->setValue("other/recentFiltersList",recentFilters);
}

void MainWindow::removeCurrentFilters(const QString &filtersName)
{
    recentFilters.removeAll(filtersName);
    updateRecentFiltersActions();

    // write settings
    QDltSettingsManager::getInstance()->setValue("other/recentFiltersList",filtersName);
}

void MainWindow::setMcast(bool b_mcast)
{
    QDltSettingsManager::getInstance()->setValue("other/multicast",b_mcast);
}


void MainWindow::setInterfaceTypeSelection(int selectindex)
{
    QDltSettingsManager::getInstance()->setValue("other/iftypeindex",selectindex);
}

void MainWindow::setCurrentHostname(const QString &hostName)
{
    recentHostnames.removeAll(hostName);
    recentHostnames.prepend(hostName);
    while (recentHostnames.size() > MaxRecentHostnames)
        recentHostnames.removeLast();

    /* Write settings for recent hostnames*/
    QDltSettingsManager::getInstance()->setValue("other/recentHostnameList",recentHostnames);
}


void MainWindow::setCurrentMCAddress(const QString &mcastaddress)
{
    recent_multicastAddresses.removeAll(mcastaddress);
    recent_multicastAddresses.prepend(mcastaddress);
    while (recent_multicastAddresses.size() > MaxRecentHostnames)
      {
        recent_multicastAddresses.removeLast();
      }
    QDltSettingsManager::getInstance()->setValue("other/recentHostMulticastAddresses",recent_multicastAddresses);
}


void MainWindow::setCurrentEthIF(const QString &EthIfName)
{
    QDltSettingsManager::getInstance()->setValue("other/recentEthernetInterface",EthIfName);
}


void MainWindow::setCurrentIPPort(const QString &portName)
{
    recentIPPorts.removeAll(portName);
    recentIPPorts.prepend(portName);
    while (recentIPPorts.size() > MaxRecentPorts)
        recentIPPorts.removeLast();

    /* Write settings for recent ports */
    QDltSettingsManager::getInstance()->setValue("other/recentIPPortList",recentIPPorts);
}

void MainWindow::setCurrentUDPPort(const QString &portName)
{
    recentUDPPorts.removeAll(portName);
    recentUDPPorts.prepend(portName);
    while (recentUDPPorts.size() > MaxRecentPorts)
        recentUDPPorts.removeLast();

    /* Write settings for recent ports */
    QDltSettingsManager::getInstance()->setValue("other/recentUDPPortList",recentUDPPorts);
}

void MainWindow::sendUpdates(EcuItem* ecuitem)
{
    /* update default log level, trace status and timing packets */
    if (true == ecuitem->sendGetSoftwareVersion)
    {
        controlMessage_GetSoftwareVersion(ecuitem);
    }
    if (ecuitem->sendDefaultLogLevel)
    {
        controlMessage_SetDefaultLogLevel(ecuitem,ecuitem->loglevel);
        controlMessage_SetDefaultTraceStatus(ecuitem,ecuitem->tracestatus);
        controlMessage_SetVerboseMode(ecuitem,ecuitem->verbosemode);
    }

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

            if(dsrChanged)
            {
                pluginManager.stateChanged(num,QDltConnection::QDltConnectionOnline,ecuitem->getHostname());
            }
            else
            {
                pluginManager.stateChanged(num,QDltConnection::QDltConnectionOffline,ecuitem->getHostname());
            }

        }
    }
}

void MainWindow::stateChangedIP(QAbstractSocket::SocketState socketState)
{
    /* signal emited when connection state changed */
    //qDebug() << "stateChangedIP" << socketState << __LINE__ << __FILE__;
    /* find socket which emited signal */
    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
        if( ecuitem && ecuitem->socket == sender())
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

            switch(socketState){
            case QAbstractSocket::UnconnectedState:
                pluginManager.stateChanged(num,QDltConnection::QDltConnectionOffline,ecuitem->getHostname());
                break;
            case QAbstractSocket::ConnectingState:
                pluginManager.stateChanged(num,QDltConnection::QDltConnectionConnecting,ecuitem->getHostname());
                break;
            case QAbstractSocket::ConnectedState:
                pluginManager.stateChanged(num,QDltConnection::QDltConnectionOnline,ecuitem->getHostname());
                break;
            case QAbstractSocket::ClosingState:
                pluginManager.stateChanged(num,QDltConnection::QDltConnectionOffline,ecuitem->getHostname());
                break;
            default:
                pluginManager.stateChanged(num,QDltConnection::QDltConnectionOffline,ecuitem->getHostname());
                break;
            }
        }
    }
}

//----------------------------------------------------------------------------
// Search functionalities
//----------------------------------------------------------------------------

void MainWindow::on_action_menuSearch_Find_triggered()
{
    if (searchDlg->needTimeRangeReset() && qfile.size() > 0) {
        QDltMsg firstMessage, lastMessage;
        const bool success =
                (qfile.getMsg(0, firstMessage) && qfile.getMsg(qfile.size() - 1, lastMessage));
        if (success) {
            qint64 firstTimestampMSecsSinceEpoch = firstMessage.getTime() * 1000 + firstMessage.getMicroseconds() / 1000;
            QDateTime firstTimestamp = QDateTime::fromMSecsSinceEpoch(firstTimestampMSecsSinceEpoch);

            qint64 lastTimestampMSecsSinceEpoch = lastMessage.getTime() * 1000 + lastMessage.getMicroseconds() / 1000;
            QDateTime lastTimestamp = QDateTime::fromMSecsSinceEpoch(lastTimestampMSecsSinceEpoch);

            searchDlg->setTimeRange(firstTimestamp, lastTimestamp);
        }
    }

    searchDlg->open();
    searchDlg->selectText();
}

//----------------------------------------------------------------------------
// Plugin functionalities
//----------------------------------------------------------------------------

void MainWindow::loadPlugins()
{
    /* load plugins from subdirectory plugins, from directory if set in settings and from /usr/share/dlt-viewer/plugins in Linux */
    QStringList errList;
    if(settings->pluginsPath)
    {
        errList = pluginManager.loadPlugins(settings->pluginsPathName);
    }
    else
    {
        errList = pluginManager.loadPlugins(QString());
    }

    if(errList.size() > 0)
    {
        // We have some error messages from the plugin manager
        QStringList::const_iterator iter;
        for(iter = errList.constBegin(); iter != errList.constEnd(); ++iter)
            QMessageBox::warning(0, QString("DLT Viewer"), (*iter).toLocal8Bit().constData());
    }

    // Initialize Plugin Prio
    pluginManager.initPluginPriority(settings->pluginExecutionPrio);

    // Update settings with current priorities (maybe some plugins are not available anymore)
    settings->pluginExecutionPrio = pluginManager.getPluginPriorities();
    //qDebug() << settings->pluginExecutionPrio;

    /* update plugin widgets */
    QList<QDltPlugin*> plugins = pluginManager.getPlugins();
    for (int idx = 0; idx < plugins.size();idx++ )
    {
      QDltPlugin* plugin = plugins[idx];

      plugin->initMainTableView( ui->tableView );

      PluginItem* item = new PluginItem(0,plugin);

      plugin->setMode((QDltPlugin::Mode) QDltSettingsManager::getInstance()->value("plugin/pluginmodefor"+plugin->name(),QVariant(QDltPlugin::ModeDisable)).toInt());
      qDebug() << "Load plugin" << plugin->name() << plugin->pluginVersion();
      if(plugin->isViewer())
      {
        item->widget = plugin->initViewer();
        item->dockWidget = new MyPluginDockWidget(item,this);
        item->dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
        item->dockWidget->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        item->dockWidget->setWidget(item->widget);
        item->dockWidget->setObjectName(plugin->name());

        addDockWidget(Qt::LeftDockWidgetArea, item->dockWidget);

        if(plugin->getMode() != QDltPlugin::ModeShow)
        {
            item->dockWidget->hide();
        }
      }

      item->update();
      project.plugin->addTopLevelItem(item);

    }

    /* initialise control interface */
    qcontrol.silentmode = QDltOptManager::getInstance()->issilentMode();
    qcontrol.commandlinemode = QDltOptManager::getInstance()->isCommandlineMode();
    pluginManager.initControl(&qcontrol);
}

void MainWindow::updatePluginsECUList()
{
    QStringList list;

    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);

        list.append(ecuitem->id + " (" + ecuitem->description + ")");
    }
    pluginManager.initConnections(list);
}

void MainWindow::updatePlugins() {
    for(int num = 0; num < project.plugin->topLevelItemCount (); num++) {
        PluginItem *item = (PluginItem*)project.plugin->topLevelItem(num);

        updatePlugin(item);
    }

}

/* is called when the status of a plugin was changed ( disabled/enabled/show )  */
void MainWindow::updatePlugin(PluginItem *item)
{
    item->takeChildren();
    bool ret = true;

    QString err_text = item->getPlugin()->error();
    QString conffilename = item->getFilename();

    if ( item->getMode() == QDltPlugin::ModeEnable ) // this is just for user information: what is going on ...
    {
        qDebug() << "Enable" << item->getPlugin()->name() << "with" << conffilename << "configuration file";
    }
    else if ( item->getMode() == QDltPlugin::ModeShow )
    {
        qDebug() << "Show" << item->getPlugin()->name() << "with" << conffilename << "configuration file";
    }
    else if ( item->getMode() == QDltPlugin::ModeDisable )
    {
        qDebug() << "Disable" << item->getPlugin()->name();
    }
    else
    {
        qDebug() << "Unknown mode" << item->getMode() << "in" << item->getPlugin()->name();
    }

    //We should not need error handling when disabling the plugins, so we only call loadConfig when enabling the plugin !
    if (item->getMode() != QDltPlugin::ModeDisable) // that means any kind of enabling
    {
        // in case there is an explicitely given file or directpory name we want to use this of course
     //qDebug() << "Versionstring" << target_version_string << target_version_string.isEmpty();
     if(settings->pluginsAutoloadPath != 0 && ( conffilename.isEmpty() == true ) && ( target_version_string.isEmpty() == false ) && ( pluginsEnabled == true )  )
         {
            qDebug() << "Trigger autoload with version" << target_version_string;
            pluginsAutoload(target_version_string);
         }
     else // we eventually provoke an error message, but sometimes even a decoder plugin works with default settings
       {
       ret = item->getPlugin()->loadConfig(conffilename);
       }
        //qDebug() << "Enable or show" << item->getPlugin()->getName() << __LINE__ << __FILE__;
        if ( false == ret )
        {
            if ( true == QDltOptManager::getInstance()->issilentMode() )
             {
              QString err_header = "Plugin ";
              err_header.append(item->getName());
              QString err_body = err_header;
              err_body.append(" returned error: ");
              err_body.append(item->getPlugin()->error());
              err_body.append(" in loadConfig!");
              ErrorMessage(QMessageBox::Critical,err_header,err_body);
             }
             else
             {
              QString err_header = "Plugin Error: ";
              err_header.append(item->getName());
              QString err_body = err_header;
              err_body.append(" returned error:\n");
              err_body.append(item->getPlugin()->error());
              err_body.append("\nin loadConfig!");
              ErrorMessage(QMessageBox::Critical,err_header,err_body);
             }
        }
        else if ( 0 < err_text.length() )
        {
            //we have no error, but the plugin complains about something
            QString err_header = "Plugin Warning: ";
            err_header.append(item->getName());
            QString err_body = err_header;
            err_body.append(" returned message:\n");
            err_body.append(err_text);
            err_body.append("\nin loadConfig. ");
            ErrorMessage(QMessageBox::Warning,err_header,err_body);
        }
    }


    QStringList list = item->getPlugin()->infoConfig();
    for(int num=0;num<list.size();num++)
    {
        item->addChild(new QTreeWidgetItem(QStringList(list.at(num)),1000));
    }

    item->update(); //update the table view in plugin tab

    if(item->dockWidget)
    {
        if(item->getMode() == QDltPlugin::ModeShow)
        {
            item->dockWidget->show();
        }
        else
        {
            item->dockWidget->hide();
        }
    }
}

void MainWindow::versionString(const QDltMsg &msg)
{
    // get the version string from the version message
    // Skip the ServiceID, Status and Length bytes and start from the String containing the ECU Software Version
    QByteArray payload = msg.getPayload();
    QByteArray data = payload.mid(9,(payload.size()>262)?256:(payload.size()-9));

    target_version_string = QDlt::toAscii(data,true);
    target_version_string = target_version_string.trimmed(); // remove all white spaces at beginning and end

    //qDebug() << "Versionstring"<< target_version_string << __LINE__ ;

    autoloadPluginsVersionStrings.append(target_version_string);
    QFontMetrics fm = QFontMetrics(statusFileVersion->font());
    QString versionString = "Version: " + autoloadPluginsVersionStrings.join("\r\n");
    statusFileVersion->setText(fm.elidedText(versionString.simplified(), Qt::ElideRight, statusFileVersion->width()));
    statusFileVersion->setToolTip(versionString);

    if((settings->pluginsAutoloadPath) != 0 && ( pluginsEnabled == true ))
    {
        pluginsAutoload(target_version_string);
    }
}


void MainWindow::triggerPluginsAutoload()
{
  if((settings->pluginsAutoloadPath) != 0 && ( pluginsEnabled == true ))
  {
   pluginsAutoload(target_version_string);
  }
}

void MainWindow::pluginsAutoload(QString version)
{
    // Iterate through all enabled decoder plugins
    for(int num = 0; num < project.plugin->topLevelItemCount(); num++)
    {
        PluginItem *item = (PluginItem*)project.plugin->topLevelItem(num);

        if( item->getMode() != QDltPlugin::ModeDisable && item->getPlugin()->isDecoder() )
        {
            QString searchPath = settings->pluginsAutoloadPathName+ "/" + item->getName();

            qDebug() << "AutoloadPlugins search path for" << item->getName()  << "is" << searchPath;

            // search for files in plugin directory which contains version string
            QStringList nameFilter("*"+version+"*");
            QDir directory(searchPath);
            QStringList txtFilesAndDirectories = directory.entryList(nameFilter);

            if(false == txtFilesAndDirectories.isEmpty() )
             {
                if(txtFilesAndDirectories.size()>1)
                    txtFilesAndDirectories.sort(); // sort if several files are found

                // file with version string found
                QString filename = searchPath + "/" + txtFilesAndDirectories[0];
                // check if filename already loaded
                if(item->getFilename()!=filename)
                 {
                    // load new configuration
                    item->setFilename(filename);
                    item->getPlugin()->loadConfig(filename);
                    item->update();
                 }
                else
                 {
                    qDebug() << "AutoloadPlugins already loaded:" << filename;
                 }
             }
            else
             {
              qDebug() << "No content found for pattern " << version << "in" << searchPath;
             }
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
        if(!item->getPlugin()->isViewer())
            dlg.removeMode(2); // remove show mode, if no viewer plugin
        dlg.setType(item->getType());
        if(dlg.exec()) {
            /* Check if there was a change that requires a refresh */
            if(item->getMode() != dlg.getMode())
                callInitFile = true;
            if(item->getMode() == QDltPlugin::ModeShow && dlg.getMode() != QDltPlugin::ModeDisable)
                callInitFile = false;
            if(dlg.getMode() == QDltPlugin::ModeShow && item->getMode() != QDltPlugin::ModeDisable)
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
            applyConfigEnabled(true);
        }
    }
    else
    {
        ErrorMessage(QMessageBox::Warning,QString("DLT Viewer"),QString("No Plugin selected!"));
    }

}

void MainWindow::on_action_menuPlugin_Show_triggered() {

    /* get selected plugin */
    QList<QTreeWidgetItem *> list = project.plugin->selectedItems();
    if((list.count() == 1) ) {
        PluginItem* item = (PluginItem*) list.at(0);

        if(item->getMode() != QDltPlugin::ModeShow){
            int oldMode = item->getMode();

            item->setMode( QDltPlugin::ModeShow );
            item->savePluginModeToSettings();
            updatePlugin(item);

            if(oldMode == QDltPlugin::ModeDisable){
                applyConfigEnabled(true);
            }
        }else{
             ErrorMessage(QMessageBox::Warning,QString("DLT Viewer"),QString("The selected Plugin is already active."));
        }
    }
    else {
        ErrorMessage(QMessageBox::Warning,QString("DLT Viewer"),QString("No Plugin selected!"));
    }

}

void MainWindow::on_action_menuPlugin_Hide_triggered() {
    /* get selected plugin */
    QList<QTreeWidgetItem *> list = project.plugin->selectedItems();
    if((list.count() == 1) ) {
        PluginItem* item = (PluginItem*) list.at(0);

        if(item->getMode() == QDltPlugin::ModeShow){
            item->setMode( QDltPlugin::ModeEnable );
            item->savePluginModeToSettings();
            updatePlugin(item);
        }else{
            ErrorMessage(QMessageBox::Warning,QString("DLT Viewer"),QString("No Plugin selected!"));

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

        if(item->getMode() == QDltPlugin::ModeDisable){
            item->setMode( QDltPlugin::ModeEnable );
            item->savePluginModeToSettings();
            updatePlugin(item);
            applyConfigEnabled(true);
        }else{
            QMessageBox::warning(0, QString("DLT Viewer"),
                                 QString("The selected Plugin is already deactivated."));
        }
    }
    else
    {
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No Plugin selected!"));
    }

    if(pluginsEnabled == true){
       QList<QDltPlugin*> activeViewerPlugins;
       activeViewerPlugins = pluginManager.getViewerPlugins();
       for(int i = 0; i < activeViewerPlugins.size(); i++)
       {
          QDltPlugin *item = (QDltPlugin*)activeViewerPlugins.at(i);
          item->initFileStart(&qfile);
       }
    }
}

void MainWindow::on_action_menuPlugin_Disable_triggered()
{
    /* get selected plugin */
    QList<QTreeWidgetItem *> list = project.plugin->selectedItems();
    if((list.count() == 1) ) {
        PluginItem* item = (PluginItem*) list.at(0);

        if(item->getMode() != QDltPlugin::ModeDisable){
            item->setMode( QDltPlugin::ModeDisable );
            item->savePluginModeToSettings();
            updatePlugin(item);
            applyConfigEnabled(true);
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

void MainWindow::filterIndexStart()
{
    QModelIndexList list = ui->tableView->selectionModel()->selection().indexes();

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

    quint64 pos = qfile.getMsgFilterPos(index.row());
    ui->lineEditFilterStart->setText(QString("%1").arg(pos));
}

void MainWindow::filterIndexEnd()
{
    QModelIndexList list = ui->tableView->selectionModel()->selection().indexes();

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

    quint64 pos = qfile.getMsgFilterPos(index.row());
    ui->lineEditFilterEnd->setText(QString("%1").arg(pos));
}

//Groups DLT logs by ECU ID and displays progress to the user.
void MainWindow::splitLogsEcuid()
{
    QAbstractTableModel* sourceModel = qobject_cast<QAbstractTableModel*>(ui->tableView->model());
    int rowCount = sourceModel->rowCount();
    if (qfile.getNumberOfFiles() > 0) {
        filtergrouplogs *filterLogsEcuid = new filtergrouplogs(this);
        // Get the path of the currently loaded DLT file
        QString currentFilePath = qfile.getFileName(0);
        QStringList ecuIds = filterLogsEcuid->extractEcuIds(currentFilePath);
        if (ecuIds.isEmpty()) {
            QMessageBox::information(this, "No DLT file found", "No DLT file is opened... Open a DLT File.");
            delete filterLogsEcuid;
            return;
        }

        /* Progress dialog */
        QProgressDialog progress("Grouping DLT Logs by ECU ID...", "Cancel", 0, rowCount, this);
        progress.setWindowModality(Qt::ApplicationModal);
        progress.setMinimumDuration(0);
        progress.setValue(0);
        progress.setWindowTitle("Grouping Progress");
        progress.show();
        for (int i = 0; i < rowCount; ++i) {
            progress.setValue(i + 1);
            QCoreApplication::processEvents();
            if (progress.wasCanceled()) {
                delete filterLogsEcuid;
                return;
            }
        }

        // Set up all necessary references
        filterLogsEcuid->setSourceModel(sourceModel);
        filterLogsEcuid->setDltFile(&qfile);
        filterLogsEcuid->setPluginManager(&pluginManager);

        filterLogsEcuid->ecuIdTabs();
    } else {
        QMessageBox::warning(this, "Warning", "No DLT file is currently loaded.");
        return;
    }
}

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
    msg.setMsg(data,true,settings->supportDLTv2Decoding);
    msg.setIndex(qfile.getMsgFilterPos(index.row()));

    /* decode message if necessary */
    iterateDecodersForMsg(msg,!QDltOptManager::getInstance()->issilentMode());

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
    dlg.setMessageId_min(msg.getMessageId());
    dlg.setMessageId_max(0);
    if(dlg.exec()==1) {
        FilterItem* item = new FilterItem(0);
        project.filter->addTopLevelItem(item);
        filterDialogRead(dlg,item);
        filterIsChanged = true;
    }
}

void MainWindow::filterAdd()
{
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
        filterIsChanged = true;
    }
}

void MainWindow::on_action_menuFilter_Save_As_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save DLT Filters"), workingDirectory.getDlfDirectory(), tr("DLT Filter File (*.dlf);;All files (*.*)"));

    if(fileName.isEmpty())
        return;

    const QFileInfo fileInfo(fileName);

    workingDirectory.setDlfDirectory(fileInfo.absolutePath());

    if (const auto extension = fileInfo.suffix(); extension.isEmpty()) {
        fileName.append(".dlf");
    }

    if(project.SaveFilter(fileName)) {
        setCurrentFilters(fileName);
    } else {
        QMessageBox::critical(0, "DLT Viewer", "Save DLT Filter file failed!");
    }
    filterIsChanged = false;
}


void MainWindow::on_action_menuFilter_Load_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Load DLT Filter file"), workingDirectory.getDlfDirectory(), tr("DLT Filter Files (*.dlf);;All files (*.*)"));

    if(!fileName.isEmpty())
    {
        openDlfFile(fileName,true);
        filterIsChanged = false;
    }
}

void MainWindow::on_action_menuFilter_Add_triggered() {
    /* show filter dialog */
    FilterDialog dlg;

    if(dlg.exec()==1) {
        FilterItem* item = new FilterItem(0);
        project.filter->addTopLevelItem(item);
        filterDialogRead(dlg,item);
        filterIsChanged = true;
    }
}

void MainWindow::filterDialogWrite(FilterDialog &dlg,FilterItem* item)
{
    dlg.setType((int)(item->filter.type));

    dlg.setName(item->filter.name);
    dlg.setEcuId(item->filter.ecuid);
    dlg.setApplicationId(item->filter.apid);
    dlg.setContextId(item->filter.ctid);
    dlg.setHeaderText(item->filter.header);
    dlg.setPayloadText(item->filter.payload);
    dlg.setRegexSearchText(item->filter.regex_search);
    dlg.setRegexReplaceText(item->filter.regex_replace);

    /* Important to set the log level min and max before setting the */
    /* corresponding checkboxes enableLogLevelXXX. */
    /* If not checkboxes will be set, when values unequal zero by functions */
    /* on_comboBoxLogLevelXXX_currentIndexChanged. */
    dlg.setLogLevelMax(item->filter.logLevelMax);
    dlg.setLogLevelMin(item->filter.logLevelMin);

    dlg.setEnableRegexp_Appid(item->filter.enableRegexp_Appid);
    dlg.setEnableRegexp_Context(item->filter.enableRegexp_Context);
    dlg.setEnableRegexp_Header (item->filter.enableRegexp_Header);
    dlg.setEnableRegexp_Payload(item->filter.enableRegexp_Payload);
    dlg.setIgnoreCase_Header(item->filter.ignoreCase_Header);
    dlg.setIgnoreCase_Payload(item->filter.ignoreCase_Payload);
    dlg.setActive(item->filter.enableFilter);
    dlg.setEnableEcuId(item->filter.enableEcuid);
    dlg.setEnableApplicationId(item->filter.enableApid);
    dlg.setEnableContextId(item->filter.enableCtid);
    dlg.setEnableHeaderText(item->filter.enableHeader);
    dlg.setEnablePayloadText(item->filter.enablePayload);
    dlg.setEnableCtrlMsgs(item->filter.enableCtrlMsgs);
    dlg.setEnableLogLevelMax(item->filter.enableLogLevelMax);
    dlg.setEnableLogLevelMin(item->filter.enableLogLevelMin);
    dlg.setEnableMarker(item->filter.enableMarker);
    dlg.setEnableMessageId(item->filter.enableMessageId);
    dlg.setEnableRegexSearchReplace(item->filter.enableRegexSearchReplace);

    dlg.setFilterColour(item->filter.filterColour);

    dlg.setMessageId_max(item->filter.messageIdMax);
    dlg.setMessageId_min(item->filter.messageIdMin);

}

void MainWindow::filterDialogRead(FilterDialog &dlg,FilterItem* item)
{
    item->filter.type = (QDltFilter::FilterType)(dlg.getType());

    item->filter.name = dlg.getName();

    item->filter.ecuid = dlg.getEcuId();
    item->filter.apid = dlg.getApplicationId();
    item->filter.ctid = dlg.getContextId();
    item->filter.header = dlg.getHeaderText();
    item->filter.payload = dlg.getPayloadText();
    item->filter.regex_search = dlg.getRegexSearchText();
    item->filter.regex_replace = dlg.getRegexReplaceText();

    item->filter.enableRegexp_Appid = dlg.getEnableRegexp_Appid();
    item->filter.enableRegexp_Context = dlg.getEnableRegexp_Context();
    item->filter.enableRegexp_Header = dlg.getEnableRegexp_Header();
    item->filter.enableRegexp_Payload = dlg.getEnableRegexp_Payload();
    item->filter.ignoreCase_Header = dlg.getIgnoreCase_Header();
    item->filter.ignoreCase_Payload = dlg.getIgnoreCase_Payload();
    item->filter.enableFilter = dlg.getEnableActive();
    item->filter.enableEcuid = dlg.getEnableEcuId();
    item->filter.enableApid = dlg.getEnableApplicationId();
    item->filter.enableCtid = dlg.getEnableContextId();
    item->filter.enableHeader = dlg.getEnableHeaderText();
    item->filter.enablePayload = dlg.getEnablePayloadText();
    item->filter.enableCtrlMsgs = dlg.getEnableCtrlMsgs();
    item->filter.enableLogLevelMax = dlg.getEnableLogLevelMax();
    item->filter.enableLogLevelMin = dlg.getEnableLogLevelMin();
    item->filter.enableMarker = dlg.getEnableMarker();
    item->filter.enableMessageId = dlg.getEnableMessageId();
    item->filter.enableRegexSearchReplace = dlg.getEnableRegexSearchReplace();

    item->filter.filterColour = dlg.getFilterColour();
    item->filter.logLevelMax = dlg.getLogLevelMax();
    item->filter.logLevelMin = dlg.getLogLevelMin();
    item->filter.messageIdMax=dlg.getMessageId_max();
    item->filter.messageIdMin=dlg.getMessageId_min();

    /* update filter item */
    item->update();
    on_filterWidget_itemSelectionChanged();

    /* Update filters in qfile and either update
     * view or pulse the button depending on if it is a filter or
     * marker. */
    filterUpdate();
    if(item->filter.isPositive() || item->filter.isNegative())
    {
        applyConfigEnabled(true);
    }
    if(item->filter.isMarker())
    {
        tableModel->modelChanged();
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
            filterIsChanged = true;
        }
    }
    else {
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No Filter selected!"));
    }
}

void MainWindow::on_action_menuFilter_Edit_triggered()
{
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
            filterIsChanged = true;
        }
    }
    else {
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No Filter selected!"));
    }
}

void MainWindow::on_action_menuFilter_Delete_triggered()
{
    /* skip delete action if filter list is not visible */
    if(!ui->tabPFilter->isVisible()) {
        return;
    }

    FilterTreeWidget* filterWidget = static_cast<FilterTreeWidget*>(project.filter);
    filterWidget->deleteSelected();
    filterIsChanged = true;
}

void MainWindow::onactionmenuFilter_SetAllActiveTriggered()
{
    QTreeWidget *widget;

    /* get currently visible filter list in user interface */
    if(ui->tabPFilter->isVisible()) {
        widget = project.filter;
    }
    else
        return;

    if(widget->selectedItems().size())
    {
        for(int i = 0; i < widget->selectedItems().size(); i++)
        {
            FilterItem *tmp = (FilterItem*)widget->selectedItems().at(i);
            tmp->filter.enableFilter = true;
            tmp->setCheckState(0, Qt::Checked);
        }
    }
    else
    {
        for(int i = 0; i < widget->topLevelItemCount(); i++)
        {
            FilterItem *tmp = (FilterItem*)widget->topLevelItem(i);
            tmp->filter.enableFilter = true;
            tmp->setCheckState(0, Qt::Checked);
        }
    }

    applyConfigEnabled(true);

    on_filterWidget_itemSelectionChanged();
}

void MainWindow::onactionmenuFilter_SetAllInactiveTriggered()
{
    QTreeWidget *widget;

    /* get currently visible filter list in user interface */
    if(ui->tabPFilter->isVisible()) {
        widget = project.filter;
    }
    else
        return;

    if(widget->selectedItems().size())
    {
        for(int i = 0; i < widget->selectedItems().size(); i++)
        {
            FilterItem *tmp = (FilterItem*)widget->selectedItems().at(i);
            tmp->filter.enableFilter = false;
            tmp->setCheckState(0, Qt::Unchecked);
        }
    }
    else
    {
        for(int i = 0; i < widget->topLevelItemCount(); i++)
        {
            FilterItem *tmp = (FilterItem*)widget->topLevelItem(i);
            tmp->filter.enableFilter = false;
            tmp->setCheckState(0, Qt::Unchecked);
        }
    }

    applyConfigEnabled(true);

    on_filterWidget_itemSelectionChanged();
}

void MainWindow::on_action_menuFilter_Clear_all_triggered()
{
    /* delete complete filter list */
    project.filter->clear();
    applyConfigEnabled(true);
    filterIsChanged = false;
}

void MainWindow::filterUpdate()
{
    QDltFilter *filter;

    /* update all filters from filter configuration to DLT filter list */

    /* clear old filter list */
    qfile.clearFilter();

    /* iterate through all filters */
    for(int num = 0; num < project.filter->topLevelItemCount (); num++)
    {
        FilterItem *item = (FilterItem*)project.filter->topLevelItem(num);

        filter = new QDltFilter();
        *filter = item->filter;

        if(item->filter.isMarker())
        {
            //item->setBackground(0,QColor(item->filter.filterColour));
            item->setBackground(1,QColor(item->filter.filterColour));
            //item->setForeground(0,QColor(0xff,0xff,0xff));
            item->setForeground(1,DltUiUtils::optimalTextColor(QColor(item->filter.filterColour)));
        }
        else
        {
            //item->setBackground(0,QColor(0xff,0xff,0xff));
            item->setBackground(1,QColor(0xff,0xff,0xff));
            //item->setForeground(0,QColor(0xff,0xff,0xff));
            item->setForeground(1,DltUiUtils::optimalTextColor(QColor(0xff,0xff,0xff)));

            if (QDltSettingsManager::UI_Colour::UI_Dark == QDltSettingsManager::getInstance()->uiColour)
            {
                //item->setBackground(0,QColor(31,31,31));
                item->setBackground(1,QColor(31,31,31));
                //item->setForeground(0,QColor(0xff,0xff,0xff));
                item->setForeground(1,DltUiUtils::optimalTextColor(QColor(31,31,31)));
            }
        }

        // resize active column automatically
        project.filter->resizeColumnToContents(0);

        if(filter->enableRegexp_Appid || filter->enableRegexp_Context || filter->enableRegexp_Header || filter->enableRegexp_Payload)
        {
            if(!filter->compileRegexps())
            {
                // This is also validated in the UI part
                qDebug() << "Error compiling a regexp\nin" << __FILE__ << __LINE__;
            }
        }

        qfile.addFilter(filter);
    }
    qfile.updateSortedFilter();
}

void MainWindow::on_tableView_customContextMenuRequested(QPoint pos)
{
    /* show custom pop menu  for configuration */
    QPoint globalPos = ui->tableView->mapToGlobal(pos);
    QMenu menu(ui->tableView);
    QAction *action;

    action = new QAction("&Copy Selection to Clipboard", &menu);
    connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuConfig_Copy_to_clipboard_triggered()));
    menu.addAction(action);

    action = new QAction("C&opy Selection Payload to Clipboard", &menu);
    action->setShortcut(QKeySequence("Ctrl+P"));
    connect(action, SIGNAL(triggered()), this, SLOT(onActionMenuConfigCopyPayloadToClipboardTriggered()));
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("Copy Selection for &Jira to Clipboard", &menu);
    connect(action, SIGNAL(triggered()), this, SLOT(onActionMenuConfigCopyJiraToClipboardTriggered()));
    menu.addAction(action);

    action = new QAction("Copy Selection for J&ira (+Head) to Clipboard", &menu);
    connect(action, SIGNAL(triggered()), this, SLOT(onActionMenuConfigCopyJiraHeadToClipboardTriggered()));
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("&Export...", &menu);
    if(qfile.sizeFilter() <= 0)
    {
        action->setEnabled(false);
    }
    else
    {
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionExport_triggered()));
    }
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("&Filter Add", &menu);
    connect(action, SIGNAL(triggered()), this, SLOT(filterAddTable()));
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("Load Filter(s)...", &menu);
    connect(action, SIGNAL(triggered()), this, SLOT(on_action_menuFilter_Load_triggered()));
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("Resize columns to fit", &menu);
    connect(action, SIGNAL(triggered()), ui->tableView, SLOT(resizeColumnsToContents()));
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("Mark/Unmark line(s)", &menu);
    action->setShortcut(QKeySequence("Ctrl+M"));
    connect(action, SIGNAL(triggered()), this, SLOT(mark_unmark_lines()));
    menu.addAction(action);

    action = new QAction("Unmark all lines", &menu);
    connect(action, SIGNAL(triggered()), this, SLOT(unmark_all_lines()));
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("Filter Index Start", &menu);
    connect(action, SIGNAL(triggered()), this, SLOT(filterIndexStart()));
    menu.addAction(action);

    action = new QAction("Filter Index End", &menu);
    connect(action, SIGNAL(triggered()), this, SLOT(filterIndexEnd()));
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("Group DLT logs by ECU ID", &menu);
    connect(action, SIGNAL(triggered()), this, SLOT(splitLogsEcuid()));
    menu.addAction(action);

    /* show popup menu */
    menu.exec(globalPos);
}



void MainWindow::on_tableView_SearchIndex_customContextMenuRequested(QPoint pos)
{
    /* show custom pop menu  for search table */
    QPoint globalPos = ui->tableView_SearchIndex->mapToGlobal(pos);
    QMenu menu(ui->tableView_SearchIndex);
    QAction *action;

    action = new QAction("&Copy Selection to Clipboard", this);
    connect(action, &QAction::triggered, this, &MainWindow::onActionMenuConfigSearchTableCopyToClipboardTriggered);
    menu.addAction(action);

    action = new QAction("C&opy Selection Payload to Clipboard", this);
    connect(action, &QAction::triggered, this, &MainWindow::onActionMenuConfigSearchTableCopyPayloadToClipboardTriggered);
    menu.addAction(action);

    menu.addSeparator();
    action = new QAction("Copy Selection for &Jira to Clipboard", this);
    connect(action, &QAction::triggered, this, &MainWindow::onActionMenuConfigSearchTableCopyJiraToClipboardTriggered);
    menu.addAction(action);

    action = new QAction("Copy Selection for J&ira (+Head) to Clipboard", this);
    connect(action, &QAction::triggered, this, &MainWindow::onActionMenuConfigSearchTableCopyJiraHeadToClipboardTriggered);
    menu.addAction(action);

    menu.addSeparator();
    action = new QAction("Resize columns to fit", this);
    connect(action, SIGNAL(triggered()), ui->tableView_SearchIndex, SLOT(resizeColumnsToContents()));
    menu.addAction(action);

    menu.addSeparator();
    action = new QAction("Export all rows in DLT Format...", this);
    connect(action, &QAction::triggered, this, &MainWindow::onActionMenuConfigSearchTableExportDltTriggered);
    menu.addAction(action);

    menu.addSeparator();

    /* show popup menu */
    menu.exec(globalPos);
}

void MainWindow::onActionMenuConfigSearchTableCopyToClipboardTriggered()
{
    exportSelection_searchTable(QDltExporter::FormatClipboard);
}

void MainWindow::onActionMenuConfigSearchTableCopyPayloadToClipboardTriggered()
{
    exportSelection_searchTable(QDltExporter::FormatClipboardPayloadOnly);
}

void MainWindow::onActionMenuConfigSearchTableCopyJiraToClipboardTriggered()
{
    exportSelection_searchTable(QDltExporter::FormatClipboardJiraTable);
}

void MainWindow::onActionMenuConfigSearchTableCopyJiraHeadToClipboardTriggered()
{
    exportSelection_searchTable(QDltExporter::FormatClipboardJiraTableHead);
}

void MainWindow::onActionMenuConfigSearchTableExportDltTriggered()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export Search Results as DLT"),
        workingDirectory.getExportDirectory(),
        tr("DLT Files (*.dlt);;All files (*.*)")
    );
    if (fileName.isEmpty())
        return;
    workingDirectory.setExportDirectory(QFileInfo(fileName).absolutePath());
    exportSelection_searchTable(QDltExporter::FormatDlt, fileName);
}

void MainWindow::keyPressEvent ( QKeyEvent * event )
{
    if(event->matches(QKeySequence::Copy))
    {
        if(ui->tableView->hasFocus())
        {
            exportSelection(true,false);
        }

        if(ui->tableView_SearchIndex->hasFocus())
        {
            exportSelection_searchTable();
        }
    }
    if(event->matches(QKeySequence::Paste))
    {
        QMessageBox::warning(this, QString("Paste"),
                             QString("pressed"));
    }
    if(event->matches(QKeySequence::Cut))
    {
        if(ui->tableView->hasFocus())
        {
            exportSelection(true,false);
        }

        if(ui->tableView_SearchIndex->hasFocus())
        {
            exportSelection_searchTable();
        }
    }

    // Access menu bar
    if(event->key() == Qt::Key_F10)
    {
        if(ui->menuBar->activeAction() != NULL)
        {
            ui->menuBar->setActiveAction(NULL);
            this->setFocus();
        }
        else
        {
            ui->menuBar->setFocus();
            if(ui->menuBar->children().length() > 1)
            {
                QMenu *firstMenu = (QMenu *) ui->menuBar->children()[1];

                if(firstMenu->actions().length() > 0)
                {
                    ui->menuBar->setActiveAction(firstMenu->actions()[0]);

                    QKeyEvent rightPress(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
                    qApp->sendEvent(ui->menuBar, &rightPress);
                }
            }
        }
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
    QStringList filenames;

    if (event->mimeData()->hasUrls())
    {
        QStringList importFilenames;
        for(int num = 0;num<event->mimeData()->urls().size();num++)
        {
            QUrl url = event->mimeData()->urls()[num];
            filename = url.toLocalFile();

            if(filename.endsWith(".dlt", Qt::CaseInsensitive))
            {
                filenames.append(filename);
                workingDirectory.setDltDirectory(QFileInfo(filename).absolutePath());
            }
            else if(filename.endsWith(".dlp", Qt::CaseInsensitive))
            {
                /* Project file dropped */
                openDlpFile(filename);
            }
            else if(filename.endsWith(".dlf", Qt::CaseInsensitive))
            {
                /* Filter file dropped */
                openDlfFile(filename,true);
            }
            else if(filename.endsWith(".pcap", Qt::CaseInsensitive))
                importFilenames.append(filename);
            else if(filename.endsWith(".mf4", Qt::CaseInsensitive))
                importFilenames.append(filename);
            else
            {
                /* ask for active decoder plugin to load configuration */
                QStringList items;
                QList<QDltPlugin*> list = pluginManager.getDecoderPlugins();
                for(int num=0;num<list.size();num++)
                {
                    items << list[num]->name();
                }

                /* check if decoder plugin list is empty */
                if(list.size()==0)
                {
                    /* show warning */
                    QMessageBox::warning(this, QString("Drag&Drop"),
                                         QString("No decoder plugin active to load configuration of file:\n")+filename);
                    return;
                }
                QString item="";
                bool ok;
                if(list.size()!=1)
                {

                item = QInputDialog::getItem(this, tr("DLT Viewer"),
                                                         tr("Select Plugin to load configuration:"), items, 0, false, &ok);
                }
                else
                {
                   item=items.at(0);
                   ok=true;
                }
                if (ok && !item.isEmpty())
                {
                    QDltPlugin* plugin = pluginManager.findPlugin(item);
                    if(plugin)
                    {
                        plugin->loadConfig(filename);
                        for(int num = 0; num < project.plugin->topLevelItemCount (); num++)
                        {
                            PluginItem *pluginitem = (PluginItem*)project.plugin->topLevelItem(num);
                            if(pluginitem->getPlugin() == plugin)
                            {
                                /* update plugin */
                                pluginitem->setFilename( filename );

                                /* update plugin item */
                                updatePlugin(pluginitem);
                                applyConfigEnabled(true);

                                ui->tabWidget->setCurrentWidget(ui->tabPlugin);

                                break;
                            }
                        }
                    }
                }
            }
        }
        if(!importFilenames.isEmpty())
        {
            on_action_menuFile_Clear_triggered();
            QDltImporter *importerThread = new QDltImporter(&outputfile,importFilenames);
            importerThread->setPcapPorts(settings->importerPcapPorts);
            connect(importerThread, &QDltImporter::progress,    this, &MainWindow::progress);
            connect(importerThread, &QDltImporter::resultReady, this, &MainWindow::handleImportResults);
            connect(importerThread, &QDltImporter::finished,    importerThread, &QObject::deleteLater);
            statusProgressBar->show();
            importerThread->start();
        }
        if(!filenames.isEmpty())
        {
            /* DLT log file dropped */
            openDltFile(filenames);
            outputfileIsTemporary = false;
            outputfileIsFromCLI   = false;
        }
    }
    else
    {
        QMessageBox::warning(this, QString("Drag&Drop"),
                             QString("No dlt file or project file or other file dropped!\n")+filename);
    }
}

void MainWindow::sectionInTableDoubleClicked(int logicalIndex){
    ui->tableView->resizeColumnToContents(logicalIndex);
}

void MainWindow::on_pluginWidget_itemExpanded(QTreeWidgetItem* item)
{
    PluginItem *plugin = (PluginItem*)item;
    plugin->takeChildren();
    QStringList list = plugin->getPlugin()->infoConfig();
    for(int num=0;num<list.size();num++)
    {
        plugin->addChild(new QTreeWidgetItem(QStringList(list.at(num)),1000));
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
            tmp->filter.enableFilter = false;
        }
        else
        {
            tmp->filter.enableFilter = true;
        }
        applyConfigEnabled(true);
    }
}

void MainWindow::iterateDecodersForMsg(QDltMsg &msg, int triggeredByUser)
{
    if ( pluginsEnabled == true )
    {
    pluginManager.decodeMsg(msg,triggeredByUser);
    }
}

void MainWindow::on_action_menuConfig_Collapse_All_ECUs_triggered()
{
    ui->configWidget->collapseAll();
}


void MainWindow::onActionMenuConfigSaveAllECUsTriggered()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save DLT Filters"), workingDirectory.getDltDirectory(), tr("Save APID/CTID list (*.csv);;All files (*.*)"));
    QFile asciiFile(filename);
    asciiFile.open(QIODevice::WriteOnly);

    // go over ECU Items
    for(int num = 0; num < project.ecu->topLevelItemCount (); num++)
    {
        EcuItem *ecuitem = (EcuItem*)project.ecu->topLevelItem(num);
        if ( NULL == ecuitem)
            return;
        asciiFile.write(QString("ECU;%1\n").arg(ecuitem->id).toLatin1().constData());
        // go over APIDs
        for(int numapp = 0; numapp < ecuitem->childCount(); numapp++)
        {
            ApplicationItem * appitem = (ApplicationItem *) ecuitem->child(numapp);
            asciiFile.write(QString("%1;;%2\n").arg(appitem->id).arg(appitem->description).toLatin1().constData());
            // go over CTIDs
            for(int numcontext = 0; numcontext < appitem->childCount(); numcontext++)
            {
                ContextItem * conitem = (ContextItem *) appitem->child(numcontext);
                    /* set new log level and trace status */
                    asciiFile.write(QString(";%1;%2\n").arg(conitem->id).arg(conitem->description).toLatin1().constData());
                }
            }
        }
    asciiFile.close();
}




void MainWindow::on_action_menuConfig_Expand_All_ECUs_triggered()
{
    ui->configWidget->expandAll();
}

void MainWindow::on_action_menuConfig_Copy_to_clipboard_triggered()
{
    exportSelection(true,false);
}

void MainWindow::onActionMenuConfigCopyPayloadToClipboardTriggered()
{
    exportSelection(true,false,QDltExporter::FormatClipboardPayloadOnly);
}

void MainWindow::onActionMenuConfigCopyJiraToClipboardTriggered()
{
    exportSelection(true,false,QDltExporter::FormatClipboardJiraTable);
}

void MainWindow::onActionMenuConfigCopyJiraHeadToClipboardTriggered()
{
    exportSelection(true,false,QDltExporter::FormatClipboardJiraTableHead);
}

void MainWindow::on_action_menuFilter_Append_Filters_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Load DLT Filter file"), workingDirectory.getDlfDirectory(), tr("DLT Filter Files (*.dlf);;All files (*.*)"));

    openDlfFile(fileName,false);
    filterIsChanged = true;
}

int MainWindow::nearest_line(int line)
{

    if (line < 0 || line > qfile.size()-1){
        return -1;
    }

    // If filters are off, just go directly to the row
    // If filters are enabled and no search result are matched, jump to beginning
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
        int lastFound = 0;

        QVector<qint64> filterIndices = qfile.getIndexFilter();

        if(!filterIndices.isEmpty())
        {
            lastFound = filterIndices.indexOf(line);
            if(lastFound < 0)
            {
                QVector<qint64> sortedIndices = filterIndices;
                std::sort(sortedIndices.begin(), sortedIndices.end());

                int lastIndex = sortedIndices[0];

                for(auto index: sortedIndices)
                {
                    if(index > line)
                    {
                        break;
                    }
                    lastIndex = index;
                }
                lastFound = filterIndices.indexOf(lastIndex);
            }
        }
        row = lastFound;
    }
    return row;
}

void MainWindow::jumpToMsgSignal(int index)
{
    jump_to_line(index);
}

void MainWindow::markerSignal()
{
    controlMessage_Marker();
}

void MainWindow::reopenFileSignal()
{
    applyConfigEnabled(false);
    filterUpdate();

    // force always reparsing of log file
    reloadLogFile(false);
}

bool MainWindow::jump_to_line(int line)
{

    int row = nearest_line(line);
    int column = -1;
    if (0 > row)
        return false;

    ui->tableView->selectionModel()->clear();

    // maybe a more elegant way exists... anyway this works
    if(project.settings->showIndex == 1)
        column = FieldNames::Index;
    else if(project.settings->showTime == 1)
        column = FieldNames::Time;
    else if(project.settings->showTimestamp == 1)
        column = FieldNames::TimeStamp;
    else if(project.settings->showCount == 1)
        column = FieldNames::Counter;
    else if(project.settings->showEcuId == 1)
        column = FieldNames::EcuId;
    else if(project.settings->showApId == 1)
        column = FieldNames::AppId;
    else if(project.settings->showCtId == 1)
        column = FieldNames::ContextId;
    else if(project.settings->showSessionId == 1)
        column = FieldNames::SessionId;
    else if(project.settings->showArguments == 1)
        column = FieldNames::ArgCount;
    else if(project.settings->showPayload == 1)
        column = FieldNames::Payload;

    QModelIndex idx = tableModel->index(row, column, QModelIndex());
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


void MainWindow::on_actionToggle_FiltersEnabled_triggered(bool checked)
{
    ui->filtersEnabled->setChecked(checked);
    ui->applyConfig->setFocus(); // have to set different focus first, so that scrollTo() works
    on_applyConfig_clicked();
}

void MainWindow::on_actionToggle_SortByTimeEnabled_triggered(bool checked)
{
    ui->checkBoxSortByTime->setChecked(checked);
    ui->applyConfig->setFocus(); // have to set different focus first, so that scrollTo() works
    on_applyConfig_clicked();
}

void MainWindow::on_actionSort_By_Timestamp_triggered(bool checked)
{
    ui->checkBoxSortByTimestamp->setChecked(checked);
    ui->applyConfig->setFocus(); // have to set different focus first, so that scrollTo() works
    on_applyConfig_clicked();
}

void MainWindow::on_actionAutoScroll_triggered(bool checked)
{
    int autoScrollOld = settings->autoScroll;

    // Mapping: button to variable
    settings->autoScroll = (checked?Qt::Checked:Qt::Unchecked);

    if (autoScrollOld!=settings->autoScroll)
        settingsDlg->writeSettings(this);

    // inform plugins about changed autoscroll status
    pluginManager.autoscrollStateChanged(settings->autoScroll);
}

void MainWindow::on_actionConnectAll_triggered()
{
    connectAll();
}

void MainWindow::on_actionDisconnectAll_triggered()
{
    disconnectAll();
}

/* This one is called when plugins are enabled / disabled in  menu !*/
void MainWindow::on_actionToggle_PluginsEnabled_triggered(bool checked)
{
    pluginsEnabled = checked;
    ui->pluginsEnabled->setChecked(pluginsEnabled); // set checkbox in UI
    QDltSettingsManager::getInstance()->setValue("startup/pluginsEnabled", pluginsEnabled);
    dltIndexer->setPluginsEnabled(pluginsEnabled);
    ui->applyConfig->setFocus(); // have to set different focus first, so that scrollTo() works
    syncCheckBoxesAndMenu();
    applyConfigEnabled(true);
}

/* This one is called when the checkbox "Plugins Enabled" is checked/unchecked */
void MainWindow::on_pluginsEnabled_toggled(bool checked)
{
    pluginsEnabled = checked;
    QDltSettingsManager::getInstance()->setValue("startup/pluginsEnabled", pluginsEnabled); // set settings
    dltIndexer->setPluginsEnabled(pluginsEnabled); // inform indexer
    // now we should correlate the "plugin menu entry to disable / enable"
    syncCheckBoxesAndMenu();
    applyConfigEnabled(true);
}

void MainWindow::on_filtersEnabled_toggled(bool checked)
{
    //QDltSettingsManager::getInstance()->setValue("startup/filtersEnabled", checked);
    QDltSettingsManager::getInstance()->setValue("startup/filtersEnabled", checked);
    ui->checkBoxSortByTime->setEnabled(checked);
    ui->checkBoxSortByTimestamp->setEnabled(checked);
    ui->checkBoxFilterRange->setEnabled(checked);
    ui->lineEditFilterStart->setEnabled(ui->checkBoxFilterRange->isChecked() & checked);
    ui->lineEditFilterEnd->setEnabled(ui->checkBoxFilterRange->isChecked() & checked);

    applyConfigEnabled(true);
}

void MainWindow::on_checkBoxSortByTime_toggled(bool checked)
{
    QDltSettingsManager::getInstance()->setValue("startup/sortByTimeEnabled", checked);
    if(checked)
    {
        QDltSettingsManager::getInstance()->setValue("startup/sortByTimestampEnabled", false);
        ui->checkBoxSortByTimestamp->setChecked(false);
    }
    ui->actionToggle_SortByTimeEnabled->setChecked(ui->checkBoxSortByTime->isChecked());
    ui->actionSort_By_Timestamp->setChecked(ui->checkBoxSortByTimestamp->isChecked());
    applyConfigEnabled(true);
}

void MainWindow::on_checkBoxSortByTimestamp_toggled(bool checked)
{
    if(checked)
    {
        QDltSettingsManager::getInstance()->setValue("startup/sortByTimeEnabled", false);
        ui->checkBoxSortByTime->setChecked(false);
    }
    QDltSettingsManager::getInstance()->setValue("startup/sortByTimestampEnabled", checked);
    ui->actionToggle_SortByTimeEnabled->setChecked(ui->checkBoxSortByTime->isChecked());
    ui->actionSort_By_Timestamp->setChecked(ui->checkBoxSortByTimestamp->isChecked());
    applyConfigEnabled(true);
}

void MainWindow::syncCheckBoxesAndMenu()
{
    auto pluginList = pluginManager.getPlugins();

    for(auto& plugin : pluginList)
        plugin->configurationChanged();

    ui->actionToggle_SortByTimeEnabled->setChecked(ui->checkBoxSortByTime->isChecked());
    ui->actionSort_By_Timestamp->setChecked(ui->checkBoxSortByTimestamp->isChecked());

    ui->actionToggle_PluginsEnabled->setChecked(ui->pluginsEnabled->isChecked());
    if (ui->pluginsEnabled->isChecked())
        {
            ui->actionToggle_PluginsEnabled->setText("Disable Plugins");
        }
        else
        {
            ui->actionToggle_PluginsEnabled->setText("Enable Plugins");
        }

    ui->actionToggle_FiltersEnabled->setChecked(ui->filtersEnabled->isChecked());
    if (ui->filtersEnabled->isChecked())
        {
            ui->actionToggle_FiltersEnabled->setText("Disable Filters");
        }
        else
        {
            ui->actionToggle_FiltersEnabled->setText("Enable Filters");
        }


}

void MainWindow::on_applyConfig_clicked()
{
    syncCheckBoxesAndMenu();
    applyConfigEnabled(false);
    filterUpdate();
    reloadLogFile(true);
    triggerPluginsAutoload();
}

void MainWindow::clearSelection()
{
    previousSelection.clear();
    ui->tableView->selectionModel()->clear();
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
        //qDebug() << "Save Selection " << i << " at line " << qfile.getMsgFilterPos(sr);
    }
}

void MainWindow::restoreSelection()
{
    int firstIndex = 0;
    //QModelIndex scrollToTarget = tableModel->index(0, 0);
    QItemSelection newSelection;

    // clear current selection model
    ui->tableView->selectionModel()->clear();

    // check if anything was selected
    if(previousSelection.count()==0)
        return;

    // we need to find visible column, otherwise scrollTo does not work, e.g. if Index is disabled
    int col = 0;
    for(col = 0; col <= ui->tableView->model()->columnCount(); col++)
    {
       if(!ui->tableView->isColumnHidden(col))
       {
           break;
       }
    }

    // restore all selected lines
    for(int j=0;j<previousSelection.count();j++)
    {
        int nearestIndex = nearest_line(previousSelection.at(j));

        //qDebug() << "Restore Selection" << j << "at index" << nearestIndex << "at line" << previousSelection.at(0);

        if(j==0)
        {
            firstIndex = nearestIndex;
        }

        QModelIndex idx = tableModel->index(nearestIndex, col);

        newSelection.select(idx, idx);
    }

    // set all selections
    ui->tableView->selectionModel()->select(newSelection, QItemSelectionModel::Select|QItemSelectionModel::Rows);

    // scroll to first selected row
    ui->tableView->setFocus();  // focus must be set before scrollto is possible
    QModelIndex idx = tableModel->index(firstIndex, col, QModelIndex());
    ui->tableView->scrollTo(idx, QAbstractItemView::PositionAtTop);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if(index > 1)
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

void MainWindow::filterCountChanged()
{
    // update filters on the DLT file itself
    filterUpdate();
    // update the currently shown table
    tableModel->modelChanged();
    // enable the "Apply" button
    applyConfigEnabled(true);
    // update the menu entries based on current selection
    on_filterWidget_itemSelectionChanged();
}

void MainWindow::searchTableRenewed()
{
    if ( 0 < m_searchtableModel->get_SearchResultListSize())
    {
        QString hits = QString("Search Results: %L1").arg(m_searchtableModel->get_SearchResultListSize());
        ui->dockWidgetSearchIndex->show();
        ui->dockWidgetSearchIndex->setWindowTitle(hits);
    }
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

void MainWindow::on_comboBoxFilterSelection_currentTextChanged(const QString &arg1)
{
    /* load current selected filter */
    if(!arg1.isEmpty() && project.LoadFilter(arg1,!ui->checkBoxAppendDefaultFilter->isChecked()))
    {
        workingDirectory.setDlfDirectory(QFileInfo(arg1).absolutePath());
        setCurrentFilters(arg1);

       /* Activate filter and create index there as usual */
       on_applyConfig_clicked();

       ui->tabWidget->setCurrentWidget(ui->tabPFilter);
       on_filterWidget_itemSelectionChanged();
    }
}

void MainWindow::on_actionDefault_Filter_Reload_triggered()
{
    QDir dir;
    //qDebug() << "MainWindow::on_actionDefault_Filter_Reload_triggered" << __LINE__;
    /* clear combobox default filter */
    ui->comboBoxFilterSelection->clear();

    /* add "no default filter" entry */
    ui->comboBoxFilterSelection->addItem("");

    /* clear default filter list */
    defaultFilter.clear();

    // check if default filter enabled
    if(!settings->defaultFilterPath)
    {
        return;
    }

    /* get the filter path */
    dir.setPath(settings->defaultFilterPathName);

    /* update tooltip */
    ui->comboBoxFilterSelection->setToolTip(QString("Multifilterlist in folder %1").arg(dir.absolutePath()));

    /* check if directory for configuration exists */
    if(!dir.exists())
    {
        /* directory does not exist, make it */
        if(!dir.mkpath(dir.absolutePath()))
        {
            /* creation of directory fails */
            QMessageBox::critical(0, QString("DLT Viewer"),
                                           QString("Cannot create directory to store cache files!\n\n")+dir.absolutePath(),
                                           QMessageBox::Ok,
                                           QMessageBox::Ok);
            return;
        }
    }

    /* load the default filter list */
    defaultFilter.load(dir.absolutePath());

    // default filter list update combobox
    for (const auto *filterList : defaultFilter.defaultFilterList) {
        ui->comboBoxFilterSelection->addItem(filterList->getFilename());
    }
}

void MainWindow::on_actionDefault_Filter_Create_Index_triggered()
{
    /* reset default filter list and reload from directory all default filter */
    reloadLogFileDefaultFilter();
}

void MainWindow::applyConfigEnabled(bool enabled)
{
    //qDebug() << "applyConfigEnabled" << enabled << __LINE__;
    if(true == enabled)
    {
        /* show apply config button */
       // ui->applyConfig->startPulsing(pulseButtonColor);
        ui->actionApply_Configuration->setCheckable(true);
        ui->actionApply_Configuration->setChecked(true);
        ui->applyConfig->setEnabled(true);

        /* reset default filter selection and default filter index */
        // resetDefaultFilter();
    }
    else
    {
        /* hide apply config button */
        //ui->applyConfig->stopPulsing();
        ui->actionApply_Configuration->setCheckable(false);
        ui->applyConfig->setEnabled(false);
    }
}

void MainWindow::resetDefaultFilter()
{
    /* reset all default filter index */
    defaultFilter.clearFilterIndex();

    /* select "no default filter" entry */
    ui->comboBoxFilterSelection->setCurrentIndex(0); //no default filter anymore
}

void MainWindow::on_pushButtonDefaultFilterUpdateCache_clicked()
{
    on_actionDefault_Filter_Create_Index_triggered();
}

void MainWindow::on_actionMarker_triggered()
{
    controlMessage_Marker();
}

void MainWindow::onAddActionToHistory()
{
    QString searchText = searchDlg->getText();

    if((!searchHistory.contains(searchText,Qt::CaseInsensitive)) && !searchText.isEmpty())
    {
        searchHistory.prepend(searchText);
    }

    int searchHistorySize = searchHistory.size();
    for (int i = 0;i < searchHistorySize && i < MaxSearchHistory; i++)
    {
        searchHistoryActs[i]->setText(searchHistory[i]);
        searchHistoryActs[i]->setVisible(true);
    }
}

void MainWindow::onSearchProgressChanged(bool isInProgress)
{
    isSearchOngoing = isInProgress;
    ui->menuBar->setEnabled(!isInProgress);
    ui->mainToolBar->setEnabled(!isInProgress);
    if(!isInProgress)
        searchInput->resetProgress();

    ui->actionFindNext->setEnabled(!isInProgress);
    ui->actionFindPrevious->setEnabled(!isInProgress);
    ui->actionFind->setEnabled(!isInProgress);
    ui->actionRegExp->setEnabled(!isInProgress);
    searchInput->setState(isInProgress ? SearchForm::State::PROGRESS : SearchForm::State::INPUT);

    ui->dockWidgetProject->setEnabled(!isInProgress);
}

QString MainWindow::GetConnectionType(int iTypeNumber)
{
   QString port;
   switch (iTypeNumber)
   {
   case EcuItem::INTERFACETYPE_TCP:
       port=QString("TCP");
       break;
   case EcuItem::INTERFACETYPE_UDP:
       port=QString("UDP");
       break;
   case EcuItem::INTERFACETYPE_SERIAL_DLT:
       port=QString("Serial DLT");
       break;
   case EcuItem::INTERFACETYPE_SERIAL_ASCII:
       port=QString("Serial ASCII");
       break;
   default:
       port=QString("UNDEFINED");
       break;
   }

  return port;

}

void MainWindow::indexDone()
{
    qint64 fileerrors = dltIndexer->getfileerrors();
    statusFileError->setText(QString("FileErr: %L1").arg(fileerrors));
}

void MainWindow::indexStart()
{
    statusFileError->setText(QString("FileErr: %L1").arg("-"));
}

void MainWindow::openSupportedFile(const QString& path)
{
    static const QStringList ext = QStringList() << ".dlt" << ".dlf" << ".dlp" << ".pcap" << ".mf4";

    auto result = std::find_if(ext.begin(), ext.end(),
                               [&path](const QString& el) { return path.toLower().endsWith(el); });
    switch (result - ext.begin()) {
    case 0: /* this represents index in "ext" list */
        openDltFile(QStringList() << path);
        outputfileIsTemporary = false;
        break;
    case 1:
        openDlfFile(path, true);
        break;
    case 2:
        openDlpFile(path);
        break;
    case 3: {
        QDltImporter* importerThread = new QDltImporter(&outputfile, path);
        importerThread->setPcapPorts(settings->importerPcapPorts);
        connect(importerThread, &QDltImporter::progress, this, &MainWindow::progress);
        connect(importerThread, &QDltImporter::resultReady, this, &MainWindow::handleImportResults);
        connect(importerThread, &QDltImporter::finished, importerThread, &QObject::deleteLater);
        statusProgressBar->show();
        importerThread->start();
    } break;
    case 4: {
        QDltImporter* importerThread = new QDltImporter(&outputfile, path);
        importerThread->setPcapPorts(settings->importerPcapPorts);
        connect(importerThread, &QDltImporter::progress, this, &MainWindow::progress);
        connect(importerThread, &QDltImporter::resultReady, this, &MainWindow::handleImportResults);
        connect(importerThread, &QDltImporter::finished, importerThread, &QObject::deleteLater);
        statusProgressBar->show();
        importerThread->start();
    } break;
    default:
        break;
    }
}

void MainWindow::on_checkBoxFilterRange_stateChanged(int arg1)
{
    applyConfigEnabled(true);

    ui->lineEditFilterStart->setEnabled(arg1==Qt::Checked);
    ui->lineEditFilterEnd->setEnabled(arg1==Qt::Checked);
}

void MainWindow::on_lineEditFilterStart_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    applyConfigEnabled(true);
}

void MainWindow::on_lineEditFilterEnd_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    applyConfigEnabled(true);
}

void MainWindow::handleImportResults(const QString &)
{
    statusProgressBar->hide();
    reloadLogFile();
}

void MainWindow::handleExportResults(const QString &)
{
    statusProgressBar->hide();
}
