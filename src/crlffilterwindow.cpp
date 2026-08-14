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

#include "crlffilterwindow.h"
#include "mainwindow.h"
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
    
    rebuildTimer = new QTimer(this);
    rebuildTimer->setSingleShot(true);
    rebuildTimer->setInterval(500);
    connect(rebuildTimer, &QTimer::timeout, this, &CrlfFilterWindow::onRebuildTimerTimeout);
    
    lastFilteredMessageCount = -1;
    rebuildScheduled = false;
    rebuildInProgress = false;
    lastBuildCanceled = false;
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

// Creates a single window displaying all CRLF messages
void CrlfFilterWindow::createCrlfWindow() {
    if (!dltFile || dltFile->size() == 0) {
        QMessageBox::information(nullptr, "No DLT file", "No DLT file is currently loaded.");
        return;
    }
    
    if (!sourceModelOfDLT) {
        QMessageBox::critical(nullptr, "Error", "No source model available for CRLF filtering.");
        return;
    }
    
    if (crlfWindow && crlfWindow->isVisible()) {
        crlfWindow->raise();
        crlfWindow->activateWindow();
        return;
    }

    QWidget* parentWidget = qobject_cast<QWidget*>(parent());
    
    if (!crlfFilterProxy) {
        crlfFilterProxy = new IndexRowReferenceModel(this);
    }
    crlfFilterProxy->setSourceModel(sourceModelOfDLT);
    
    lastBuildCanceled = false;
    rebuildCrlfModel();
    if(lastBuildCanceled)
    {
        return;
    }

    if(crlfFilterProxy->rowCount() == 0)
    {
        QMessageBox::information(parentWidget, "No CRLF Messages", 
            "No messages containing CRLF characters (\\r, \\n, or \\r\\n) were found in the current DLT file.");
        return;
    }

    crlfWindow = new QWidget(parentWidget);
    crlfWindow->setAttribute(Qt::WA_DeleteOnClose);
    crlfWindow->resize(1200, 700);
    
    connect(crlfWindow, &QWidget::destroyed, this, &CrlfFilterWindow::cleanup);
    connect(crlfWindow, &QWidget::destroyed, this, &QObject::deleteLater);
    
    crlfWindow->setWindowFlags(Qt::Window);
    
    QVBoxLayout* layout = new QVBoxLayout(crlfWindow);

    QToolBar* toolbar = new QToolBar;
    QHBoxLayout* topRowLayout = new QHBoxLayout();
    topRowLayout->addStretch();
    topRowLayout->addWidget(toolbar);
    layout->addLayout(topRowLayout);

    QPushButton* exportButton = new QPushButton("Export CRLF Messages");
    exportButton->setToolTip("Export all CRLF messages to DLT file");
    toolbar->addWidget(exportButton);
    connect(exportButton, &QPushButton::clicked, this, &CrlfFilterWindow::onExportFilteredCrlfLogsClicked);

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
    applyColumnSettings();
    
    connect(crlfTableView, &QTableView::doubleClicked, this, &CrlfFilterWindow::onCrlfMessageDoubleClicked);
    layout->addWidget(crlfTableView);
      
    statusLabel = new QLabel();
    
    // Apply theme-appropriate styling
    QPalette palette = statusLabel->palette();
    bool isDarkMode = palette.color(QPalette::Window).lightness() < palette.color(QPalette::WindowText).lightness();
    statusLabel->setStyleSheet(isDarkMode ? 
        "QLabel { padding: 5px; background-color: #3c3c3c; border-top: 1px solid #555; color: #ffffff; }" :
        "QLabel { padding: 5px; background-color: #f0f0f0; border-top: 1px solid #ccc; color: #000000; }");
    
    layout->addWidget(statusLabel);

    updateMessageCount(crlfFilterProxy->rowCount());
    lastFilteredMessageCount = dltFile->sizeFilter();

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
        QModelIndexList selectedIndices;
        selectedIndices.reserve(rowCount);

        for (int row = 0; row < rowCount; ++row) {
            if (progress && progress->wasCanceled()) {
                return;
            }

            const int sourceRow = crlfFilterProxy->sourceRowAt(row);
            if(sourceRow >= 0 && sourceRow < sourceModelOfDLT->rowCount())
            {
                selectedIndices.append(sourceModelOfDLT->index(sourceRow, 0));
            }

            if (progress && (row % 5 == 0 || row == rowCount - 1)) {
                progress->setValue(row + 1);
                progress->setLabelText(QString("Processing message %1 of %2...").arg(row + 1).arg(rowCount));
                QCoreApplication::processEvents();
            }
        }

        if (selectedIndices.isEmpty()) {
            if (progress) {
                progress->close();
                delete progress;
            }
            QMessageBox::warning(crlfWindow, "Export Error", "No valid CRLF messages found to export.");
            return;
        }

        if (progress) {
            progress->setLabelText("Creating DLT file...");
            progress->setRange(0, 0);  // Indeterminate progress for file export
        }
        QCoreApplication::processEvents();

        QDltExporter* exporter = new QDltExporter(
            dltFile,
            fileName,
            pluginManager,
            QDltExporter::FormatDlt,
            QDltExporter::SelectionSelected,
            &selectedIndices,
            0,
            0,
            0,
            ',',
            "DLTVIEWER",
            nullptr
            );

        exporter->exportMessages();
        delete exporter;

        if (progress) {
            progress->setLabelText("Export completed successfully!");
            QCoreApplication::processEvents();
            progress->close();
            delete progress;
        }
        QMessageBox::information(crlfWindow, "Export Complete", 
                               QString("Successfully exported %1 CRLF messages to %2")
                               .arg(selectedIndices.size()).arg(fileName));

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
    if (sourceModelOfDLT) {
        disconnect(sourceModelOfDLT, nullptr, this, nullptr);
    }
    
    sourceModelOfDLT = model;
    
    if (sourceModelOfDLT) {
        connect(sourceModelOfDLT, &QAbstractTableModel::modelReset, this, &CrlfFilterWindow::onSourceModelReset);
        connect(sourceModelOfDLT, &QAbstractTableModel::layoutChanged, this, &CrlfFilterWindow::onSourceModelDataChanged);
        
        if (QObject* parentObj = parent()) {
            disconnect(parentObj, SIGNAL(dltFileLoaded()), this, SLOT(onSourceModelDataChanged()));
            connect(parentObj, SIGNAL(dltFileLoaded()), this, SLOT(onSourceModelDataChanged()));
        }
    }

    if(crlfFilterProxy)
    {
        crlfFilterProxy->setSourceModel(sourceModelOfDLT);
    }
}

