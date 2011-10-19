#include <QTreeView>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QTemporaryFile>
#include <QPluginLoader>
#include <QSettings>
#include <QPushButton>
#include <QKeyEvent>
#include <QClipboard>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QLineEdit>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ecudialog.h"
#include "applicationdialog.h"
#include "contextdialog.h"
#include "multiplecontextdialog.h"
#include "filterdialog.h"
#include "plugindialog.h"
#include "settingsdialog.h"
#include "injectiondialog.h"

#include "version.h"

#define DLT_VIEWER_LIST_BUFFER_SIZE 100024

extern char buffer[DLT_VIEWER_LIST_BUFFER_SIZE];

MainWindow::MainWindow(QString filename, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer(this),
    qcontrol(this)
{
    ui->setupUi(this);

    /* Enable Drops */
    setAcceptDrops(true);

    ui->configWidget->sortByColumn(0, Qt::AscendingOrder); // column/order to sort by
    ui->configWidget->setSortingEnabled(true);             // should cause sort on add

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

    /* Initialize recent projects */
    for (int i = 0; i < MaxRecentFilters; ++i) {
        recentFiltersActs[i] = new QAction(this);
        recentFiltersActs[i]->setVisible(false);
        connect(recentFiltersActs[i], SIGNAL(triggered()), this, SLOT(openRecentFilters()));
        ui->menuRecent_Filters->addAction(recentFiltersActs[i]);
    }

    /* Settings */
    bmwsettings = new QSettings("BMW","DLT Viewer");

    recentFiles = bmwsettings->value("other/recentFileList").toStringList();
    recentProjects = bmwsettings->value("other/recentProjectList").toStringList();
    recentFilters = bmwsettings->value("other/recentFiltersList").toStringList();
    settings.readSettings();

    /* Update recent file and project actions */
    updateRecentFileActions();
    updateRecentProjectActions();
    updateRecentFiltersActions();

    /* initialise DLT file handling */
    tableModel = new TreeModel("Hello Tree");
    tableModel->size = 0;
    tableModel->qfile = &qfile;
    ui->tableView->setModel(tableModel);

    tableModel->mutex = &mutex;
    tableModel->project = &project;
    connect((QObject*)(ui->tableView->verticalScrollBar()), SIGNAL(valueChanged(int)), this, SLOT(tableViewValueChanged(int)));
    QItemSelectionModel *sm = ui->tableView->selectionModel();
    connect(sm, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this, SLOT(on_tableViewTriggerSelectionModel(QModelIndex,QModelIndex)));


    workingDirectory = bmwsettings->value("work/workingDirectory",QDir::currentPath()).toString();

    /* initialise project configuration */
    project.ecu = ui->configWidget;
    project.pfilter = ui->pfilterWidget;
    project.plugin = ui->pluginWidget;
    project.settings = &settings;
    ui->configWidget->setHeaderHidden(false);
    ui->pfilterWidget->setHeaderHidden(false);
    ui->pluginWidget->setHeaderHidden(false);

    /* Load Plugins before loading default project */
    loadPlugins();

    /* initialise statusbar */
    statusFilename = new QLabel();
    statusBar()->addWidget(statusFilename);
    statusBytesReceived = new QLabel();
    statusBar()->addWidget(statusBytesReceived);
    statusBytesReceived->setText("Recv: 0");
    totalBytesRcvd = 0;  
    statusByteErrorsReceived = new QLabel();
    statusBar()->addWidget(statusByteErrorsReceived);
    statusByteErrorsReceived->setText("Recv Errors: 0");
    totalByteErrorsRcvd = 0;



    searchDlg = new SearchDialog(this);
    searchDlg->file = &qfile;
    searchDlg->table = ui->tableView;
    searchDlg->plugin = project.plugin;

    /* initialize tool bar */
    QAction *action;
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/document-new.png"), tr("&New"));
    connect(action, SIGNAL(triggered()), this, SLOT(on_actionNew_triggered()));
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/document-open.png"), tr("&Open"));
    connect(action, SIGNAL(triggered()), this, SLOT(on_actionOpen_triggered()));
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/edit-clear.png"), tr("&Clear"));
    connect(action, SIGNAL(triggered()), this, SLOT(on_actionClear_triggered()));
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/document-save-as.png"), tr("&Save As"));
    connect(action, SIGNAL(triggered()), this, SLOT(on_actionSaveAs_triggered()));
    ui->mainToolBar->addSeparator();
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/network-transmit-receive.png"), tr("Connec&t"));
    connect(action, SIGNAL(triggered()), this, SLOT(connectAll()));
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/network-offline.png"), tr("&Disconnect"));
    connect(action, SIGNAL(triggered()), this, SLOT(disconnectAll()));
    ui->mainToolBar->addSeparator();
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/preferences-desktop.png"), tr("S&ettings"));
    connect(action, SIGNAL(triggered()), this, SLOT(on_actionSettings_triggered()));

    ui->mainToolBar->addSeparator();

    scrollbutton = new QPushButton(QIcon(":/toolbar/png/go-bottom.png"),tr(""),this);
    scrollbutton->setFlat(true);
    scrollbutton->setCheckable(true);
    scrollbutton->setToolTip(tr("Auto Scroll"));
    action = ui->mainToolBar->addWidget(scrollbutton);
    connect(scrollbutton, SIGNAL(toggled(bool)), this, SLOT(autoscrollToggled(bool)));
    ui->mainToolBar->addSeparator();

    filterbutton = new QPushButton(QIcon(":/toolbar/png/filter.png"),tr(""),this);
    filterbutton->setFlat(true);
    filterbutton->setCheckable(true);
    filterbutton->setToolTip(tr("Enable Filter"));
    action = ui->mainToolBar->addWidget(filterbutton);
    connect(filterbutton, SIGNAL(toggled(bool)), this, SLOT(filterToggled(bool)));

    ui->mainToolBar->addSeparator();

    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/system-search.png"), tr("Find"));
    connect(action, SIGNAL(triggered()), this, SLOT(on_actionFind_triggered()));
    //action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/go-previous.png"), tr("Search previous"));
    //connect(action, SIGNAL(triggered()), this, SLOT(on_actionSearch_Continue_triggered()));
    searchTextToolbar = new QLineEdit(ui->mainToolBar);
    searchDlg->appendLineEdit(searchTextToolbar);
    connect(searchTextToolbar, SIGNAL(textEdited(QString)),searchDlg,SLOT(on_lineEditText_textEditedFromToolbar(QString)));
    connect(searchTextToolbar, SIGNAL(returnPressed()),searchDlg,SLOT(on_actionFind_Next_triggered()));
    action = ui->mainToolBar->addWidget(searchTextToolbar);
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/go-previous.png"), tr("Find Previous"));
    connect(action, SIGNAL(triggered()), searchDlg, SLOT(on_actionFind_Previous_triggered()));
    action = ui->mainToolBar->addAction(QIcon(":/toolbar/png/go-next.png"), tr("Find Next"));
    connect(action, SIGNAL(triggered()), searchDlg, SLOT(on_actionFind_Next_triggered()));

    updateScrollButton();

    /* menu configuration */
    ui->actionGet_Default_Log_Level->setEnabled(false);
    ui->actionSet_Default_Log_Level->setEnabled(false);
    ui->actionGet_Local_Time->setEnabled(false);
    ui->actionGet_Software_Version->setEnabled(false);
    ui->actionStore_Config->setEnabled(false);
    ui->actionGet_Log_Info->setEnabled(false);
    ui->actionSet_Log_Level->setEnabled(false);
    ui->actionSet_All_Log_Levels->setEnabled(false);
    ui->actionReset_to_Factory_Default->setEnabled(false);
    ui->actionSend_Injection->setEnabled(false);
    //ui->actionFilter_Add->setEnabled(true);
    //ui->actionFilter_Delete->setEnabled(false);
    //ui->actionFilter_Clear_all->setEnabled(false);
    //ui->actionFilter_Edit->setEnabled(false);
    ui->actionPlugin_Edit->setEnabled(false);
    ui->actionApplication_Add->setEnabled(false);
    ui->actionApplication_Edit->setEnabled(false);
    ui->actionApplication_Delete->setEnabled(false);
    ui->actionContext_Add->setEnabled(false);
    ui->actionContext_Edit->setEnabled(false);
    ui->actionContext_Delete->setEnabled(false);
    ui->actionECU_Add->setEnabled(true);
    ui->actionECU_Edit->setEnabled(false);
    ui->actionECU_Delete->setEnabled(false);
    ui->actionECU_Delete_All_Contexts->setEnabled(false);
    ui->actionECU_Connect->setEnabled(false);
    ui->actionECU_Disconnect->setEnabled(false);

    /* initialize injection */
    injectionAplicationId.clear();
    injectionContextId.clear();
    injectionServiceId.clear();
    injectionData.clear();

    /* set table size */
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
    connect(ui->tableView->horizontalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(sectionInTableDoubleClicked(int)));

    //ui->tableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    //ui->tableView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);

    /* Apply loaded settings */
    applySettings();

    /* Load default project file */
    this->setWindowTitle(QString("DLT Viewer - unnamed project - Version : %1 %2").arg(PACKAGE_VERSION).arg(PACKAGE_VERSION_STATE));
    if(settings.defaultProjectFile)
    {
        if(project.Load(settings.defaultProjectFileName))
        {
            /* Applies project settings and save it to registry */
            applySettings();
            settings.writeSettings();

            this->setWindowTitle(QString("DLT Viewer - "+settings.defaultProjectFileName+" - Version : %1 %2").arg(PACKAGE_VERSION).arg(PACKAGE_VERSION_STATE));
        }
        else
            QMessageBox::critical(0, QString("DLT Viewer"),
                                 QString("Cannot load default project \"%1\"")
                                 .arg(settings.defaultProjectFileName));
    }

    /* load default log file */
    statusFilename->setText("no log file loaded");
    if(settings.defaultLogFile)
    {
        outputfile.setFileName(settings.defaultLogFileName);
        if(outputfile.open(QIODevice::WriteOnly|QIODevice::Append))
            reloadLogFile();
        else
            QMessageBox::critical(0, QString("DLT Viewer"),
                                 QString("Cannot load default log file \"%1\"\n%2")
                                 .arg(settings.defaultLogFileName)
                                 .arg(outputfile.errorString()));

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

    /* load dlt or project file with program start */
    if(!filename.isEmpty())
    {
        if(filename.endsWith(".dlt"))
        {
            /* DLT log file used */
            logfileOpen(filename);
        }
        else if(filename.endsWith(".dlp"))
        {
            /* Project file used */
            projectfileOpen(filename);
        }
        else
        {
            QMessageBox::warning(this, QString("Open File"),
                                 QString("No DLT log file or project file used!\n")+filename);
        }

    }

    /* Load the plugins description files after loading default project */
    updatePlugins();

    /* Update the ECU list in control plugins */
    updatePluginsECUList();

    /* auto connect */
    if(settings.autoConnect)
    {
        connectAll();
    }

    /* start timer for autoconnect */
    connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer.start(1000);

    /* start timer for serial read */
    connect(&timerRead, SIGNAL(timeout()), this, SLOT(timeoutRead()));
    timerRead.start(100);

    /* show settings */
    ui->dockWidgetView->hide();

    restoreGeometry(bmwsettings->value("geometry").toByteArray());
    restoreState(bmwsettings->value("windowState").toByteArray());
}

MainWindow::~MainWindow()
{
    QSettings settings("BMW","DLT Viewer");

    /* store last working directory */
    settings.setValue("work/workingDirectory",workingDirectory);

    delete ui;
    delete tableModel;
    delete searchDlg;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("BMW","DLT Viewer");

    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());

    QMainWindow::closeEvent(event);
}

void MainWindow::on_actionNew_triggered()
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

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open DLT Log file"), workingDirectory, tr("DLT Files (*.dlt);;All files (*.*)"));

    if(fileName.isEmpty())
        return;

    /* change current working directory */
    workingDirectory = QFileInfo(fileName).absolutePath();

    logfileOpen(fileName);

    searchDlg->setMatch(false);
    searchDlg->setOnceClicked(false);
    searchDlg->setStartLine(-1);
}

