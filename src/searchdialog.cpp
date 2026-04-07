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
 * \file searchdialog.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "qdltoptmanager.h"
#include "tablemodel.h"

#include <dltmessagematcher.h>

#include <QApplication>
#include <QMessageBox>
#include <QPixmap>
#include <QSettings>
#include <QSignalBlocker>
#include <QColorDialog>
#include <QAction>
#include <QPointer>
#include <QThreadPool>
#include <QThread>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

#include <mutex>

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{
    ui->setupUi(this);

    connect(&m_findAllWatcher, &QFutureWatcher<int>::finished,
            this, &SearchDialog::onFindAllFinished);

    regexpCheckBox = ui->checkBoxRegExp;
    match = false;
    startLine = -1;

    lineEdits.append(ui->lineEditSearch);
    table = nullptr;

    // at start we want to know if single step search or "fill search table mode" is active !
    bool checked = QDltSettingsManager::getInstance()->value("other/search/checkBoxSearchIndex", bool(true)).toBool();
    ui->checkBoxFindAll->setChecked(checked);

    checked = QDltSettingsManager::getInstance()->value("other/search/checkBoxHeader", bool(true)).toBool();
    ui->checkBoxHeader->setChecked(checked);

    checked = QDltSettingsManager::getInstance()->value("other/search/checkBoxCasesensitive", bool(true)).toBool();
    ui->checkBoxCaseSensitive->setChecked(checked);

    checked = QDltSettingsManager::getInstance()->value("other/search/checkBoxRegEx", bool(true)).toBool();
    ui->checkBoxRegExp->setChecked(checked);

    ui->stackedWidgetRange->setCurrentIndex(0); // default Timestamp range
    connect(ui->radioTimestamp, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked)
            ui->stackedWidgetRange->setCurrentIndex(0);
    });
    connect(ui->radioTime, &QRadioButton::toggled, this, [this] (bool checked) {
        if (checked) {
            // switch from timestamp range to time range requires time range reset
            m_timeRangeResetNeeded = true;
            ui->stackedWidgetRange->setCurrentIndex(1);
        }
    });
    // user interaction with time range edits sets need for reset to false
    connect(ui->dateTimeStart, &QDateTimeEdit::dateTimeChanged, this, [this]() {
        m_timeRangeResetNeeded = false;
    });
    connect(ui->dateTimeEnd, &QDateTimeEdit::dateTimeChanged, this, [this]() {
        m_timeRangeResetNeeded = false;
    });

    // OK button triggers find next
    connect(this, &SearchDialog::accepted, this, &SearchDialog::findNextClicked);

    fSilentMode = !QDltOptManager::getInstance()->issilentMode();

    updateColorbutton();
}

SearchDialog::~SearchDialog()
{
    // Avoid use-after-free if a background Find-All search is still running.
    if (m_findAllWatcher.isRunning())
    {
        isSearchCancelled.store(true, std::memory_order_relaxed);
        m_findAllWatcher.future().cancel();
        m_findAllWatcher.waitForFinished();
    }

    clearCacheHistory();
    delete ui;
}

void SearchDialog::selectText() {
    ui->lineEditSearch->setFocus();
    ui->lineEditSearch->selectAll();
}

void SearchDialog::setHeader(bool header) { ui->checkBoxHeader->setCheckState(header?Qt::Checked:Qt::Unchecked);}
void SearchDialog::setPayload(bool payload) { ui->checkBoxPayload->setCheckState(payload?Qt::Checked:Qt::Unchecked);}
void SearchDialog::setCaseSensitive(bool caseSensitive) { ui->checkBoxCaseSensitive->setCheckState(caseSensitive?Qt::Checked:Qt::Unchecked);}
void SearchDialog::setRegExp(bool regExp) { ui->checkBoxRegExp->setCheckState(regExp?Qt::Checked:Qt::Unchecked);}
void SearchDialog::setNextClicked(bool next){nextClicked = next;}
void SearchDialog::setMatch(bool matched){match=matched;}

