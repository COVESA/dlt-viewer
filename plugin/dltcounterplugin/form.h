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

class DltCounterPlugin;

namespace DltCounter {
    namespace Ui {
        class Form;
    }

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(DltCounterPlugin *_plugin, QWidget *parent = 0);
    ~Form();

    void handlePrevious(const QString &ctid, int missingCounter);
    void handleNext(const QString &ctid, int missingCounter);
    int findPreviousCounter(const QString &ctid, int counter);
    int findNextCounter(const QString &ctid, int counter);

private:
    Ui::Form *ui;
    DltCounterPlugin *plugin;

private slots:
    void on_CounterpushButton_clicked();
    void on_ExportpushButton_clicked();
};

} // namespace DummyCounter
#endif // FORM_H
