/**
 * @licence app begin@
 * Copyright (C) 2015 Advanced Driver Information Technology
 *
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \author Christoph Lipka <clipka@jp.adit-jv.com> ADIT 2015
 *
 * \file dltlogstorageconfigcreatorplugin.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef LOGSTORAGECONFIGCREATORFORM_H
#define LOGSTORAGECONFIGCREATORFORM_H

#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QSettings>
#include "logstoragefilter.h"

namespace Ui {
class LogstorageConfigCreatorForm;
}

class LogstorageConfigCreatorForm : public QWidget
{
    Q_OBJECT

public:
    explicit LogstorageConfigCreatorForm(QWidget *parent = 0);
    ~LogstorageConfigCreatorForm();

protected:
    /* set default filter configuration */
    void setFilterDefaults();
    /* validate a certain filter configuration, will be called before added to filters list */
    bool validateFilter();
    /* remove a filter */
    void remove_filter(bool update);

private slots:
    void on_pushButton_New_clicked();
    void on_pushButton_Add_clicked();
    void on_pushButton_Delete_clicked();
    void on_pushButton_SaveFile_clicked();
    void on_pushButton_LoadFile_clicked();
    void set_button_text_Add_to_Update(int idx);
    void set_button_text_Update_to_Add();
    void load_filter(int idx);

private:
    Ui::LogstorageConfigCreatorForm *ui;

    QString filterToText(QString filter) { return QString("[" + filter + "]"); }
    /* handle to store filter configurations */
    QHash<QString, LogstorageFilter> *filters;
    int currentFilter;
};

#endif // LOGSTORAGECONFIGCREATORFORM_H
