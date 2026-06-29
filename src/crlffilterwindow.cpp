#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QToolBar>
#include <QTableView>
#include <QLabel>
#include <QMessageBox>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QTimer>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QThread>

#include "crlffilterwindow.h"
#include "mainwindow.h"
#include "fieldnames.h"
#include "qdltfile.h"
#include "qdltexporter.h"
#include "qdltsettingsmanager.h"

CrlfFilterWindow::CrlfFilterWindow(QObject* parent) : QObject(parent) {
    m_sourceModelOfDLT = nullptr;
    m_crlfProjectionModel = nullptr;
    m_crlfWindow = nullptr;
    m_crlfTableView = nullptr;
    m_statusLabel = nullptr;
    m_dltFile = nullptr;
    m_pluginManager = nullptr;
    m_messageStore = nullptr;
    m_indexService = nullptr;
    m_externalDecodeCacheService = nullptr;
    
    // Initialize debouncing mechanism
    rebuildTimer = new QTimer(this);
    rebuildTimer->setSingleShot(true);
    rebuildTimer->setInterval(500);
    connect(rebuildTimer, &QTimer::timeout, this, &CrlfFilterWindow::onRebuildTimerTimeout);
    
    m_lastFilteredMessageCount = -1;
    m_rebuildScheduled = false;
    m_rebuildInProgress = false;
}

void CrlfFilterWindow::invalidateCache() {
    m_crlfCache.clear();
    m_messageDataCache.clear();
    m_lastCacheValidCount = -1;
    m_bulkCrlfIndexBuilt = false;
    CDecodeCacheService *activeDecodeCache = m_externalDecodeCacheService ? m_externalDecodeCacheService : &m_decodeCacheService;
    if (m_dltFile) {
        activeDecodeCache->clearForFile(m_dltFile);
    } else {
        activeDecodeCache->clear();
    }
}

// Check if a message contains CRLF characters
bool CrlfFilterWindow::containsCrlf(const QString& payload) {
    return payload.contains("\r") || payload.contains("\n");
}

// Update window title and status label with message count
void CrlfFilterWindow::updateMessageCount(int count) {
    QString countText = QString("CRLF Messages (%1 found)").arg(count);
    if (crlfWindow) {
        crlfWindow->setWindowTitle(countText);
    }
    if (statusLabel) {
        statusLabel->setText(QString("Total CRLF messages: %1 ").arg(count));
    }
}

// Apply column settings to table view
void CrlfFilterWindow::applyColumnSettings() {
    if (!crlfTableView || !crlfFilterProxy) {
        return;
    }
    
    auto settings = QDltSettingsManager::getInstance();
    for (int col = 0; col < crlfFilterProxy->columnCount(); ++col) {
        bool show = FieldNames::getColumnShown(static_cast<FieldNames::Fields>(col), settings);
        crlfTableView->setColumnHidden(col, !show);
        if (show) {
            int width = FieldNames::getColumnWidth(static_cast<FieldNames::Fields>(col), settings);
            crlfTableView->setColumnWidth(col, width);
        }
    }
}

// Create headers for CRLF table model
QStringList CrlfFilterWindow::createTableHeaders() {
    QStringList headers;
    auto settings = QDltSettingsManager::getInstance();
    headers << FieldNames::getName(FieldNames::Index, settings)
            << FieldNames::getName(FieldNames::Time, settings)
            << FieldNames::getName(FieldNames::TimeStamp, settings)
            << FieldNames::getName(FieldNames::Counter, settings)
            << FieldNames::getName(FieldNames::EcuId, settings)
            << FieldNames::getName(FieldNames::AppId, settings)
            << FieldNames::getName(FieldNames::ContextId, settings)
            << FieldNames::getName(FieldNames::SessionId, settings)
            << FieldNames::getName(FieldNames::Type, settings)
            << FieldNames::getName(FieldNames::Subtype, settings)
            << FieldNames::getName(FieldNames::Mode, settings)
            << FieldNames::getName(FieldNames::MessageId, settings)
            << FieldNames::getName(FieldNames::ArgCount, settings)
            << FieldNames::getName(FieldNames::Payload, settings);
    return headers;
}