// Sets the DLT file reference
void CrlfFilterWindow::setDltFile(QDltFile* file) {
    dltFile = file;
    lastFilteredMessageCount = -1;

    if (crlfWindow && crlfWindow->isVisible() && dltFile) {
        if (!rebuildScheduled && !rebuildTimer->isActive() && !rebuildInProgress) {
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
    rebuildScheduled = false;
    rebuildInProgress = false;
    lastBuildCanceled = false;
    
    if (rebuildTimer && rebuildTimer->isActive()) {
        rebuildTimer->stop();
    }
    
    if (sourceModelOfDLT) {
        disconnect(sourceModelOfDLT, nullptr, this, nullptr);
    }
    
    if (crlfTableView && crlfFilterProxy) {
        crlfTableView->setModel(nullptr);
    }
    
    if (crlfFilterProxy) {
        crlfFilterProxy->deleteLater();
    }
    
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
    
    const int sourceRow = crlfFilterProxy->sourceRowAt(index.row());
    if(sourceRow < 0 || !dltFile)
    {
        return;
    }

    if(sourceRow >= dltFile->sizeFilter())
    {
        return;
    }

    int absolutePosition = dltFile->getMsgFilterPos(sourceRow);
    if (absolutePosition < 0 || absolutePosition >= dltFile->size()) {
        return;
    }

    emit jumpToMessageRequested(absolutePosition);

    if (QWidget* parentWidget = qobject_cast<QWidget*>(parent())) {
        parentWidget->raise();
        parentWidget->activateWindow();
    }
}

// Handle when source model data changes
void CrlfFilterWindow::onSourceModelDataChanged() {
    if (!crlfWindow || !crlfWindow->isVisible() || !dltFile) {
        return;
    }

    int currentFilteredCount = dltFile->sizeFilter();
    if (dltFile->size() == 0 || currentFilteredCount == 0) {
        lastFilteredMessageCount = 0;
        onSourceModelReset();
        return;
    }

    if (sourceModelOfDLT && sourceModelOfDLT->rowCount() != currentFilteredCount) {
        if (!rebuildScheduled && !rebuildTimer->isActive() && !rebuildInProgress) {
            rebuildScheduled = true;
            rebuildTimer->setInterval(750);
            rebuildTimer->start();
        }
        return;
    }

    rebuildTimer->setInterval(500);

    if (!rebuildScheduled && !rebuildTimer->isActive() && !rebuildInProgress) {
        rebuildScheduled = true;
        rebuildTimer->start();
    }
}

// Handle when source model is reset/cleared
void CrlfFilterWindow::onSourceModelReset() {
    if (rebuildTimer->isActive()) {
        rebuildTimer->stop();
    }
    rebuildScheduled = false;
    rebuildInProgress = false;
    lastFilteredMessageCount = -1;

    if (!crlfWindow || !crlfWindow->isVisible() || !crlfFilterProxy) {
        return;
    }

    if (dltFile && dltFile->sizeFilter() > 0) {
        rebuildScheduled = true;
        rebuildTimer->start();
    } else {
        crlfFilterProxy->setRowReferences(QVector<int>());

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

    lastBuildCanceled = false;

    if (!dltFile || dltFile->size() == 0) {
        crlfFilterProxy->setRowReferences(QVector<int>());
        updateMessageCount(0);
        return;
    }

    int totalFilteredMessages = dltFile->sizeFilter();
    if (totalFilteredMessages == 0) {
        crlfFilterProxy->setRowReferences(QVector<int>());
        updateMessageCount(0);
        lastFilteredMessageCount = 0;
        return;
    }

    const int rowsToProcess = qMin(totalFilteredMessages, sourceModelOfDLT ? sourceModelOfDLT->rowCount() : 0);

    QVector<int> crlfRows;
    crlfRows.reserve(qMax(1, rowsToProcess / 10));

    bool needsProgress = (totalFilteredMessages > 2000) && !isMainWindowBusy();

    QProgressDialog* buildProgress = nullptr;
    if (needsProgress) {
        buildProgress = new QProgressDialog("Rebuilding CRLF data...", "Cancel", 0, totalFilteredMessages, crlfWindow);
        buildProgress->setWindowModality(Qt::WindowModal);
        buildProgress->setMinimumDuration(0);
        buildProgress->show();
    }

    int processCount = 0;
    for (int i = 0; i < rowsToProcess; i++) {
        if (buildProgress && buildProgress->wasCanceled()) {
            lastBuildCanceled = true;
            crlfFilterProxy->setRowReferences(QVector<int>());
            buildProgress->close();
            delete buildProgress;
            lastFilteredMessageCount = totalFilteredMessages;
            return;
        }

        if(dltFile)
        {
            const int absoluteRow = dltFile->getMsgFilterPos(i);
            if(absoluteRow >= 0 && absoluteRow < dltFile->size())
            {
                QDltMsg msg;
                if(dltFile->getMsg(absoluteRow, msg))
                {
                    const QString rawPayload = msg.toStringPayload();
                    if(containsCrlf(rawPayload))
                    {
                        crlfRows.append(i);
                    }
                }
            }
        }

        processCount++;
        if (buildProgress && processCount % 200 == 0) {
            buildProgress->setValue(i);
            QCoreApplication::processEvents();
        }
    }

    if (buildProgress) {
        buildProgress->close();
        delete buildProgress;
    }

    crlfFilterProxy->setRowReferences(crlfRows);

    updateMessageCount(crlfRows.size());
    lastFilteredMessageCount = totalFilteredMessages;

    applyColumnSettings();
}

// Debounced rebuild triggered by timer
void CrlfFilterWindow::onRebuildTimerTimeout() {
    rebuildScheduled = false;
    
    if (!crlfWindow || !crlfWindow->isVisible()) {
        return;
    }
    
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
        const bool wasVisible = crlfWindow->isVisible();
        if(wasVisible)
        {
            crlfWindow->hide();
        }

        rebuildCrlfModel();

        if(lastBuildCanceled)
        {
            if(wasVisible)
            {
                crlfWindow->show();
                crlfWindow->raise();
                crlfWindow->activateWindow();
            }
            return;
        }

        if(crlfFilterProxy && crlfFilterProxy->rowCount() > 0)
        {
            crlfWindow->show();
            crlfWindow->raise();
            crlfWindow->activateWindow();
        }
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

// Check if parent MainWindow has background operations in progress
bool CrlfFilterWindow::isMainWindowBusy() const {
    MainWindow* mainWindow = qobject_cast<MainWindow*>(parent());
    if (mainWindow) {
        return mainWindow->isBackgroundOperationInProgress();
    }
    return false;
}
