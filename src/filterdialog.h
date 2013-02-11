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
 * \file filterdialog.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>
#include <QColorDialog>
#include <project.h>

namespace Ui {
    class FilterDialog;
}

class FilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilterDialog(QWidget *parent = 0);
    ~FilterDialog();

    void setType(int value);
    int getType();

    void setName(QString name);
    QString getName();

    void setActive(bool state);
    bool getEnableActive();

    void setEcuId(QString id);
    QString getEcuId();
    void setEnableEcuId(bool state);
    bool getEnableEcuId();

    void setApplicationId(QString id);
    QString getApplicationId();
    void setEnableApplicationId(bool state);
    bool getEnableApplicationId();

    void setContextId(QString id);
    QString getContextId();
    void setEnableContextId(bool state);
    bool getEnableContextId();

    void setHeaderText(QString id);
    QString getHeaderText();
    void setEnableHeaderText(bool state);
    bool getEnableHeaderText();

    void setPayloadText(QString id);
    QString getPayloadText();
    void setEnablePayloadText(bool state);
    bool getEnablePayloadText();

    void setFilterColour(QColor color);
    QColor getFilterColour();


    void setLogLevelMax(int value);
    int getLogLevelMax();
    void setEnableLogLevelMax(bool state);
    bool getEnableLogLevelMax();

    void setLogLevelMin(int value);
    int getLogLevelMin();
    void setEnableLogLevelMin(bool state);
    bool getEnableLogLevelMin();

    void setEnableCtrlMsgs(bool state);
    bool getEnableCtrlMsgs();

private:
    Ui::FilterDialog *ui;

public slots:
    void on_buttonSelectColor_clicked();
    void on_comboBoxType_currentIndexChanged(int index);
private slots:
    //void on_lineEditEcuId_editingFinished();
    void on_lineEditApplicationId_textEdited(const QString &arg1);
    void on_lineEditEcuId_textEdited(const QString &arg1);
    void on_lineEditContextId_textEdited(const QString &arg1);
    void on_lineEditHeaderText_textEdited(const QString &arg1);
    void on_lineEditPayloadText_textEdited(const QString &arg1);
    void on_comboBoxLogLevelMax_currentIndexChanged(int index);
    void on_comboBoxLogLevelMin_currentIndexChanged(int index);
};

#endif // FILTERDIALOG_H