// Creates a single window displaying all CRLF messages
void CrlfFilterWindow::createCrlfWindow() {
    // Validate prerequisites
    if (!m_dltFile || m_dltFile->size() == 0) {
        QMessageBox::information(nullptr, "No DLT file", "No DLT file is currently loaded.");
        return;
    }
    
    if (!m_sourceModelOfDLT) {
        QMessageBox::critical(nullptr, "Error", "No source model available for CRLF filtering.");
        return;
    }
    
    // Check if a CRLF window is already open (prevent multiple instances)
    if (crlfWindow && crlfWindow->isVisible()) {
        crlfWindow->raise();
        crlfWindow->activateWindow();
        return;
    }

    // First, prepare the data with progress dialog before creating window
    QWidget* parentWidget = qobject_cast<QWidget*>(parent());
    
    // Create the model if it doesn't exist
    if (!m_crlfProjectionModel) {
        m_crlfProjectionModel = new CProjectionTableModel(this);
    }
    m_crlfProjectionModel->setSourceModel(m_sourceModelOfDLT);
    
    // Check if no filtered messages exist
    int totalFilteredMessages = m_dltFile->sizeFilter();
    if (totalFilteredMessages == 0) {
        QMessageBox::information(parentWidget, "No Messages", "No messages are available for CRLF filtering.");
        return;
    }
    
    // Show progress dialog for data preparation
    QProgressDialog* prepareProgress = new QProgressDialog("Preparing CRLF data...", "Cancel", 0, totalFilteredMessages, parentWidget);
    prepareProgress->setWindowModality(Qt::ApplicationModal);
    prepareProgress->setMinimumDuration(0);
    prepareProgress->setWindowTitle("CRLF Filter");
    prepareProgress->show();
    
    int addedCount = 0;
    
    // Process all currently filtered messages to find CRLF messages
    prepareProgress->setMaximum(qMax(1, totalFilteredMessages));
    prepareProgress->setLabelText("Preparing CRLF data...");
    
    for (int i = 0; i < totalFilteredMessages; i++) {
        if (prepareProgress->wasCanceled()) {
            // User cancelled - clear the model and exit
            crlfFilterProxy->removeRows(0, crlfFilterProxy->rowCount());
            prepareProgress->close();
            delete prepareProgress;
            return;
        }
        
        int actualPos = dltFile->getMsgFilterPos(i);
        if (actualPos >= 0) {
            // Check CRLF directly on this filtered message
            QVariantList messageData = this->extractMessageData(i);
            if (!messageData.isEmpty()) {
                bool hasCrlf = messageData.last().toBool();
                if (hasCrlf) {
                    messageData.removeLast(); // Remove the CRLF flag
                    
                    // Create a row with the cached message data
                    QList<QStandardItem*> rowItems;
                    
                    // Store the actual position as the display index
                    QStandardItem* indexItem = new QStandardItem(QString::number(actualPos));
                    indexItem->setData(actualPos, Qt::UserRole);  // For export and navigation
                    indexItem->setData(i, Qt::UserRole + 1);      // For internal use
                    rowItems << indexItem;
                    
                    // Use pre-processed data from cache
                    for (int j = 1; j < messageData.size(); j++) { // Skip index (j=0)
                        rowItems << new QStandardItem(messageData[j].toString());
                    }
                    
                    crlfFilterProxy->appendRow(rowItems);
                    addedCount++;
                }
            }
        }
        
        // Update progress based on all filtered messages processed
        if (i % 10 == 0 || i == totalFilteredMessages - 1) {
            prepareProgress->setValue(i + 1);
            QCoreApplication::processEvents();
        }
    }
    prepareProgress->close();
    delete prepareProgress;
    
    // Check if any CRLF messages were found
    if (addedCount == 0) {
        QMessageBox::information(parentWidget, "No CRLF Messages", 
            "No messages containing CRLF characters (\\r, \\n, or \\r\\n) were found in the current DLT file.");
        return;
    }

    // Data preparation successful, now create and show the window
    crlfWindow = new QWidget(parentWidget);
    crlfWindow->setAttribute(Qt::WA_DeleteOnClose);
    crlfWindow->resize(1200, 700);
    
    // Connect window close event to cleanup
    connect(crlfWindow, &QWidget::destroyed, this, &CrlfFilterWindow::cleanup);
    connect(crlfWindow, &QWidget::destroyed, this, &QObject::deleteLater);
    
    crlfWindow->setWindowFlags(Qt::Window);
    
    // Create layout and UI components
    QVBoxLayout* layout = new QVBoxLayout(crlfWindow);

    // Add toolbar with export button
    QToolBar* toolbar = new QToolBar;
    QHBoxLayout* topRowLayout = new QHBoxLayout();
    topRowLayout->addStretch();
    topRowLayout->addWidget(toolbar);
    layout->addLayout(topRowLayout);

    QPushButton* exportButton = new QPushButton("Export CRLF Messages");
    exportButton->setToolTip("Export all CRLF messages to DLT file");
    toolbar->addWidget(exportButton);
    connect(exportButton, &QPushButton::clicked, this, &CrlfFilterWindow::onExportFilteredCrlfLogsClicked);

    // Create table view with the already prepared model
    crlfTableView = new QTableView;
    crlfTableView->setModel(crlfFilterProxy);
    crlfTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    crlfTableView->setAlternatingRowColors(true);
    crlfTableView->verticalHeader()->setVisible(false);
    crlfTableView->setSortingEnabled(false);
    crlfTableView->horizontalHeader()->setSortIndicatorShown(false);
    crlfTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    crlfTableView->horizontalHeader()->setStretchLastSection(true); // Enable stretching for better payload display
    crlfTableView->setWordWrap(false); // Disable word wrap but allow horizontal scrolling
    // Apply column settings like main window
    applyColumnSettings();
    
    connect(crlfTableView, &QTableView::doubleClicked, this, &CrlfFilterWindow::onCrlfMessageDoubleClicked);
    layout->addWidget(crlfTableView);
      
    // Add status bar
    statusLabel = new QLabel();
    
    // Apply theme-appropriate styling
    QPalette palette = statusLabel->palette();
    bool isDarkMode = palette.color(QPalette::Window).lightness() < palette.color(QPalette::WindowText).lightness();
    statusLabel->setStyleSheet(isDarkMode ? 
        "QLabel { padding: 5px; background-color: #3c3c3c; border-top: 1px solid #555; color: #ffffff; }" :
        "QLabel { padding: 5px; background-color: #f0f0f0; border-top: 1px solid #ccc; color: #000000; }");
    
    layout->addWidget(statusLabel);

    // Update window title and status with message count
    updateMessageCount(m_crlfProjectionModel->rowCount());
    
    // Apply column settings and initialize tracking state
    applyColumnSettings();
    m_lastFilteredMessageCount = m_dltFile->sizeFilter();

    // Show window after everything is prepared
    crlfWindow->show();
    crlfWindow->raise();
    crlfWindow->activateWindow();
}

