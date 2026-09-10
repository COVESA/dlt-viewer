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
 * \file SearchDialog.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "qdltoptmanager.h"
#include "qdltfile.h"
#include "tablemodel.h"
#include "decodecacheservice.h"

#include <dltmessagematcher.h>

#include <QApplication>
#include <QMessageBox>
#include <QPixmap>
#include <QSettings>
#include <QSignalBlocker>
#include <QColorDialog>
#include <QAction>
#include <QPointer>
#include <QDebug>
#include <QThread>
#include <QThreadPool>
#include <QThreadStorage>
#include <QtConcurrent/QtConcurrent>

#include <cstdint>
#include <limits>
#include <mutex>
#include <utility>

namespace {

class SearchProjectionSnapshot
{
public:
    explicit SearchProjectionSnapshot(const QDltFile *file)
        : m_fileSize(file ? file->size() : 0),
          m_identityProjection(!file || !file->isFilter())
    {
        if (file && !m_identityProjection)
            m_filteredRows = file->getIndexFilter();
    }

    int size() const
    {
        return m_identityProjection ? m_fileSize : m_filteredRows.size();
    }

    int globalIndexAt(int row) const
    {
        if (row < 0 || row >= size())
            return -1;

        const qint64 globalIndex = m_identityProjection
            ? static_cast<qint64>(row)
            : m_filteredRows.at(row);
        if (globalIndex < 0 || globalIndex >= m_fileSize)
            return -1;

        return static_cast<int>(globalIndex);
    }

private:
    int m_fileSize{0};
    bool m_identityProjection{true};
    QVector<qint64> m_filteredRows;
};

struct SearchWorkChunk { int begin{0}; int end{0}; };

struct LoadedSearchMessage
{
    std::uint64_t index;
    QDltMsg message;
};

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
static QThreadPool &findAllThreadPool()
{
    static QThreadPool pool;
    static std::once_flag configured;
    std::call_once(configured, []() {
        pool.setMaxThreadCount(qMax(1, qMin(4, QThread::idealThreadCount())));
    });
    return pool;
}
#endif

} // namespace

CSearchDialog::CSearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{
    ui->setupUi(this);

    connect(&m_findAllWatcher, &QFutureWatcher<std::vector<std::uint64_t>>::finished,
            this, &CSearchDialog::onFindAllFinished);

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
    connect(this, &CSearchDialog::accepted, this, &CSearchDialog::findNextClicked);

    fSilentMode = !QDltOptManager::getInstance()->issilentMode();

    updateColorbutton();
}

CSearchDialog::~CSearchDialog()
{
    // Wait for workers so lambdas cannot outlive this dialog or its UI state.
    if (m_findAllWatcher.isRunning())
    {
        isSearchCancelled.store(true, std::memory_order_relaxed);
        m_findAllWatcher.future().cancel();
        m_findAllWatcher.waitForFinished();
    }

    clearCacheHistory();
    delete ui;
}

void CSearchDialog::selectText() {
    ui->lineEditSearch->setFocus();
    ui->lineEditSearch->selectAll();
}

void CSearchDialog::setHeader(bool header) { ui->checkBoxHeader->setCheckState(header?Qt::Checked:Qt::Unchecked);}
void CSearchDialog::setPayload(bool payload) { ui->checkBoxPayload->setCheckState(payload?Qt::Checked:Qt::Unchecked);}
void CSearchDialog::setCaseSensitive(bool caseSensitive) { ui->checkBoxCaseSensitive->setCheckState(caseSensitive?Qt::Checked:Qt::Unchecked);}
void CSearchDialog::setRegExp(bool regExp) { ui->checkBoxRegExp->setCheckState(regExp?Qt::Checked:Qt::Unchecked);}
void CSearchDialog::setNextClicked(bool next){nextClicked = next;}
void CSearchDialog::setMatch(bool matched){match=matched;}

void CSearchDialog::setTimeRange(const QDateTime& min, const QDateTime& max) {
    ui->dateTimeStart->setDateTimeRange(min, max);
    ui->dateTimeEnd->setDateTimeRange(min, max);
    ui->dateTimeStart->setDateTime(min);
    ui->dateTimeEnd->setDateTime(max);
}

bool CSearchDialog::needTimeRangeReset() const { return m_timeRangeResetNeeded; }

void CSearchDialog::appendLineEdit(QLineEdit *lineEdit){ lineEdits.append(lineEdit);}

QString CSearchDialog::getText() { return ui->lineEditSearch->text(); }

void CSearchDialog::invalidateDecodeCache()
{
    if (m_decodeCacheService)
        m_decodeCacheService->clearForFile(file);
    if (m_searchtablemodel)
        m_searchtablemodel->invalidateDecodeCache();
}

