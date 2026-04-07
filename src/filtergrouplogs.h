#ifndef FILTERGROUPLOGS_H
#define FILTERGROUPLOGS_H

#include <QObject>
#include <QInputDialog>
#include <QFileDialog>
#include <QProgressDialog>
#include <QTableView>
#include <QAbstractTableModel>

#include "qdltfile.h"
#include "qdltpluginmanager.h"
#include "sortfilterproxymodel.h"

class filtergrouplogs : public QObject {
    Q_OBJECT

public:
  
  explicit filtergrouplogs(QObject* parent = nullptr);
  // Extracts unique ECU IDs from a DLT file
  QStringList extractEcuIds(const QString& dltFilePath);
  // Sets the source model for DLT data
  void setSourceModel(QAbstractTableModel* model);
  // Sets the DLT file reference
  void setDltFile(QDltFile* dltFile);
  // Sets the plugin manager reference
  void setPluginManager(QDltPluginManager* pluginManager);
  // Creates tabs for each ECU ID and sets up the tab window UI
  void ecuIdTabs();
  // Opens a dialog to select and merge multiple ECU tabs
  void openMergeTabsDialog();
  // Merges selected ECU tabs into a single tab
  void mergeTabs();
  // Handles closing of a tab and updates internal tab tracking
  void onTabCloseRequested(int index);
  // Exports the filtered DLT logs from the selected tab to a file
  void onExportFilteredLogsClicked();

  private :
    EcuIdFilterProxyModel* ecuIdFilterProxy;
    QAbstractTableModel* sourceModelOfDLT;
    QTabWidget* mergedTabWidget;
    QDltFile* dltFile;
    QDltPluginManager* pluginManager;

    QMap<QString, QWidget*> mergedTabs;
    QMap<QWidget*, QStringList> tabToSelectedIds;
    QMap<int, QString> indexofMergedTabs;
    QMap<QString, QTableView*> ecuTabViews;

    QSet<QString> selectedEcuIdSet;
    QStringList extractedEcuIds;

    int ecuColumnIndex = 4;
};

#endif // FILTERGROUPLOGS_H
