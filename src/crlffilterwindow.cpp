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

#include "crlffilterwindow.h"
#include "fieldnames.h"
#include "qdltfile.h"
#include "qdltexporter.h"
#include "qdltsettingsmanager.h"

CrlfFilterWindow::CrlfFilterWindow(QObject* parent) : QObject(parent) {
    sourceModelOfDLT = nullptr;
    crlfFilterProxy = nullptr;
    crlfWindow = nullptr;
    crlfTableView = nullptr;
    statusLabel = nullptr;
    dltFile = nullptr;
    pluginManager = nullptr;
    
    // Initialize debouncing mechanism
    rebuildTimer = new QTimer(this);
    rebuildTimer->setSingleShot(true);
    rebuildTimer->setInterval(300);
    connect(rebuildTimer, &QTimer::timeout, this, &CrlfFilterWindow::onRebuildTimerTimeout);
    
    lastFilteredMessageCount = -1;
    rebuildScheduled = false;
    rebuildInProgress = false;
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
    if (!dltFile || dltFile->size() == 0) {
        QMessageBox::information(nullptr, "No DLT file", "No DLT file is currently loaded.");
        return;
    }
    
    if (!sourceModelOfDLT) {
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
    if (!crlfFilterProxy) {
        crlfFilterProxy = new QStandardItemModel(this);
        crlfFilterProxy->setHorizontalHeaderLabels(createTableHeaders());
    } else {
        // Clear existing data
        crlfFilterProxy->removeRows(0, crlfFilterProxy->rowCount());
    }
    
    // Check if no filtered messages exist
    int totalFilteredMessages = dltFile->sizeFilter();
    if (totalFilteredMessages == 0) {
        QMessageBox::information(parentWidget, "No Messages", "No messages are available for CRLF filtering.");
        return;
    }
    
    // Show progress dialog for data preparation
    QProgressDialog prepareProgress("Preparing CRLF data...", "Cancel", 0, totalFilteredMessages, parentWidget);
    prepareProgress.setWindowModality(Qt::ApplicationModal);
    prepareProgress.setMinimumDuration(0);
    prepareProgress.setWindowTitle("CRLF Filter");
    prepareProgress.show();
    
    int addedCount = 0;
    for (int i = 0; i < totalFilteredMessages; i++) {
        if (prepareProgress.wasCanceled()) {
            // User cancelled - clear the model and exit
            crlfFilterProxy->removeRows(0, crlfFilterProxy->rowCount());
            prepareProgress.close();
            return;
        }
        
        QDltMsg msg;
        int actualPos = dltFile->getMsgFilterPos(i);
        if (actualPos >= 0 && dltFile->getMsg(actualPos, msg)) {
            // Apply plugin decoding if plugins are enabled
            if (pluginManager) {
                pluginManager->decodeMsg(msg, true); // Silent mode
            }
            
            QString rawPayload = msg.toStringPayload();
            if (containsCrlf(rawPayload)) {
                // Process payload for display like main table model
                QString payload = rawPayload.simplified().remove(QChar::Null);
                if (dltFile) {
                    dltFile->applyRegExString(msg, payload);
                }
                
                // Create a row with the message data
                QList<QStandardItem*> rowItems;
                
                // Store the actual position as the display index
                QStandardItem* indexItem = new QStandardItem(QString::number(actualPos));
                indexItem->setData(actualPos, Qt::UserRole);  // For export and navigation
                indexItem->setData(i, Qt::UserRole + 1);      // For internal use
                rowItems << indexItem;
                
                // Add all fields in the same order as FieldNames enum
                rowItems << new QStandardItem(msg.getTimeString())
                        << new QStandardItem(QString::number(msg.getTimestamp()))
                        << new QStandardItem(QString::number(msg.getMessageCounter()))
                        << new QStandardItem(msg.getEcuid())
                        << new QStandardItem(msg.getApid())
                        << new QStandardItem(msg.getCtid())
                        << new QStandardItem(QString::number(msg.getSessionid()))
                        << new QStandardItem(msg.getTypeString())
                        << new QStandardItem(msg.getSubtypeString())
                        << new QStandardItem(msg.getModeString())
                        << new QStandardItem(QString::number(msg.getMessageId()))
                        << new QStandardItem(QString::number(msg.getNumberOfArguments()))
                        << new QStandardItem(payload);
                
                crlfFilterProxy->appendRow(rowItems);
                addedCount++;
            }
        }
        
        // Update progress every 100 messages
        if (i % 100 == 0) {
            prepareProgress.setValue(i);
            QCoreApplication::processEvents();
        }
    }
    prepareProgress.close();
    
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
    updateMessageCount(crlfFilterProxy->rowCount());
    
    // Apply column settings and initialize tracking state
    applyColumnSettings();
    lastFilteredMessageCount = dltFile->sizeFilter();

    // Show window after everything is prepared
    crlfWindow->show();
    crlfWindow->raise();
    crlfWindow->activateWindow();
}

// Exports all filtered CRLF DLT logs to a file
void CrlfFilterWindow::onExportFilteredCrlfLogsClicked() {
    if (!dltFile || !crlfFilterProxy || !crlfTableView || !crlfWindow) {
        QMessageBox::information(nullptr, "Export Error", "No CRLF data available to export or window is not properly initialized.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(crlfWindow, "Export CRLF Messages", 
                                                    "crlf_messages.dlt", 
                                                    "DLT Files (*.dlt);;All Files (*)");
    if (fileName.isEmpty()) {
        return;
    }

    int rowCount = crlfFilterProxy->rowCount();
    if (rowCount == 0) {
        QMessageBox::information(crlfWindow, "Export", "No CRLF messages to export.");
        return;
    }

    QProgressDialog progress("Exporting CRLF messages...", "Cancel", 0, rowCount, crlfWindow);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.show();

    try {
        QVector<qint64> messageIndices;
        messageIndices.reserve(rowCount);  // Pre-allocate for better performance
        
        // Get actual message positions from UserRole data for proper export
        for (int row = 0; row < rowCount; ++row) {
            if (progress.wasCanceled()) {
                return;
            }
            
            // Get the actual file position stored in UserRole during model building
            QModelIndex indexItem = crlfFilterProxy->index(row, 0);
            if (indexItem.isValid()) {
                QVariant positionData = indexItem.data(Qt::UserRole);
                if (positionData.isValid()) {
                    int actualFilePosition = positionData.toInt();
                    if (actualFilePosition >= 0) {
                        messageIndices.append(actualFilePosition);
                    }
                }
            }
            
            // Update progress every 10 rows to reduce UI overhead
            if (row % 10 == 0 || row == rowCount - 1) {
                progress.setValue(row + 1);
                QCoreApplication::processEvents();
            }
        }
        
        if (messageIndices.isEmpty()) {
            progress.close();
            QMessageBox::warning(crlfWindow, "Export Error", "No valid CRLF messages found to export.");
            return;
        }

        // Create proper model indices by mapping absolute positions to filtered positions
        QModelIndexList selectedIndices;
        selectedIndices.reserve(messageIndices.size());
        
        // Map our absolute positions to corresponding rows in the filtered model
        // Skip any messages that are no longer available in the current filtered view
        int totalFilteredMessages = dltFile->sizeFilter();
        int foundMessages = 0;
        
        for (qint64 targetAbsolutePos : messageIndices) {
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
        }
        
        if (selectedIndices.isEmpty()) {
            progress.close();
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
        QDltExporter* exporter = new QDltExporter(
            dltFile,                                    // Source DLT file
            fileName,                                   // Output filepath
            pluginManager,                              // Plugin manager (can be nullptr)
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

        progress.close();
        QMessageBox::information(crlfWindow, "Export Complete", 
                               QString("Successfully exported %1 CRLF messages to %2\n\n%3")
                               .arg(foundMessages).arg(fileName).arg(exportMessage));

    } catch (const std::exception &e) {
        progress.close();
        QMessageBox::critical(crlfWindow, "Export Error", QString("Failed to export: %1").arg(e.what()));
    } catch (...) {
        progress.close();
        QMessageBox::critical(crlfWindow, "Export Error", "An unexpected error occurred during export.");
    }
}

// Sets the source model for DLT data
void CrlfFilterWindow::setSourceModel(QAbstractTableModel* model) {
    // Disconnect from previous model if any
    if (sourceModelOfDLT) {
        disconnect(sourceModelOfDLT, nullptr, this, nullptr);
    }
    
    sourceModelOfDLT = model;
    
    if (sourceModelOfDLT) {
        // Batch connect signals for better performance
        connect(sourceModelOfDLT, &QAbstractTableModel::modelReset, this, &CrlfFilterWindow::onSourceModelReset);
        connect(sourceModelOfDLT, &QAbstractTableModel::layoutChanged, this, &CrlfFilterWindow::onSourceModelDataChanged);
        connect(sourceModelOfDLT, &QAbstractTableModel::dataChanged, this, &CrlfFilterWindow::onSourceModelDataChanged);
        
        // Connect to parent's dltFileLoaded signal for file changes
        if (QObject* parentObj = parent()) {
            connect(parentObj, SIGNAL(dltFileLoaded()), this, SLOT(onSourceModelDataChanged()));
        }
    }
}

// Sets the DLT file reference
void CrlfFilterWindow::setDltFile(QDltFile* file) {
    dltFile = file;
    lastFilteredMessageCount = -1;  // Reset tracking state
    
    // Update window if visible and file is available
    if (crlfWindow && crlfWindow->isVisible() && dltFile) {
        if (dltFile->size() == 0) {
            onSourceModelReset();
        } else if (!rebuildScheduled && !rebuildTimer->isActive() && !rebuildInProgress) {
            rebuildScheduled = true;
            rebuildTimer->start();
        }
    }
}

// Sets the plugin manager reference
void CrlfFilterWindow::setPluginManager(QDltPluginManager* manager) {
    pluginManager = manager;
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
    if (sourceModelOfDLT) {
        disconnect(sourceModelOfDLT, nullptr, this, nullptr);
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
    crlfTableView = nullptr;
    crlfFilterProxy = nullptr;
    statusLabel = nullptr;
    sourceModelOfDLT = nullptr;
    crlfWindow = nullptr;
    dltFile = nullptr;
    pluginManager = nullptr;
}

// Handle double-click on CRLF message row to navigate to main window
void CrlfFilterWindow::onCrlfMessageDoubleClicked(const QModelIndex& index) {
    if (!index.isValid() || !crlfFilterProxy || !dltFile) {
        return;
    }
    
    // Get data from first column of clicked row (where UserRole data is stored)
    QModelIndex firstColumnIndex = index.sibling(index.row(), 0);
    if (!firstColumnIndex.isValid()) {
        return;
    }
    
    // Get the actual file position for navigation from UserRole
    QVariant positionData = firstColumnIndex.data(Qt::UserRole);
    if (!positionData.isValid()) {
        return;
    }
    
    int actualPosition = positionData.toInt();
    if (actualPosition < 0 || actualPosition >= dltFile->size()) {
        return;
    }
    
    // Emit signal to request navigation and bring main window to front
    emit jumpToMessageRequested(actualPosition);
    
    if (QWidget* parentWidget = qobject_cast<QWidget*>(parent())) {
        parentWidget->raise();
        parentWidget->activateWindow();
    }
}

// Handle when source model data changes
void CrlfFilterWindow::onSourceModelDataChanged() {
    // Early returns for invalid states
    if (!crlfWindow || !crlfWindow->isVisible() || !dltFile) {
        if (!dltFile && crlfWindow && crlfWindow->isVisible()) {
            onSourceModelReset();
        }
        return;
    }
    
    // Handle empty file state immediately
    int currentFilteredCount = dltFile->sizeFilter();
    if (dltFile->size() == 0 || currentFilteredCount == 0) {
        lastFilteredMessageCount = 0;
        onSourceModelReset();
        return;
    }
    
    // Skip if no actual change in message count
    if (currentFilteredCount == lastFilteredMessageCount) {
        return;
    }
    
    // Avoid overlapping rebuild operations
    if (rebuildScheduled || rebuildTimer->isActive() || rebuildInProgress) {
        return;
    }
    
    // Schedule rebuild for any change in message count
    rebuildScheduled = true;
    rebuildTimer->start();
}

// Handle when source model is reset/cleared
void CrlfFilterWindow::onSourceModelReset() {
    // Reset all state flags and counters
    if (rebuildTimer->isActive()) {
        rebuildTimer->stop();
    }
    rebuildScheduled = false;
    rebuildInProgress = false;
    lastFilteredMessageCount = 0;
    
    // Early return if window not visible or proxy not available
    if (!crlfWindow || !crlfWindow->isVisible() || !crlfFilterProxy) {
        return;
    }
    
    // Clear and reset the model
    crlfFilterProxy->clear();
    crlfFilterProxy->setHorizontalHeaderLabels(createTableHeaders());
    
    // Apply settings and update UI
    if (crlfTableView) {
        applyColumnSettings();
    }
    updateMessageCount(0);
}

// Rebuild the CRLF data model with current DLT file data
void CrlfFilterWindow::rebuildCrlfModel() {
    if (!crlfFilterProxy) {
        return;
    }
    
    if (!dltFile || dltFile->size() == 0) {
        // No file or empty file - clear the model
        crlfFilterProxy->removeRows(0, crlfFilterProxy->rowCount());
        updateMessageCount(0);
        return;
    }
    
    // Clear existing data
    crlfFilterProxy->removeRows(0, crlfFilterProxy->rowCount());
    
    // Check if no filtered messages exist
    int totalFilteredMessages = dltFile->sizeFilter();
    if (totalFilteredMessages == 0) {
        updateMessageCount(0);
        lastFilteredMessageCount = 0;
        return;
    }
    
    // For rebuilds (when window already exists), show progress over the window
    QProgressDialog buildProgress("Rebuilding CRLF data...", "Cancel", 0, totalFilteredMessages, crlfWindow);
    buildProgress.setWindowModality(Qt::WindowModal);
    buildProgress.setMinimumDuration(0);
    buildProgress.show();
    
    int addedCount = 0;
    for (int i = 0; i < totalFilteredMessages; i++) {
        if (buildProgress.wasCanceled()) {
            // User cancelled - clear the model, set count to 0, and exit
            crlfFilterProxy->removeRows(0, crlfFilterProxy->rowCount());
            buildProgress.close();
            updateMessageCount(0);
            lastFilteredMessageCount = totalFilteredMessages;
            return;
        }
        
        QDltMsg msg;
        int actualPos = dltFile->getMsgFilterPos(i);
        if (actualPos >= 0 && dltFile->getMsg(actualPos, msg)) {
            // Apply plugin decoding if plugins are enabled
            if (pluginManager) {
                pluginManager->decodeMsg(msg, true); // Silent mode
            }
            
            QString rawPayload = msg.toStringPayload();
            if (containsCrlf(rawPayload)) {
                // Process payload for display like main table model
                QString payload = rawPayload.simplified().remove(QChar::Null);
                if (dltFile) {
                    dltFile->applyRegExString(msg, payload);
                }
                
                // Create a row with the message data
                QList<QStandardItem*> rowItems;
                
                // Store the actual position as the display index
                QStandardItem* indexItem = new QStandardItem(QString::number(actualPos));
                indexItem->setData(actualPos, Qt::UserRole);  // For export and navigation
                indexItem->setData(i, Qt::UserRole + 1);      // For internal use
                rowItems << indexItem;
                
                // Add all fields in the same order as FieldNames enum
                rowItems << new QStandardItem(msg.getTimeString())
                        << new QStandardItem(QString::number(msg.getTimestamp()))
                        << new QStandardItem(QString::number(msg.getMessageCounter()))
                        << new QStandardItem(msg.getEcuid())
                        << new QStandardItem(msg.getApid())
                        << new QStandardItem(msg.getCtid())
                        << new QStandardItem(QString::number(msg.getSessionid()))
                        << new QStandardItem(msg.getTypeString())
                        << new QStandardItem(msg.getSubtypeString())
                        << new QStandardItem(msg.getModeString())
                        << new QStandardItem(QString::number(msg.getMessageId()))
                        << new QStandardItem(QString::number(msg.getNumberOfArguments()))
                        << new QStandardItem(payload);
                
                crlfFilterProxy->appendRow(rowItems);
                addedCount++;
            }
        }
        
        // Update progress every 100 messages
        if (i % 100 == 0) {
            buildProgress.setValue(i);
            QCoreApplication::processEvents();
        }
    }
    buildProgress.close();
    
    // Update UI with final count
    updateMessageCount(addedCount);
    lastFilteredMessageCount = totalFilteredMessages;
    
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
    
    if (!dltFile || dltFile->size() == 0) {
        onSourceModelReset();
    } else {
        rebuildInProgress = true;
        rebuildCrlfModel();
        rebuildInProgress = false;
    }
}

// Public method to refresh the CRLF window with latest data
void CrlfFilterWindow::refreshWindow() {
    if (crlfWindow && dltFile) {
        rebuildCrlfModel();
    }
}

// Public method to show and activate the CRLF window
void CrlfFilterWindow::showAndActivate() {
    if (crlfWindow) {
        crlfWindow->activateWindow();
        crlfWindow->raise();
        crlfWindow->show();
    }
}

// Public method to close the CRLF window
void CrlfFilterWindow::closeWindow() {
    if (crlfWindow) {
        crlfWindow->close();
    }
}
