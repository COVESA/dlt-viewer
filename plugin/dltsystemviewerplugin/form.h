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
#include <QTreeWidgetItem>
#include "plugininterface.h"

namespace DltSystemViewer {
namespace Ui {
    class Form;
}

class ProcessItem  : public QTreeWidgetItem
{
public:

    ProcessItem(QTreeWidgetItem *parent = 0);
    ~ProcessItem();

    unsigned int lastTimestamp;
};

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();

    void addProcesses(int pid, QString data,QDltMsg &msg);

    void setUser(QString text);
    void setNice(QString text);
    void setSystem(QString text);

    void deleteAllProccesses();

private slots:
    void on_pushButtonClear_clicked();

private:
    Ui::Form *ui;

};

} //namespace DltSystemViewer

#endif // FORM_H
