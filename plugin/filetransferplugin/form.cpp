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
 * \file form.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 *
 * Changes:
 * Date         Author          Description
 * 30.11.2016   Gernot Wirschal Added extra function to save single file with right mouse click
 *                              bug fix for "save all selected": in Form::itemChanged
 *                              where slectedFiles did count < 0
 *
 */

#include "form.h"
#include "ui_form.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include "imagepreviewdialog.h"
#include "textviewdialog.h"
#include <QtDebug>

using namespace FileTransferPlugin;

Form::Form(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::Form)
{
    selectedFiles=0;

    ui->setupUi(this);

    ui->treeWidget->sortByColumn(COLUMN_FILEDATE, Qt::AscendingOrder); // column/order to sort by
    ui->treeWidget->setSortingEnabled(true);             // should cause sort on add
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),this, SLOT(itemChanged(QTreeWidgetItem*,int)));
    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this, SLOT(itemDoubleClicked(QTreeWidgetItem*,int)));
    connect(ui->treeWidget->header(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(sectionInTableDoubleClicked(int)));

    connect(this, SIGNAL( additem_signal(File *)), this, SLOT(additem_slot(File *) ) );
    connect(this, SIGNAL( handleupdate_signal(QString, QString, int)), this, SLOT(updatefile_slot(QString,QString,int) ) );
    connect(this, SIGNAL( export_signal(QDir, QString *, bool * )), this, SLOT(export_slot(QDir, QString *, bool * ) ) );
    connect(this, SIGNAL( handle_errorsignal(QString,QString,QString,QString)), this, SLOT(error_slot(QString,QString,QString,QString) ) );
}

Form::~Form()
{
    delete ui;
}

void Form::setAutoSave(QString path, bool save)
{
    autosave=save;
    savepath=path;
    return;
}



QTreeWidget* Form::getTreeWidget()
{
    return ui->treeWidget;
}

void Form::sectionInTableDoubleClicked(int logicalIndex)
{
        ui->treeWidget->resizeColumnToContents(logicalIndex);
        return;
}

void Form::on_selectButton_clicked()
{
    QTreeWidgetItemIterator it(ui->treeWidget,QTreeWidgetItemIterator::NotChecked|QTreeWidgetItemIterator::NoChildren);
    while (*it) {
        File *tmp = dynamic_cast<File*>(*it);

        if (tmp != NULL)
        {
            if(tmp->isComplete())
            {
                tmp->setCheckState(COLUMN_CHECK, Qt::Checked);
            }
        }
        ++it;
    }
    return;
}


void Form::on_deselectButton_clicked()
{
    QTreeWidgetItemIterator it(ui->treeWidget,QTreeWidgetItemIterator::NoChildren );//| QTreeWidgetItemIterator::Checked);

    while (*it)
    {
        File *tmp = dynamic_cast<File*>(*it);

        if (tmp != NULL)
        {
            if(tmp->isComplete())
            {
                tmp->setCheckState(COLUMN_CHECK, Qt::Unchecked);

            }
        }
        ++it;
    }
    return;
}

void Form::itemChanged(QTreeWidgetItem* item,int i)
{

    if(i == COLUMN_CHECK)
    {
        File *tmp = dynamic_cast<File*>(item);

        if (tmp != NULL)
        {

            if(tmp->isComplete() && (tmp->checkState(COLUMN_CHECK) == Qt::Checked) )
            {
                selectedFiles++;

            }
            else
            {
                tmp->setCheckState(COLUMN_CHECK, Qt::Unchecked);
                if (--selectedFiles < 0)
                    selectedFiles = 0;

            }
        } // NULL
    } // COLUMN_CHECK
 return;
}

void Form::clearSelectedFiles()
{
    selectedFiles = 0;
}



void Form::on_saveButton_clicked()
{
    if(selectedFiles <= 0)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("No files are selected.");
        msgBox.setWindowTitle("Filtransfer Plugin");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    savetofile();
    return;
}