void MainWindow::logfileOpen(QString fileName)
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

void MainWindow::on_actionImport_DLT_Stream_triggered()
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

void MainWindow::on_actionImport_DLT_Stream_with_Serial_Header_triggered()
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

void MainWindow::on_actionAppend_DLT_File_triggered()
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

    QProgressDialog progress("Append log file", "Abort Loading", 0, 100, this);
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

void MainWindow::on_actionExport_ASCII_triggered()
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

    QProgressDialog fileprogress("Export to ASCII...", "Abort", 0, qfile.sizeFilter(), this);
    fileprogress.setWindowTitle("DLT Viewer");
    fileprogress.setWindowModality(Qt::WindowModal);
    fileprogress.show();
    for(int num = 0;num< qfile.sizeFilter();num++)
    {
        fileprogress.setValue(num);

        /* get message form log file */
        data = qfile.getMsgFilter(num);
        msg.setMsg(data);

        /* decode message is necessary */
        for(int num2 = 0; num2 < project.plugin->topLevelItemCount (); num2++)
        {
            PluginItem *item = (PluginItem*)project.plugin->topLevelItem(num2);

            if(item->plugindecoderinterface && item->plugindecoderinterface->isMsg(msg))
            {
                item->plugindecoderinterface->decodeMsg(msg);
                break;
            }
        }

        /* get message ASCII text */
        text.clear();
        text += QString("%1 ").arg(qfile.getMsgFilterPos(num));
        text += msg.toStringHeader();
        text += " ";
        text += msg.toStringPayload();
        text += "\n";

        /* write to file */
        outfile.write(text.toAscii().data());

    }

    outfile.close();
}

void MainWindow::on_actionExport_Selection_triggered()
{
    exportSelection(false,true);
}

void MainWindow::on_actionExport_Selection_ASCII_triggered()
{
    exportSelection(true,true);
}

