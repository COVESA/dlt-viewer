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
#include <QThread>
#include <QFileDialog>

#include "crlffilterwindow.h"
#include "fieldnames.h"
#include "qdltfile.h"
#include "qdltexporter.h"
#include "qdltsettingsmanager.h"
#include "qdltoptmanager.h"

CrlfFilterWindow::CrlfFilterWindow(QObject* parent) : QObject(parent) {
    sourceModelOfDLT = nullptr;
    crlfFilterProxy = nullptr;
    crlfWindow = nullptr;
    crlfTableView = nullptr;
    dltFile = nullptr;
    pluginManager = nullptr;
}

// Creates a single window displaying all CRLF messages
void CrlfFilterWindow::createCrlfWindow() {
    // Check if there are any CRLF messages
    if (!dltFile || dltFile->size() == 0) {
        QMessageBox::information(nullptr, "No DLT file", "No DLT file is currently loaded.");
        return;
    }

    // First: Count CRLF messages with analyzing popup
    int crlfMessageCount = 0;
    QProgressDialog countProgress("Analyzing DLT messages for CRLF patterns...", "Cancel", 0, dltFile->size());
    countProgress.setWindowModality(Qt::ApplicationModal);
    countProgress.setMinimumDuration(500);
    countProgress.setWindowTitle("CRLF Analysis Progress");
    countProgress.show();

    for (int i = 0; i < dltFile->size(); i++) {
        if (countProgress.wasCanceled()) {
            return;
        }
        countProgress.setValue(i);
        QCoreApplication::processEvents();
        
        QDltMsg msg;
        if (dltFile->getMsg(i, msg)) {
            QString payload = msg.toStringPayload();
            if (payload.contains("\r") || payload.contains("\n")) {
                crlfMessageCount++;
            }
        }
    }
    countProgress.close();
    
    if (crlfMessageCount == 0) {
        QMessageBox::information(nullptr, "No CRLF Messages", "No messages containing CRLF characters (\\r, \\n, or \\r\\n) were found in the current DLT file.");
        return;
    }

    // Second: Show preparing window popup after analysis is complete
    QProgressDialog preparingProgress("Preparing CRLF Filter Window...", "Cancel", 0, 100);
    preparingProgress.setWindowModality(Qt::ApplicationModal);
    preparingProgress.setMinimumDuration(0);
    preparingProgress.setWindowTitle("CRLF Filter - Preparation");
    preparingProgress.show();
    preparingProgress.setValue(10);
    QCoreApplication::processEvents();
    
    QThread::msleep(300);
    preparingProgress.setValue(50);
    preparingProgress.setLabelText("Setting up CRLF filter window...");
    QCoreApplication::processEvents();

    // Create main window with proper parent relationship
    QWidget* parentWidget = qobject_cast<QWidget*>(parent());
    crlfWindow = new QWidget(parentWidget);
    crlfWindow->setAttribute(Qt::WA_DeleteOnClose);
    crlfWindow->setWindowTitle(QString("CRLF Messages (%1 found)").arg(crlfMessageCount));
    crlfWindow->resize(1200, 700);
    
    // Connect window close event to cleanup
    connect(crlfWindow, &QWidget::destroyed, this, &CrlfFilterWindow::cleanup);
    connect(crlfWindow, &QWidget::destroyed, this, &QObject::deleteLater);
    
    crlfWindow->setWindowFlags(Qt::Window);
    crlfWindow->setAttribute(Qt::WA_ShowWithoutActivating, false);
    
    preparingProgress.setValue(90);
    QCoreApplication::processEvents();
    
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

    // Create the table view with CRLF filter
    crlfFilterProxy = new EcuIdFilterProxyModel(crlfWindow);
    if (!sourceModelOfDLT) {
        QMessageBox::critical(nullptr, "Error", "No source model available for CRLF filtering.");
        return;
    }
    crlfFilterProxy->setSourceModel(sourceModelOfDLT);
    crlfFilterProxy->setDltFile(dltFile);
    //Model reset protection
    connect(sourceModelOfDLT, &QAbstractTableModel::modelAboutToBeReset, this, [this]() {
        if (crlfFilterProxy) {
            crlfFilterProxy->setSourceModel(nullptr);
        }
    });
    
    connect(sourceModelOfDLT, &QAbstractTableModel::modelReset, this, [this]() {
        if (crlfFilterProxy && sourceModelOfDLT) {
            crlfFilterProxy->setSourceModel(sourceModelOfDLT);
            crlfFilterProxy->invalidate();
        }
    });
    
    // Protection for layout changes
    connect(sourceModelOfDLT, &QAbstractTableModel::layoutAboutToBeChanged, this, [this]() {
        if (crlfFilterProxy) {
            crlfFilterProxy->setSourceModel(nullptr);
        }
    });
    
    connect(sourceModelOfDLT, &QAbstractTableModel::layoutChanged, this, [this]() {
        if (crlfFilterProxy && sourceModelOfDLT) {
            crlfFilterProxy->setSourceModel(sourceModelOfDLT);
            crlfFilterProxy->invalidate();
        }
    });
    
    // Add protection for row operations
    connect(sourceModelOfDLT, &QAbstractTableModel::rowsAboutToBeInserted, this, [this]() {
        if (crlfFilterProxy) {
            crlfFilterProxy->setSourceModel(nullptr);
        }
    });
    
    connect(sourceModelOfDLT, &QAbstractTableModel::rowsInserted, this, [this]() {
        if (crlfFilterProxy && sourceModelOfDLT) {
            crlfFilterProxy->setSourceModel(sourceModelOfDLT);
            crlfFilterProxy->invalidate();
        }
    });
    
    connect(sourceModelOfDLT, &QAbstractTableModel::rowsAboutToBeRemoved, this, [this]() {
        if (crlfFilterProxy) {
            crlfFilterProxy->setSourceModel(nullptr);
        }
    });
    
    connect(sourceModelOfDLT, &QAbstractTableModel::rowsRemoved, this, [this]() {
        if (crlfFilterProxy && sourceModelOfDLT) {
            crlfFilterProxy->setSourceModel(sourceModelOfDLT);
            crlfFilterProxy->invalidate();
        }
    });
    
    // Now enable CRLF filtering
    crlfFilterProxy->setCrlfFilter(true);
  
    crlfTableView = new QTableView;
    crlfTableView->setModel(crlfFilterProxy);
    crlfTableView->horizontalHeader()->setStretchLastSection(true);
    crlfTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    crlfTableView->setAlternatingRowColors(true);
    
    // Data is already sorted by index ascending
    crlfTableView->setSortingEnabled(false);
    crlfTableView->horizontalHeader()->setSortIndicatorShown(false);
    
    // Make columns adjustable/resizable 
    crlfTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    crlfTableView->horizontalHeader()->setStretchLastSection(false);
    
    // Configure column visibility and width
    auto settings = QDltSettingsManager::getInstance();
    
    for (int col = 0; col < crlfFilterProxy->columnCount(); ++col) {
        bool show = FieldNames::getColumnShown(static_cast<FieldNames::Fields>(col), settings);
        crlfTableView->setColumnHidden(col, !show);
        if (show) {
            int width = FieldNames::getColumnWidth(static_cast<FieldNames::Fields>(col), settings);
            crlfTableView->setColumnWidth(col, width);
        }
    }

    // Connect double-click signal for navigation to main window
    connect(crlfTableView, &QTableView::doubleClicked, this, &CrlfFilterWindow::onCrlfMessageDoubleClicked);
    
    layout->addWidget(crlfTableView);
      
    // Add status bar
    QLabel* statusLabel = new QLabel(QString("Total CRLF messages: %1 ").arg(crlfMessageCount));
    statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #f0f0f0; border-top: 1px solid #ccc; }");
    layout->addWidget(statusLabel);

    // Complete preparation and show window on top
    preparingProgress.setValue(100);
    preparingProgress.setLabelText("CRLF window ready!");
    QCoreApplication::processEvents();
    
    // Close preparation dialog and show the CRLF window
    preparingProgress.close();
    
    // Show window and bring to front
    crlfWindow->show();
    crlfWindow->raise();
    crlfWindow->activateWindow();
    
    // Ensure window stays visible
    QTimer::singleShot(100, [this]() {
        if (crlfWindow) {
            crlfWindow->raise();
            crlfWindow->activateWindow();
        }
    });
}

