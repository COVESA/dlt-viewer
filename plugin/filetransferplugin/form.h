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
 * \file form.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QTreeWidget>
#include "file.h"
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

private:
    Ui::Form *ui;

    int selectedFiles;

public slots:
    void itemChanged(QTreeWidgetItem* item,int);
    void itemDoubleClicked ( QTreeWidgetItem * item, int column );
    void sectionInTableDoubleClicked(int logicalIndex);

private slots:
    void on_treeWidget_customContextMenuRequested(QPoint pos);
    void on_actionDelete_triggered();
    void on_actionSave_triggered();
    void on_clearAllButton_clicked();
    void on_saveButton_clicked();
    void on_selectButton_clicked();
    void on_deselectButton_clicked();
};

#endif // FORM_H