void MainWindow::exportSelection(bool ascii,bool file)
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

    QProgressDialog fileprogress("Export...", "Abort", 0, list.count(), this);
    fileprogress.setWindowTitle("DLT Viewer");
    fileprogress.setWindowModality(Qt::WindowModal);
    fileprogress.show();
    for(int num=0; num < list.count();num++)
    {
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
               for(int num2 = 0; num2 < project.plugin->topLevelItemCount (); num2++)
               {
                   PluginItem *item = (PluginItem*)project.plugin->topLevelItem(num2);

                   if(item->plugindecoderinterface && item->plugindecoderinterface->isMsg(msg))
                   {
                       item->plugindecoderinterface->decodeMsg(msg);
                       break;
                   }
               }

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
                   outfile.write(text.toAscii().data());
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

void MainWindow::on_actionSaveAs_triggered()
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

void MainWindow::on_actionClear_triggered()
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
                       ui->textBrowser->setText("");
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

void MainWindow::reloadLogFile()
{
    QDltMsg msg;
    QByteArray data;
    //QProgressDialog progress("Loading log file", "Abort Loading", 0, 100, this);

    /* Show progress bar */

    /* open file, create filter index and update model view */
    qfile.open(outputfile.fileName());

    QProgressDialog fileprogress("Parsing DLT file...", "Abort", 0, qfile.size(), this);
    fileprogress.setWindowTitle("DLT Viewer");
    fileprogress.setWindowModality(Qt::WindowModal);
    fileprogress.show();
    qfile.clearFilterIndex();
    for(int num=0;num<qfile.size();num++) {
        fileprogress.setValue(num);
        data = qfile.getMsg(num);
        msg.setMsg(data);
        for(int num2 = 0; num2 < project.plugin->topLevelItemCount (); num2++)
        {
            PluginItem *item = (PluginItem*)project.plugin->topLevelItem(num2);

            if(item->plugindecoderinterface && item->plugindecoderinterface->isMsg(msg))
            {
                item->plugindecoderinterface->decodeMsg(msg);
                break;
            }
        }
        if(qfile.checkFilter(msg)) {
            qfile.addFilterIndex(num);
        }
    }

    // qfile.createIndexFilter();

    ui->tableView->selectionModel()->clear();
    tableModel->size = qfile.sizeFilter();
    tableModel->modelChanged();

    /* update plugins */
    for(int num = 0; num < project.plugin->topLevelItemCount (); num++)
    {
        PluginItem *item = (PluginItem*)project.plugin->topLevelItem(num);

        if(item->pluginviewerinterface && (item->mode != PluginItem::ModeDisable))
        {
            item->pluginviewerinterface->initFile(&qfile);
        }
    }

    /* set name of opened log file in status bar */
    statusFilename->setText(outputfile.fileName());
}

void MainWindow::applySettings()
{
    QFont tableViewFont = ui->tableView->font();
    tableViewFont.setPointSize(settings.fontSize);
    ui->tableView->setFont(tableViewFont);
    // Rescale the height of a row to choosen font size + 8 pixels
    ui->tableView->verticalHeader()->setDefaultSectionSize(settings.fontSize+8);

    settings.showIndex?ui->tableView->showColumn(0):ui->tableView->hideColumn(0);
    settings.showTime?ui->tableView->showColumn(1):ui->tableView->hideColumn(1);
    settings.showTimestamp?ui->tableView->showColumn(2):ui->tableView->hideColumn(2);
    settings.showCount?ui->tableView->showColumn(3):ui->tableView->hideColumn(3);

    settings.showEcuId?ui->tableView->showColumn(4):ui->tableView->hideColumn(4);
    settings.showApId?ui->tableView->showColumn(5):ui->tableView->hideColumn(5);
    switch(settings.showApIdDesc){
        case 0:
            tableModel->showApIdDesc=0;
            break;
        case 1:
            tableModel->showApIdDesc=1;
            break;
        }

    settings.showCtId?ui->tableView->showColumn(6):ui->tableView->hideColumn(6);
    switch(settings.showCtIdDesc){
       case 0:
           tableModel->showCtIdDesc=0;
           break;
       case 1:
           tableModel->showCtIdDesc=1;
           break;
       }

    settings.showType?ui->tableView->showColumn(7):ui->tableView->hideColumn(7);

    settings.showSubtype?ui->tableView->showColumn(8):ui->tableView->hideColumn(8);
    settings.showMode?ui->tableView->showColumn(9):ui->tableView->hideColumn(9);
    settings.showNoar?ui->tableView->showColumn(10):ui->tableView->hideColumn(10);
    settings.showPayload?ui->tableView->showColumn(11):ui->tableView->hideColumn(11);
}

void MainWindow::on_actionSettings_triggered()
{
    /* show settings dialog */
    settings.writeDlg();
    settings.workingDirectory = workingDirectory;

    if(settings.exec()==1)
    {
        /* change settings and store settings persistently */
        settings.readDlg();
        settings.writeSettings();
        workingDirectory = settings.workingDirectory;

        /* Apply settings to table */
        applySettings();

        updateScrollButton();
    }
}

void MainWindow::on_actionQuit_triggered()
{
    /* TODO: Add quit code here */
    this->close();

}


void MainWindow::on_actionProjectNew_triggered()
{
    /* TODO: Ask for saving project if changed */

    /* create new project */

    this->setWindowTitle(QString("DLT Viewer - unnamed project - Version : %1 %2").arg(PACKAGE_VERSION).arg(PACKAGE_VERSION_STATE));
    project.Clear();

    /* Update the ECU list in control plugins */
    updatePluginsECUList();

}

void MainWindow::on_actionProjectOpen_triggered()
{
    /* TODO: Ask for saving project if changed */

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open DLT Project file"), workingDirectory, tr("DLT Project Files (*.dlp);;All files (*.*)"));

    /* open existing project */
    if(!fileName.isEmpty())
    {
        /* change current working directory */
        workingDirectory = QFileInfo(fileName).absolutePath();

        projectfileOpen(fileName);

        filterUpdate();
    }

}

void MainWindow::projectfileOpen(QString fileName)
{
    /* Open existing project */
    if(project.Load(fileName))
    {
        /* Applies project settings and save it to registry */
        applySettings();
        settings.writeSettings();

        this->setWindowTitle(QString("DLT Viewer - "+fileName+" - Version : %1 %2").arg(PACKAGE_VERSION).arg(PACKAGE_VERSION_STATE));

        /* Load the plugins description files after loading project */
        updatePlugins();

        setCurrentProject(fileName);

        /* Update the ECU list in control plugins */
        updatePluginsECUList();

    }
}

void MainWindow::on_actionProjectSave_triggered()
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

void MainWindow::on_actionECU_Add_triggered()
{   
    QStringList hostnameListPreset;
    hostnameListPreset << "localhost";

    QStringList portListPreset;
    portListPreset << "COM0" << "COM1" << "/dev/ttyUSB0" << "/dev/ttyUSB1" << "/dev/ttyS0" << "/dev/ttyS1";

    /* show ECU configuration dialog */
    EcuDialog dlg("ECU","A new ECU",0,"localhost",DLT_DAEMON_TCP_PORT,"COM0",19,DLT_LOG_INFO,DLT_TRACE_STATUS_OFF,1,false,true,false,false,false,false,true);

    /* Read settings for recent hostnames and ports */
    recentHostnames = bmwsettings->value("other/recentHostnameList",hostnameListPreset).toStringList();
    recentPorts = bmwsettings->value("other/recentPortList",portListPreset).toStringList();

    dlg.setHostnameList(recentHostnames);
    dlg.setPortList(recentPorts);

    if(dlg.exec()==1)
    {
        /* add new ECU to configuration */
        EcuItem* ecuitem = new EcuItem(0);
        ecuitem->id = dlg.id();
        ecuitem->description = dlg.description();
        ecuitem->interfacetype = dlg.interfacetype();
        ecuitem->hostname = dlg.hostname();
        ecuitem->tcpport = dlg.tcpport();
        ecuitem->port = dlg.port();
        ecuitem->baudrate = dlg.baudrate();
        ecuitem->loglevel = dlg.loglevel();
        ecuitem->tracestatus = dlg.tracestatus();
        ecuitem->verbosemode = dlg.verbosemode();
        ecuitem->sendSerialHeaderSerial = dlg.sendSerialHeaderSerial();
        ecuitem->sendSerialHeaderTcp = dlg.sendSerialHeaderTcp();
        ecuitem->syncSerialHeaderSerial = dlg.syncSerialHeaderSerial();
        ecuitem->syncSerialHeaderTcp = dlg.syncSerialHeaderTcp();
        ecuitem->timingPackets = dlg.timingPackets();
        ecuitem->sendGetLogInfo = dlg.sendGetLogInfo();
        ecuitem->updateDataIfOnline = dlg.update();

        /* new qdlt library */
        ecuitem->tcpcon.setTcpPort(dlg.tcpport());
        ecuitem->tcpcon.setHostname(dlg.hostname());
        ecuitem->tcpcon.setSendSerialHeader(dlg.sendSerialHeaderTcp());
        ecuitem->tcpcon.setSyncSerialHeader(dlg.syncSerialHeaderSerial());
        ecuitem->serialcon.setBaudrate(dlg.baudrate());
        ecuitem->serialcon.setPort(dlg.port());
        ecuitem->serialcon.setSendSerialHeader(dlg.sendSerialHeaderSerial());
        ecuitem->serialcon.setSyncSerialHeader(dlg.syncSerialHeaderTcp());

        /* update ECU item */
        ecuitem->update();

        /* add ECU to configuration */
        project.ecu->addTopLevelItem(ecuitem);

        /* Update settings for recent hostnames and ports */
        setCurrentHostname(ecuitem->hostname);
        setCurrentPort(ecuitem->port);

        /* Update the ECU list in control plugins */
        updatePluginsECUList();
    }
}

void MainWindow::on_actionECU_Edit_triggered()
{
    /* find selected ECU in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        QStringList hostnameListPreset;
        hostnameListPreset << "localhost";

        QStringList portListPreset;
        portListPreset << "COM0" << "COM1" << "/dev/ttyUSB0" << "/dev/ttyUSB1" << "/dev/ttyS0" << "/dev/ttyS1";

        EcuItem* ecuitem = (EcuItem*) list.at(0);

        /* show ECU configuration dialog */
        EcuDialog dlg(ecuitem->id,ecuitem->description,ecuitem->interfacetype,ecuitem->hostname,ecuitem->tcpport,ecuitem->port,ecuitem->baudrate,
                      ecuitem->loglevel,ecuitem->tracestatus,ecuitem->verbosemode,ecuitem->sendSerialHeaderTcp,ecuitem->sendSerialHeaderSerial,ecuitem->syncSerialHeaderTcp,ecuitem->syncSerialHeaderSerial,
                      ecuitem->timingPackets,ecuitem->sendGetLogInfo,ecuitem->updateDataIfOnline);

        /* Read settings for recent hostnames and ports */
        recentHostnames = bmwsettings->value("other/recentHostnameList",hostnameListPreset).toStringList();
        recentPorts = bmwsettings->value("other/recentPortList",portListPreset).toStringList();

        setCurrentHostname(ecuitem->hostname);
        setCurrentPort(ecuitem->port);

        dlg.setHostnameList(recentHostnames);
        dlg.setPortList(recentPorts);

        if(dlg.exec())
        {
            /* change settings of ECU configuration */
            ecuitem->id = dlg.id();
            ecuitem->description = dlg.description();

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

            ecuitem->interfacetype = dlg.interfacetype();
            ecuitem->hostname = dlg.hostname();
            ecuitem->tcpport = dlg.tcpport();
            ecuitem->port = dlg.port();
            ecuitem->baudrate = dlg.baudrate();
            ecuitem->loglevel = dlg.loglevel();
            ecuitem->tracestatus = dlg.tracestatus();
            ecuitem->verbosemode = dlg.verbosemode();
            ecuitem->sendSerialHeaderSerial = dlg.sendSerialHeaderSerial();
            ecuitem->sendSerialHeaderTcp = dlg.sendSerialHeaderTcp();
            ecuitem->syncSerialHeaderSerial = dlg.syncSerialHeaderSerial();
            ecuitem->syncSerialHeaderTcp = dlg.syncSerialHeaderTcp();
            ecuitem->timingPackets = dlg.timingPackets();
            ecuitem->sendGetLogInfo = dlg.sendGetLogInfo();
            ecuitem->updateDataIfOnline = dlg.update();

            /* new qdlt library */
            ecuitem->tcpcon.setTcpPort(dlg.tcpport());
            ecuitem->tcpcon.setHostname(dlg.hostname());
            ecuitem->tcpcon.setSendSerialHeader(dlg.sendSerialHeaderTcp());
            ecuitem->tcpcon.setSyncSerialHeader(dlg.syncSerialHeaderSerial());
            ecuitem->serialcon.setBaudrate(dlg.baudrate());
            ecuitem->serialcon.setPort(dlg.port());
            ecuitem->serialcon.setSendSerialHeader(dlg.sendSerialHeaderSerial());
            ecuitem->serialcon.setSyncSerialHeader(dlg.syncSerialHeaderTcp());

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

void MainWindow::on_actionECU_Delete_triggered()
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

void MainWindow::on_actionECU_Delete_All_Contexts_triggered()
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

void MainWindow::on_actionApplication_Add_triggered()
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
            ecuitem->setExpanded(1);
        }
    }
}