void CSearchDialog::abortSearch()
{
    isSearchCancelled.store(true, std::memory_order_relaxed);
    if (m_findAllWatcher.isRunning())
    {
        m_findAllWatcher.future().cancel();
        m_findAllWatcher.waitForFinished();
    }
}

void CSearchDialog::reportProgress(int progress)
{
    emit searchProgressValueChanged(progress);
}

void CSearchDialog::publishPartialMatches(const std::vector<std::uint64_t> &matches)
{
    if (matches.empty() || !m_searchtablemodel)
        return;

    m_searchtablemodel->add_SearchResultEntries(matches);
    emit refreshedSearchIndex();
}

void CSearchDialog::startParallelFindAll(QRegularExpression searchTextRegExp)
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

    // Use an implicitly shared projection so workers get a stable view without a UI-thread copy.
    const SearchProjectionSnapshot projection(file);
    const int total = projection.size();

    if (total <= 0)
    {
        emit searchProgressChanged(false);
        return;
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
    const DltMessageMatcher::Pattern searchPattern = useRegExp
        ? DltMessageMatcher::Pattern{searchTextRegExp}
        : DltMessageMatcher::Pattern{searchText};
    const auto createMatcher = [=]() {
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
        return matcher;
    };

    // QDltFile I/O is guarded by mutexQDlt; setMsg/toStringX work on local copies —
    // parallel workers are safe when plugin decode is not needed.
    // Fall back to single-thread when doDecode=true (plugins are not thread-safe).
    const QPointer<CSearchDialog> dlg(this);
    QDltFile* filePtr = file;
    const bool dltv2Support = filePtr->getDLTv2Support();
    QDltPluginManager* pluginPtr = pluginManager;
    CDecodeCacheService* decodeCache = m_decodeCacheService;
    QThreadPool *findAllPool = &findAllThreadPool();

    auto future = QtConcurrent::run([=]() -> std::vector<std::uint64_t> {
        if (!doDecode)
        {
            // Parallel path: each worker does its own file read + parse + match.
            const int workerCount = findAllPool->maxThreadCount();
            const int maxChunkSize = 20000;
            const int chunkCount = qMax(1, qMax(qMin(total, workerCount * 8),
                                                 (total + maxChunkSize - 1) / maxChunkSize));
            const int chunkSize  = qMax(1, (total + chunkCount - 1) / chunkCount);

            QVector<SearchWorkChunk> chunks;
            chunks.reserve(chunkCount);
            for (int b = 0; b < total; b += chunkSize)
                chunks.push_back({b, qMin(b + chunkSize, total)});

            std::atomic<int> completedChunks{0};
            const int nChunks = chunks.size();

            auto mapChunk = [=, &completedChunks](const SearchWorkChunk &chunk) -> std::vector<std::uint64_t> {
                QFile workerReader;
                QDltMsg msg;
                DltMessageMatcher matcher = createMatcher();
                std::vector<std::uint64_t> localMatches;
                localMatches.reserve(qMax(1, (chunk.end - chunk.begin) / 16));

                for (int i = chunk.begin; i < chunk.end; ++i)
                {
                    if (dlg && dlg->isSearchCancelled.load(std::memory_order_relaxed))
                        break;

                    const int msgIndex = projection.globalIndexAt(i);
                    if (msgIndex < 0)
                        continue;

                    const QByteArray messageBytes = filePtr->getMsg(msgIndex, workerReader);
                    if (messageBytes.isEmpty())
                        continue;
                    if (!msg.setMsg(messageBytes, true, dltv2Support)
                        && (dltv2Support || !msg.setMsg(messageBytes, true, true)))
                        continue;
                    msg.setIndex(msgIndex);

                    if (matcher.match(msg, searchPattern))
                        localMatches.push_back(static_cast<std::uint64_t>(msgIndex));
                }

                const int done = completedChunks.fetch_add(1, std::memory_order_relaxed) + 1;
                const int progress = done * 99 / nChunks;
                if (dlg)
                {
                    QMetaObject::invokeMethod(dlg, [dlg, progress]() {
                        if (dlg) dlg->reportProgress(progress);
                    }, Qt::QueuedConnection);
                    // Show matches as each chunk completes, instead of waiting for the whole search to finish.
                    if (!localMatches.empty())
                    {
                        QMetaObject::invokeMethod(dlg, [dlg, batch = localMatches]() {
                            if (dlg) dlg->publishPartialMatches(batch);
                        }, Qt::QueuedConnection);
                    }
                }

                return localMatches;
            };

            auto reduceMatches = [](std::vector<std::uint64_t> &result,
                                    const std::vector<std::uint64_t> &chunk) {
                result.insert(result.end(), chunk.begin(), chunk.end());
            };

            auto matches = QtConcurrent::blockingMappedReduced<std::vector<std::uint64_t>>(
                findAllPool, chunks, mapChunk, reduceMatches,
                QtConcurrent::OrderedReduce | QtConcurrent::SequentialReduce);

            if (dlg)
                QMetaObject::invokeMethod(dlg, [dlg]() {
                    if (dlg) dlg->reportProgress(100);
                }, Qt::QueuedConnection);
            return matches;
        }

        // File loading and parsing are parallel; shared decoder plugins remain serialized.
        std::vector<std::uint64_t> matches;
        matches.reserve(qMax(1, total / 16));
        const int workerCount = findAllPool->maxThreadCount();
        const int maxChunkSize = 20000;
        const int chunkCount = qMax(1, qMax(qMin(total, workerCount * 8),
                                             (total + maxChunkSize - 1) / maxChunkSize));
        const int chunkSize = qMax(1, (total + chunkCount - 1) / chunkCount);
        QThreadStorage<QFile*> workerReaders;

        for (int begin = 0; begin < total; begin += chunkSize)
        {
            const int end = qMin(begin + chunkSize, total);
            QVector<int> rows;
            rows.reserve(end - begin);
            for (int row = begin; row < end; ++row)
                rows.push_back(row);

            auto loadMessage = [=, &workerReaders](int row) -> LoadedSearchMessage {
                const int msgIndex = projection.globalIndexAt(row);
                if (msgIndex < 0)
                    return {std::numeric_limits<std::uint64_t>::max(), {}};

                QFile *workerReader = workerReaders.localData();
                if (!workerReader)
                {
                    workerReader = new QFile;
                    workerReaders.setLocalData(workerReader);
                }

                const QByteArray messageBytes = filePtr->getMsg(msgIndex, *workerReader);
                if (messageBytes.isEmpty())
                    return {std::numeric_limits<std::uint64_t>::max(), {}};

                QDltMsg message;
                if (!message.setMsg(messageBytes, true, dltv2Support)
                    && (dltv2Support || !message.setMsg(messageBytes, true, true)))
                    return {std::numeric_limits<std::uint64_t>::max(), {}};
                message.setIndex(msgIndex);
                return {static_cast<std::uint64_t>(msgIndex), std::move(message)};
            };

            auto appendLoaded = [](std::vector<LoadedSearchMessage> &result,
                                   const LoadedSearchMessage &loaded) {
                if (loaded.index != std::numeric_limits<std::uint64_t>::max())
                    result.push_back(loaded);
            };

            const auto loadedMessages = QtConcurrent::blockingMappedReduced<std::vector<LoadedSearchMessage>>(
                findAllPool, rows, loadMessage, appendLoaded,
                QtConcurrent::OrderedReduce | QtConcurrent::SequentialReduce);

            DltMessageMatcher matcher = createMatcher();
            const std::size_t matchesBeforeChunk = matches.size();
            for (const LoadedSearchMessage &loaded : loadedMessages)
            {
                if (dlg && dlg->isSearchCancelled.load(std::memory_order_relaxed))
                    break;

                QDltMsg message = loaded.message;
                decodeCache->decode(pluginPtr, dlg ? dlg->fSilentMode : 0, message);
                if (matcher.match(message, searchPattern))
                    matches.push_back(loaded.index);
            }

            if (dlg)
            {
                const int progress = qMin(99, end * 99 / total);
                QMetaObject::invokeMethod(dlg, [dlg, progress]() {
                    if (dlg) dlg->reportProgress(progress);
                }, Qt::QueuedConnection);
                // Show matches as each chunk completes, instead of waiting for the whole search to finish.
                if (matches.size() > matchesBeforeChunk)
                {
                    std::vector<std::uint64_t> newMatches(matches.begin() + matchesBeforeChunk, matches.end());
                    QMetaObject::invokeMethod(dlg, [dlg, newMatches]() {
                        if (dlg) dlg->publishPartialMatches(newMatches);
                    }, Qt::QueuedConnection);
                }
            }

            if (dlg && dlg->isSearchCancelled.load(std::memory_order_relaxed))
                break;
        }

        if (dlg)
            QMetaObject::invokeMethod(dlg, [dlg]() {
                if (dlg) dlg->reportProgress(100);
            }, Qt::QueuedConnection);
        return matches;
    });

    m_findAllWatcher.setFuture(future);
#endif
}