void SearchDialog::setTimeRange(const QDateTime& min, const QDateTime& max) {
    ui->dateTimeStart->setDateTimeRange(min, max);
    ui->dateTimeEnd->setDateTimeRange(min, max);
    ui->dateTimeStart->setDateTime(min);
    ui->dateTimeEnd->setDateTime(max);
}

bool SearchDialog::needTimeRangeReset() const { return m_timeRangeResetNeeded; }

void SearchDialog::appendLineEdit(QLineEdit *lineEdit){ lineEdits.append(lineEdit);}

QString SearchDialog::getText() { return ui->lineEditSearch->text(); }

void SearchDialog::abortSearch()
{
    isSearchCancelled.store(true, std::memory_order_relaxed);
    if (m_findAllWatcher.isRunning())
        m_findAllWatcher.future().cancel();
}

void SearchDialog::reportProgress(int progress)
{
    emit searchProgressValueChanged(progress);
}

void SearchDialog::appendFindAllMatchesChunk(const QList<unsigned long>& entries)
{
    if (!m_searchtablemodel)
        return;

    if (entries.isEmpty())
        return;

    const bool wasEmpty = (m_searchtablemodel->get_SearchResultListSize() == 0);
    // Preserve the scan order (which matches the current filtered/sorted view).
    // Do not sort by raw msg index; that breaks ordering when the view is sorted by time/timestamp.
    m_searchtablemodel->add_SearchResultEntries(entries);

    m_findAllAddedSinceLastUiUpdate += entries.size();

    // Throttle table refreshes to keep UI responsive.
    const qint64 nowMs = m_findAllUiUpdateTimer.elapsed();
    const bool timeToUpdate = (nowMs - m_findAllLastUiUpdateMs) >= 200;
    const bool manyNewItems = m_findAllAddedSinceLastUiUpdate >= 2000;

    if (wasEmpty || timeToUpdate || manyNewItems)
    {
        m_findAllLastUiUpdateMs = nowMs;
        m_findAllAddedSinceLastUiUpdate = 0;
        emit refreshedSearchIndex();
    }
}

void SearchDialog::startParallelFindAll(QRegularExpression searchTextRegExp)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Q_UNUSED(searchTextRegExp);
    // Parallel Find-All is only enabled for Qt6+.
    return;
