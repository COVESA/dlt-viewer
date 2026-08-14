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
#include <QEventLoop>
#include <algorithm>

#include "filtergrouplogs.h"
#include "fieldnames.h"
#include "qdltfile.h"
#include "qdltexporter.h"
#include "qdltsettingsmanager.h"

filtergrouplogs::filtergrouplogs(QObject* parent) : QObject(parent) {
    sourceModelOfDLT = nullptr;
    mergedTabWidget = nullptr;
    dltFile = nullptr;
    pluginManager = nullptr;
}

// Extracts unique ECU IDs from a DLT file
QStringList filtergrouplogs::extractEcuIds() {
    rebuildGroupedIndex();
    return extractedEcuIds;
}

void filtergrouplogs::rebuildGroupedIndex(QProgressDialog *progress)
{
    ecuRowReferences.clear();
    extractedEcuIds.clear();

    if(!sourceModelOfDLT)
    {
        return;
    }

    const int totalRows = sourceModelOfDLT->rowCount();
    for(int row = 0; row < totalRows; ++row)
    {
        if(progress)
        {
            if(progress->wasCanceled())
            {
                ecuRowReferences.clear();
                extractedEcuIds.clear();
                return;
            }

            if((row % 100) == 0 || row + 1 == totalRows)
            {
                progress->setValue(row + 1);
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            }
        }

        const QModelIndex ecuIndex = sourceModelOfDLT->index(row, ecuColumnIndex);
        if(!ecuIndex.isValid())
        {
            continue;
        }

        const QString ecuId = sourceModelOfDLT->data(ecuIndex, Qt::DisplayRole).toString().trimmed();
        if(ecuId.isEmpty())
        {
            continue;
        }

        ecuRowReferences[ecuId].append(row);
    }

    extractedEcuIds = ecuRowReferences.keys();
    extractedEcuIds.sort();
}

QVector<int> filtergrouplogs::rowsForEcuSet(const QSet<QString> &ecuIds) const
{
    QVector<int> rows;
    for(const QString &ecuId : ecuIds)
    {
        rows += ecuRowReferences.value(ecuId);
    }

    std::sort(rows.begin(), rows.end());
    rows.erase(std::unique(rows.begin(), rows.end()), rows.end());
    return rows;
}

void filtergrouplogs::createOrUpdateTab(const QString &tabName, const QVector<int> &rows)
{
    if(!mergedTabWidget)
    {
        return;
    }

    if(!mergedTabs.contains(tabName))
    {
        IndexRowReferenceModel *model = new IndexRowReferenceModel(this);
        model->setSourceModel(sourceModelOfDLT);
        model->setRowReferences(rows);

        QTableView *view = new QTableView;
        view->setModel(model);
        view->horizontalHeader()->setStretchLastSection(true);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->resizeColumnsToContents();

        auto settings = QDltSettingsManager::getInstance();
        for(int col = 0; col < model->columnCount(); ++col)
        {
            const bool show = FieldNames::getColumnShown(static_cast<FieldNames::Fields>(col), settings);
            view->setColumnHidden(col, !show);
            if(show)
            {
                const int width = FieldNames::getColumnWidth(static_cast<FieldNames::Fields>(col), settings);
                view->setColumnWidth(col, width);
            }
        }

        const int tabIndex = mergedTabWidget->addTab(view, tabName);
        mergedTabWidget->tabBar()->setTabButton(tabIndex, QTabBar::RightSide, nullptr);

        mergedTabs[tabName] = view;
        ecuTabViews[tabName] = view;
        ecuTabModels[tabName] = model;
    }
    else
    {
        IndexRowReferenceModel *model = ecuTabModels.value(tabName, nullptr);
        if(model)
        {
            model->setRowReferences(rows);
        }
    }
}

