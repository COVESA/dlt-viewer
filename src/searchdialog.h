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
#include <QCheckBox>
#include <QColor>
#include <QDateTime>
#include <QElapsedTimer>
#include <QHash>
#include <QLineEdit>
#include <QFutureWatcher>
#include <QRegularExpression>
#include <QStringList>
#include <QTableView>

#include <atomic>

#include "searchtablemodel.h"

namespace Ui {
class SearchDialog;
}

class QDltFile;
class QDltPluginManager;

class SearchDialog : public QDialog {
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = nullptr);
    ~SearchDialog();

    void focusRow(long int searchLine);
    void selectText();
    void setMatch(bool matched);
    void setStartLine(long int start);
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

    QDltFile *file{nullptr};
    QTableView *table{nullptr};
    QDltPluginManager *pluginManager{nullptr};
    QCheckBox *regexpCheckBox{nullptr};

    void setTimeRange(const QDateTime &min, const QDateTime &max);
    bool needTimeRangeReset() const;
private:
    Ui::SearchDialog *ui{nullptr};
    SearchTableModel *m_searchtablemodel{nullptr};

    std::atomic_bool isSearchCancelled{false};
    QFutureWatcher<int> m_findAllWatcher;
    QElapsedTimer m_findAllUiUpdateTimer;
    qint64 m_findAllLastUiUpdateMs{0};
    int m_findAllAddedSinceLastUiUpdate{0};

    long int startLine{-1};
    bool nextClicked{true};
    bool match{false};
    bool fSilentMode{false};
    bool is_TimeStampSearchSelected{false};

    double  dTimeStampStart{0.0};
    double  dTimeStampStop{0.0};
    bool m_timeRangeResetNeeded{true};

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

    void startParallelFindAll(QRegularExpression searchTextRegExp);
    void reportProgress(int progress);
    void onFindAllFinished();
    void appendFindAllMatchesChunk(const QList<unsigned long>& entries);

    int find();

    bool getSearchFromBeginning();
    bool getHeader();
    bool getPayload();
    bool getCaseSensitive();
    bool getRegExp();
    bool getNextClicked();
    bool searchtoIndex();
    bool foundLine(long int searchLine);
    QString getApIDText();
    QString getCtIDText();
    QString getTimeStampStart();
    QString getTimeStampEnd();
    QList < QList <unsigned long>> m_searchHistory;
    QList<QLineEdit*> lineEdits;

private slots:
    void on_lineEditSearch_textEdited(QString newText);
    void on_buttonHighlightColor_clicked();

    void on_checkBoxFindAll_toggled(bool checked);

    void on_checkBoxHeader_toggled(bool checked);

    void on_checkBoxCaseSensitive_toggled(bool checked);

    void on_checkBoxRegExp_toggled(bool checked);

public slots:
    void textEditedFromToolbar(QString newText);
    void findNextClicked();
    void findPreviousClicked();
    void loadSearchHistory();
    void abortSearch();
    void saveSearchHistory(QStringList& searchHistory);
    void loadSearchHistoryList(QStringList& searchHistory);

signals:
    void refreshedSearchIndex();
    void addActionHistory();
    void searchProgressChanged(bool isInProgress);
    void searchProgressValueChanged(int progress);
};

#endif // SEARCHDIALOG_H
