#include <QListWidget>
#include <QBoxLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QPushButton>
#include <QToolBar>
#include <QTableView>
#include <QLabel>
#include <QMessageBox>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QProgressBar>
#include <QThread>

#include <algorithm>

#include "filtergrouplogs.h"
#include "fieldnames.h"
#include "indexservice.h"
#include "qdltfileprojection.h"
#include "qdltfile.h"
#include "qdltexporter.h"
#include "qdltsettingsmanager.h"

CFilterGroupLogs::CFilterGroupLogs(QObject* parent) : QObject(parent) {
    m_sourceModelOfDLT = nullptr;
    m_mergedTabWidget = nullptr;
    dltFile = nullptr;
    pluginManager = nullptr;
    messageStore = nullptr;
    indexService = nullptr;
    decodeCacheService = nullptr;
}

// Extracts unique ECU IDs from a DLT file
QStringList CFilterGroupLogs::extractEcuIds(const QString& dltFilePath) {
    if (messageStore && dltFile) {
        QSet<QString> uniqueEcuIds;
        const auto &allIds = messageStore->snapshotAllMessageIds();
        QDltMsg msg;
        for (const MessageId messageId : allIds) {
            if (!messageStore->message(messageId, msg))
                continue;

            const QString ecuId = msg.getEcuid();
            if (!ecuId.isEmpty())
                uniqueEcuIds.insert(ecuId);
        }
        m_extractedEcuIds = QStringList(uniqueEcuIds.begin(), uniqueEcuIds.end());
        return m_extractedEcuIds;
    }

    if (decodeCacheService && dltFile) {
        QSet<QString> uniqueEcuIds;
        const int total = dltFile->size();
        const bool decodeEnabled = false;
        const int triggeredByUser = 0;
        QDltMsg msg;
        for (int i = 0; i < total; ++i) {
            if (!decodeCacheService->message(dltFile,
                                               pluginManager,
                                               i,
                                               decodeEnabled,
                                               triggeredByUser,
                                               msg,
                                               true)) {
                continue;
            }

            const QString ecuId = msg.getEcuid();
            if (!ecuId.isEmpty())
                uniqueEcuIds.insert(ecuId);
        }
        m_extractedEcuIds = QStringList(uniqueEcuIds.begin(), uniqueEcuIds.end());
        return m_extractedEcuIds;
    }

    QDltFile dltFile;
    QSet<QString> uniqueEcuIds;
    if (!dltFile.open(dltFilePath)) {
        return QStringList();
    }
    if (!dltFile.createIndex()) {
        dltFile.close();
        return QStringList();
    }
    CDecodeCacheService localDecodeCacheService;
    CDecodeCacheService *decodeService = decodeCacheService ? decodeCacheService : &localDecodeCacheService;
    for (int i = 0; i < dltFile.size(); i++) {
        QDltMsg msg;
        const bool decodeEnabled = false;
        const int triggeredByUser = 0;
        const bool ok = decodeService->message(&dltFile,
                                               pluginManager,
                                               i,
                                               decodeEnabled,
                                               triggeredByUser,
                                               msg,
                                               true);

        if (ok) {
            QString ecuId = msg.getEcuid();
            if (!ecuId.isEmpty()) {
                uniqueEcuIds.insert(ecuId);
            }
        }
    }
    dltFile.close();
    m_extractedEcuIds = QStringList(uniqueEcuIds.begin(), uniqueEcuIds.end());
    return m_extractedEcuIds;
}

void CFilterGroupLogs::setMessageStore(CMessageStore *messageStore)
{
    this->messageStore = messageStore;
}

void CFilterGroupLogs::setIndexService(const CIndexService *indexService)
{
    this->indexService = indexService;
}

void CFilterGroupLogs::setDecodeCacheService(CDecodeCacheService *decodeCacheService)
{
    this->decodeCacheService = decodeCacheService;
}

