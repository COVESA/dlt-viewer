#ifndef CRLFFILTERWINDOW_H
#define CRLFFILTERWINDOW_H

#include <QObject>
#include <QTableView>
#include <QAbstractTableModel>
#include <QWidget>

#include "qdltfile.h"
#include "qdltpluginmanager.h"
#include "sortfilterproxymodel.h"

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
    
    // Exports all filtered CRLF DLT logs to a file
    void onExportFilteredCrlfLogsClicked();
    
    // Cleanup method to properly disconnect from models
    void cleanup();
    
    // Check if source model is still valid
    bool isSourceModelValid() const;

signals:
    // Signal to request navigation to a specific message in the main window
    void jumpToMessageRequested(int messageIndex);

private slots:
    // Handle double-click on CRLF message row to navigate to main window
    void onCrlfMessageDoubleClicked(const QModelIndex& index);

private:
    EcuIdFilterProxyModel* crlfFilterProxy;
    QAbstractTableModel* sourceModelOfDLT;
    QWidget* crlfWindow;
    QTableView* crlfTableView;
    QDltFile* dltFile;
    QDltPluginManager* pluginManager;
};

#endif // CRLFFILTERWINDOW_H
