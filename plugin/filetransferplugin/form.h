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
 * \file form.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QTreeWidget>
#include "file.h"

namespace FileTransferPlugin {
    namespace Ui {
        class Form;
    }


class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();
    QTreeWidget* getTreeWidget();
    void clearSelectedFiles();
    void setAutoSave(QString path, bool autosave);

signals:
    void additem_signal(File *f);
    void export_signal(QDir dir, QString *errorText, bool *success);
    void handleupdate_signal(QString filestring, QString packetnumber, int index);
    void err_signal(QDltMsg *msg);
    void handle_errorsignal(QString filesname, QString errorCode1, QString errorCode2, QString time);


private:
    Ui::Form *ui;
    int selectedFiles=0;
    bool autosave=false;
    QString savepath="";

public slots:
    void itemChanged(QTreeWidgetItem* item,int);
    void itemDoubleClicked ( QTreeWidgetItem * item, int column );

private slots:
    void sectionInTableDoubleClicked(int logicalIndex);
    void on_treeWidget_customContextMenuRequested(QPoint pos);
    void on_actionSave_triggered();
    void on_saveButton_clicked();
    void on_selectButton_clicked();
    void on_deselectButton_clicked();
    void savetofile();
    void on_saveRightButtonClicked();
    void additem_slot(File *f);
    void updatefile_slot(QString filestring, QString packetnumber, int index);
    void export_slot(QDir dir, QString *errorText, bool *success);
    void error_slot(QString filesname, QString errorCode1, QString errorCode2, QString time);
};

}  //namespace FileTransferPlugin

#endif // FORM_H
