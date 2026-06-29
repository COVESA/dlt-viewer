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

class CFilterGroupLogs : public QObject {
    Q_OBJECT

public:
  //! Construct the ECU grouping helper.
  explicit CFilterGroupLogs(QObject* parent = nullptr);
  //! Extract unique ECU IDs from a DLT file path.
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
    QAbstractTableModel* m_sourceModelOfDLT;
    QTabWidget* m_mergedTabWidget;
    QDltFile* dltFile;
    QDltPluginManager* pluginManager;

    QMap<QString, QWidget*> m_mergedTabs;
    QMap<QWidget*, QStringList> m_tabToSelectedIds;
    QMap<int, QString> m_indexOfMergedTabs;
    QMap<QString, QTableView*> m_ecuTabViews;
    std::map<QString, std::vector<int>> m_ecuSourceRowProjection;

    QSet<QString> m_selectedEcuIdSet;
    QStringList m_extractedEcuIds;

    int m_ecuColumnIndex = 4;
};

#endif // FILTERGROUPLOGS_H