#else
    if (!file)
        return;

    if (m_findAllWatcher.isRunning())
    {
        isSearchCancelled.store(true, std::memory_order_relaxed);
        m_findAllWatcher.future().cancel();
        return;
    }

    isSearchCancelled.store(false, std::memory_order_relaxed);

    m_findAllUiUpdateTimer.restart();
    m_findAllLastUiUpdateMs = 0;
    m_findAllAddedSinceLastUiUpdate = 0;

    m_searchtablemodel->clear_SearchResults();
    emit refreshedSearchIndex();

    const int total = file->sizeFilter();
    if (total <= 0)
    {
        emit searchProgressChanged(false);
        return;
    }


    // Snapshot the current filter mapping once on the UI thread.
    // QDltFile's filter index isn't guaranteed thread-safe for concurrent reads.
    const bool useFilterSnapshot = file->isFilter();
    std::shared_ptr<QVector<int>> filterPositions;
    if (useFilterSnapshot)
    {
        filterPositions = std::make_shared<QVector<int>>();
        filterPositions->reserve(total);
        for (int i = 0; i < total; ++i)
            filterPositions->push_back(file->getMsgFilterPos(i));
    }

    const bool msgIdEnabled = QDltSettingsManager::getInstance()->value("startup/showMsgId", true).toBool();
    const QString msgIdFormat = QDltSettingsManager::getInstance()->value("startup/msgIdFormat", "0x%x").toString();
    const bool pluginsEnabled = QDltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool();

    // Optimization: only decode when payload search is enabled.
    const bool payloadEnabled = getPayload();
    const bool doDecode = pluginsEnabled && payloadEnabled;

    const bool headerEnabled = getHeader();
    const bool caseSensitive = getCaseSensitive();
    const bool useRegExp = getRegExp();
    const QString searchText = getText();

    const QString apid = stApid;
    const QString ctid = stCtid;

    const bool timestampRangeEnabled = (ui->radioTimestamp->isChecked() && is_TimeStampSearchSelected);
    const double tsStart = dTimeStampStart;
    const double tsStop = dTimeStampStop;
    const bool timeRangeEnabled = ui->radioTime->isChecked();
    const QDateTime timeStart = ui->dateTimeStart->dateTime();
    const QDateTime timeEnd = ui->dateTimeEnd->dateTime();

    struct Chunk {
        int begin;
        int end;
    };

    // Use a dedicated pool so Find-All doesn't consume the entire global pool.
    // Also lets us cap concurrency deterministically.
    QThreadPool* const findAllPool = []() -> QThreadPool* {
        static QThreadPool pool;
        static std::once_flag once;
        std::call_once(once, []() {
            const int ideal = QThread::idealThreadCount();
            const int capped = (ideal > 0) ? qMin(4, ideal) : 4;
            pool.setMaxThreadCount(qMax(1, capped));
            pool.setThreadPriority(QThread::NormalPriority);
        });
        return &pool;
    }();

    const int maxThreads = qMax(1, findAllPool->maxThreadCount());

    // Use more chunks than threads so some chunks complete early and we can show results sooner.
    // Keep it bounded to avoid too many tasks.
    // Also cap chunk size so any single task doesn't run for too long.
    const int maxChunkSize = 20000;
    const int baseChunks = qMin(total, maxThreads * 8);
    const int minChunksForMaxSize = (total + maxChunkSize - 1) / maxChunkSize; // ensures chunkSize <= maxChunkSize
    const int desiredChunks = qMax(baseChunks, minChunksForMaxSize);
    const int chunkSize = qMax(1, (total + desiredChunks - 1) / desiredChunks);

    QVector<Chunk> chunks;
    chunks.reserve((total + chunkSize - 1) / chunkSize);
    for (int begin = 0; begin < total; begin += chunkSize)
    {
        const int end = qMin(total - 1, begin + chunkSize - 1);
        chunks.push_back(Chunk{begin, end});
    }

    auto processed = std::make_shared<std::atomic<int>>(0);
    const QPointer<SearchDialog> dlg(this);
    QDltFile* filePtr = file;
    QDltPluginManager* pluginPtr = pluginManager;

    auto mapFn = [=](const Chunk& chunk) -> QList<unsigned long> {
        QList<unsigned long> matches;
        matches.reserve(qMax(0, chunk.end - chunk.begin + 1) / 16);

        DltMessageMatcher matcher;
        matcher.setCaseSentivity(caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
        matcher.setSearchAppId(apid);
        matcher.setSearchCtxId(ctid);
        if (timestampRangeEnabled)
            matcher.setTimestampRange(tsStart, tsStop);
        if (timeRangeEnabled)
            matcher.setTimeRange(timeStart, timeEnd);
        if (msgIdEnabled)
            matcher.setMessageIdFormat(msgIdFormat);
        matcher.setHeaderSearchEnabled(headerEnabled);
        matcher.setPayloadSearchEnabled(payloadEnabled);

        QDltMsg msg;
        QByteArray buf;

        for (int i = chunk.begin; i <= chunk.end; ++i)
        {
            if (dlg && dlg->isSearchCancelled.load(std::memory_order_relaxed))
                break;

            const int msgIndex = (filterPositions ? filterPositions->at(i) : i);
            if (msgIndex < 0)
                continue;

            buf = filePtr->getMsg(msgIndex);
            if (buf.isEmpty())
                continue;

            msg.setMsg(buf);
            msg.setIndex(msgIndex);

            if (doDecode && pluginPtr)
                pluginPtr->decodeMsg(msg, dlg ? dlg->fSilentMode : 0);

            const bool ok = useRegExp ? matcher.match(msg, searchTextRegExp)
                                      : matcher.match(msg, searchText);
            if (ok)
                matches.append(static_cast<unsigned long>(msgIndex));

            const int done = processed->fetch_add(1, std::memory_order_relaxed) + 1;
            if ((done % 2000) == 0)
            {
                const int progress = static_cast<int>(done * 100.0 / total);
                if (dlg)
                {
                    QMetaObject::invokeMethod(dlg, [dlg, progress]() {
                        if (dlg)
                            dlg->reportProgress(progress);
                    }, Qt::QueuedConnection);
                }
            }
        }

        return matches;
    };

    auto reduceFn = [dlg](int& matchCount, const QList<unsigned long>& matches) {
        matchCount += matches.size();
        if (dlg && !matches.isEmpty())
        {
            QMetaObject::invokeMethod(dlg, [dlg, matches]() {
                if (dlg)
                    dlg->appendFindAllMatchesChunk(matches);
            }, Qt::QueuedConnection);
        }
    };

    auto future = QtConcurrent::mappedReduced<int>(
        findAllPool,
        chunks,
        mapFn,
        reduceFn,
        0,
        QtConcurrent::OrderedReduce | QtConcurrent::SequentialReduce);
    m_findAllWatcher.setFuture(future);
#endif
}

