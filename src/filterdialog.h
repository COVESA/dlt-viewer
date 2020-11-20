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
#include <QAbstractButton>
#include "project.h"
#include "qdlt.h"

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

    void setEnableRegexp_Appid(bool state);
    bool getEnableRegexp_Appid();

    void setEnableRegexp_Context(bool state);
    bool getEnableRegexp_Context();

    void setEnableRegexp_Header(bool state);
    bool getEnableRegexp_Header();

    void setEnableRegexp_Payload(bool state);
    bool getEnableRegexp_Payload();

    void setIgnoreCase_Header(bool state);
    bool getIgnoreCase_Header();

    void setIgnoreCase_Payload(bool state);
    bool getIgnoreCase_Payload();


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

    void setEnableMessageId(bool state);
    bool getEnableMessageId();
    unsigned int getMessageId_min();
    unsigned int getMessageId_max();
    void setMessageId_min(unsigned int min);
    void setMessageId_max(unsigned int max);

    void setFilterColour(QColor color);
    QString getFilterColour();


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

    void setEnableMarker(bool state);
    bool getEnableMarker();

    void setEnableRegexSearchReplace(bool state);
    bool getEnableRegexSearchReplace();

    void setRegexSearchText(const QString&);
    QString getRegexSearchText();

    void setRegexReplaceText(const QString&);
    QString getRegexReplaceText();

private:
    Ui::FilterDialog *ui;
    unsigned int msgIdMin=0;
    unsigned int msgIdMax=0;
public slots:
    void on_buttonSelectColor_clicked();
    void on_comboBoxType_currentIndexChanged(int index);
    void checkMsgIdValid(const QString&);
    void on_checkboxMessageId_stateChanged(int state);
    void on_checkRegex(const QString&);
    void validate();
private slots:
    void on_lineEditApplicationId_textEdited(const QString &arg1);
    void on_lineEditEcuId_textEdited(const QString &arg1);
    void on_lineEditContextId_textEdited(const QString &arg1);
    void on_lineEditHeaderText_textEdited(const QString &arg1);
    void on_lineEditPayloadText_textEdited(const QString &arg1);
    void on_comboBoxLogLevelMax_currentIndexChanged(int index);
    void on_comboBoxLogLevelMin_currentIndexChanged(int index);
    void on_checkBoxMarkerClicked();
    void on_pushButton_c0_clicked();
    void on_pushButton_c1_clicked();
    void on_pushButton_c2_clicked();
    void on_pushButton_c3_clicked();
    void on_pushButton_c4_clicked();
    void on_pushButton_c5_clicked();
    void on_pushButton_c6_clicked();
    void on_pushButton_c7_clicked();
    void on_pushButton_c8_clicked();
    void on_pushButton_c9_clicked();
    void on_pushButton_c10_clicked();
    void on_buttonGroup_filterType_buttonClicked( int id );

};

#endif // FILTERDIALOG_H