// Creates tabs for each ECU ID and sets up the tab window UI
void CFilterGroupLogs::ecuIdTabs(){
    if (!dltFile || !m_sourceModelOfDLT) {
        QMessageBox::information(nullptr, "No DLT file opened", "No DLT file is opened. Please open a DLT file");
        return;
    }

    QStringList availableEcuIds = m_extractedEcuIds;
    if (availableEcuIds.isEmpty()) {
        QMessageBox::information(nullptr, "No ECU IDs", "No ECU IDs were found in the current DLT file.");
        return;
    }

    CIndexService localIndexService;
    const CIndexService *activeIndexService = indexService ? indexService : &localIndexService;
    const std::vector<int> filteredProjection =
        activeIndexService->snapshotProjection(buildActiveFilteredProjection(dltFile));
    m_ecuSourceRowProjection.clear();

    QSet<QString> normalizedRequestedEcus;
    for (const QString &ecuId : availableEcuIds) {
        normalizedRequestedEcus.insert(ecuId.trimmed().toLower());
    }

    QDltMsg msg;
    const int triggeredByUser = 0;
    const bool decodeEnabled = false;
    for (int sourceRow = 0; sourceRow < static_cast<int>(filteredProjection.size()); ++sourceRow) {
        const int globalIndex = filteredProjection.at(static_cast<std::size_t>(sourceRow));
        if (globalIndex < 0) {
            continue;
        }

        bool gotMessage = false;
        if (decodeCacheService) {
            gotMessage = decodeCacheService->message(dltFile,
                                                       pluginManager,
                                                       globalIndex,
                                                       decodeEnabled,
                                                       triggeredByUser,
                                                       msg,
                                                       true);
        } else if (messageStore) {
            const MessageId messageId = messageStore->messageIdForGlobalIndex(globalIndex);
            gotMessage = (messageId != kInvalidMessageId) && messageStore->message(messageId, msg);
        }

        if (!gotMessage) {
            continue;
        }

        const QString normalizedEcu = msg.getEcuid().trimmed().toLower();
        if (!normalizedRequestedEcus.contains(normalizedEcu)) {
            continue;
        }

        m_ecuSourceRowProjection[normalizedEcu].push_back(sourceRow);
    }

    /* Main Tab Window */
    QWidget* tabWindow = new QWidget;
    tabWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(tabWindow, &QWidget::destroyed, this, &QObject::deleteLater);
    tabWindow->setWindowTitle("DLT Logs by ECU");
    tabWindow->resize(1000, 600);
    QVBoxLayout* layout = new QVBoxLayout(tabWindow);

    QToolBar* toolbar = new QToolBar;
    QHBoxLayout* topRowLayout = new QHBoxLayout();
    topRowLayout->addStretch();
    topRowLayout->addWidget(toolbar);
    layout->addLayout(topRowLayout);

    QPushButton* mergeTabsButton = new QPushButton("Merge Tabs");
    toolbar->addWidget(mergeTabsButton);
    QPushButton* exportButton = new QPushButton("Export");
    toolbar->addWidget(exportButton);

    connect(mergeTabsButton, &QPushButton::clicked, this, &CFilterGroupLogs::openMergeTabsDialog);
    connect(exportButton, &QPushButton::clicked, this, &CFilterGroupLogs::onExportFilteredLogsClicked);

    /* Tab Widget */
    m_mergedTabWidget = new QTabWidget(tabWindow);
    layout->addWidget(m_mergedTabWidget);
    m_mergedTabWidget->setTabsClosable(true);
    connect(m_mergedTabWidget, &QTabWidget::tabCloseRequested, this, &CFilterGroupLogs::onTabCloseRequested);

    int totalEcuIds = availableEcuIds.size();
    // Show loading dialog before launching tab window
    QProgressDialog loadingDialog("Preparing ECU ID tabs...", "Cancel", 0, 100, nullptr);
    loadingDialog.setWindowModality(Qt::ApplicationModal);
    loadingDialog.setMinimumDuration(0);
    loadingDialog.setValue(0);
    loadingDialog.setWindowTitle("Loading Tabs");
    loadingDialog.show();
    for (int i = 0; i < availableEcuIds.size(); ++i) {
        if (loadingDialog.wasCanceled()) {
            loadingDialog.close();
            return;
        }
        const QString &ecuId = availableEcuIds[i];
        int progressValue = ((i + 1) * 100) / totalEcuIds;
        loadingDialog.setValue(progressValue);
        QCoreApplication::processEvents();

        ProjectionTableModel *projectionModel = new ProjectionTableModel(this);
        projectionModel->setSourceModel(m_sourceModelOfDLT);
        const auto projectionIt = m_ecuSourceRowProjection.find(ecuId.trimmed().toLower());
        projectionModel->setProjectionRows(projectionIt != m_ecuSourceRowProjection.end() ? projectionIt->second : std::vector<int>());

        QTableView* view = new QTableView;
        view->setModel(projectionModel);
        view->horizontalHeader()->setStretchLastSection(true);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->resizeColumnsToContents();
        // Hide unwanted columns
        auto settings = QDltSettingsManager::getInstance();
        for (int col = 0; col < projectionModel->columnCount(); ++col) {
            bool show = FieldNames::getColumnShown(static_cast<FieldNames::Fields>(col), settings);
            view->setColumnHidden(col, !show);
            if (show) {
                int width = FieldNames::getColumnWidth(static_cast<FieldNames::Fields>(col), settings);
                view->setColumnWidth(col, width);
            }
        }
        int tabIndex = m_mergedTabWidget->addTab(view, ecuId);
        m_ecuTabViews[ecuId] = view;
        m_mergedTabWidget->tabBar()->setTabButton(tabIndex, QTabBar::RightSide, nullptr);
    }
    loadingDialog.close();
    tabWindow->setAttribute(Qt::WA_ShowModal, true);
    tabWindow->show();
}

