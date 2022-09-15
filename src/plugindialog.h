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
 * \file plugindialog.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include <QDialog>
#include "workingdirectory.h"

namespace Ui {
    class PluginDialog;
}

class PluginDialog : public QDialog
{
    Q_OBJECT

public:

    explicit PluginDialog(QWidget *parent = 0);
    ~PluginDialog();

    WorkingDirectory workingDirectory;

    void setName(QString name);
    void setPluginVersion(QString version);
    void setPluginInterfaceVersion(QString version);
    void setMode(int mode);
    void removeMode(int mode);
    void setType(int type);
    void setFilename(QString filename);

    int getMode();
    int getType();
    QString getFilename();

private:
    Ui::PluginDialog *ui;

private slots:
    void on_toolButton_clicked();
};

#endif // PLUGINDIALOG_H