void MainWindow::on_actionApplication_Edit_triggered()
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

void MainWindow::on_actionApplication_Delete_triggered()
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

void MainWindow::on_actionContext_Add_triggered()
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
            appitem->setExpanded(1);

            /* send new default log level to ECU, if connected and if selected in dlg */
            if(dlg.update())
            {
                EcuItem* ecuitem = (EcuItem*) appitem->parent();
                SetLogLevel(ecuitem,appitem->id,conitem->id,conitem->loglevel);
                SetTraceStatus(ecuitem,appitem->id,conitem->id,conitem->tracestatus);

                /* update status */
                conitem->status = ContextItem::valid;
                conitem->update();
            }
        }
    }


}

void MainWindow::on_actionContext_Edit_triggered()
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
                    SetLogLevel(ecuitem,appitem->id,conitem->id,conitem->loglevel);
                    SetTraceStatus(ecuitem,appitem->id,conitem->id,conitem->tracestatus);

                    /* update status */
                    conitem->status = ContextItem::valid;
                    conitem->update();
                }
            }
        }
    }
}

void MainWindow::on_actionContext_MultipleContextsEdit_triggered()
{
    /* find selected context in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() > 1))
    {
        ContextItem* conitem = (ContextItem*) list.at(0);

        /* show Context configuration dialog */
        MultipleContextDialog dlg(conitem->loglevel,conitem->tracestatus);
        if(dlg.exec())
        {
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
                             SetLogLevel(ecuitem,appitem->id,conitem->id,conitem->loglevel);
                             SetTraceStatus(ecuitem,appitem->id,conitem->id,conitem->tracestatus);

                             /* update status */
                             conitem->status = ContextItem::valid;
                             conitem->update();
                         }
                     }
                }
            }

        }
    }
}

void MainWindow::on_actionContext_Delete_triggered()
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

void MainWindow::on_configWidget_itemClicked(QTreeWidgetItem* /* item */, int /* column */)
{
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        /* TODO: Enable/Disable menu entries here */
    }
    if((list.count() == 1) && (list.at(0)->type() == context_type))
    {
        /* TODO: Enable/Disable menu entries here */
    }
    if((list.count() == 1) && (list.at(0)->type() == application_type))
    {
        /* TODO: Enable/Disable menu entries here */
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
        /* Multiple contexts are selected */

        action = new QAction("&Multiple Contexts Edit...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionContext_MultipleContextsEdit_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("DLT &Set Log Levels...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionSet_Log_Level_triggered()));
        menu.addAction(action);
    }
    else if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        /* ECU is selected */

        action = new QAction("ECU Add...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionECU_Add_triggered()));
        menu.addAction(action);

        action = new QAction("ECU Edit...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionECU_Edit_triggered()));
        menu.addAction(action);

        action = new QAction("ECU Delete", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionECU_Delete_triggered()));
        menu.addAction(action);

        action = new QAction("ECU Delete All Contexts", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionECU_Delete_All_Contexts_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("Application Add...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionApplication_Add_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("ECU Connect", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionECU_Connect_triggered()));
        menu.addAction(action);

        action = new QAction("ECU Disconnect", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionECU_Disconnect_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("DLT Get Log Info", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionGet_Log_Info_triggered()));
        menu.addAction(action);

        action = new QAction("DLT Set All Log Levels", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionSet_All_Log_Levels_triggered()));
        menu.addAction(action);

        action = new QAction("DLT Get Default Log Level", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionGet_Default_Log_Level_triggered()));
        menu.addAction(action);

        action = new QAction("DLT Set Default Log Level", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionSet_Default_Log_Level_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("Store Config", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionStore_Config_triggered()));
        menu.addAction(action);

        action = new QAction("Reset to Factory Default", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionReset_to_Factory_Default_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("Send Injection...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionSend_Injection_triggered()));
        menu.addAction(action);

        action = new QAction("Get Software Version", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionGet_Software_Version_triggered()));
        menu.addAction(action);

        action = new QAction("Get Local Time", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionGet_Local_Time_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("&Filter Add", this);
        connect(action, SIGNAL(triggered()), this, SLOT(filterAdd()));
        menu.addAction(action);

    }
    else if((list.count() == 1) && (list.at(0)->type() == application_type))
    {
        /* Application is selected */

        action = new QAction("&Application Edit...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionApplication_Edit_triggered()));
        menu.addAction(action);

        action = new QAction("A&pplication Delete...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionApplication_Delete_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("&Context Add...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionContext_Add_triggered()));
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
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionContext_Edit_triggered()));
        menu.addAction(action);

        action = new QAction("C&ontext Delete...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionContext_Delete_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("DLT &Set Log Level...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionSet_Log_Level_triggered()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("&Filter Add", this);
        connect(action, SIGNAL(triggered()), this, SLOT(filterAdd()));
        menu.addAction(action);

        menu.addSeparator();

        action = new QAction("Send Injection...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionSend_Injection_triggered()));
        menu.addAction(action);
    }
    else
    {
        /* nothing is selected */

        action = new QAction("ECU Add...", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionECU_Add_triggered()));
        menu.addAction(action);
    }

    /* show popup menu */
    menu.exec(globalPos);

}



void MainWindow::on_filterWidget_customContextMenuRequested(QPoint /* pos */ )
{

}

void MainWindow::on_nfilterWidget_customContextMenuRequested(QPoint pos)
{
    on_pfilterWidget_customContextMenuRequested(pos);
}

void MainWindow::on_markerWidget_customContextMenuRequested(QPoint pos)
{
    on_pfilterWidget_customContextMenuRequested(pos);
}

void MainWindow::on_pfilterWidget_customContextMenuRequested(QPoint pos)
{

    /* show custom pop menu  for filter configuration */
    QPoint globalPos = ui->pfilterWidget->mapToGlobal(pos);
    QMenu menu(project.ecu);
    QAction *action;
    QList<QTreeWidgetItem *> list = project.pfilter->selectedItems();

    action = new QAction("Save Filter(s)...", this);
    connect(action, SIGNAL(triggered()), this, SLOT(on_actionFilter_Save_As_triggered()));
    menu.addAction(action);

    action = new QAction("Load Filter(s)...", this);
    connect(action, SIGNAL(triggered()), this, SLOT(on_actionFilter_Load_triggered()));
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("Filter Add...", this);
    connect(action, SIGNAL(triggered()), this, SLOT(on_actionFilter_Add_triggered()));
    menu.addAction(action);

    action = new QAction("Filter Edit...", this);
    connect(action, SIGNAL(triggered()), this, SLOT(on_actionFilter_Edit_triggered()));
    menu.addAction(action);

    action = new QAction("Filter Delete", this);
    connect(action, SIGNAL(triggered()), this, SLOT(on_actionFilter_Delete_triggered()));
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("Filter Duplicate...", this);
    connect(action, SIGNAL(triggered()), this, SLOT(on_actionFilter_Duplicate_triggered()));
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
        connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlugin_Edit_triggered()));
        menu.addAction(action);
        if(item->pluginviewerinterface)
        {
            action = new QAction("Plugin Show", this);
            connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlugin_Show_triggered()));
            menu.addAction(action);
            action = new QAction("Plugin Hide", this);
            connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlugin_Hide_triggered()));
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
        on_actionECU_Add_triggered();
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