// Opens a dialog to select and merge multiple ECU tabs
void CFilterGroupLogs::openMergeTabsDialog()
{
    QDialog dialog(m_mergedTabWidget);
    dialog.setWindowTitle("Select Tabs to Merge");
    QVBoxLayout* mergeTabsListLayout = new QVBoxLayout(&dialog);
    QListWidget* listWidget = new QListWidget(&dialog);
    listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    for (const QString& id : m_ecuTabViews.keys()) {
        if (m_mergedTabWidget->indexOf(m_ecuTabViews[id]) >= 0) {
            listWidget->addItem(id);
        }
    }
    dialog.resize(150, 150);
    mergeTabsListLayout->addWidget(listWidget);
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mergeTabsListLayout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() == QDialog::Accepted) {
        for (QListWidgetItem* item : listWidget->selectedItems()) {
            m_selectedEcuIdSet << item->text();
        }
        if (m_selectedEcuIdSet.size() >= 2) {
            mergeTabs();
        }
    }
}

// Merges selected ECU tabs into a single tab
void CFilterGroupLogs::mergeTabs()
{
    if (m_selectedEcuIdSet.isEmpty())
        return;
    QStringList selectedIds = m_selectedEcuIdSet.values();
    QString tabKey = selectedIds.join(" / ");
    if (m_mergedTabs.contains(tabKey)) {
        int existingIndex = m_mergedTabWidget->indexOf(m_mergedTabs[tabKey]);
        if (existingIndex != -1)
            m_mergedTabWidget->setCurrentIndex(existingIndex);
        return;
    }
    QDialog* loadingDialog = new QDialog(m_mergedTabWidget);
    loadingDialog->setWindowTitle("Merging Tabs");
    loadingDialog->setWindowModality(Qt::ApplicationModal);

    QVBoxLayout* loadingLayout = new QVBoxLayout(loadingDialog);
    loadingLayout->addWidget(new QLabel("Merging selected ECU tabs...", loadingDialog));

    QProgressBar* progressBar = new QProgressBar(loadingDialog);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setMinimumWidth(200);
    loadingLayout->addWidget(progressBar);

    loadingDialog->setLayout(loadingLayout);
    loadingDialog->show();

    for (int i = 0; i <= 100; i += 20) {
        progressBar->setValue(i);
        QCoreApplication::processEvents();
    }

    std::vector<int> mergedProjectionRows;
    mergedProjectionRows.reserve(static_cast<std::size_t>(m_sourceModelOfDLT->rowCount()));
    for (const QString &selectedId : selectedIds) {
        const auto projectionIt = m_ecuSourceRowProjection.find(selectedId.trimmed().toLower());
        if (projectionIt == m_ecuSourceRowProjection.end()) {
            continue;
        }

        const std::vector<int> &rows = projectionIt->second;
        for (const int row : rows) {
            mergedProjectionRows.push_back(row);
        }
    }

    std::sort(mergedProjectionRows.begin(), mergedProjectionRows.end());
    mergedProjectionRows.erase(std::unique(mergedProjectionRows.begin(), mergedProjectionRows.end()), mergedProjectionRows.end());

    ProjectionTableModel *projectionModel = new ProjectionTableModel(this);
    projectionModel->setSourceModel(m_sourceModelOfDLT);
    projectionModel->setProjectionRows(mergedProjectionRows);

    QTableView* mergedView = new QTableView;
    mergedView->setModel(projectionModel);
    mergedView->horizontalHeader()->setStretchLastSection(true);
    mergedView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mergedView->resizeColumnsToContents();
    auto settings = QDltSettingsManager::getInstance();
    for (int col = 0; col < projectionModel->columnCount(); ++col) {
        bool show = FieldNames::getColumnShown(static_cast<FieldNames::Fields>(col), settings);
        mergedView->setColumnHidden(col, !show);
    }
    int mergedtabIndex = m_mergedTabWidget->addTab(mergedView, tabKey);
    m_mergedTabWidget->setCurrentIndex(mergedtabIndex);
    m_indexOfMergedTabs[mergedtabIndex] = tabKey;
    m_mergedTabs[tabKey] = mergedView;
    m_tabToSelectedIds[mergedView] = selectedIds;
    m_selectedEcuIdSet.clear();
    loadingDialog->close();
    loadingDialog->deleteLater();
}

