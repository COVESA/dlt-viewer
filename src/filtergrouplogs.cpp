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

#include "filtergrouplogs.h"
#include "fieldnames.h"
#include "qdltfile.h"
#include "qdltexporter.h"
#include "qdltsettingsmanager.h"

filtergrouplogs::filtergrouplogs(QObject* parent) : QObject(parent) {
    sourceModelOfDLT = nullptr;
    ecuIdFilterProxy = nullptr;
    mergedTabWidget = nullptr;
    dltFile = nullptr;
    pluginManager = nullptr;
}

// Extracts unique ECU IDs from a DLT file
QStringList filtergrouplogs::extractEcuIds(const QString& dltFilePath) {
    QDltFile dltFile;
    QSet<QString> uniqueEcuIds;
    if (!dltFile.open(dltFilePath)) {
        return QStringList();
    }
    if (!dltFile.createIndex()) {
        dltFile.close();
        return QStringList();
    }
    for (int i = 0; i < dltFile.size(); i++) {
        QDltMsg msg;
        if (dltFile.getMsg(i, msg)) {
            QString ecuId = msg.getEcuid();
            if (!ecuId.isEmpty()) {
                uniqueEcuIds.insert(ecuId);
            }
        }
    }
    dltFile.close();
    extractedEcuIds = QStringList(uniqueEcuIds.begin(), uniqueEcuIds.end());
    return extractedEcuIds;
}

// Creates tabs for each ECU ID and sets up the tab window UI
void filtergrouplogs::ecuIdTabs(){
    QStringList availableEcuIds = extractedEcuIds;

    /* Main Tab Window */
    QWidget* tabWindow = new QWidget;
    tabWindow->setAttribute(Qt::WA_DeleteOnClose);
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

    connect(mergeTabsButton, &QPushButton::clicked, this, &filtergrouplogs::openMergeTabsDialog);
    connect(exportButton, &QPushButton::clicked, this, &filtergrouplogs::onExportFilteredLogsClicked);

    /* Tab Widget */
    mergedTabWidget = new QTabWidget(tabWindow);
    layout->addWidget(mergedTabWidget);
    mergedTabWidget->setTabsClosable(true);
    connect(mergedTabWidget, &QTabWidget::tabCloseRequested, this, &filtergrouplogs::onTabCloseRequested);

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
        QThread::msleep(30);

        ecuIdFilterProxy = new EcuIdFilterProxyModel(this);
        if (!ecuIdFilterProxy) {
            continue;
        }
        ecuIdFilterProxy->setSourceModel(sourceModelOfDLT);
        ecuIdFilterProxy->setEcuColumn(ecuColumnIndex);
        ecuIdFilterProxy->setEcuId(ecuId);
        QTableView* view = new QTableView;
        view->setModel(ecuIdFilterProxy);
        view->horizontalHeader()->setStretchLastSection(true);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->resizeColumnsToContents();
        // Hide unwanted columns
        auto settings = QDltSettingsManager::getInstance();
        for (int col = 0; col < ecuIdFilterProxy->columnCount(); ++col) {
            bool show = FieldNames::getColumnShown(static_cast<FieldNames::Fields>(col), settings);
            view->setColumnHidden(col, !show);
            if (show) {
                int width = FieldNames::getColumnWidth(static_cast<FieldNames::Fields>(col), settings);
                view->setColumnWidth(col, width);
            }
        }
        int tabIndex = mergedTabWidget->addTab(view, ecuId);
        ecuTabViews[ecuId] = view;
        mergedTabWidget->tabBar()->setTabButton(tabIndex, QTabBar::RightSide, nullptr);
    }
    loadingDialog.close();
    tabWindow->setAttribute(Qt::WA_ShowModal, true);
    tabWindow->show();
}

// Opens a dialog to select and merge multiple ECU tabs
void filtergrouplogs::openMergeTabsDialog()
{
    QDialog dialog(mergedTabWidget);
    dialog.setWindowTitle("Select Tabs to Merge");
    QVBoxLayout* mergeTabsListLayout = new QVBoxLayout(&dialog);
    QListWidget* listWidget = new QListWidget(&dialog);
    listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    for (const QString& id : ecuTabViews.keys()) {
        if (mergedTabWidget->indexOf(ecuTabViews[id]) >= 0) {
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
            selectedEcuIdSet << item->text();
        }
        if (selectedEcuIdSet.size() >= 2) {
            mergeTabs();
        }
    }
}

// Merges selected ECU tabs into a single tab
void filtergrouplogs::mergeTabs()
{
    if (selectedEcuIdSet.isEmpty())
        return;
    QStringList selectedIds = selectedEcuIdSet.values();
    QString tabKey = selectedIds.join(" / ");
    if (mergedTabs.contains(tabKey)) {
        int existingIndex = mergedTabWidget->indexOf(mergedTabs[tabKey]);
        if (existingIndex != -1)
            mergedTabWidget->setCurrentIndex(existingIndex);
        return;
    }
    QDialog* loadingDialog = new QDialog(mergedTabWidget);
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
        QThread::msleep(30);
    }

    ecuIdFilterProxy = new EcuIdFilterProxyModel(this);
    ecuIdFilterProxy->setSourceModel(sourceModelOfDLT);
    ecuIdFilterProxy->setEcuColumn(ecuColumnIndex);
    ecuIdFilterProxy->setEcuIdList(QSet<QString>(selectedIds.begin(), selectedIds.end()));
    QTableView* mergedView = new QTableView;
    mergedView->setModel(ecuIdFilterProxy);
    mergedView->horizontalHeader()->setStretchLastSection(true);
    mergedView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mergedView->resizeColumnsToContents();
    auto settings = QDltSettingsManager::getInstance();
    for (int col = 0; col < ecuIdFilterProxy->columnCount(); ++col) {
        bool show = FieldNames::getColumnShown(static_cast<FieldNames::Fields>(col), settings);
        mergedView->setColumnHidden(col, !show);
    }
    int mergedtabIndex = mergedTabWidget->addTab(mergedView, tabKey);
    mergedTabWidget->setCurrentIndex(mergedtabIndex);
    indexofMergedTabs[mergedtabIndex] = tabKey;
    mergedTabs[tabKey] = mergedView;
    tabToSelectedIds[mergedView] = selectedIds;
    selectedEcuIdSet.clear();
    loadingDialog->close();
    loadingDialog->deleteLater();
}