void Form::savetofile()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Save file to directory"), QDir::currentPath(), QFileDialog::DontResolveSymlinks);
    if(path != NULL)
    {
        QDir::setCurrent(path); // because we want to keep the last selected path to offer next time again
        QTreeWidgetItemIterator it(ui->treeWidget,QTreeWidgetItemIterator::NoChildren );
        QString text;
        QString infoText;
        QString detailedText;
        QMessageBox msgBox;
        QString absolutePath;
        QString FiletoSave;
        File *tmpfile;
        unsigned int number_of_saved_files=0;
        msgBox.setWindowTitle("Filetransfer Plugin");
        bool errorOccured = true;

        while (*it)
        {
            tmpfile = dynamic_cast<File*>(*it);
            if (tmpfile == NULL)
            {
              qDebug()<< "File handle is NULL";
              msgBox.setIcon(QMessageBox::Critical);
              text += ("Not vaild file selected");
              msgBox.setText(text);
              msgBox.setStandardButtons(QMessageBox::Ok);
              msgBox.setDefaultButton(QMessageBox::Ok);
              msgBox.exec();
            }
            else if ( !tmpfile->isComplete() && (tmpfile->checkState(COLUMN_CHECK) == Qt::Checked))
            {
              // if it is not selected we do not care ...
              FiletoSave = tmpfile->getFilename();
              qDebug()<< "File is not complete" <<  FiletoSave;
              msgBox.setIcon(QMessageBox::Critical);
              text += ("File is not complete: ");
              text += FiletoSave;
              msgBox.setText(text);
              msgBox.setStandardButtons(QMessageBox::Ok);
              msgBox.setDefaultButton(QMessageBox::Ok);
              msgBox.exec();
            }
            else if (tmpfile->checkState(COLUMN_CHECK) == Qt::Checked)
            {
                FiletoSave = tmpfile->getFilename();
                absolutePath = path+"//"+FiletoSave;
                if(!tmpfile->saveFile(absolutePath) )
                {
                    qDebug()<< "File save incomplete: " <<  FiletoSave;
                    text = ("File save incomplete");
                    infoText ="The selected file "+FiletoSave+ " was not saved to "+path+".\n";
                    detailedText = tmpfile->getFilenameOnTarget() + "\n";
                    msgBox.setIcon(QMessageBox::Critical);
                    msgBox.setText(text);
                    msgBox.setInformativeText(infoText);
                    msgBox.setDetailedText(detailedText);
                    msgBox.setStandardButtons(QMessageBox::Ok);
                    msgBox.setDefaultButton(QMessageBox::Ok);
                    msgBox.exec();
                }
                else
                {
                    qDebug()<< "Successfully saved " <<  FiletoSave;
                    number_of_saved_files++;
                    errorOccured = false; // we indicate that at least one file save was successful
                }
            }
            else // file is not marked
            {
               FiletoSave = tmpfile->getFilename();
               qDebug()<< "File " <<  FiletoSave <<  " not marked for save";
            }
            ++it;
        } // while

       if(false == errorOccured ) // no error
        {
            msgBox.setIcon(QMessageBox::Information);
            text = ("Save successful");
            infoText = QString("%1 selected files were saved to %2\n").arg(number_of_saved_files).arg(path);
            msgBox.setText(text);
            msgBox.setInformativeText(infoText);
            msgBox.setDetailedText("");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
        }
    }// if path
    else
    {
     qDebug()<< "Filesave canceled";
    }
    return; // void, but anyway ...
}


void Form::on_saveRightButtonClicked()
{
    QString FiletoSave;
    QString path = QFileDialog::getExistingDirectory(this, tr("Save file to directory"), QDir::currentPath(), QFileDialog::DontResolveSymlinks);

    if(path != NULL)
    {
        QDir::setCurrent(path); // because we want to keep the last selected path to offer next time again
        QTreeWidgetItemIterator it(ui->treeWidget,QTreeWidgetItemIterator::NoChildren );
        QMessageBox msgBox;
        QModelIndex index_to_widget = ui->treeWidget->currentIndex();
        QString text;
        QString infoText;
        QString detailedText;
        File *tmpfile;
        unsigned int index_of_selected_entry = index_to_widget.row();
        msgBox.setWindowTitle("Filetransfer Plugin");

        // so we set the pointer to actually highlighted entry below the mouse
        for ( unsigned int i=0; i < index_of_selected_entry; i++)
        {
            it++;
        }

        tmpfile = dynamic_cast<File*>(*it);
        FiletoSave = tmpfile->getFilename();

        if (tmpfile != NULL && tmpfile->isComplete() )
            {
                QString absolutePath = path+"//"+FiletoSave;
                qDebug()<< "Try to save " << FiletoSave << "to " << absolutePath;

                if(!tmpfile->saveFile(absolutePath) )
                {
                    text = ("File save incomplete");
                    infoText = FiletoSave;
                    infoText += " was not saved to "+path+".\n";
                    detailedText += tmpfile->getFilenameOnTarget() + "\n";
                    msgBox.setIcon(QMessageBox::Critical);
               }
               else
               {
                 msgBox.setIcon(QMessageBox::Information);
                 text = ("File save successful");
                 infoText = FiletoSave+"\n";
                 infoText += ("was saved to \n "+path+"\n");
                 qDebug()<< "Saved " << FiletoSave;
               }

            } // if tmp != NULL
            else
            {
               msgBox.setIcon(QMessageBox::Information);
               text = ("Not vaild file selected");
               qDebug()<< "Not vaild file selected";
            }
            msgBox.setText(text);
            msgBox.setInformativeText(infoText);
            msgBox.setDetailedText(detailedText);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
    } // path = null
    return;
}