void MainWindow::on_actionECU_Connect_triggered()
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

void MainWindow::on_actionECU_Disconnect_triggered()
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
                connect(&ecuitem->socket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(stateChanged(QAbstractSocket::SocketState)));
                ecuitem->socket.connectToHost(ecuitem->hostname,ecuitem->tcpport);
            }
        }
        else
        {
            /* Serial */
            if(!ecuitem->serialport)
                ecuitem->serialport = new QextSerialPort(ecuitem->port);

            if(ecuitem->serialport->isOpen())
                   ecuitem->serialport->close();
            ecuitem->serialport->open(QIODevice::ReadWrite);

            //connect(ecuitem->serialport, SIGNAL(readyRead()), this, SLOT(readyRead()));
            if(ecuitem->serialport->isOpen())
            {
                ecuitem->serialport->setBaudRate((BaudRateType)ecuitem->baudrate);
                ecuitem->serialport->setDataBits(DATA_8);
                ecuitem->serialport->setParity(PAR_NONE);
                ecuitem->serialport->setStopBits(STOP_1);
                ecuitem->serialport->setFlowControl(FLOW_OFF);

                ecuitem->serialport->setTimeout(0,1);

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

        if(  (settings.showCtId && settings.showCtIdDesc) || (settings.showApId && settings.showApIdDesc) ){
            GetLogInfo(ecuitem);
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

void MainWindow::read(EcuItem* ecuitem)
{
    uint8_t *buf;
    int32_t bytesRcvd = 0;
    DltMessage msg;
    QByteArray data;
    QDltMsg qmsg;

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

                if ((settings.writeControl && is_ctrl_msg) || (!is_ctrl_msg))
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
                ReceiveControlMessage(ecuitem,msg);
            }

        }

        /* remove parsed data block from buffer */
        ecuitem->data.remove(0,ecuitem->data.size()-bytesRcvd);

        if (outputfile.isOpen() )
        {
            /* read received messages in DLT file parser and update DLT message list view */
            /* update indexes  and table view */
            int oldsize = qfile.size();
            qfile.updateIndex();
            for(int num=oldsize;num<qfile.size();num++) {
                data = qfile.getMsg(num);
                qmsg.setMsg(data);
                for(int num2 = 0; num2 < project.plugin->topLevelItemCount (); num2++)
                {
                    PluginItem *item = (PluginItem*)project.plugin->topLevelItem(num2);

                    if(item->plugindecoderinterface && item->plugindecoderinterface->isMsg(qmsg))
                    {
                        item->plugindecoderinterface->decodeMsg(qmsg);
                        break;
                    }
                }
                if(qfile.checkFilter(qmsg)) {
                    qfile.addFilterIndex(num);
                }
            }
            tableModel->size = qfile.sizeFilter();
            tableModel->modelChanged();
            //Line below would resize the payload column automatically so that the whole content is readable
            //ui->tableView->resizeColumnToContents(11); //Column 11 is the payload column
            if(settings.autoScroll) {
                ui->tableView->scrollToBottom();
            }

            /* update plugins */
            for(int num = 0; num < project.plugin->topLevelItemCount (); num++) {
                PluginItem *item = (PluginItem*)project.plugin->topLevelItem(num);

                if(item->pluginviewerinterface && (item->mode != PluginItem::ModeDisable)) {
                    item->pluginviewerinterface->updateFile();
                }
            }
        }
    }
}

void MainWindow::on_tableViewTriggerSelectionModel(QModelIndex indexNew,QModelIndex /* indexOld */)
{
    on_tableView_clicked(indexNew);
}

void MainWindow::on_tableView_clicked(QModelIndex index)
{
    /* update plugins */
    for(int num = 0; num < project.plugin->topLevelItemCount (); num++)
    {
        PluginItem *item = (PluginItem*)project.plugin->topLevelItem(num);

        if(item->pluginviewerinterface && (item->mode != PluginItem::ModeDisable))
        {
            item->pluginviewerinterface->selectedIdxMsg(index.row());
        }
    }
}

void MainWindow::ReceiveControlMessage(EcuItem *ecuitem, DltMessage &msg)
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

                        SetContext(ecuitem,QString(apid),QString(ctid),contextDescription,log_level,trace_status);
                    }

                    if (status==7)
                    {
                        QString applicationDescription;
                        uint16_t application_description_length=0,application_description_length_tmp=0;
                        DLT_MSG_READ_VALUE(application_description_length_tmp,ptr,length,uint16_t);
                        application_description_length=DLT_ENDIAN_GET_16(msg.standardheader->htyp,application_description_length_tmp);
                        applicationDescription = QString(QByteArray((char*)ptr,application_description_length));
                        SetApplication(ecuitem,QString(apid),applicationDescription);
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

void MainWindow::SendControlMessage(EcuItem* ecuitem,DltMessage &msg, QString appid, QString contid)
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
        if (settings.writeControl)
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
        for(int num2 = 0; num2 < project.plugin->topLevelItemCount (); num2++)
        {
            PluginItem *item = (PluginItem*)project.plugin->topLevelItem(num2);

            if(item->plugindecoderinterface && item->plugindecoderinterface->isMsg(qmsg))
            {
                item->plugindecoderinterface->decodeMsg(qmsg);
                break;
            }
        }
        if(qfile.checkFilter(qmsg)) {
            qfile.addFilterIndex(num);
        }
    }
    tableModel->size = qfile.sizeFilter();
    tableModel->modelChanged();
    if(settings.autoScroll) {
        ui->tableView->scrollToBottom();
    }

}

void MainWindow::on_actionGet_Default_Log_Level_triggered()
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

void MainWindow::on_actionSet_Default_Log_Level_triggered()
{
    /* get selected ECU in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        EcuItem* ecuitem = (EcuItem*) list.at(0);

        /* send set default log level request */
        SetDefaultLogLevel(ecuitem,ecuitem->loglevel);
        SetDefaultTraceStatus(ecuitem,ecuitem->tracestatus);

        /* update status */
        ecuitem->status = EcuItem::valid;
        ecuitem->update();
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No ECU selected in configuration!"));
}

void MainWindow::on_actionSet_Log_Level_triggered()
{
    /* get selected context in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == context_type))
    {
        ContextItem* conitem = (ContextItem*) list.at(0);
        ApplicationItem* appitem = (ApplicationItem*) conitem->parent();
        EcuItem* ecuitem = (EcuItem*) appitem->parent();

        /* send set log level and trace status request */
        SetLogLevel(ecuitem,appitem->id,conitem->id,conitem->loglevel);
        SetTraceStatus(ecuitem,appitem->id,conitem->id,conitem->tracestatus);

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
                SetLogLevel(ecuitem,appitem->id,conitem->id,conitem->loglevel);
                SetTraceStatus(ecuitem,appitem->id,conitem->id,conitem->tracestatus);

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

void MainWindow::on_actionSet_All_Log_Levels_triggered()
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
                SetLogLevel(ecuitem,appitem->id,conitem->id,conitem->loglevel);
                SetTraceStatus(ecuitem,appitem->id,conitem->id,conitem->tracestatus);

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

void MainWindow::on_actionGet_Log_Info_triggered()
{
    /* get selected ECU in configuration */
    QList<QTreeWidgetItem *> list = project.ecu->selectedItems();
    if((list.count() == 1) && (list.at(0)->type() == ecu_type))
    {
        EcuItem* ecuitem = (EcuItem*) list.at(0);
        GetLogInfo(ecuitem);
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                             QString("No ECU selected in configuration!"));

}

void MainWindow::SetLogLevel(EcuItem* ecuitem, QString app, QString con,int log_level)
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
    SendControlMessage(ecuitem,msg,QString(""),QString(""));

    /* free message */
    dlt_message_free(&msg,0);
}

void MainWindow::SetDefaultLogLevel(EcuItem* ecuitem, int status)
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
    SendControlMessage(ecuitem,msg,QString(""),QString(""));

    /* free message */
    dlt_message_free(&msg,0);
}

void MainWindow::SetTraceStatus(EcuItem* ecuitem,QString app, QString con,int status)
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
    SendControlMessage(ecuitem,msg,QString(""),QString(""));

    /* free message */
    dlt_message_free(&msg,0);

}

void MainWindow::SetDefaultTraceStatus(EcuItem* ecuitem, int status)
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
    SendControlMessage(ecuitem,msg,QString(""),QString(""));

    /* free message */
    dlt_message_free(&msg,0);
}