void SearchDialog::onFindAllFinished()
{
    // Ensure the last batch of incremental updates is reflected.
    emit refreshedSearchIndex();

    emit searchProgressChanged(false);

    // Do not rebuild the model here; it was built incrementally during reduce.
    cacheSearchHistory();

    match = (m_searchtablemodel && m_searchtablemodel->get_SearchResultListSize() > 0);
    const int colourResult = match ? 1 : 0;

    for (int i = 0; i < lineEdits.size(); ++i)
        setSearchColour(lineEdits.at(i), colourResult);

    if (!match)
        setStartLine(-1);

}

bool SearchDialog::getHeader()
{
    return (ui->checkBoxHeader->checkState() == Qt::Checked);
}

bool SearchDialog::getPayload()
{
    return (ui->checkBoxPayload->checkState() == Qt::Checked);
}

bool SearchDialog::getRegExp()
{
    return (ui->checkBoxRegExp->checkState() == Qt::Checked);
}

bool SearchDialog::getNextClicked(){return nextClicked;}

QString SearchDialog::getApIDText(){ return ui->lineEditApld->text();}
QString SearchDialog::getCtIDText(){ return ui->lineEditCtid->text();}

QString SearchDialog::getTimeStampStart()
{
    //qDebug() << "content of start time" << ui->timeStartlineEdit->text()<< __LINE__;
    return ui->lineEditTimestampStart->text();
}

QString SearchDialog::getTimeStampEnd()
{
    //qDebug() << "content of end time" << ui->timeEndlineEdit->text() << __LINE__;
    return ui->lineEditTimestampEnd->text();
}

bool SearchDialog::getCaseSensitive()
{
    //qDebug() << "getCaseSensitive is" << ui->checkBoxCaseSensitive->checkState() << __LINE__;
    return (ui->checkBoxCaseSensitive->checkState() == Qt::Checked);
}

bool SearchDialog::searchtoIndex()
{
    //qDebug() << "searchtoIndex is" << ui->checkBoxSearchIndex->checkState() << __LINE__;
    return (ui->checkBoxFindAll->checkState() == Qt::Checked);
}


bool SearchDialog::getSearchFromBeginning()
{
    return (ui->radioButtonPosBeginning->isChecked());
}

void SearchDialog::setStartLine(long int start)
{
  startLine=start;
}

