#ifndef CRLFFILTERWINDOW_H
#define CRLFFILTERWINDOW_H

#include <QObject>
#include <QTableView>
#include <QAbstractTableModel>
#include <QStandardItemModel>
#include <QWidget>
#include <QLabel>
#include <QTimer>

#include "qdltfile.h"
#include "qdltpluginmanager.h"

class CrlfFilterWindow : public QObject {
    Q_OBJECT

public:
    explicit CrlfFilterWindow(QObject* parent = nullptr);
    
    // Sets the source model for DLT data
    void setSourceModel(QAbstractTableModel* model);
    
    // Sets the DLT file reference
    void setDltFile(QDltFile* dltFile);
    
    // Sets the plugin manager reference
    void setPluginManager(QDltPluginManager* pluginManager);
    
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
    
    // Create headers for CRLF table model
    QStringList createTableHeaders();
    
    // Helper methods for code reuse and optimization
    bool containsCrlf(const QString& payload);
    void updateMessageCount(int count);
    void applyColumnSettings();
    
    QStandardItemModel* crlfFilterProxy;
    QAbstractTableModel* sourceModelOfDLT;
    QWidget* crlfWindow;
    QTableView* crlfTableView;
    QLabel* statusLabel;
    QDltFile* dltFile;
    QDltPluginManager* pluginManager;
    
    // Debouncing mechanism to prevent frequent updates
    QTimer* rebuildTimer;
    int lastFilteredMessageCount;
    bool rebuildScheduled;
    bool rebuildInProgress;
};

#endif // CRLFFILTERWINDOW_H