// Exports all filtered CRLF DLT logs to a file
void CrlfFilterWindow::onExportFilteredCrlfLogsClicked() {
    if (!m_dltFile || !m_crlfProjectionModel || !m_sourceModelOfDLT || !m_crlfTableView || !m_crlfWindow) {
        QMessageBox::information(nullptr, "Export Error", "No CRLF data available to export or window is not properly initialized.");
        return;
    }
    
    int rowCount = crlfFilterProxy->rowCount();
    if (rowCount == 0) {
        QMessageBox::information(crlfWindow, "Export", "No CRLF messages to export.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(crlfWindow, "Export CRLF Messages", 
                                                    "crlf_messages.dlt", 
                                                    "DLT Files (*.dlt);;All Files (*)");
    if (fileName.isEmpty()) {
        return;
    }

    // Show progress dialog only if MainWindow is not busy to avoid overlapping dialogs
    QProgressDialog* progress = nullptr;
    bool showExportProgress = !isMainWindowBusy();
    
    if (showExportProgress) {
        progress = new QProgressDialog("Exporting CRLF messages...", "Cancel", 0, rowCount, crlfWindow);
        progress->setWindowModality(Qt::WindowModal);
        progress->setMinimumDuration(0);  // Show immediately
        progress->setValue(0);  // Initialize progress value
        progress->show();
        QCoreApplication::processEvents();  // Force progress dialog to appear
    }

    try {
        QVector<qint64> messageIndices;
        messageIndices.reserve(rowCount);  // Pre-allocate for better performance
        
        // Get actual message positions from UserRole data for proper export
        for (int row = 0; row < rowCount; ++row) {
            if (progress && progress->wasCanceled()) {
                return;
            }

            const int sourceRow = m_crlfProjectionModel->sourceRowForRow(row);
            const QModelIndex sourceIndex = m_sourceModelOfDLT->index(sourceRow, 0);
            if (sourceIndex.isValid()) {
                selectedIndices.append(sourceIndex);
            }
            
            // Update progress more frequently to ensure visibility
            if (progress && (row % 5 == 0 || row == rowCount - 1)) {
                progress->setValue(row + 1);
                progress->setLabelText(QString("Processing message %1 of %2...").arg(row + 1).arg(rowCount));
                QCoreApplication::processEvents();
                if (rowCount < 100) {
                    QThread::msleep(10);  // 10ms delay for small exports
                }
            }
        }
        
        if (messageIndices.isEmpty()) {
            if (progress) {
                progress->close();
                delete progress;
            }
            QMessageBox::warning(crlfWindow, "Export Error", "No valid CRLF messages found to export.");
            return;
        }

        // Create proper model indices by mapping absolute positions to filtered positions
        QModelIndexList selectedIndices;
        selectedIndices.reserve(messageIndices.size());
        
        // Update progress dialog for the mapping phase
        if (progress) {
            progress->setLabelText("Mapping messages to current view...");
            progress->setRange(0, messageIndices.size());
            progress->setValue(0);
        }
        QCoreApplication::processEvents();
        
        // Map our absolute positions to corresponding rows in the filtered model
        // Skip any messages that are no longer available in the current filtered view
        int totalFilteredMessages = dltFile->sizeFilter();
        int foundMessages = 0;
        
        for (int msgIndex = 0; msgIndex < messageIndices.size(); ++msgIndex) {
            if (progress && progress->wasCanceled()) {
                if (progress) {
                    delete progress;
                }
                return;
            }
            
            qint64 targetAbsolutePos = messageIndices[msgIndex];
            // Find which filtered row corresponds to this absolute position
            for (int filteredRow = 0; filteredRow < totalFilteredMessages; ++filteredRow) {
                int absolutePos = dltFile->getMsgFilterPos(filteredRow);
                if (absolutePos == targetAbsolutePos) {
                    // Found the matching filtered row - create model index for it
                    if (sourceModelOfDLT && filteredRow < sourceModelOfDLT->rowCount()) {
                        selectedIndices.append(sourceModelOfDLT->index(filteredRow, 0));
                        foundMessages++;
                    }
                    break;
                }
            }
            // If not found, message is no longer in current filtered view - skip it silently
            
            // Update progress more frequently during mapping
            if (progress && (msgIndex % 5 == 0 || msgIndex == messageIndices.size() - 1)) {
                progress->setValue(msgIndex + 1);
                progress->setLabelText(QString("Mapping message %1 of %2...").arg(msgIndex + 1).arg(messageIndices.size()));
                QCoreApplication::processEvents();
            }
        }
        
        if (selectedIndices.isEmpty()) {
            if (progress) {
                progress->close();
                delete progress;
            }
            QMessageBox::warning(crlfWindow, "Export Warning", 
                               "None of the CRLF messages are currently visible in the main window filter.\n"
                               "Please adjust your main window filters and try again.");
            return;
        }
        
        // Inform user if some messages were skipped due to filtering
        QString exportMessage = QString("Found %1 of %2 CRLF messages in current view")
                               .arg(foundMessages).arg(messageIndices.size());
        if (foundMessages < messageIndices.size()) {
            exportMessage += "\n(Some messages filtered out by main window)";
        }
        
        // Create and configure the exporter with proper DLT format support
        if (progress) {
            progress->setLabelText("Creating DLT file...");
            progress->setRange(0, 0);  // Indeterminate progress for file export
        }
        QCoreApplication::processEvents();
        
        QDltExporter* exporter = new QDltExporter(
            m_dltFile,                                    // Source DLT file
            fileName,                                   // Output filepath
            m_pluginManager,                              // Plugin manager (can be nullptr)
            QDltExporter::FormatDlt,                    // Export in DLT format
            QDltExporter::SelectionSelected,            // Export selected messages
            &selectedIndices,                           // List of valid model indices
            0,                                          // Automatic time settings
            0,                                          // UTC offset
            0,                                          // DST
            ',',                                        // Delimiter (not used for DLT format)
            "DLTVIEWER",                                // Signature
            nullptr                                     // No parent to avoid threading issues
            );
        
        // Export the messages
        exporter->exportMessages();
        delete exporter;
        
        // Ensure minimum visibility time for progress dialog
        if (progress) {
            progress->setLabelText("Export completed successfully!");
            QCoreApplication::processEvents();
            QThread::msleep(500);  // Show completion message for half a second
            progress->close();
            delete progress;
        }
        QMessageBox::information(crlfWindow, "Export Complete", 
                               QString("Successfully exported %1 CRLF messages to %2\n\n%3")
                               .arg(foundMessages).arg(fileName).arg(exportMessage));

    } catch (const std::exception &e) {
        if (progress) {
            progress->close();
            delete progress;
        }
        QMessageBox::critical(crlfWindow, "Export Error", QString("Failed to export: %1").arg(e.what()));
    } catch (...) {
        if (progress) {
            progress->close();
            delete progress;
        }
        QMessageBox::critical(crlfWindow, "Export Error", "An unexpected error occurred during export.");
    }
}

// Sets the source model for DLT data
void CrlfFilterWindow::setSourceModel(QAbstractTableModel* model) {
    // Disconnect from previous model if any
    if (m_sourceModelOfDLT) {
        disconnect(m_sourceModelOfDLT, nullptr, this, nullptr);
    }
    
    m_sourceModelOfDLT = model;
    
    if (m_sourceModelOfDLT) {
        connect(m_sourceModelOfDLT, &QAbstractTableModel::modelReset, this, &CrlfFilterWindow::onSourceModelReset);
        connect(m_sourceModelOfDLT, &QAbstractTableModel::layoutChanged, this, &CrlfFilterWindow::onSourceModelDataChanged);
        
        // Connect to parent's dltFileLoaded signal for file changes
        if (QObject* parentObj = parent()) {
            connect(parentObj, SIGNAL(dltFileLoaded()), this, SLOT(onSourceModelDataChanged()));
        }
    }
}

// Sets the DLT file reference
void CrlfFilterWindow::setDltFile(QDltFile* file) {
    m_dltFile = file;
    m_lastFilteredMessageCount = -1;  // Reset tracking state
    
    // Update window if visible and file is available
    if (m_crlfWindow && m_crlfWindow->isVisible() && m_dltFile) {
        if (m_dltFile->size() == 0) {
            onSourceModelReset();
        } else if (!rebuildScheduled && !rebuildTimer->isActive() && !rebuildInProgress) {
            rebuildScheduled = true;
            rebuildTimer->start();
        }
    }
}

// Sets the plugin manager reference
void CrlfFilterWindow::setPluginManager(QDltPluginManager* manager) {
    m_pluginManager = manager;
}

void CrlfFilterWindow::setMessageStore(CMessageStore *messageStore)
{
    this->m_messageStore = messageStore;
}

void CrlfFilterWindow::setIndexService(const CIndexService *indexService)
{
    this->m_indexService = indexService;
}

void CrlfFilterWindow::setDecodeCacheService(CDecodeCacheService *decodeCacheService)
{
    m_externalDecodeCacheService = decodeCacheService;
}

// Cleanup method to properly disconnect from models/signals
void CrlfFilterWindow::cleanup() {
    // Reset state flags first to prevent any new operations
    rebuildScheduled = false;
    rebuildInProgress = false;
    
    // Stop any pending rebuild operations
    if (rebuildTimer && rebuildTimer->isActive()) {
        rebuildTimer->stop();
    }
    
    // Disconnect from source model to prevent further updates
    if (m_sourceModelOfDLT) {
        disconnect(m_sourceModelOfDLT, nullptr, this, nullptr);
    }
    
    // Clean up UI components - only detach model if both objects exist
    if (crlfTableView && crlfFilterProxy) {
        crlfTableView->setModel(nullptr);
    }
    
    // Schedule proxy model for deletion
    if (crlfFilterProxy) {
        crlfFilterProxy->deleteLater();
    }
    
    // Reset all pointers (no individual null checks needed)
    m_crlfTableView = nullptr;
    m_crlfProjectionModel = nullptr;
    m_statusLabel = nullptr;
    m_sourceModelOfDLT = nullptr;
    m_crlfWindow = nullptr;
    m_dltFile = nullptr;
    m_pluginManager = nullptr;
}

// Handle double-click on CRLF message row to navigate to main window
void CrlfFilterWindow::onCrlfMessageDoubleClicked(const QModelIndex& index) {
    if (!index.isValid() || !m_crlfProjectionModel || !m_dltFile) {
        return;
    }
    
    // Get data from first column of clicked row (where UserRole data is stored)
    QModelIndex firstColumnIndex = index.sibling(index.row(), 0);
    if (!firstColumnIndex.isValid()) {
        return;
    }

    CIndexService localIndexService;
    const CIndexService *activeIndexService = m_indexService ? m_indexService : &localIndexService;
    const std::vector<int> filteredProjection =
        activeIndexService->snapshotProjection(buildActiveFilteredProjection(m_dltFile));

    if (sourceRow >= static_cast<int>(filteredProjection.size())) {
        return;
    }

    const int actualPosition = filteredProjection.at(static_cast<std::size_t>(sourceRow));
    if (actualPosition < 0 || actualPosition >= m_dltFile->size()) {
        return;
    }    
    // Verify the message is still in the current filtered view before navigating
    bool foundInFilteredView = false;
    int totalFilteredMessages = dltFile->sizeFilter();
    
    for (int i = 0; i < totalFilteredMessages; i++) {
        int filteredAbsolutePos = dltFile->getMsgFilterPos(i);
        if (filteredAbsolutePos == absolutePosition) {
            foundInFilteredView = true;
            break;
        }
    }
    // Only navigate if message is found in current filtered view
    if (foundInFilteredView) {
        emit jumpToMessageRequested(absolutePosition);
        
        if (QWidget* parentWidget = qobject_cast<QWidget*>(parent())) {
            parentWidget->raise();
            parentWidget->activateWindow();
        }
    }
}

// Handle when source model data changes
void CrlfFilterWindow::onSourceModelDataChanged() {
    // Early returns for invalid states
    if (!m_crlfWindow || !m_crlfWindow->isVisible() || !m_dltFile) {
        if (!m_dltFile && m_crlfWindow && m_crlfWindow->isVisible()) {
            onSourceModelReset();
        }
        return;
    }
    
    // Handle empty file state immediately
    int currentFilteredCount = m_dltFile->sizeFilter();
    if (m_dltFile->size() == 0 || currentFilteredCount == 0) {
        m_lastFilteredMessageCount = 0;
        onSourceModelReset();
        return;
    }
    
    // Check for significant data changes that require cache invalidation
    int significantChange = abs(currentFilteredCount - lastFilteredMessageCount);
    
    // For filter changes, always invalidate cache since different messages may be visible even if the count is similar
    if (lastFilteredMessageCount > 0 && significantChange > 0) {
        // Any change in filtered count means different messages are visible - invalidate cache
        this->invalidateCache();
    }
    bool countChanged = (currentFilteredCount != lastFilteredMessageCount);
    
    // Additional validation: During model transitions, delay rebuild for stability
    if (m_sourceModelOfDLT && m_sourceModelOfDLT->rowCount() != currentFilteredCount) {
        // Model is in transition - schedule rebuild with delay for stability
        if (!rebuildScheduled && !rebuildTimer->isActive() && !rebuildInProgress) {
            rebuildScheduled = true;
            rebuildTimer->setInterval(750); // Longer delay for stability during transitions
            rebuildTimer->start();
        }
        return;
    }
    
    // Reset normal timer interval
    rebuildTimer->setInterval(500);
    
    // Always rebuild when filtered message count changes (filters applied/removed)
    if (countChanged) {
        // Avoid overlapping rebuild operations
        if (!rebuildScheduled && !rebuildTimer->isActive() && !rebuildInProgress) {
            rebuildScheduled = true;
            rebuildTimer->start();
        }
    }
}

// Handle when source model is reset/cleared
void CrlfFilterWindow::onSourceModelReset() {
    // Reset all state flags and counters
    if (rebuildTimer->isActive()) {
        rebuildTimer->stop();
    }
    rebuildScheduled = false;
    rebuildInProgress = false;
    lastFilteredMessageCount = -1;
    
    // Invalidate cache on model reset
    this->invalidateCache();
    
    // Early return if window not visible or proxy not available
    if (!crlfWindow || !crlfWindow->isVisible() || !crlfFilterProxy) {
        return;
    }
    
    // Don't show empty window during transitions - schedule rebuild instead
    if (m_dltFile && m_dltFile->sizeFilter() > 0) {
        // Schedule rebuild rather than showing empty window
        rebuildScheduled = true;
        rebuildTimer->start();
    } else {
        // Only clear if there's genuinely no data
        crlfFilterProxy->clear();
        crlfFilterProxy->setHorizontalHeaderLabels(createTableHeaders());
        
        // Apply settings and update UI
        if (crlfTableView) {
            applyColumnSettings();
        }
        updateMessageCount(0);
    }
}

// Rebuild the CRLF data model with current DLT file data
void CrlfFilterWindow::rebuildCrlfModel() {
    if (!crlfFilterProxy) {
        return;
    }
    
    if (!m_dltFile || m_dltFile->size() == 0) {
        // No file or empty file - clear the model
        crlfFilterProxy->removeRows(0, crlfFilterProxy->rowCount());
        updateMessageCount(0);
        return;
    }
    
    // Clear existing data
    crlfFilterProxy->removeRows(0, crlfFilterProxy->rowCount());
    
    // Check if no filtered messages exist
    int totalFilteredMessages = m_dltFile->sizeFilter();
    if (totalFilteredMessages == 0) {
        updateMessageCount(0);
        lastFilteredMessageCount = 0;
        return;
    }
    
    //Build CRLF index if needed
    if (!bulkCrlfIndexBuilt) {
        buildBulkCrlfIndex();
        // If user cancelled during index building, exit early
        if (!bulkCrlfIndexBuilt) {
            updateMessageCount(0);
            return;
        }
    }
    
    //Now rebuild the model data
    bool needsProgress = (totalFilteredMessages > 2000) && !isMainWindowBusy();
    
    QProgressDialog* buildProgress = nullptr;
    if (needsProgress) {
        buildProgress = new QProgressDialog("Rebuilding CRLF data...", "Cancel", 0, totalFilteredMessages, crlfWindow);
        buildProgress->setWindowModality(Qt::WindowModal);
        buildProgress->setMinimumDuration(0);
        buildProgress->show();
    }
    
    int addedCount = 0;
    
    // Process only messages currently visible in filtered view
    int processCount = 0;
    for (int i = 0; i < totalFilteredMessages; i++) {
        if (buildProgress && buildProgress->wasCanceled()) {
            // User cancelled - clear the model, set count to 0, and exit
            crlfFilterProxy->removeRows(0, crlfFilterProxy->rowCount());
            buildProgress->close();
            delete buildProgress;
            updateMessageCount(0);
            lastFilteredMessageCount = totalFilteredMessages;
            return;
        }
        
        int actualPos = dltFile->getMsgFilterPos(i);
        if (actualPos >= 0) {
            // Check CRLF directly on this filtered message and suppress index building since we already built it above
            QVariantList messageData = this->extractMessageData(i, true);
            if (!messageData.isEmpty()) {
                bool hasCrlf = messageData.last().toBool();
                if (hasCrlf) {
                    messageData.removeLast(); // Remove the CRLF flag
                    
                    // Create a row with the cached message data
                    QList<QStandardItem*> rowItems;
                    
                    // Store the actual position as the display index
                    QStandardItem* indexItem = new QStandardItem(QString::number(actualPos));
                    indexItem->setData(actualPos, Qt::UserRole);  // For export and navigation
                    indexItem->setData(i, Qt::UserRole + 1);      // For internal use
                    rowItems << indexItem;
                    
                    // Use pre-processed data from cache
                    for (int j = 1; j < messageData.size(); j++) { // Skip index (j=0)
                        rowItems << new QStandardItem(messageData[j].toString());
                    }
                    
                    crlfFilterProxy->appendRow(rowItems);
                    addedCount++;
                }
            }
        }
        
        processCount++;
        // Update progress less frequently for better performance
        if (buildProgress && processCount % 200 == 0) {
            buildProgress->setValue(i);
            QCoreApplication::processEvents();
        }
    }
    
    if (buildProgress) {
        buildProgress->close();
        delete buildProgress;
    }
    
    // Update UI with final count
    updateMessageCount(addedCount);
    lastFilteredMessageCount = totalFilteredMessages;
    lastCacheValidCount = totalFilteredMessages;
    
    // Apply column settings
    applyColumnSettings();
}

// Debounced rebuild triggered by timer
void CrlfFilterWindow::onRebuildTimerTimeout() {
    rebuildScheduled = false;
    
    if (!crlfWindow || !crlfWindow->isVisible()) {
        return;
    }
    
    // Prevent overlapping rebuilds
    if (rebuildInProgress) {
        return;
    }
    
    if (!m_dltFile || m_dltFile->size() == 0) {
        onSourceModelReset();
    } else {
        rebuildInProgress = true;
        rebuildCrlfModel();
        rebuildInProgress = false;
    }
}

// Public method to refresh the CRLF window with latest data
void CrlfFilterWindow::refreshWindow() {
    if (m_crlfWindow && m_dltFile && m_crlfProjectionModel) {
        m_crlfProjectionModel->setSourceModel(m_sourceModelOfDLT);
        rebuildCrlfModel();
    } else if (!m_crlfWindow && m_dltFile) {
        // Window was closed but object still exists - recreate the window
        createCrlfWindow();
    }
}

// Public method to show and activate the CRLF window
void CrlfFilterWindow::showAndActivate() {
    if (crlfWindow) {
        crlfWindow->activateWindow();
        crlfWindow->raise();
        crlfWindow->show();
    } else if (dltFile) {
        createCrlfWindow();
    }
}

// Public method to close the CRLF window
void CrlfFilterWindow::closeWindow() {
    if (crlfWindow) {
        crlfWindow->close();
    }
}

std::vector<int> CrlfFilterWindow::buildCrlfProjectionRows(QWidget *progressParent,
                                                           const QString &progressLabel,
                                                           bool *wasCancelled)
{
    std::vector<int> rows;
    if (wasCancelled)
        *wasCancelled = false;

    if (!m_dltFile)
        return rows;

    CIndexService localIndexService;
    const CIndexService *activeIndexService = m_indexService ? m_indexService : &localIndexService;
    const std::vector<int> filteredProjection =
        activeIndexService->snapshotProjection(buildActiveFilteredProjection(m_dltFile));
    const int totalFilteredMessages = static_cast<int>(filteredProjection.size());
    rows.reserve(static_cast<std::size_t>(qMax(0, totalFilteredMessages / 8)));

    const bool decodeEnabled = QDltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool();
    const int triggeredByUser = !QDltOptManager::getInstance()->issilentMode();

    QProgressDialog buildProgress(progressLabel, "Cancel", 0, totalFilteredMessages, progressParent);
    buildProgress.setWindowModality(Qt::WindowModal);
    buildProgress.setMinimumDuration(0);
    buildProgress.show();

    CDecodeCacheService *activeDecodeCache = m_externalDecodeCacheService ? m_externalDecodeCacheService : &m_decodeCacheService;
    QDltMsg msg;
    for (int sourceRow = 0; sourceRow < totalFilteredMessages; ++sourceRow) {
        if (buildProgress.wasCanceled()) {
            if (wasCancelled)
                *wasCancelled = true;
            break;
        }

        const int globalIndex = filteredProjection.at(static_cast<std::size_t>(sourceRow));
        if (globalIndex < 0)
            continue;

        bool gotMessage = false;
        if (activeDecodeCache) {
            gotMessage = activeDecodeCache->message(m_dltFile,
                                                    m_pluginManager,
                                                    globalIndex,
                                                    decodeEnabled,
                                                    triggeredByUser,
                                                    msg,
                                                    true);
        }

        if (!gotMessage && m_messageStore) {
            const MessageId messageId = m_messageStore->messageIdForGlobalIndex(globalIndex);
            gotMessage = (messageId != kInvalidMessageId) && m_messageStore->message(messageId, msg);
        }
        }
    }
    
    return data;
}