void SearchDialog::setSearchColour(QLineEdit *lineEdit,int result)
{
    QPalette palette = lineEdit->palette();
    QColor text0 = QColor(255,255,255);
    QColor text1 = QColor(0,0,0);
    QColor background0 = QColor(255,102,102);
    QColor background1 = QColor(255,255,255);

    if (QDltSettingsManager::UI_Colour::UI_Dark == QDltSettingsManager::getInstance()->uiColour)
    {
        background1 = QColor(31,31,31);
        text1 = QColor(255,255,255);
    }

    switch(result){
    case 0:
        palette.setColor(QPalette::Text,text0);
        lineEdit->setPalette(palette);
        palette.setColor(QPalette::Base,background0);
        lineEdit->setPalette(palette);
        break;
    case 1:
        palette.setColor(QPalette::Text,text1);
        lineEdit->setPalette(palette);
        palette.setColor(QPalette::Base,background1);
        lineEdit->setPalette(palette);
        break;
    }
}

void SearchDialog::focusRow(long int searchLine)
{
    TableModel *model = qobject_cast<TableModel *>(table->model());
    QModelIndex idx = model->index(searchLine, 0, QModelIndex());
    //qDebug() << "Focus row in message table window" << searchLine << __FILE__ << __LINE__;

    table->scrollTo(idx, QAbstractItemView::EnsureVisible);
    table->scrollTo(idx, QAbstractItemView::PositionAtCenter);

    model->setMarker(searchLine, highlightColor);

    model->setLastSearchIndex(searchLine);
    table->selectionModel()->clear();
    model->modelChanged();
}