void MainWindow::SetVerboseMode(EcuItem* ecuitem, int verbosemode)
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
    SendControlMessage(ecuitem,msg,QString(""),QString(""));

    /* free message */
    dlt_message_free(&msg,0);
}

void MainWindow::SetTimingPackets(EcuItem* ecuitem, bool enable)
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
    SendControlMessage(ecuitem,msg,QString(""),QString(""));

    /* free message */
    dlt_message_free(&msg,0);
}

void MainWindow::GetLogInfo(EcuItem* ecuitem)
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
    SendControlMessage(ecuitem,msg,QString(""),QString(""));

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
    SendControlMessage(ecuitem,msg,QString(""),QString(""));

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
        SendControlMessage(ecuitem,msg,injectionAplicationId,injectionContextId);

        /* free message */
        dlt_message_free(&msg,0);
    }
}

void MainWindow::on_actionStore_Config_triggered()
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

void MainWindow::on_actionReset_to_Factory_Default_triggered()
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

void MainWindow::on_actionGet_Software_Version_triggered()
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

void MainWindow::on_actionGet_Local_Time_triggered()
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
            SendControlMessage(ecuitem,msg,injectionAplicationId,injectionContextId);

            /* free message */
            dlt_message_free(&msg,0);
        }
    }
}

void MainWindow::on_actionSend_Injection_triggered()
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

void MainWindow::SetApplication(EcuItem *ecuitem, QString apid, QString appdescription)
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
    ecuitem->setExpanded(1);
}

void MainWindow::SetContext(EcuItem *ecuitem, QString apid, QString ctid,QString ctdescription,int log_level,int trace_status)
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
            appitem->setExpanded(1);

            return;
        }
    }

    /* No app and no con found */
    ApplicationItem* appitem = new ApplicationItem(ecuitem);
    appitem->id = apid;
    appitem->description = QString("");
    appitem->update();
    ecuitem->addChild(appitem);
    ecuitem->setExpanded(1);
    ContextItem* conitem = new ContextItem(appitem);
    conitem->id = ctid;
    conitem->loglevel = log_level;
    conitem->tracestatus = trace_status;
    conitem->description = ctdescription;
    conitem->status = ContextItem::valid;
    conitem->update();
    appitem->addChild(conitem);
    appitem->setExpanded(1);
}

void MainWindow::on_action_Info_triggered()
{
    QMessageBox::information(0, QString("DLT Viewer"),
                             QString("Package Version : %1 %2\n").arg(PACKAGE_VERSION).arg(PACKAGE_VERSION_STATE)+
                             QString("Package Revision: %1\n\n").arg(PACKAGE_REVISION)+
                             QString("Build Date: %1\n").arg(__DATE__)+
                             QString("Build Time: %1\n\n").arg(__TIME__)+
#if (BYTE_ORDER==BIG_ENDIAN)
                             QString("Architecture: Big Endian\n\n")+
#else
                             QString("Architecture: Little Endian\n\n")+
#endif
                             QString("(C) 2010 BMW AG\n"));
}


void MainWindow::on_pluginWidget_itemSelectionChanged()
{
    QList<QTreeWidgetItem *> list = project.plugin->selectedItems();
    ui->actionPlugin_Edit->setEnabled(list.count() == 1);
}

void MainWindow::on_filterWidget_itemSelectionChanged()
{
    //QList<QTreeWidgetItem *> list = project.pfilter->selectedItems();
    //ui->actionFilter_Delete->setEnabled(list.count() == 1);
    //ui->actionFilter_Edit->setEnabled(list.count() == 1);
}

void MainWindow::on_configWidget_itemSelectionChanged()
{
    /* get selected ECU from configuration */
    EcuItem* ecuitem = 0;
    ApplicationItem* appitem = 0;
    ContextItem* conitem = 0;

    getSelectedItems(&ecuitem,&appitem,&conitem);

    ui->actionGet_Default_Log_Level->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->actionSet_Default_Log_Level->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->actionGet_Local_Time->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->actionGet_Software_Version->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->actionStore_Config->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->actionGet_Log_Info->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->actionSet_Log_Level->setEnabled(conitem && ecuitem->connected);
    ui->actionSet_All_Log_Levels->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->actionReset_to_Factory_Default->setEnabled(ecuitem && ecuitem->connected && !appitem);
    ui->actionSend_Injection->setEnabled(ecuitem && ecuitem->connected && !appitem);

    ui->actionApplication_Add->setEnabled(ecuitem && !appitem);
    ui->actionApplication_Edit->setEnabled(appitem && !conitem);
    ui->actionApplication_Delete->setEnabled(appitem && !conitem);
    ui->actionContext_Add->setEnabled(appitem && !conitem);
    ui->actionContext_Edit->setEnabled(conitem);
    ui->actionContext_Delete->setEnabled(conitem);

    ui->actionECU_Add->setEnabled(true);
    ui->actionECU_Edit->setEnabled(ecuitem && !appitem);
    ui->actionECU_Delete->setEnabled(ecuitem && !appitem);
    ui->actionECU_Delete_All_Contexts->setEnabled(ecuitem && !appitem);
    ui->actionECU_Connect->setEnabled(ecuitem && !appitem && !ecuitem->tryToConnect);
    ui->actionECU_Disconnect->setEnabled(ecuitem && !appitem && ecuitem->tryToConnect);

}

void MainWindow::autoscrollToggled(bool state)
{
    int autoScrollOld = settings.autoScroll;

    // Mapping: button to variable
    settings.autoScroll = (state?Qt::Checked:Qt::Unchecked);

    if (autoScrollOld!=settings.autoScroll)
        settings.writeSettings();
}

void MainWindow::updateScrollButton()
{
    // Mapping: variable to button
    if (settings.autoScroll == Qt::Unchecked )
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
    bmwsettings->setValue("other/recentFileList",recentFiles);
}

void MainWindow::removeCurrentFile(const QString &fileName)
{
    recentFiles.removeAll(fileName);
    updateRecentFileActions();

    // write settings
    bmwsettings->setValue("other/recentFileList",recentFiles);
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
            settings.writeSettings();

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
    bmwsettings->setValue("other/recentProjectList",recentProjects);
}

void MainWindow::removeCurrentProject(const QString &projectName)
{
    recentProjects.removeAll(projectName);
    updateRecentProjectActions();

    // write settings
    bmwsettings->setValue("other/recentProjectList",recentProjects);
}


void MainWindow::openRecentFilters()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QString fileName;

    if (action)
    {
        fileName = action->data().toString();

        if(!fileName.isEmpty() && project.LoadFilter(fileName))
        {
            filterUpdate();
            setCurrentFilters(fileName);
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
    bmwsettings->setValue("other/recentFiltersList",recentFilters);
}

void MainWindow::removeCurrentFilters(const QString &filtersName)
{
    recentFilters.removeAll(filtersName);
    updateRecentFiltersActions();

    // write settings
    bmwsettings->setValue("other/recentFiltersList",filtersName);
}


void MainWindow::setCurrentHostname(const QString &hostName)
{
    recentHostnames.removeAll(hostName);
    recentHostnames.prepend(hostName);
    while (recentHostnames.size() > MaxRecentHostnames)
        recentHostnames.removeLast();

    /* Write settings for recent hostnames*/
    bmwsettings->setValue("other/recentHostnameList",recentHostnames);
}

void MainWindow::setCurrentPort(const QString &portName)
{
    recentPorts.removeAll(portName);
    recentPorts.prepend(portName);
    while (recentPorts.size() > MaxRecentPorts)
        recentPorts.removeLast();

    /* Write settings for recent ports */
    bmwsettings->setValue("other/recentPortList",recentPorts);
}

void MainWindow::tableViewValueChanged(int value)
{
    int maximum = ((QAbstractSlider *)(ui->tableView->verticalScrollBar()))->maximum();

    if (value==maximum)
    {
        /* Only enable, if disabled */
        if (settings.autoScroll==Qt::Unchecked)
        {
            /* enable scrolling */
            autoscrollToggled(Qt::Checked);
            updateScrollButton();
        }
    }
    else
    {
        /* Only disable, if enabled */
        if (settings.autoScroll==Qt::Checked)
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
    SetDefaultLogLevel(ecuitem,ecuitem->loglevel);
    SetDefaultTraceStatus(ecuitem,ecuitem->tracestatus);
    SetVerboseMode(ecuitem,ecuitem->verbosemode);
    SetTimingPackets(ecuitem,ecuitem->timingPackets);

    if (ecuitem->sendGetLogInfo)
       GetLogInfo(ecuitem);

    /* update status */
    ecuitem->status = EcuItem::valid;
    ecuitem->update();

}

void MainWindow::stateChanged(QAbstractSocket::SocketState socketState)
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
        }
    }
}

