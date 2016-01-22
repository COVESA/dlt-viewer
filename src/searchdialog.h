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
 * \file searchdialog.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include "qdlt.h"
#include <QTableView>
#include <QTreeWidget>
#include <QCheckBox>
#include <QCache>

#include "searchtablemodel.h"

namespace Ui {
    class SearchDialog;
}


class SearchDialog : public QDialog
{
    Q_OBJECT

private:
    Ui::SearchDialog *ui;
    SearchTableModel *m_searchtablemodel;

    int startLine;    
    bool nextClicked;
    bool match;
    bool onceClicked;
    bool is_PayloadStartFound;
    bool is_PayloadEndFound;
    bool is_PayLoadRangeValid;
    QString payloadStart;
    QString payloadEnd;
    QString tempPayLoad;

    bool is_payLoadSearchSelected;
    bool is_TimeStampSearchSelected;
    bool is_TimeStampRangeValid;

    QHash<QString, QList <unsigned long>> cachedHistoryKey;

public:
    explicit SearchDialog(QWidget *parent = 0);
    ~SearchDialog();

    bool payLoadValidityCheck();

    void setText(QString text);
    void selectText();
    void setHeader(bool header);
    void setPayload(bool payload);
    void setCaseSensitive(bool caseSensitive);
    void setRegExp(bool regExp);
    void setMatch(bool matched);
    void setStartLine(int start);
    void setNextClicked(bool next);
    void setSearchColour(QLineEdit *lineEdit,int result);
    void setEnabledPrevious(bool enabled);
    void setSearchPosition(bool beginning);
    void setOnceClicked(bool clicked);
    void appendLineEdit(QLineEdit *lineEdit);
    void setApIDText(QString);
    void setCtIDText(QString);
    void cacheSearchHistory();
    void clearCacheHistory();
    QList < QList <unsigned long>> m_searchHistory;

    QString getText();
    bool getHeader();
    bool getPayload();
    bool getCaseSensitive();
    bool getRegExp();
    bool getSearchFromBeginning();
    bool getNextClicked();
    bool getMatch();
    bool getClicked();
    bool getOnceClicked();
    bool searchtoIndex();
    int getStartLine();
    QString getApIDText();
    QString getCtIDText();
    QString getTimeStampStart();
    QString getTimeStampEnd();

    int find();
    void findProcess(int searchLine, int searchBorder, QRegExp &searchTextRegExp,QString apID, QString ctID, QString tStart, QString tEnd, QString payloadStart, QString payloadEnd);
    bool timeStampCheck(int searchLine );
    void registerSearchTableModel(SearchTableModel *model);
    /**
     * @brief foundLine
     * @param searchLine
     * @return true, if search can be breaked here, false if it should continue
     */
    bool foundLine(int searchLine);
    void addToSearchIndex(int searchLine);
    QDltFile *file;
    QTableView *table;
    QDltPluginManager *pluginManager;
    QList<QLineEdit*> *lineEdits;
    QCheckBox *regexpCheckBox;
    QCheckBox *CheckBoxSearchtoList;

    QString getPayLoadStampStart();
    QString getPayLoadStampEnd();
    bool timeStampPayloadValidityCheck(int searchLine);
private:
    void updateColorbutton();
    void focusRow(int searchLine);

private slots:
    void on_lineEditText_textEdited(QString newText);
    void on_pushButtonPrevious_clicked();
    void on_pushButtonNext_clicked();
    void on_pushButtonColor_clicked();


    void on_checkBoxSearchIndex_toggled(bool checked);

public slots:
    void textEditedFromToolbar(QString newText);
    void findNextClicked();
    void findPreviousClicked();
    void loadSearchHistory();

signals:
    void refreshedSearchIndex();
    void addActionHistory();
    void searchProgressChanged(bool isInProgress);
};

#endif // SEARCHDIALOG_H