int SearchDialog::find()
{
    isSearchCancelled.store(false, std::memory_order_relaxed);

    emit addActionHistory();
    QRegularExpression searchTextRegExpression;
    is_TimeStampSearchSelected = false;
    long int searchBorder;
    long int lStartLine;

    emit searchProgressChanged(true);

    if(file->sizeFilter()==0)
    {
        emit searchProgressChanged(false);
        return 0;
    }

   if( ( (match == true) || ( getSearchFromBeginning() == false )) && false == searchtoIndex() )
    {
        // single step search
        QModelIndexList list = table->selectionModel()->selection().indexes();
        if(list.count() > 0)
        {
            QModelIndex index;
            for(int num=0; num < list.count();num++)
            {
                index = list[num];
                if(index.column()==0)
                {
                    break;
                }
            }
            setStartLine(index.row());
        }
    }
   else
   {
      focusRow(-1);
   }


    if ( true == getSearchFromBeginning() )
    {
      //qDebug() << "Start from the beginning" << __LINE__;
    }
    else
    {
        if (table->selectionModel() != nullptr )
         {
          {
           if ( false == table->selectionModel()->selectedIndexes().isEmpty() )
            {
             if (table->selectionModel()->selectedIndexes().first().row() > -1)
              {
               lStartLine = table->selectionModel()->selectedIndexes().first().row();
               setStartLine( lStartLine );
              }
           }
          }
         }
    }

    searchBorder = startLine;
    if(searchBorder < 0 || searchtoIndex())
    {
        if(getNextClicked() || searchtoIndex())
        {
            searchBorder = file->sizeFilter()==0?0:file->sizeFilter()-1;
        }
        else
        {
            searchBorder = 0;
        }

    }

    if(getRegExp() == true)
    {
        searchTextRegExpression.setPattern(getText());
        if (searchTextRegExpression.isValid() == false)
        {
            if ( false == fSilentMode)
            {
            QMessageBox::warning(0, QString("Search"), QString("Invalid regular expression!"));
            }
            emit searchProgressChanged(false);
            return 1;
        }

        int options = QRegularExpression::DotMatchesEverythingOption;
        if (!getCaseSensitive())
            options |= QRegularExpression::CaseInsensitiveOption;
        searchTextRegExpression.setPatternOptions(static_cast<QRegularExpression::PatternOption>(options));
    }

    // check timestamp search pattern
    const QString timeStampStartTime = getTimeStampStart();
    const QString timeStampStopTime = getTimeStampEnd();

    if (!timeStampStartTime.isEmpty() && !timeStampStopTime.isEmpty())
    {
        dTimeStampStart = timeStampStartTime.toDouble();
        dTimeStampStop = timeStampStopTime.toDouble();
        if( (dTimeStampStop -  dTimeStampStart) >= 0 )
         {
         //qDebug() << "Timestamp search enabled" << dTimeStampStart << dTimeStampStop << __LINE__;
         is_TimeStampSearchSelected = true;
         }
        else
        {
         qDebug() << "Invalid timestamp range" << dTimeStampStart << dTimeStampStop << __LINE__;
         is_TimeStampSearchSelected = false;
         if ( false == fSilentMode)
         {
         QMessageBox::warning(0, QString("Search"), QString("Invalid timestamp range !"));
         }
         emit searchProgressChanged(false);
         return 1;
        }
    }

    // check APID and CTID search
    stApid = getApIDText();
    stCtid = getCtIDText();
    if( stApid.size() > 0 || stCtid.size() > 0 ) // so we need to consider what is given here
    {
        if( stApid.size() > 4 || stCtid.size() > 4 )
        {
            qDebug() << "Given APID or CTID exceeds limit !";
            if ( false == fSilentMode)
            {
            QMessageBox::warning(0, QString("Search"), QString("Given APID or CTID exceeds limit !"));
            }
            emit searchProgressChanged(false);
            return 2;
        }
    }

    if (searchtoIndex() == true)
    {
        // Find-All search:
        // - Qt6+: run in parallel (QtConcurrent)
        // - Qt5: run single-threaded (existing findMessages loop)
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // Run Find All on a worker thread so the UI stays responsive.
        // Completion will update the model and emit searchProgressChanged(false).
        startParallelFindAll(searchTextRegExpression);
        return 1;
    #else
        findMessages(startLine, searchBorder, searchTextRegExpression);
        emit refreshedSearchIndex();
        cacheSearchHistory();
        match = (m_searchtablemodel && m_searchtablemodel->get_SearchResultListSize() > 0);
        emit searchProgressChanged(false);
        return match ? 1 : 0;
    #endif
    }

    findMessages(startLine,searchBorder,searchTextRegExpression);

    emit searchProgressChanged(false);

    if(match == true )
    {
        return 1;
    }
    setStartLine(-1); // so we do not miss index 0 any longer ...
    return 0;
}