//----------------------------------------------------------------------------
// Search functionalities
//----------------------------------------------------------------------------

void MainWindow::on_actionFind_triggered()
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

    /* first load plugins in working directory */
    pluginsDir.setPath(QDir().currentPath());
    pluginsDir.cd("plugins");
    loadPluginsPath(pluginsDir);

    /* second load plugins from user set plugins directory */
    if(settings.pluginsPath)
    {
        pluginsDir.setPath(settings.pluginsPathName);
        if(pluginsDir.exists())
        {
            loadPluginsPath(pluginsDir);
        }
    }

    /* third load plugins from system directory in linux */
    pluginsDir.setPath("/usr/share/dlt-viewer/plugins");
    if(pluginsDir.exists())
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
                    item->name = plugininterface->name();
                    item->pluginVersion = plugininterface->pluginVersion();
                    item->pluginInterfaceVersion = plugininterface->pluginInterfaceVersion();
                    item->update();

                    project.plugin->addTopLevelItem(item);

                    QDltPluginViewerInterface *pluginviewerinterface = qobject_cast<QDltPluginViewerInterface *>(plugin);
                    if(pluginviewerinterface)
                    {
                        item->pluginviewerinterface = pluginviewerinterface;
                        item->widget = item->pluginviewerinterface->initViewer();

                        if(item->widget)
                        {
                            item->dockWidget = new QDockWidget(item->name,this);
                            item->dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
                            item->dockWidget->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
                            item->dockWidget->setWidget(item->widget);
                            item->dockWidget->setObjectName(item->name);

                            addDockWidget(Qt::LeftDockWidgetArea, item->dockWidget);

                            if(item->mode != PluginItem::ModeShow)
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

    item->plugininterface->loadConfig(item->filename);

    if(item->plugincontrolinterface)
        item->plugincontrolinterface->initControl(&qcontrol);

    QStringList list = item->plugininterface->infoConfig();
    for(int num=0;num<list.size();num++) {
        item->addChild(new QTreeWidgetItem(QStringList(list.at(num))));
    }

    item->update();

    if(item->dockWidget) {
        if(item->mode == PluginItem::ModeShow) {
            item->dockWidget->show();
        }
        else {
            item->dockWidget->hide();
        }
    }
}

void MainWindow::on_actionPlugin_Edit_triggered() {
    /* get selected plugin */
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
        dlg.setName(item->name);
        dlg.setPluginVersion(item->pluginVersion);
        dlg.setPluginInterfaceVersion(item->pluginInterfaceVersion);
        dlg.setFilename(item->filename);
        dlg.setMode(item->mode);        
        if(!item->pluginviewerinterface)
            dlg.removeMode(2); // remove show mode, if no viewer plugin
        dlg.setType(item->type);
        dlg.workingDirectory = workingDirectory;
        if(dlg.exec()) {
            workingDirectory = dlg.workingDirectory;
            item->filename = dlg.getFilename();
            item->mode = dlg.getMode();
            item->type = dlg.getType();

            /* update plugin item */
            updatePlugin(item);
        }
    }
    else
        QMessageBox::warning(0, QString("DLT Viewer"),
                            QString("No Plugin selected!"));

}

void MainWindow::on_actionPlugin_Show_triggered() {

    /* get selected plugin */
    QList<QTreeWidgetItem *> list = project.plugin->selectedItems();
    if((list.count() == 1) ) {
        PluginItem* item = (PluginItem*) list.at(0);

        if(item->pluginviewerinterface)
            item->dockWidget->show();
            //item->pluginviewerinterface->showViewer();
    }
    else {
        QMessageBox::warning(0, QString("DLT Viewer"),
                            QString("No Plugin selected!"));
    }

}

void MainWindow::on_actionPlugin_Hide_triggered() {
    /* get selected plugin */
    QList<QTreeWidgetItem *> list = project.plugin->selectedItems();
    if((list.count() == 1) ) {
        PluginItem* item = (PluginItem*) list.at(0);

        if(item->pluginviewerinterface)
            item->dockWidget->hide();
            //item->pluginviewerinterface->hideViewer();

    }
    else {
        QMessageBox::warning(0, QString("DLT Viewer"),
                            QString("No Plugin selected!"));
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
    for(int num2 = 0; num2 < project.plugin->topLevelItemCount (); num2++)
    {
        PluginItem *item = (PluginItem*)project.plugin->topLevelItem(num2);

        if(item->plugindecoderinterface && item->plugindecoderinterface->isMsg(msg))
        {
            item->plugindecoderinterface->decodeMsg(msg);
            break;
        }
    }

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

        item->type = (FilterItem::FilterType)(dlg.getType());

        item->ecuId = dlg.getEcuId();
        item->applicationId = dlg.getApplicationId();
        item->contextId = dlg.getContextId();
        item->headerText = dlg.getHeaderText();
        item->payloadText = dlg.getPayloadText();

        item->enableEcuId = dlg.getEnableEcuId();
        item->enableApplicationId = dlg.getEnableApplicationId();
        item->enableContextId = dlg.getEnableContextId();
        item->enableHeaderText = dlg.getEnableHeaderText();
        item->enablePayloadText = dlg.getEnablePayloadText();
        item->enableCtrlMsgs = dlg.getEnableCtrlMsgs();
        item->enableLogLevelMax = dlg.getEnableLogLevelMax();
        item->enableLogLevelMin = dlg.getEnableLogLevelMin();

        item->filterColour = dlg.getFilterColour();
        item->setBackground(0,dlg.getFilterColour());

        item->logLevelMax = dlg.getLogLevelMax();
        item->logLevelMin = dlg.getLogLevelMin();

        /* update filter item */
        item->update();

        /* add filter to list */
        project.pfilter->addTopLevelItem(item);

        /* update filter list in DLT log file */
        filterUpdate();

        /* reload DLT log file */
        reloadLogFile();

        filterToggled(true);
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

        item->type = (FilterItem::FilterType)(dlg.getType());

        item->name = dlg.getName();

        item->ecuId = dlg.getEcuId();
        item->applicationId = dlg.getApplicationId();
        item->contextId = dlg.getContextId();
        item->headerText = dlg.getHeaderText();
        item->payloadText = dlg.getPayloadText();

        item->enableEcuId = dlg.getEnableEcuId();
        item->enableApplicationId = dlg.getEnableApplicationId();
        item->enableContextId = dlg.getEnableContextId();
        item->enableHeaderText = dlg.getEnableHeaderText();
        item->enablePayloadText = dlg.getEnablePayloadText();
        item->enableCtrlMsgs = dlg.getEnableCtrlMsgs();
        item->enableLogLevelMax = dlg.getEnableLogLevelMax();
        item->enableLogLevelMin = dlg.getEnableLogLevelMin();

        item->filterColour = dlg.getFilterColour();
        item->setBackground(0,dlg.getFilterColour());

        item->logLevelMax = dlg.getLogLevelMax();
        item->logLevelMin = dlg.getLogLevelMin();

        /* update filter item */
        item->update();

        /* add filter to list */
        project.pfilter->addTopLevelItem(item);

        /* update filter list in DLT log file */
        filterUpdate();

        /* reload DLT log file */
        reloadLogFile();

        filterToggled(true);
    }
}

void MainWindow::on_actionFilter_Save_As_triggered()
{

    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save DLT Filters"), workingDirectory, tr("DLT Filter File (*.dlf);;All files (*.*)"));

    if(!fileName.isEmpty())
    {
        project.SaveFilter(fileName);
        setCurrentFilters(fileName);
    }
}


void MainWindow::on_actionFilter_Load_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Load DLT Filter file"), workingDirectory, tr("DLT Filter Files (*.dlf);;All files (*.*)"));

    if(!fileName.isEmpty() && project.LoadFilter(fileName))
    {
        filterUpdate();
        setCurrentFilters(fileName);
    }
}