void CSearchDialog::onFindAllFinished()
{
    std::vector<std::uint64_t> matches;
    if (!m_findAllWatcher.isCanceled())
        matches = m_findAllWatcher.future().result();

    // Replace whatever was shown incrementally with the authoritative, correctly-ordered result.
    if (m_searchtablemodel)
    {
        m_searchtablemodel->clear_SearchResults();
        if (!matches.empty())
            m_searchtablemodel->add_SearchResultEntries(matches);
    }

    // Ensure the last batch of incremental updates is reflected.
    emit refreshedSearchIndex();

    emit searchProgressChanged(false);

    // Do not rebuild the model here; it was built incrementally during reduce.
    cacheSearchHistory();

    match = !matches.empty();
    const int colourResult = match ? 1 : 0;

    for (int i = 0; i < lineEdits.size(); ++i)
        setSearchColour(lineEdits.at(i), colourResult);

    if (!match)
        setStartLine(-1);

}

bool CSearchDialog::getHeader()
{
    return (ui->checkBoxHeader->checkState() == Qt::Checked);
}

bool CSearchDialog::getPayload()
{
    return (ui->checkBoxPayload->checkState() == Qt::Checked);
}

bool CSearchDialog::getRegExp()
{
    return (ui->checkBoxRegExp->checkState() == Qt::Checked);
}