void SearchDialog::findMessages(long int searchLine, long int searchBorder, QRegularExpression &searchTextRegExp)
{

    QDltMsg msg;
    QByteArray buf;
    int ctr = 0;
    Qt::CaseSensitivity is_Case_Sensitive = Qt::CaseInsensitive;

    if(getCaseSensitive() == true)
    {
        is_Case_Sensitive = Qt::CaseSensitive;
    }

    m_searchtablemodel->clear_SearchResults();

    bool msgIdEnabled=QDltSettingsManager::getInstance()->value("startup/showMsgId", true).toBool();
    QString msgIdFormat=QDltSettingsManager::getInstance()->value("startup/msgIdFormat", "0x%x").toString();

    DltMessageMatcher matcher;
    matcher.setCaseSentivity(is_Case_Sensitive);
    matcher.setSearchAppId(stApid);
    matcher.setSearchCtxId(stCtid);

    if (ui->radioTimestamp->isChecked() && is_TimeStampSearchSelected) {
        matcher.setTimestampRange(dTimeStampStart, dTimeStampStop);
    }
    if (ui->radioTime->isChecked()) {
        matcher.setTimeRange(ui->dateTimeStart->dateTime(), ui->dateTimeEnd->dateTime());
    }

    if (msgIdEnabled) {
        matcher.setMessageIdFormat(msgIdFormat);
    }
    matcher.setHeaderSearchEnabled(getHeader());
    matcher.setPayloadSearchEnabled(getPayload());

    do
    {
        ctr++; // for file progress indication

        if(getNextClicked() || searchtoIndex())
        {
            searchLine++;
            if(searchLine >= file->sizeFilter())
            {
                searchLine = 0;
            }
        }
        else // go back
        {
            searchLine--;
            if(searchLine <= -1)
            {
                searchLine = file->sizeFilter()-1;
            }
        }

        // Update progress every 0.5%
        if(searchLine%1000 == 0)
        {
            QApplication::processEvents();
            if (isSearchCancelled.load(std::memory_order_relaxed)) {
                break;
            }
            emit searchProgressValueChanged(static_cast<int>(ctr * 100.0 / file->sizeFilter()));
        }

        /* get the message with the selected item id */
        buf = file->getMsgFilter(searchLine);
        msg.setMsg(buf);
        msg.setIndex(file->getMsgFilterPos(searchLine));

        /* decode the message if desired - could this call be avoided as the message is already decoded elsewhere ? */
        if(QDltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool())
        {
            //qDebug() << "Decode" << __LINE__;
            pluginManager->decodeMsg(msg, fSilentMode);
        }

        const bool matchFound = getRegExp() ? matcher.match(msg, searchTextRegExp) : matcher.match(msg, getText());
        if (!matchFound)
        {
            match = false;
            continue;
        }

        if (foundLine(searchLine))
            break;
        else
            continue;
    }
    while( searchBorder != searchLine );
}

bool SearchDialog::foundLine(long int searchLine)
{
    match = true;

    if (searchtoIndex() == true)
    {
        addToSearchIndex(searchLine);
        emit refreshedSearchIndex();
    }
    else
    {
        focusRow(searchLine); // focus the line ein message table view
        setStartLine(searchLine);
        //qDebug() << "Single line hit in  " << searchLine << __LINE__;
        return true;//found single result, and breaking here
    }
    return false;//don't break search here
}

void SearchDialog::findNextClicked()
{
    setNextClicked(true);

    // In "Find All" mode, work happens asynchronously.
    // Colour is updated on completion.
    if (searchtoIndex())
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        (void)find();
        return;
#else
        int result = find();
        for (int i = 0; i < lineEdits.size(); i++)
            setSearchColour(lineEdits.at(i), result);
        return;
#endif
    }

    int result = find();
    for(int i=0; i<lineEdits.size();i++)
        setSearchColour(lineEdits.at(i),result);
}

void SearchDialog::findPreviousClicked()
{
    setNextClicked(false);

    if (searchtoIndex())
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        (void)find();
        return;
#else
        int result = find();
        for (int i = 0; i < lineEdits.size(); i++)
            setSearchColour(lineEdits.at(i), result);
        return;
#endif
    }

    int result = find();
    for(int i=0; i<lineEdits.size();i++)
        setSearchColour(lineEdits.at(i),result);
}

void SearchDialog::on_lineEditSearch_textEdited(QString newText)
{
        {
            // block signal so that it does not trigger a setText back on lineEdits->at(0)!
            QSignalBlocker signalBlocker(lineEdits.at(1));
            lineEdits.at(1)->setText(newText);
        }
        for(int i=0; i<lineEdits.size();i++){
            if(lineEdits.at(0)->text().isEmpty())
                setSearchColour(lineEdits.at(i),1);
        }
}
void SearchDialog::textEditedFromToolbar(QString newText)
{
        {
            // block signal so that it does not trigger a setText back on lineEdits->at(1)!
            QSignalBlocker signalBlocker(lineEdits.at(0));
            lineEdits.at(0)->setText(newText);
        }
        for(int i=0; i<lineEdits.size();i++){
            if(lineEdits.at(0)->text().isEmpty())
                setSearchColour(lineEdits.at(i),1);
        }
}