// Handles closing of a tab and updates internal tab tracking
void filtergrouplogs::onTabCloseRequested(int index) {
    QWidget* widget = mergedTabWidget->widget(index);
    if (!widget)
        return;
    // Find the correct tabKey for this widget
    QString tabKey;
    for (auto it = mergedTabs.begin(); it != mergedTabs.end(); ++it) {
        if (it.value() == widget) {
            tabKey = it.key();
            break;
        }
    }
    if (tabKey.isEmpty())
        return;
    mergedTabWidget->removeTab(index);
    mergedTabs.remove(tabKey);
    tabToSelectedIds.remove(widget);
    widget->deleteLater();
    indexofMergedTabs.clear();
    // Rearrange tab indices once after deletion of any tab
    for (int i = 0; i < mergedTabWidget->count(); ++i) {
        QWidget* w = mergedTabWidget->widget(i);
        for (auto it = mergedTabs.begin(); it != mergedTabs.end(); ++it) {
            if (it.value() == w) {
                indexofMergedTabs[i] = it.key();
                break;
            }
        }
    }
}

// Exports the filtered DLT logs from the selected tab to a file
void filtergrouplogs::onExportFilteredLogsClicked() {
    if (!dltFile || !sourceModelOfDLT) {
        QMessageBox::information(nullptr, "No DLT file opened", "No DLT file is opened. Please open a DLT file");
        return;
    }
    
    QStringList tabNames;
    for (int i = 0; i < mergedTabWidget->count(); ++i) {
        tabNames << mergedTabWidget->tabText(i);
    }
    if (tabNames.isEmpty()) {
        QMessageBox::information(mergedTabWidget, "Export", "No tabs available to export.");
        return;
    }
    bool ok = false;
    QString selectedTab = QInputDialog::getItem(mergedTabWidget, "Select Tab to Export",
                                                "Choose Tab:", tabNames, 0, false, &ok);
    if (!ok || selectedTab.isEmpty()) {
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(mergedTabWidget, "Export DLT Logs", 
                                                    selectedTab + ".dlt", 
                                                    "DLT Files (*.dlt);;All Files (*)");
    if (fileName.isEmpty()) {
        return;
    }

    int tabIndex = tabNames.indexOf(selectedTab);
    QWidget* tabWidget = mergedTabWidget->widget(tabIndex);
    QTableView* tableView = qobject_cast<QTableView*>(tabWidget);
    if (!tableView) {
        QMessageBox::critical(mergedTabWidget, "Export Error", "Could not find table view for selected tab.");
        return;
    }
    EcuIdFilterProxyModel* proxyModel = qobject_cast<EcuIdFilterProxyModel*>(tableView->model());
    if (!proxyModel) {
        QMessageBox::critical(mergedTabWidget, "Export Error", "Could not access filtering model.");
        return;
    }

    QProgressDialog progress("Exporting filtered DLT messages...", "Cancel", 0, 100, mergedTabWidget);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.show();
    try {
        QModelIndexList selectedIndices;
        int rowCount = proxyModel->rowCount();
        if (rowCount == 0) {
            QMessageBox::information(mergedTabWidget, "Export", "No messages to export in selected tab.");
            return;
        }

        // Map proxy model indices back to source model indices
        for (int row = 0; row < rowCount; ++row) {
            if (progress.wasCanceled()) {
                return;
            }
            QModelIndex proxyIndex = proxyModel->index(row, 0);
            QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
            if (sourceIndex.isValid()) {
                selectedIndices.append(sourceIndex);
            }
            progress.setValue((row * 50) / rowCount);
            QCoreApplication::processEvents();
        }
        if (selectedIndices.isEmpty()) {
            QMessageBox::information(mergedTabWidget, "Export", "No valid messages found to export.");
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
            QThread::msleep(100);
        }
        progress.setValue(100);
        if (exportResult.isEmpty()) {
            QMessageBox::information(mergedTabWidget, "Export Complete", 
                                     QString("Successfully exported filtered DLT messages to:\n%1").arg(fileName));
        } else {
            QMessageBox::information(mergedTabWidget, "Export Complete", 
                                     QString("Export completed with result:\n%1").arg(exportResult));
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(mergedTabWidget, "Export Error", 
                              QString("An error occurred during export:\n%1").arg(e.what()));
    } catch (...) {
        QMessageBox::critical(mergedTabWidget, "Export Error", 
                              "An unknown error occurred during export.");
    }
}

// Sets the source model for DLT data
void filtergrouplogs::setSourceModel(QAbstractTableModel* model) {
    sourceModelOfDLT = model;
}

// Sets the DLT file reference
void filtergrouplogs::setDltFile(QDltFile* file) {
    dltFile = file;
}

// Sets the plugin manager reference
void filtergrouplogs::setPluginManager(QDltPluginManager* manager) {
    pluginManager = manager;
}