// Exports all filtered CRLF DLT logs to a file
void CrlfFilterWindow::onExportFilteredCrlfLogsClicked() {
    if (!dltFile || !sourceModelOfDLT || !crlfFilterProxy || !crlfTableView) {
        QMessageBox::information(nullptr, "Export Error", "No CRLF data available to export.");
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

    QProgressDialog progress("Exporting CRLF messages...", "Cancel", 0, 100, crlfWindow);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.show();

    try {
        QModelIndexList selectedIndices;
        
        // Map all proxy model indices back to source model indices
        for (int row = 0; row < rowCount; ++row) {
            if (progress.wasCanceled()) {
                return;
            }
            
            QModelIndex proxyIndex = crlfFilterProxy->index(row, 0);
            QModelIndex sourceIndex = crlfFilterProxy->mapToSource(proxyIndex);
            selectedIndices.append(sourceIndex);
            
            // Update progress
            int progressValue = static_cast<int>((static_cast<double>(row + 1) / rowCount) * 100);
            progress.setValue(progressValue);
            QCoreApplication::processEvents();
        }

        // Create and configure the exporter
        QDltExporter* exporter = new QDltExporter(
            dltFile,                                    // Source DLT file
            fileName,                                   // Output filepath
            pluginManager,                              // Plugin manager (can be nullptr)
            QDltExporter::FormatDlt,                    // Export in DLT format
            QDltExporter::SelectionSelected,            // Export selected messages
            &selectedIndices,                           // List of selected indices
            0,                                          // Automatic time settings
            0,                                          // UTC offset
            0,                                          // DST
            ',',                                        // Delimiter (not used for DLT format)
            "DLTVIEWER",                                // Signature
            nullptr                                     // No parent to avoid threading issues
            );
        exporter->exportMessages();
        delete exporter;

        progress.close();
        QMessageBox::information(crlfWindow, "Export Complete", 
                               QString("Successfully exported %1 CRLF messages to %2")
                               .arg(selectedIndices.size()).arg(fileName));

    } catch (const std::exception &e) {
        progress.close();
        QMessageBox::critical(crlfWindow, "Export Error", QString("Failed to export: %1").arg(e.what()));
    }
}

// Sets the source model for DLT data
void CrlfFilterWindow::setSourceModel(QAbstractTableModel* model) {
    sourceModelOfDLT = model;
}

// Sets the DLT file reference
void CrlfFilterWindow::setDltFile(QDltFile* file) {
    dltFile = file;
}

// Sets the plugin manager reference
void CrlfFilterWindow::setPluginManager(QDltPluginManager* manager) {
    pluginManager = manager;
}

// Cleanup method to properly disconnect from models/signals
void CrlfFilterWindow::cleanup() {
    if (crlfFilterProxy) {
        crlfFilterProxy->setSourceModel(nullptr);
        crlfFilterProxy = nullptr;
    }
    if (sourceModelOfDLT) {
        disconnect(sourceModelOfDLT, nullptr, this, nullptr);
    }
    if (crlfWindow) {
        crlfWindow->close();
        crlfWindow = nullptr;
    }
    crlfTableView = nullptr;
}

// Check if source model is still valid
bool CrlfFilterWindow::isSourceModelValid() const {
    return sourceModelOfDLT != nullptr && crlfFilterProxy != nullptr && crlfFilterProxy->sourceModel() == sourceModelOfDLT;
}

// Handle double-click on CRLF message row to navigate to main window
void CrlfFilterWindow::onCrlfMessageDoubleClicked(const QModelIndex& index) {
    if (!index.isValid() || !crlfFilterProxy || !sourceModelOfDLT) {
        return;
    }
    
    // Map the proxy model index back to the source model index
    QModelIndex sourceIndex = crlfFilterProxy->mapToSource(index);
    if (!sourceIndex.isValid()) {
        return;
    }
    
    // Get the actual message index (row number in the source model)
    int messageIndex = sourceIndex.row();
    
    // Emit signal to request navigation to this message in the main window
    emit jumpToMessageRequested(messageIndex);
    
    // Optionally bring the main window to front
    if (QWidget* parentWidget = qobject_cast<QWidget*>(parent())) {
        parentWidget->raise();
        parentWidget->activateWindow();
    }
}