void SearchDialog::on_buttonHighlightColor_clicked()
{
    QString color = QDltSettingsManager::getInstance()->value("other/searchResultColor", QString("#00AAFF")).toString();
    QColor oldColor(color);
    QColor newColor = QColorDialog::getColor(oldColor, this, "Pick color for Search Highlight");
    if(false == newColor.isValid())
    {
        // User cancelled
        return;
    }

    QDltSettingsManager::getInstance()->setValue("other/searchResultColor", newColor.name());
    updateColorbutton();
}

void SearchDialog::updateColorbutton()
{
    QString color = QDltSettingsManager::getInstance()->value("other/searchResultColor", QString("#00AAFF")).toString();
    QColor lhlColor(color);
    highlightColor = lhlColor;
    QPixmap px(12, 12);
    px.fill(highlightColor);
    ui->buttonHighlightColor->setIcon(px);
}


void SearchDialog::addToSearchIndex(long int searchLine)
{
    //qDebug() << "Add hit line to search table" << searchLine << __LINE__;
    m_searchtablemodel->add_SearchResultEntry(file->getMsgFilterPos(searchLine));    
 }

void SearchDialog::registerSearchTableModel(SearchTableModel *model)
{
    m_searchtablemodel = model;    
}


void SearchDialog::loadSearchHistory()
{
    // getting text of the action button clicked to load search history.
    QAction *action = qobject_cast<QAction *>(sender());
    QString text;
    if(action)
    {
        text = action->text();
    }

    // creating a local list to store the indexes related to the key retrieved from the cache.
    QList <unsigned long> tmp ;
    if(cachedHistoryKey.size() > 0)
    {
        tmp = cachedHistoryKey[text];

        //deleting the previous search list and adding the cached search obtained to the model.
        m_searchtablemodel->clear_SearchResults();
        for (int i = 0;i < tmp.size();i++)
        {
            m_searchtablemodel->add_SearchResultEntry(tmp.at(i));
        }
    }
    emit refreshedSearchIndex();
}

void SearchDialog::cacheSearchHistory()
{
    // if it is a new search then add all the indexes of the search to a list(m_searchHistory).
    QString searchBoxText = getText();  
    m_searchHistory.append(m_searchtablemodel->m_searchResultList);
    cachedHistoryKey.insert(searchBoxText,m_searchHistory.last());    
}

void SearchDialog::clearCacheHistory()
{
    // obtaining the list of keys stored in cache
    cachedHistoryKey.clear();
}

void SearchDialog::saveSearchHistory(QStringList& searchHistory) {
    //To save the search history
    QSettings settings("MyApp", "SearchHistory");
    settings.beginWriteArray("history");
    int count = qMin(searchHistory.size(), 20);
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        settings.setValue("entry", searchHistory.at(i));
    }
    settings.endArray();
}

void SearchDialog::loadSearchHistoryList(QStringList& searchHistory)
{
  //To retrive the search history once DLT Viewer restarts
    QSettings settings("MyApp", "SearchHistory");
    searchHistory.clear();
    int size = settings.beginReadArray("history");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        searchHistory.append(settings.value("entry").toString());
    }
    settings.endArray();
}

void SearchDialog::on_checkBoxHeader_toggled(bool checked)
{
   QDltSettingsManager::getInstance()->setValue("other/search/checkBoxHeader", checked);
}

void SearchDialog::on_checkBoxFindAll_toggled(bool checked)
{
    QDltSettingsManager::getInstance()->setValue("other/search/checkBoxSearchIndex", checked);
    setStartLine(-1);
}

void SearchDialog::on_checkBoxCaseSensitive_toggled(bool checked)
{
    QDltSettingsManager::getInstance()->setValue("other/search/checkBoxCasesensitive", checked);
}

void SearchDialog::on_checkBoxRegExp_toggled(bool checked)
{
    QDltSettingsManager::getInstance()->setValue("other/search/checkBoxRegEx", checked);
}