bool CSearchDialog::getNextClicked(){return nextClicked;}

QString CSearchDialog::getApIDText(){ return ui->lineEditApld->text();}
QString CSearchDialog::getCtIDText(){ return ui->lineEditCtid->text();}

QString CSearchDialog::getTimeStampStart()
{
    //qDebug() << "content of start time" << ui->timeStartlineEdit->text()<< __LINE__;
    return ui->lineEditTimestampStart->text();
}

QString CSearchDialog::getTimeStampEnd()
{
    //qDebug() << "content of end time" << ui->timeEndlineEdit->text() << __LINE__;
    return ui->lineEditTimestampEnd->text();
}

bool CSearchDialog::getCaseSensitive()
{
    //qDebug() << "getCaseSensitive is" << ui->checkBoxCaseSensitive->checkState() << __LINE__;
    return (ui->checkBoxCaseSensitive->checkState() == Qt::Checked);
}

bool CSearchDialog::searchtoIndex()
{
    //qDebug() << "searchtoIndex is" << ui->checkBoxSearchIndex->checkState() << __LINE__;
    return (ui->checkBoxFindAll->checkState() == Qt::Checked);
}


bool CSearchDialog::getSearchFromBeginning()
{
    return (ui->radioButtonPosBeginning->isChecked());
}

void CSearchDialog::setStartLine(long int start)
{
  startLine=start;
}

void CSearchDialog::setSearchColour(QLineEdit *lineEdit,int result)
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

void CSearchDialog::focusRow(long int searchLine)
{
    CTableModel *model = qobject_cast<CTableModel *>(table->model());
    QModelIndex idx = model->index(searchLine, 0, QModelIndex());
    //qDebug() << "Focus row in message table window" << searchLine << __FILE__ << __LINE__;

    table->scrollTo(idx, QAbstractItemView::EnsureVisible);
    table->scrollTo(idx, QAbstractItemView::PositionAtCenter);

    model->setMarker(searchLine, highlightColor);

    model->setLastSearchIndex(searchLine);
    table->selectionModel()->clear();
    model->refreshVisualData();
}

int CSearchDialog::find()
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
        // Find-All uses QtConcurrent on Qt6 and the existing single-threaded loop on Qt5.
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // Run Find All on a worker thread so the UI stays responsive.
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

