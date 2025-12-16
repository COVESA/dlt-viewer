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

/**
 * @class SearchDialog
 * @brief Provides a dialog for searching messages in DLT Viewer.
 *      * Handles search parameters, search execution, result highlighting, and search history.
 */
class SearchDialog : public QDialog
{
    Q_OBJECT

public:

    /**
     * @brief Constructor for SearchDialog.
     * @param parent Parent widget.
     */
    explicit SearchDialog(QWidget *parent = 0);

    /**
     * @brief Destructor for SearchDialog.
     */
    ~SearchDialog();

    /**
     * @brief Focuses the specified row in the table.
     * @param searchLine Row number.
     */
    void focusRow(long int searchLine);

    /**
     * @brief Selects and focuses the search text field.
     */
    void selectText();
    /**
     * @brief Sets the match flag.
     * @param matched True if matched.
     */
    void setMatch(bool matched);
    /**
     * @brief Sets the start line for search.
     * @param start Line number.
     */
    void setStartLine(long int start);
    /**
     * @brief Sets the once clicked flag.
     * @param clicked True if clicked once.
     */
    void setOnceClicked(bool clicked);
    /**
     * @brief Appends a QLineEdit to the list of line edits.
     * @param lineEdit Pointer to QLineEdit.
     */
    void appendLineEdit(QLineEdit *lineEdit);
    /**
     * @brief Caches the current search history.
     */
    void cacheSearchHistory();
    /**
     * @brief Clears the cached search history.
     */
    void clearCacheHistory();
    /**
     * @brief Gets the search text.
     * @return Search text as QString.
     */
    QString getText();

    /**
     * @brief Registers the search table model.
     * @param model Pointer to SearchTableModel.
     */
    void registerSearchTableModel(SearchTableModel *model);

    QDltFile *file;
    QTableView *table;
    QDltPluginManager *pluginManager;
    QCheckBox *regexpCheckBox;

private:
    Ui::SearchDialog *ui;
    SearchTableModel *m_searchtablemodel;

    bool isSearchCancelled{false};

    long int startLine;
    long searchseconds;
    bool nextClicked;
    bool match;
    bool onceClicked;
    bool fSilentMode;
    bool is_TimeStampSearchSelected;
    bool fIs_APID_CTID_requested;

    QString TimeStampStarttime;
    QString TimeStampStoptime;
    double  dTimeStampStart;
    double  dTimeStampStop;

    QString stApid;
    QString stCtid;

    QColor highlightColor;

    QHash<QString, QList <unsigned long>> cachedHistoryKey;

    /**
     * @brief Sets the regular expression checkbox state.
     * @param regExp True to check, false to uncheck.
     */
    void setRegExp(bool regExp);
    /**
     * @brief Adds a found line to the search index.
     * @param searchLine Line number.
     */
    void addToSearchIndex(long int searchLine);
    /**
     * @brief Iterates through messages and finds matches.
     * @param searchLine Start line.
     * @param searchBorder Border line.
     * @param searchTextRegExp Regular expression for search.
     */
    void findMessages(long int searchLine, long int searchBorder, QRegularExpression &searchTextRegExp);
    /**
     * @brief Updates the color button icon.
     */
    void updateColorbutton();
    /**
     * @brief Sets the color of the search box based on result.
     * @param lineEdit Pointer to QLineEdit.
     * @param result Result code.
     */
    void setSearchColour(QLineEdit *lineEdit,int result);
    /**
     * @brief Sets the header checkbox state.
     * @param header True to check, false to uncheck.
     */
    void setHeader(bool header);
    /**
     * @brief Sets the payload checkbox state.
     * @param payload True to check, false to uncheck.
     */
    void setPayload(bool payload);
    /**
     * @brief Sets the case sensitivity checkbox state.
     * @param caseSensitive True to check, false to uncheck.
     */
    void setCaseSensitive(bool caseSensitive);
    /**
     * @brief Sets the search direction (next/previous).
     * @param next True for next, false for previous.
     */
    void setNextClicked(bool next);
    /**
     * @brief Starts timing for search duration.
     */
    void starttime(void);
    /**
     * @brief Stops timing and logs search duration.
     */
    void stoptime(void);
    /**
     * @brief Main function to perform search.
     * @return Result code.
     */
    int find();

