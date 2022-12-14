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
 * \file searchdialog.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QTableView>
#include <QTreeWidget>
#include <QCheckBox>
#include <QCache>

#include "searchtablemodel.h"


#if defined(_MSC_VER)
#include <io.h>
#include <time.h>
#include <WinSock.h>
#else
#include <unistd.h>     /* for read(), close() */
#include <sys/time.h>	/* for gettimeofday() */
#endif


namespace Ui {
    class SearchDialog;
}


class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = 0);
    ~SearchDialog();

    void focusRow(long int searchLine);
    void selectText();
    void setMatch(bool matched);
    void setStartLine(long int start);
    void setOnceClicked(bool clicked);
    void appendLineEdit(QLineEdit *lineEdit);
    void cacheSearchHistory();
    void clearCacheHistory();
    QString getText();

    void registerSearchTableModel(SearchTableModel *model);
    /**
     * @brief foundLine
     * @param searchLine
     * @return true, if search can be breaked here, false if it should continue
     */

    QDltFile *file;
    QTableView *table;
    QDltPluginManager *pluginManager;
    QCheckBox *regexpCheckBox;

private:
    Ui::SearchDialog *ui;
    SearchTableModel *m_searchtablemodel;

    long int startLine;
    long searchseconds;
    bool nextClicked;
    bool match;
    bool onceClicked;
    bool fSilentMode;
    bool is_PayloadStartFound;
    bool is_PayloadEndFound;
    bool is_PayLoadRangeValid;
    bool is_payLoadSearchSelected;
    bool is_TimeStampSearchSelected;
    bool is_TimeStampRangeValid;
    bool fIs_APID_CTID_requested;

    QString TimeStampStarttime;
    QString TimeStampStoptime;
    double  dTimeStampStart;
    double  dTimeStampStop;

    QString payloadStart;
    QString payloadEnd;
    QString tempPayLoad;
    QString stApid;
    QString stCtid;

    QColor highlightColor;

    QHash<QString, QList <unsigned long>> cachedHistoryKey;

    void setRegExp(bool regExp);
    void addToSearchIndex(long int searchLine);
    void findMessages(long int searchLine, long int searchBorder, QRegularExpression &searchTextRegExp);
    void updateColorbutton();
    void setSearchColour(QLineEdit *lineEdit,int result);
    void setHeader(bool header);
    void setPayload(bool payload);
    void setCaseSensitive(bool caseSensitive);
    void setNextClicked(bool next);

    void starttime(void);
    void stoptime(void);

    int find();

    bool getSearchFromBeginning();
    bool timeStampPayloadValidityCheck(long int searchLine);
    bool getHeader();
    bool getPayload();
    bool getCaseSensitive();
    bool getRegExp();
    bool getNextClicked();
    bool getClicked();
    bool getOnceClicked();
    bool searchtoIndex();
    bool foundLine(long int searchLine);
    bool payLoadValidityCheck();
    bool payLoadStartpatternCheck();
    bool payLoadStoppatternCheck();
    QString getApIDText();
    QString getCtIDText();
    QString getTimeStampStart();
    QString getTimeStampEnd();
    QString getPayLoadStampStart();
    QString getPayLoadStampEnd();
    QList < QList <unsigned long>> m_searchHistory;
    QList<QLineEdit*> *lineEdits;

    QCheckBox *CheckBoxSearchtoList;

private slots:
    void on_lineEditText_textEdited(QString newText);
    void on_pushButtonPrevious_clicked();
    void on_pushButtonNext_clicked();
    void on_pushButtonColor_clicked();


    void on_checkBoxSearchIndex_toggled(bool checked);

    void on_checkBoxHeader_toggled(bool checked);

    void on_checkBoxCaseSensitive_toggled(bool checked);

    void on_checkBoxRegExp_toggled(bool checked);

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