void CSearchDialog::findMessages(long int searchLine, long int searchBorder, QRegularExpression &searchTextRegExp)
{
    QDltMsg msg;
    int ctr = 0;
    Qt::CaseSensitivity is_Case_Sensitive = Qt::CaseInsensitive;

    if(getCaseSensitive() == true)
    {
        is_Case_Sensitive = Qt::CaseSensitive;
    }

    m_searchtablemodel->clear_SearchResults();

    const SearchProjectionSnapshot projection(file);
    const int filteredSize = projection.size();

    if (filteredSize == 0)
    {
        return;
    }

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
    const bool decodeEnabled = QDltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool();

    do
    {
        ctr++; // for file progress indication

        if(getNextClicked() || searchtoIndex())
        {
            searchLine++;
            if(searchLine >= filteredSize)
            {
                searchLine = 0;
            }
        }
        else // go back
        {
            searchLine--;
            if(searchLine <= -1)
            {
                searchLine = filteredSize-1;
            }
        }

        // Update progress every 0.5%
        if(searchLine%1000 == 0)
        {
            QApplication::processEvents();
            if (isSearchCancelled.load(std::memory_order_relaxed)) {
                break;
            }
            const int siPercent = static_cast<int>(ctr * 100.0 / filteredSize);
            emit searchProgressValueChanged(siPercent);
        }

        if(searchLine < 0 || searchLine >= filteredSize)
        {
            continue;
        }

        const int globalIndex = projection.globalIndexAt(static_cast<int>(searchLine));
        if(globalIndex < 0)
        {
            continue;
        }

        if(!m_decodeCacheService->message(file,
                                         pluginManager,
                                         globalIndex,
                                         decodeEnabled,
                                         fSilentMode,
                                         msg,
                                         true))
        {
            continue;
        }

        const bool matchFound = getRegExp() ? matcher.match(msg, searchTextRegExp) : matcher.match(msg, getText());
        if (!matchFound)
        {
            match = false;
            continue;
        }

        if (foundLine(searchLine, globalIndex))
            break;
        else
            continue;
    }
    while( searchBorder != searchLine );

}

bool CSearchDialog::foundLine(long int searchLine, int globalIndex)
{
    match = true;

    if (searchtoIndex() == true)
    {
        addToSearchIndex(globalIndex);
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

void CSearchDialog::findNextClicked()
{
    setNextClicked(true);

    // Find All runs asynchronously and updates colour on completion.
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

void CSearchDialog::findPreviousClicked()
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

void CSearchDialog::on_lineEditSearch_textEdited(QString newText)
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
void CSearchDialog::textEditedFromToolbar(QString newText)
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

void CSearchDialog::on_buttonHighlightColor_clicked()
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

void CSearchDialog::updateColorbutton()
{
    QString color = QDltSettingsManager::getInstance()->value("other/searchResultColor", QString("#00AAFF")).toString();
    QColor lhlColor(color);
    highlightColor = lhlColor;
    QPixmap px(12, 12);
    px.fill(highlightColor);
    ui->buttonHighlightColor->setIcon(px);
}

void CSearchDialog::addToSearchIndex(int globalIndex)
{
    //qDebug() << "Add hit global index to search table" << globalIndex << __LINE__;
    if(globalIndex >= 0)
        m_searchtablemodel->add_SearchResultEntry(globalIndex);
 }

void CSearchDialog::registerSearchTableModel(CSearchTableModel *model)
{
    m_searchtablemodel = model;    
}


void CSearchDialog::loadSearchHistory()
{
    // getting text of the action button clicked to load search history.
    QAction *action = qobject_cast<QAction *>(sender());
    QString text;
    if(action)
    {
        text = action->text();
    }

    // creating a local list to store the indexes related to the key retrieved from the cache.
    std::vector<unsigned long> tmp;
    if(cachedHistoryKey.size() > 0)
    {
        tmp = cachedHistoryKey[text];

        //deleting the previous search list and adding the cached search obtained to the model.
        m_searchtablemodel->clear_SearchResults();
        for (std::size_t i = 0; i < tmp.size(); ++i)
        {
            m_searchtablemodel->add_SearchResultEntry(tmp.at(i));
        }
    }
    emit refreshedSearchIndex();
}

void CSearchDialog::cacheSearchHistory()
{
    // if it is a new search then add all the indexes of the search to a list(m_searchHistory).
    QString searchBoxText = getText();  
    m_searchHistory.push_back(m_searchtablemodel->m_searchResultList);
    cachedHistoryKey.insert(searchBoxText, m_searchHistory.back());    
}

void CSearchDialog::clearCacheHistory()
{
    // obtaining the list of keys stored in cache
    cachedHistoryKey.clear();
}

void CSearchDialog::saveSearchHistory(QStringList& searchHistory) {
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

void CSearchDialog::loadSearchHistoryList(QStringList& searchHistory)
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

void CSearchDialog::on_checkBoxHeader_toggled(bool checked)
{
   QDltSettingsManager::getInstance()->setValue("other/search/checkBoxHeader", checked);
}

void CSearchDialog::on_checkBoxFindAll_toggled(bool checked)
{
    QDltSettingsManager::getInstance()->setValue("other/search/checkBoxSearchIndex", checked);
    setStartLine(-1);
}

void CSearchDialog::on_checkBoxCaseSensitive_toggled(bool checked)
{
    QDltSettingsManager::getInstance()->setValue("other/search/checkBoxCasesensitive", checked);
}

void CSearchDialog::on_checkBoxRegExp_toggled(bool checked)
{
    QDltSettingsManager::getInstance()->setValue("other/search/checkBoxRegEx", checked);
}