// Handles closing of a tab and updates internal tab tracking
void CFilterGroupLogs::onTabCloseRequested(int index) {
    QWidget* widget = m_mergedTabWidget->widget(index);
    if (!widget)
        return;
    // Find the correct tabKey for this widget
    QString tabKey;
    for (auto it = m_mergedTabs.begin(); it != m_mergedTabs.end(); ++it) {
        if (it.value() == widget) {
            tabKey = it.key();
            break;
        }
    }
    if (tabKey.isEmpty())
        return;
    m_mergedTabWidget->removeTab(index);
    m_mergedTabs.remove(tabKey);
    m_tabToSelectedIds.remove(widget);
    widget->deleteLater();
    m_indexOfMergedTabs.clear();
    // Rearrange tab indices once after deletion of any tab
    for (int i = 0; i < m_mergedTabWidget->count(); ++i) {
        QWidget* w = m_mergedTabWidget->widget(i);
        for (auto it = m_mergedTabs.begin(); it != m_mergedTabs.end(); ++it) {
            if (it.value() == w) {
                m_indexOfMergedTabs[i] = it.key();
                break;
            }
        }
    }
}

// Exports the filtered DLT logs from the selected tab to a file
void CFilterGroupLogs::onExportFilteredLogsClicked() {
    if (!dltFile || !m_sourceModelOfDLT) {
        QMessageBox::information(nullptr, "No DLT file opened", "No DLT file is opened. Please open a DLT file");
        return;
    }
    
    QStringList tabNames;
    for (int i = 0; i < m_mergedTabWidget->count(); ++i) {
        tabNames << m_mergedTabWidget->tabText(i);
    }
    if (tabNames.isEmpty()) {
        QMessageBox::information(m_mergedTabWidget, "Export", "No tabs available to export.");
        return;
    }
    bool ok = false;
    QString selectedTab = QInputDialog::getItem(m_mergedTabWidget, "Select Tab to Export",
                                                "Choose Tab:", tabNames, 0, false, &ok);
    if (!ok || selectedTab.isEmpty()) {
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(m_mergedTabWidget, "Export DLT Logs", 
                                                    selectedTab + ".dlt", 
                                                    "DLT Files (*.dlt);;All Files (*)");
    if (fileName.isEmpty()) {
        return;
    }

    int tabIndex = tabNames.indexOf(selectedTab);
    QWidget* tabWidget = m_mergedTabWidget->widget(tabIndex);
    QTableView* tableView = qobject_cast<QTableView*>(tabWidget);
    if (!tableView) {
        QMessageBox::critical(m_mergedTabWidget, "Export Error", "Could not find table view for selected tab.");
        return;
    }
    ProjectionTableModel* projectionModel = qobject_cast<ProjectionTableModel*>(tableView->model());
    if (!projectionModel || !projectionModel->sourceModel()) {
        QMessageBox::critical(m_mergedTabWidget, "Export Error", "Could not access projection model.");
        return;
    }

    QProgressDialog progress("Exporting filtered DLT messages...", "Cancel", 0, 100, m_mergedTabWidget);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.show();
    try {
        QModelIndexList selectedIndices;
        int rowCount = projectionModel->rowCount();
        if (rowCount == 0) {
            QMessageBox::information(m_mergedTabWidget, "Export", "No messages to export in selected tab.");
            return;
        }

        // Map projection rows back to source model indices.
        for (int row = 0; row < rowCount; ++row) {
            if (progress.wasCanceled()) {
                return;
            }
            const int sourceRow = projectionModel->sourceRowForRow(row);
            QModelIndex sourceIndex = m_sourceModelOfDLT->index(sourceRow, 0);
            if (sourceIndex.isValid()) {
                selectedIndices.append(sourceIndex);
            }
            progress.setValue((row * 50) / rowCount);
            QCoreApplication::processEvents();
        }
        if (selectedIndices.isEmpty()) {
            QMessageBox::information(m_mergedTabWidget, "Export", "No valid messages found to export.");
            return;
        }
        progress.setLabelText("Writing DLT file...");
        progress.setValue(50);
        QCoreApplication::processEvents();

        // Create and configure the exporter
        QDltExporter* exporter = new QDltExporter(
            dltFile,                                    // Source DLT file
            fileName,                                   // Output filename
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

        // Connect completion signal
        bool exportCompleted = false;
        QString exportResult;
        connect(exporter, &QDltExporter::resultReady, [&exportCompleted, &exportResult, exporter](const QString& result) {
            exportResult = result;
            exportCompleted = true;
            exporter->deleteLater();
        });
        exporter->setPriority(QThread::LowPriority);
        exporter->start();
        while (!exportCompleted && exporter->isRunning()) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            if (progress.wasCanceled()) {
                exporter->terminate();
                exporter->wait(3000);
                if (exporter) {
                    exporter->deleteLater();
                }
                return;
            }
            int currentValue = progress.value();
            if (currentValue < 90) {
                progress.setValue(currentValue + 1);
            }
        }
        progress.setValue(100);
        if (exportResult.isEmpty()) {
            QMessageBox::information(m_mergedTabWidget, "Export Complete", 
                                     QString("Successfully exported filtered DLT messages to:\n%1").arg(fileName));
        } else {
            QMessageBox::information(m_mergedTabWidget, "Export Complete", 
                                     QString("Export completed with result:\n%1").arg(exportResult));
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(m_mergedTabWidget, "Export Error", 
                              QString("An error occurred during export:\n%1").arg(e.what()));
    } catch (...) {
        QMessageBox::critical(m_mergedTabWidget, "Export Error", 
                              "An unknown error occurred during export.");
    }
}

// Sets the source model for DLT data
void CFilterGroupLogs::setSourceModel(QAbstractTableModel* model) {
    m_sourceModelOfDLT = model;
}

// Sets the DLT file reference
void CFilterGroupLogs::setDltFile(QDltFile* file) {
    dltFile = file;
}

// Sets the plugin manager reference
void CFilterGroupLogs::setPluginManager(QDltPluginManager* manager) {
    pluginManager = manager;
}

