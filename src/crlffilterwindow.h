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

#include "crlfindexviewmodel.h"
#include "qdltfile.h"
#include "qdltpluginmanager.h"

class CrlfFilterWindow : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructs the CRLF filter window controller.
     * @param parent QObject parent.
     */
    explicit CrlfFilterWindow(QObject* parent = nullptr);
    
    /**
     * @brief Sets the source table model used to mirror viewer columns.
     * @param model Source table model.
     */
    void setSourceModel(QAbstractTableModel* model);
    
    /**
     * @brief Sets the DLT file used to resolve and read message data.
     * @param dltFile DLT file instance.
     */
    void setDltFile(QDltFile* dltFile);
    
    /**
     * @brief Sets plugin manager used for optional message decoding.
     * @param pluginManager Plugin manager instance.
     */
    void setPluginManager(QDltPluginManager* pluginManager);
    
    /**
     * @brief Creates and shows the CRLF message window.
     */
    void createCrlfWindow();
    
    /**
     * @brief Rebuilds CRLF content for currently loaded data.
     */
    void refreshWindow();
    
    /**
     * @brief Brings the CRLF window to foreground.
     */
    void showAndActivate();
    
    /**
     * @brief Closes the CRLF window if open.
     */
    void closeWindow();
    
    /**
     * @brief Exports current CRLF result set to a DLT file.
     */
    void onExportFilteredCrlfLogsClicked();
    
    /**
     * @brief Disconnects model links and clears owned UI pointers.
     */
    void cleanup();

signals:
    /**
     * @brief Requests navigation to a message index in main view.
     * @param messageIndex Global message index.
     */
    void jumpToMessageRequested(int messageIndex);

private slots:
    /**
     * @brief Handles double click and asks main window to navigate.
     * @param index Clicked proxy index.
     */
    void onCrlfMessageDoubleClicked(const QModelIndex& index);
    
    /**
     * @brief Handles source model data changes.
     */
    void onSourceModelDataChanged();
    
    /**
     * @brief Handles source model reset events.
     */
    void onSourceModelReset();
    
    /**
     * @brief Timer callback used for debounced rebuild operations.
     */
    void onRebuildTimerTimeout();

private:
    /**
     * @brief Rebuilds CRLF proxy rows from current filtered messages.
     */
    void rebuildCrlfModel();
    
    /**
     * @brief Checks if payload contains CR or LF line breaks.
     * @param payload Message payload string.
     * @return True when CRLF markers are present.
     */
    bool containsCrlf(const QString& payload);

    /**
     * @brief Updates title and footer with number of CRLF messages.
     * @param count Number of CRLF matches.
     */
    void updateMessageCount(int count);

    /**
     * @brief Applies persisted column visibility/width settings.
     */
    void applyColumnSettings();
    
    // Optimized data extraction method
    QVariantList extractMessageData(int filteredIndex, bool suppressIndexBuilding = false);
    
    // Clear cache when file structure changes
    void invalidateCache();
    
    // Build bulk CRLF index for all messages (one-time expensive operation)
    void buildBulkCrlfIndex();
    
    // Check if parent MainWindow has background operations in progress
    bool isMainWindowBusy() const;
    
    QStandardItemModel* crlfFilterProxy;
    CrlfIndexViewModel* crlfFilterProxy;
    QAbstractTableModel* sourceModelOfDLT;
    QWidget* crlfWindow;
    QTableView* crlfTableView;
    QLabel* statusLabel;
    QDltFile* dltFile;
    QDltPluginManager* pluginManager;
    
    QHash<int, bool> crlfCache;  // Cache which absolute positions contain CRLF (positive + negative caching)
    QHash<int, QVariantList> messageDataCache;  // Cache processed message data
    int lastCacheValidCount;
    bool bulkCrlfIndexBuilt;
    
    // Debouncing mechanism to prevent frequent updates
    QTimer* rebuildTimer;
    int lastFilteredMessageCount;
    bool rebuildScheduled;
    bool rebuildInProgress;
};

#endif // CRLFFILTERWINDOW_H
