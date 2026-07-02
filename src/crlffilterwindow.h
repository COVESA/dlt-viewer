#ifndef CRLFFILTERWINDOW_H
#define CRLFFILTERWINDOW_H

#include <QObject>
#include <QTableView>
#include <QAbstractTableModel>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QHash>
#include <QVariantList>

#include <vector>

#include "projectiontablemodel.h"
#include "decodecacheservice.h"
#include "messagestore.h"
#include "indexservice.h"
#include "qdltfile.h"
#include "qdltpluginmanager.h"

class CrlfFilterWindow : public QObject {
    Q_OBJECT

public:
    //! Construct the CRLF filter helper.
    explicit CrlfFilterWindow(QObject* parent = nullptr);
    
    // Sets the source model for DLT data
    void setSourceModel(QAbstractTableModel* model);
    
    // Sets the DLT file reference
    void setDltFile(QDltFile* dltFile);
    
    // Sets the plugin manager reference
    void setPluginManager(QDltPluginManager* pluginManager);

    // Sets the shared message store service
    void setMessageStore(CMessageStore *messageStore);

    // Sets the shared index service
    void setIndexService(const CIndexService *indexService);

    // Sets the shared decode cache service
    void setDecodeCacheService(CDecodeCacheService *decodeCacheService);
    
    // Creates a single window displaying all CRLF messages
    void createCrlfWindow();
    
    // Refresh the CRLF window with latest data
    void refreshWindow();
    
    // Show and activate the CRLF window
    void showAndActivate();
    
    // Close the CRLF window
    void closeWindow();
    
    // Exports all filtered CRLF DLT logs to a file
    void onExportFilteredCrlfLogsClicked();
    
    // Cleanup method to properly disconnect from models
    void cleanup();

signals:
    // Signal to request navigation to a specific message in the main window
    void jumpToMessageRequested(int messageIndex);

private slots:
    // Handle double-click on CRLF message row to navigate to main window
    void onCrlfMessageDoubleClicked(const QModelIndex& index);
    
    // Handle when source model data changes
    void onSourceModelDataChanged();
    
    // Handle when source model is reset/cleared
    void onSourceModelReset();
    
    // Debounced rebuild triggered by timer
    void onRebuildTimerTimeout();

private:
    // Rebuild the CRLF data model with current DLT file data
    void rebuildCrlfModel();

    //! Clear transient CRLF/decode caches when the source view changes.
    void invalidateCache();

    //! Build source-row projection for messages containing CRLF payload.
    std::vector<int> buildCrlfProjectionRows(QWidget *progressParent,
                                             const QString &progressLabel,
                                             bool *wasCancelled = nullptr);
    
    // Create headers for CRLF table model
    // Helper methods for code reuse and optimization
    //! Check whether decoded payload text contains CRLF.
    bool containsCrlf(const QString& payload);
    //! Update the status label with the current match count.
    void updateMessageCount(int count);
    //! Apply column sizing and visibility settings.
    void applyColumnSettings();
    
    ProjectionTableModel* m_crlfProjectionModel;
    QAbstractTableModel* m_sourceModelOfDLT;
    QWidget* m_crlfWindow;
    QTableView* m_crlfTableView;
    QLabel* m_statusLabel;
    QDltFile* m_dltFile;
    QDltPluginManager* m_pluginManager;
    CMessageStore *m_messageStore;
    const CIndexService *m_indexService;
    CDecodeCacheService *m_externalDecodeCacheService;
    
    QHash<int, bool> m_crlfCache;  // Cache which absolute positions contain CRLF (positive + negative caching)
    QHash<int, QVariantList> m_messageDataCache;  // Cache processed message data
    int m_lastCacheValidCount;
    bool m_bulkCrlfIndexBuilt;
    
    // Debouncing mechanism to prevent frequent updates
    QTimer* m_rebuildTimer;
    int m_lastFilteredMessageCount;
    bool m_rebuildScheduled;
    bool m_rebuildInProgress;
    mutable CDecodeCacheService m_decodeCacheService;
};

#endif // CRLFFILTERWINDOW_H