void Form::itemDoubleClicked ( QTreeWidgetItem * item, int)
{

    File *tmp = dynamic_cast<File*>(item);
    if (tmp != NULL && tmp->isComplete())
    {
        ImagePreviewDialog img(tmp->getFilenameOnTarget(),tmp->getFileData(),this);
        if(img.isFileSupported())
        {
            img.exec();

        }
        else
        {
            TextviewDialog text(tmp->getFilenameOnTarget(),tmp->getFileData(),this);
            text.exec();
            //QMessageBox::information(this,"Image Preview","Could not open file for preview. File format not supported.");

        }
        tmp->freeFile();
    }
    return;
}

void Form::on_treeWidget_customContextMenuRequested(QPoint pos)
{
    /* show custom popup menu for configuration */
    QPoint globalPos = ui->treeWidget->mapToGlobal(pos);
    QMenu menu(ui->treeWidget);
    QAction *action;
    QList<QTreeWidgetItem *> list = ui->treeWidget->selectedItems();

    if(list.count() == 1)
    {
        action = new QAction("&Save this file", this);
        connect(action, SIGNAL(triggered()), this, SLOT(on_saveRightButtonClicked()));
        menu.addAction(action);

        /* show popup menu */
        menu.exec(globalPos);
    }
    return;
}

void Form::on_actionSave_triggered()
{
    QList<QTreeWidgetItem *> list = ui->treeWidget->selectedItems();
    if((list.count() == 1))
    {
        on_deselectButton_clicked();
        File* tmpFile = (File*)list.at(0);
        itemChanged(tmpFile,COLUMN_CHECK);
        on_saveButton_clicked();
        itemChanged(tmpFile,COLUMN_CHECK);
    }
    return;
}

void Form::updatefile_slot(QString filestring, QString packetnumber, int index)
{
    QList<QTreeWidgetItem *> result = getTreeWidget()->findItems(filestring,Qt::MatchExactly | Qt::MatchRecursive,COLUMN_FILEID);

    if(result.isEmpty())
    {
        //Transfer for this file started before sending FLST
      return;
    }
    else
    {
        File *file = (File*)result.at(0);
        if(false == file->isComplete())
        {
           file->setQFileIndexForPackage(packetnumber,index);
        }
        else
        {
          qDebug() << "Received file" << file->getFilename();
          file->setComplete();
          if ( true == autosave )
          {
           QString path = savepath +"//"+ file->getFilename();
           if ( false == file->saveFile(path) )
            {
               qDebug() << "ERROR saving" << path << __LINE__ << __FILE__;
            }
           else
            {
               qDebug() << "Auto - saved" << path;
            }
          }
        }
    }
}


void Form::additem_slot(File *f)
{

    QList<QTreeWidgetItem *> result = getTreeWidget()->findItems(f->getFileSerialNumber(),Qt::MatchExactly | Qt::MatchRecursive,COLUMN_FILEID);
    if(true == result.isEmpty())
     {
      getTreeWidget()->addTopLevelItem(f);
     }
    else
    {
    int index = getTreeWidget()->indexOfTopLevelItem(result.at(0));
    getTreeWidget()->takeTopLevelItem(index);
    getTreeWidget()->addTopLevelItem(f);
    }
}


void Form::error_slot(QString filename, QString errorCode1, QString errorCode2, QString time)
{
    File *file= new File(0);
    QList<QTreeWidgetItem *> result = getTreeWidget()->findItems(filename,Qt::MatchExactly | Qt::MatchRecursive,COLUMN_FILENAME);

    if(result.isEmpty())
    {
       getTreeWidget()->addTopLevelItem(file);
    }
    else
    {
       file = (File*)result.at(0);
       if ( NULL != file )
       {
       int index = getTreeWidget()->indexOfTopLevelItem(result.at(0));
       getTreeWidget()->takeTopLevelItem(index);
       getTreeWidget()->addTopLevelItem(file);
       }
    }

    file->errorHappens(filename,errorCode1,errorCode2,time);
    file->setFlags(Qt::NoItemFlags );
}

void Form::export_slot(QDir dir, QString *errorText, bool *success)
{
    QTreeWidgetItemIterator it(getTreeWidget(),QTreeWidgetItemIterator::NoChildren );
    unsigned int countit = 0;

    if(NULL == *it)
    {
        *errorText = " - No filetransfer files in the loaded DLT file.";
        *success = false;
        return;
    }
    while (*it)
    {
        File *tmp = dynamic_cast<File*>(*it);
        if (tmp != NULL && tmp->isComplete())
        {
            QString absolutePath = dir.filePath(tmp->getFilename());

            if(false == tmp->saveFile(absolutePath) )
            {
                *success  = false;
                qDebug() << "Error: " << absolutePath;
                *errorText += ", " + tmp->getFilenameOnTarget();
            }
            else
            {
                qDebug() << "Exported: " << absolutePath;
            }
        }
        ++it;
        countit++;
    }
    *success  = true;
    qDebug() << "Amount of exported files:" << countit;
}