void MainWindow::on_actionFilter_Add_triggered() {
    //QTreeWidget *widget;

    /* get currently visible filter list in user interface */
    //if(ui->tabPFilter->isVisible()) {
    //    widget = project.pfilter;
    //}
    //else
    //    return;

    /* show filter dialog */
    FilterDialog dlg;

    if(dlg.exec()==1) {
        FilterItem* item = new FilterItem(0);

        item->type = (FilterItem::FilterType)(dlg.getType());

        item->name = dlg.getName();
        item->ecuId = dlg.getEcuId();
        item->applicationId = dlg.getApplicationId();
        item->contextId = dlg.getContextId();
        item->headerText = dlg.getHeaderText();
        item->payloadText = dlg.getPayloadText();

        item->enableEcuId = dlg.getEnableEcuId();
        item->enableApplicationId = dlg.getEnableApplicationId();
        item->enableContextId = dlg.getEnableContextId();
        item->enableHeaderText = dlg.getEnableHeaderText();
        item->enablePayloadText = dlg.getEnablePayloadText();
        item->enableCtrlMsgs = dlg.getEnableCtrlMsgs();
        item->enableLogLevelMax = dlg.getEnableLogLevelMax();
        item->enableLogLevelMin = dlg.getEnableLogLevelMin();

        item->filterColour = dlg.getFilterColour();
        item->setBackground(0,dlg.getFilterColour());

        item->logLevelMax = dlg.getLogLevelMax();
        item->logLevelMin = dlg.getLogLevelMin();

        /* update filter item */
        item->update();

        /* add filter to list */
        project.pfilter->addTopLevelItem(item);

        /* update filter list in DLT log file */
        filterUpdate();

        /* reload DLT log file */
        reloadLogFile();

        filterToggled(true);
    }
}

void MainWindow::on_actionFilter_Duplicate_triggered() {
    QTreeWidget *widget;

    /* get currently visible filter list in user interface */
    if(ui->tabPFilter->isVisible()) {
        widget = project.pfilter;
    }
    else
        return;

    /* get selected filter form list */
    QList<QTreeWidgetItem *> list = widget->selectedItems();
    if((list.count() == 1) ) {
        FilterItem* item = (FilterItem*) list.at(0);

        /* show filter dialog */
        FilterDialog dlg;
        dlg.setType((int)(item->type));

        dlg.setName(item->name);
        dlg.setEcuId(item->ecuId);
        dlg.setApplicationId(item->applicationId);
        dlg.setContextId(item->contextId);
        dlg.setHeaderText(item->headerText);
        dlg.setPayloadText(item->payloadText);

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

        if(dlg.exec())
        {
            FilterItem* newitem = new FilterItem(0);

            newitem->type = (FilterItem::FilterType)(dlg.getType());

            newitem->name = dlg.getName();
            newitem->ecuId = dlg.getEcuId();
            newitem->applicationId = dlg.getApplicationId();
            newitem->contextId = dlg.getContextId();
            newitem->headerText = dlg.getHeaderText();
            newitem->payloadText = dlg.getPayloadText();

            newitem->enableEcuId = dlg.getEnableEcuId();
            newitem->enableApplicationId = dlg.getEnableApplicationId();
            newitem->enableContextId = dlg.getEnableContextId();
            newitem->enableHeaderText = dlg.getEnableHeaderText();
            newitem->enablePayloadText = dlg.getEnablePayloadText();
            newitem->enableCtrlMsgs = dlg.getEnableCtrlMsgs();
            newitem->enableLogLevelMax = dlg.getEnableLogLevelMax();
            newitem->enableLogLevelMin = dlg.getEnableLogLevelMin();

            newitem->filterColour = dlg.getFilterColour();
            newitem->setBackground(0,dlg.getFilterColour());

            newitem->logLevelMax = dlg.getLogLevelMax();
            newitem->logLevelMin = dlg.getLogLevelMin();

            /* update filter item */
            newitem->update();

            /* update filter list in DLT log file */
            filterUpdate();

            /* reload DLT log file */
            reloadLogFile();
        }
    }
    else {
        QMessageBox::warning(0, QString("DLT Viewer"),
                            QString("No Filter selected!"));
    }
}

void MainWindow::on_actionFilter_Edit_triggered() {
    QTreeWidget *widget;

    /* get currently visible filter list in user interface */
    if(ui->tabPFilter->isVisible()) {
        widget = project.pfilter;
    }
    else
        return;

    /* get selected filter form list */
    QList<QTreeWidgetItem *> list = widget->selectedItems();
    if((list.count() == 1) ) {
        FilterItem* item = (FilterItem*) list.at(0);

        /* show filter dialog */
        FilterDialog dlg;
        dlg.setType((int)(item->type));

        dlg.setName(item->name);
        dlg.setEcuId(item->ecuId);
        dlg.setApplicationId(item->applicationId);
        dlg.setContextId(item->contextId);
        dlg.setHeaderText(item->headerText);
        dlg.setPayloadText(item->payloadText);

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

        if(dlg.exec())
        {
            item->type = (FilterItem::FilterType)(dlg.getType());

            item->name = dlg.getName();
            item->ecuId = dlg.getEcuId();
            item->applicationId = dlg.getApplicationId();
            item->contextId = dlg.getContextId();
            item->headerText = dlg.getHeaderText();
            item->payloadText = dlg.getPayloadText();

            item->enableEcuId = dlg.getEnableEcuId();
            item->enableApplicationId = dlg.getEnableApplicationId();
            item->enableContextId = dlg.getEnableContextId();
            item->enableHeaderText = dlg.getEnableHeaderText();
            item->enablePayloadText = dlg.getEnablePayloadText();
            item->enableCtrlMsgs = dlg.getEnableCtrlMsgs();
            item->enableLogLevelMax = dlg.getEnableLogLevelMax();
            item->enableLogLevelMin = dlg.getEnableLogLevelMin();

            item->filterColour = dlg.getFilterColour();
            switch(dlg.getType()){
            case 2:
                    item->setBackground(0,dlg.getFilterColour());
                    break;
            default:
                    item->setBackground(0,QColor(255,255,255));
                    break;
            }


            item->logLevelMax = dlg.getLogLevelMax();
            item->logLevelMin = dlg.getLogLevelMin();

            /* update filter item */
            item->update();

            /* update filter list in DLT log file */
            filterUpdate();

            /* reload DLT log file */
            reloadLogFile();
        }
    }
    else {
        QMessageBox::warning(0, QString("DLT Viewer"),
                            QString("No Filter selected!"));
    }
}

void MainWindow::on_actionFilter_Delete_triggered() {
    QTreeWidget *widget;

    /* get currently visible filter list in user interface */
    if(ui->tabPFilter->isVisible()) {
        widget = project.pfilter;
    }
    else
        return;

    /* get selected filter from list */
    QList<QTreeWidgetItem *> list = widget->selectedItems();
    if((list.count() == 1) ) {
        /* delete filter */
        delete widget->takeTopLevelItem(widget->indexOfTopLevelItem(list.at(0)));

        /* update filter list in DLT log file */
        filterUpdate();

        /* reload DLT log file */
        reloadLogFile();
    }
    else {
        QMessageBox::warning(0, QString("DLT Viewer"),
                            QString("No Filter selected!"));
    }
}

void MainWindow::on_actionFilter_Clear_all_triggered() {
    /* delete complete filter list */
    project.pfilter->clear();

    /* update filter list in DLT log file */
    filterUpdate();

    /* reload DLT log file */
    reloadLogFile();

}

void MainWindow::filterUpdate() {
    QDltFilter afilter;

    /* update all filters from filter configuration to DLT filter list */

    /* clear old filter list */
    qfile.clearFilter();

    /* iterate through all positive filters */
    for(int num = 0; num < project.pfilter->topLevelItemCount (); num++)
    {
        FilterItem *item = (FilterItem*)project.pfilter->topLevelItem(num);

        afilter.ecuid = item->ecuId;
        afilter.apid = item->applicationId;
        afilter.ctid = item->contextId;
        afilter.header = item->headerText;
        afilter.payload = item->payloadText;

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

void MainWindow::filterToggled(bool state) {

    if(state && !filterbutton->isChecked())
    {
        filterbutton->setChecked(state);
    }

    /* enable/disable filter */
    qfile.enableFilter(state?Qt::Checked:Qt::Unchecked);

    /* remove any selection made before */
    ui->tableView->selectionModel()->clear();

    /* update table of log messages */
    tableModel->size = qfile.sizeFilter();
    tableModel->modelChanged();
}


void MainWindow::on_tableView_customContextMenuRequested(QPoint pos)
{
    /* show custom pop menu  for configuration */
    QPoint globalPos = ui->tableView->mapToGlobal(pos);
    QMenu menu(ui->tableView);
    QAction *action;

    action = new QAction("&Filter Add", this);
    connect(action, SIGNAL(triggered()), this, SLOT(filterAddTable()));
    menu.addAction(action);

    menu.addSeparator();

    action = new QAction("Load Filter(s)...", this);
    connect(action, SIGNAL(triggered()), this, SLOT(on_actionFilter_Load_triggered()));
    menu.addAction(action);

    /* show popup menu */
    menu.exec(globalPos);
}

void MainWindow::on_tableView_pressed(QModelIndex index)
{

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

        if(filename.endsWith(".dlt"))
        {
            /* DLT log file dropped */
            logfileOpen(filename);
        }
        else if(filename.endsWith(".dlp"))
        {
            /* Project file dropped */
            projectfileOpen(filename);
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