// Clear cache when file structure changes
void CrlfFilterWindow::invalidateCache() {
    crlfCache.clear();
    messageDataCache.clear();
    bulkCrlfIndexBuilt = false;
    lastCacheValidCount = -1;
}

// Build bulk CRLF index for all messages (one-time operation)
void CrlfFilterWindow::buildBulkCrlfIndex() {
    if (bulkCrlfIndexBuilt || !dltFile) {
        return;
    }
    
    crlfCache.clear();  // Clear existing cache before rebuilding
    int totalMessages = dltFile->sizeFilter();
    
    // Only show progress for large files
    QProgressDialog* progress = nullptr;
    if (totalMessages > 1000 && !isMainWindowBusy()) {
        QWidget* parentWidget = qobject_cast<QWidget*>(parent());
        progress = new QProgressDialog("Building CRLF index...", "Cancel", 0, totalMessages, parentWidget);
        progress->setWindowModality(Qt::ApplicationModal);
        progress->setMinimumDuration(0);
        progress->show();
    }
    
    // Process messages in chunks to reduce UI blocking
    const int chunkSize = 100;
    
    for (int i = 0; i < totalMessages; i += chunkSize) {
        if (progress && progress->wasCanceled()) {
            delete progress;
            return;
        }
        
        int endIndex = qMin(i + chunkSize, totalMessages);
        
        for (int j = i; j < endIndex; j++) {
            int actualPos = dltFile->getMsgFilterPos(j);
            if (actualPos >= 0 && actualPos < dltFile->size()) {
                QDltMsg msg;
                if (dltFile->getMsg(actualPos, msg)) {
                    if (pluginManager) {
                        pluginManager->decodeMsg(msg, true);
                    }
                    QString rawPayload = msg.toStringPayload();
                    if (containsCrlf(rawPayload)) {
                        crlfCache[actualPos] = true;   // Positive cache entry
                    } else {
                        crlfCache[actualPos] = false;  // Negative cache entry
                    }
                }
            }
        }
        
        if (progress) {
            progress->setValue(endIndex);
            QCoreApplication::processEvents();
        }
    }
    
    if (progress) {
        progress->close();
        delete progress;
    }
    
    bulkCrlfIndexBuilt = true;
}