// Creates tabs for each ECU ID and sets up the tab window UI
void filtergrouplogs::ecuIdTabs(){
    const int totalRows = sourceModelOfDLT ? sourceModelOfDLT->rowCount() : 0;
    QProgressDialog progress("Grouping DLT Logs by ECU ID...", "Cancel", 0, qMax(1, totalRows), nullptr);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(0);
    progress.setValue(0);
    progress.setWindowTitle("Grouping Progress");
    progress.show();

    rebuildGroupedIndex(&progress);
    if(progress.wasCanceled())
    {
        return;
    }

    QStringList availableEcuIds = extractedEcuIds;
    if(availableEcuIds.isEmpty())
    {
        QMessageBox::information(nullptr, "No ECU IDs found", "The opened file does not contain any ECU IDs visible in the current table view.");
        return;
    }

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

    progress.setLabelText("Preparing ECU tabs...");
    progress.setMaximum(qMax(1, availableEcuIds.size()));
    progress.setValue(0);

    for (int i = 0; i < availableEcuIds.size(); ++i) {
        if(progress.wasCanceled())
        {
            tabWindow->deleteLater();
            return;
        }

        const QString &ecuId = availableEcuIds[i];
        createOrUpdateTab(ecuId, ecuRowReferences.value(ecuId));
        tabToSelectedIds[ecuTabViews.value(ecuId)] = QStringList{ecuId};
        if((i % 5) == 0 || i + 1 == availableEcuIds.size())
        {
            progress.setValue(i + 1);
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
    }

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
    createOrUpdateTab(tabKey, rowsForEcuSet(QSet<QString>(selectedIds.begin(), selectedIds.end())));
    int mergedtabIndex = mergedTabWidget->indexOf(mergedTabs.value(tabKey));
    mergedTabWidget->setCurrentIndex(mergedtabIndex);
    indexofMergedTabs[mergedtabIndex] = tabKey;
    tabToSelectedIds[mergedTabs.value(tabKey)] = selectedIds;
    selectedEcuIdSet.clear();
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
    delete ecuTabModels.take(tabKey);
    ecuTabViews.remove(tabKey);
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
    IndexRowReferenceModel* proxyModel = qobject_cast<IndexRowReferenceModel*>(tableView->model());
    if (!proxyModel || !sourceModelOfDLT) {
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
            const int sourceRow = proxyModel->sourceRowAt(row);
            if(sourceRow >= 0 && sourceRow < sourceModelOfDLT->rowCount())
            {
                selectedIndices.append(sourceModelOfDLT->index(sourceRow, 0));
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
    if(sourceModelOfDLT)
    {
        disconnect(sourceModelOfDLT, nullptr, this, nullptr);
    }

    sourceModelOfDLT = model;

    if(sourceModelOfDLT)
    {
        connect(sourceModelOfDLT, &QAbstractItemModel::layoutChanged, this, &filtergrouplogs::onSourceModelChanged, Qt::UniqueConnection);
        connect(sourceModelOfDLT, &QAbstractItemModel::modelReset, this, &filtergrouplogs::onSourceModelChanged, Qt::UniqueConnection);
    }
}

// Sets the DLT file reference
void filtergrouplogs::setDltFile(QDltFile* file) {
    dltFile = file;
}

// Sets the plugin manager reference
void filtergrouplogs::setPluginManager(QDltPluginManager* manager) {
    pluginManager = manager;
}

void filtergrouplogs::onSourceModelChanged()
{
    if(!mergedTabWidget || mergedTabs.isEmpty())
    {
        return;
    }

    rebuildGroupedIndex();

    for(auto it = mergedTabs.begin(); it != mergedTabs.end(); ++it)
    {
        const QString tabName = it.key();
        QWidget *tabWidget = it.value();
        const QStringList selectedIds = tabToSelectedIds.value(tabWidget);

        if(selectedIds.isEmpty())
        {
            createOrUpdateTab(tabName, ecuRowReferences.value(tabName));
        }
        else
        {
            createOrUpdateTab(tabName, rowsForEcuSet(QSet<QString>(selectedIds.begin(), selectedIds.end())));
        }
    }
}
