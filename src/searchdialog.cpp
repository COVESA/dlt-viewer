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

#include <QMessageBox>
#include <QProgressBar>
#include <QProgressDialog>
#include <QSignalBlocker>
#include <QColorDialog>
#include <QAction>
#include <QPointer>
#include <QThreadPool>
#include <QThread>
#include <QDebug>

#include <QtConcurrent/QtConcurrent>

#if defined(__linux__)
#include <pthread.h>
#endif

#include <ctime>

#if !defined(_MSC_VER)
#include <sys/time.h>
#include <time.h>
#endif

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{
    ui->setupUi(this);

    connect(&m_findAllWatcher, &QFutureWatcher<QList<unsigned long>>::finished,
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
    // Avoid use-after-free if a parallel search is still running.
    if (m_findAllWatcher.isRunning())
    {
        isSearchCancelled.store(true, std::memory_order_relaxed);
        m_findAllWatcher.future().cancel();
        m_findAllWatcher.waitForFinished();
    }

    clearCacheHistory();
    delete ui;
}

void SearchDialog::starttime(const QString& searchTerm)
{
    m_searchTimerTerm = searchTerm;
    m_searchTimer.restart();
    m_searchTimerRunning = true;

    // Keep existing detailed performance measurement as well.
    performanceMeasure.start(searchTerm);
    searchTimer.start();

    // Store CPU time at start (in milliseconds)
#if defined(_MSC_VER)
    {
        const std::clock_t cpu = std::clock();
        searchCpuTimeStart = (cpu == static_cast<std::clock_t>(-1))
                                 ? 0
                                 : (static_cast<qint64>(cpu) * 1000) / CLOCKS_PER_SEC;
    }
#else
    struct timespec ts;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    searchCpuTimeStart = ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
#endif

    const std::time_t timestamp_sec = std::time(nullptr);
    searchseconds = static_cast<qint64>(timestamp_sec);
}

void SearchDialog::stoptime(qint64 messagesProcessed)
{
    if (!m_searchTimerRunning)
        return;

    m_searchTimerRunning = false;

    const qint64 elapsedMs = m_searchTimer.elapsed();
    const double elapsedSec = elapsedMs > 0 ? (elapsedMs / 1000.0) : 0.0;
    const double rate = (elapsedSec > 0.0) ? (messagesProcessed / elapsedSec) : 0.0;

    // Detailed performance report (existing feature)
    const QString perfReport = performanceMeasure.stop(messagesProcessed);
    if (!perfReport.isEmpty())
        qDebug().noquote() << perfReport;

    // Simple summary
    if (!m_searchTimerTerm.isEmpty())
    {
        qDebug().nospace() << "Search '" << m_searchTimerTerm << "' processed "
                          << messagesProcessed << " messages in " << elapsedMs
                          << " ms (" << rate << " msg/s)";
    }
    else
    {
        qDebug().nospace() << "Search processed " << messagesProcessed
                          << " messages in " << elapsedMs
                          << " ms (" << rate << " msg/s)";
    }

    const std::time_t timestamp_sec = std::time(nullptr);
    const qint64 temps = static_cast<qint64>(timestamp_sec);
    const qint64 dtemps = temps - searchseconds;
    qDebug() << "Time for search [s]" << dtemps;
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
    m_searchtablemodel->add_SearchResultEntriesSorted(entries);

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

void SearchDialog::startParallelFindAll(const QRegularExpression& searchTextRegExp)
{
    if (!file)
        return;

    if (m_findAllWatcher.isRunning())
    {
        isSearchCancelled.store(true, std::memory_order_relaxed);
        m_findAllWatcher.future().cancel();
        return;
    }

    isSearchCancelled.store(false, std::memory_order_relaxed);
    emit searchProgressChanged(true);

    starttime(getText());

    m_findAllUiUpdateTimer.restart();
    m_findAllLastUiUpdateMs = 0;
    m_findAllAddedSinceLastUiUpdate = 0;

    m_searchtablemodel->clear_SearchResults();
    emit refreshedSearchIndex();

    const int total = file->sizeFilter();
    if (total <= 0)
        return;

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

    const int maxThreads = qMax(1, QThreadPool::globalInstance()->maxThreadCount());
    // Use more chunks than threads so some chunks complete early and we can show results sooner.
    // Keep it bounded to avoid too many tasks.
    const int desiredChunks = qMin(total, maxThreads * 32);
    const int chunkSize = qMax(1, (total + desiredChunks - 1) / desiredChunks);

    QVector<Chunk> chunks;
    chunks.reserve((total + chunkSize - 1) / chunkSize);
    for (int begin = 0; begin < total; begin += chunkSize)
    {
        const int end = qMin(total - 1, begin + chunkSize - 1);
        chunks.push_back(Chunk{begin, end});
    }

    auto processed = std::make_shared<std::atomic<int>>(0);
    auto workerIdCounter = std::make_shared<std::atomic<int>>(0);
    // Unique token for this run (used to re-init thread_local state per new search).
    auto runToken = std::make_shared<int>(0);
    const QPointer<SearchDialog> dlg(this);
    QDltFile* filePtr = file;
    QDltPluginManager* pluginPtr = pluginManager;

    auto mapFn = [=](const Chunk& chunk) -> QList<unsigned long> {
        // Name the pooled worker thread once (useful for top -H / perf / debugging).
        thread_local int workerId = -1;
        thread_local bool threadNamed = false;
        if (!threadNamed)
        {
            workerId = workerIdCounter->fetch_add(1, std::memory_order_relaxed) + 1;
            const QString qtName = QStringLiteral("DLTSearch-%1").arg(workerId);
            QThread::currentThread()->setObjectName(qtName);
#if defined(__linux__)
            // Linux thread name is limited (typically 15 chars + NUL).
            const QByteArray osName = QStringLiteral("DLTSrch-%1").arg(workerId).toLatin1();
            pthread_setname_np(pthread_self(), osName.constData());
#endif
            threadNamed = true;
        }

        QList<unsigned long> matches;
        matches.reserve(qMax(0, chunk.end - chunk.begin + 1) / 16);

        // Reuse matcher per thread, but re-init it per new search run.
        thread_local const void* lastRunToken = nullptr;
        thread_local DltMessageMatcher matcher;
        if (lastRunToken != runToken.get())
        {
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
            lastRunToken = runToken.get();
        }

        QDltMsg msg;
        QByteArray buf;

        for (int i = chunk.begin; i <= chunk.end; ++i)
        {
            if (dlg && dlg->isSearchCancelled.load(std::memory_order_relaxed))
                break;

            buf = filePtr->getMsgFilter(i);
            msg.setMsg(buf);
            msg.setIndex(filePtr->getMsgFilterPos(i));

            if (doDecode && pluginPtr)
                pluginPtr->decodeMsg(msg, dlg ? dlg->fSilentMode : 0);

            const bool ok = useRegExp ? matcher.match(msg, searchTextRegExp)
                                      : matcher.match(msg, searchText);
            if (ok)
                matches.append(static_cast<unsigned long>(filePtr->getMsgFilterPos(i)));

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

    auto reduceFn = [dlg](QList<unsigned long>& result, const QList<unsigned long>& part) {
        result.append(part);
        if (dlg && !part.isEmpty())
        {
            // Append results incrementally in UI thread.
            QMetaObject::invokeMethod(dlg, [dlg, part]() {
                if (dlg)
                    dlg->appendFindAllMatchesChunk(part);
            }, Qt::QueuedConnection);
        }
    };

    auto future = QtConcurrent::mappedReduced<QList<unsigned long>>(
        chunks,
        mapFn,
        reduceFn,
        QList<unsigned long>{},
        QtConcurrent::UnorderedReduce | QtConcurrent::SequentialReduce);
    m_findAllWatcher.setFuture(future);
}

void SearchDialog::onFindAllFinished()
{
    // Ensure the last batch of incremental updates is reflected.
    emit refreshedSearchIndex();

    // Use total scanned count for perf reporting (more meaningful than hit count).
    stoptime(file ? file->sizeFilter() : 0);
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
         qDebug() << "Search starting at line" << startLine;
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

    //check timestamp search pattern
    TimeStampStarttime = getTimeStampStart();
    TimeStampStoptime = getTimeStampEnd();

    if( ( TimeStampStarttime.size() > 0 ) && ( TimeStampStoptime.size() > 0) )
    {
        dTimeStampStart = TimeStampStarttime.toDouble();
        dTimeStampStop = TimeStampStoptime.toDouble();
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

    //check APID and CTID search
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
        fIs_APID_CTID_requested = true;
    }
    else
    {
     fIs_APID_CTID_requested = false;
    }

    if (searchtoIndex() == true)
    {
        // Parallelize Find All across multiple worker threads.
        // Completion will update the model and emit searchProgressChanged(false).
        startParallelFindAll(searchTextRegExpression);
        return 1;
    }

    findMessages(startLine,searchBorder,searchTextRegExpression);

    emit searchProgressChanged(false);

    if (searchtoIndex() == true )
    {
        cacheSearchHistory();
        emit refreshedSearchIndex();
        //if at least one element has been found -> successful search
        if ( 0 < m_searchtablemodel->get_SearchResultListSize())
        {
            return 1;
        }
    }

    if(match == true )
    {
        return 1;
    }
    setStartLine(-1); // so we do not miss index 0 any longer ...
    return 0;
}

class ScopedTimer {
public:
    ScopedTimer() : m_start(std::chrono::high_resolution_clock::now()) {}

    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start).count();
        qDebug() << "Time for search: " << duration << " ms";
    }

private:
    std::chrono::high_resolution_clock::time_point m_start;
};

void SearchDialog::findMessages(long int searchLine, long int searchBorder, QRegularExpression &searchTextRegExp)
{

    QDltMsg msg;
    QByteArray buf;
    int ctr = 0;
    Qt::CaseSensitivity is_Case_Sensitive = Qt::CaseInsensitive;

    starttime(getText());
    QDltMsg::resetCacheStats();

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

    assert(!(is_TimeStampSearchSelected && is_TimeSearchSelected));
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
            if (isSearchCancelled) {
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
    stoptime(ctr);
    // QDltMsg::printCacheStats(static_cast<quint64>(ctr));
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

    // In "Find All" mode, work happens asynchronously (and potentially in parallel).
    // Colour is updated on completion.
    if (searchtoIndex())
    {
        (void)find();
        return;
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
        (void)find();
        return;
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