    /**
     * @brief Checks if search should start from beginning.
     * @return True if enabled.
     */
    bool getSearchFromBeginning();
    /**
     * @brief Checks if header search is enabled.
     * @return True if enabled.
     */
    bool getHeader();
    /**
     * @brief Checks if payload search is enabled.
     * @return True if enabled.
     */
    bool getPayload();
    /**
     * @brief Checks if case sensitive search is enabled.
     * @return True if enabled.
     */
    bool getCaseSensitive();
    /**
     * @brief Checks if regular expression search is enabled.
     * @return True if enabled.
     */
    bool getRegExp();
    /**
     * @brief Gets the search direction.
     * @return True if next, false if previous.
     */
    bool getNextClicked();
    bool getClicked();
    /**
     * @brief Gets the once clicked flag
     * @return True if clicked.
     */
    bool getOnceClicked();
    /**
     * @brief Checks if search to index is enabled.
     * @return True if enabled.
     */
    bool searchtoIndex();
    /**
     * @brief Handles actions when a matching line is found.
     * @param searchLine Line number.
     * @return True to break search, false to continue.
     */
    bool foundLine(long int searchLine);
    /**
     * @brief Gets the APID text.
     * @return APID as QString.
     */
    QString getApIDText();
    /**
     * @brief Gets the CTID text.
     * @return CTID as QString.
     */
    QString getCtIDText();
    /**
     * @brief Gets the start timestamp text.
     * @return Start timestamp as QString.
     */
    QString getTimeStampStart();
    /**
     * @brief Gets the end timestamp text.
     * @return End timestamp as QString.
     */
    QString getTimeStampEnd();
    QString getPayLoadStampStart();
    QString getPayLoadStampEnd();
    QList < QList <unsigned long>> m_searchHistory;
    QList<QLineEdit*> *lineEdits;

    QCheckBox *CheckBoxSearchtoList;

private slots:
    /**
     * @brief Slot for text edited in main search box.
     * @param newText New text.
     */
    void on_lineEditText_textEdited(QString newText);
    /**
     * @brief Slot for Previous button click.
     */
    void on_pushButtonPrevious_clicked();
    /**
     * @brief Slot for Next button click.
     */
    void on_pushButtonNext_clicked();
    /**
     * @brief Slot for Color button click.
     */
    void on_pushButtonColor_clicked();
    /**
     * @brief Slot for search index checkbox toggled.
     * @param checked Checkbox state.
     */
    void on_checkBoxSearchIndex_toggled(bool checked);
    /**
     * @brief Slot for header checkbox toggled.
     * @param checked Checkbox state.
     */
    void on_checkBoxHeader_toggled(bool checked);
    /**
     * @brief Slot for case sensitivity checkbox toggled.
     * @param checked Checkbox state.
     */
    void on_checkBoxCaseSensitive_toggled(bool checked);
    /**
     * @brief Slot for regular expression checkbox toggled.
     * @param checked Checkbox state.
     */
    void on_checkBoxRegExp_toggled(bool checked);

public slots:
    /**
     * @brief Slot for text edited from toolbar.
     * @param newText New text.
     */
    void textEditedFromToolbar(QString newText);
    /**
     * @brief Slot for Find Next action.
     */
    void findNextClicked();
    /**
     * @brief Slot for Find Previous action.
     */
    void findPreviousClicked();
    /**
     * @brief Loads search history from cache.
     */
    void loadSearchHistory();
    /**
     * @brief Aborts the ongoing search.
     */
    void abortSearch();
    /**
     * @brief Saves search history to persistent storage.
     * @param searchHistory Reference to QStringList of history.
     */
    void saveSearchHistory(QStringList& searchHistory);
    /**
     * @brief Loads search history from persistent storage.
     * @param searchHistory Reference to QStringList to populate.
     */
    void loadSearchHistoryList(QStringList& searchHistory);

signals:
    void refreshedSearchIndex();
    void addActionHistory();
    void searchProgressChanged(bool isInProgress);
    void searchProgressValueChanged(int progress);
};

#endif // SEARCHDIALOG_H
