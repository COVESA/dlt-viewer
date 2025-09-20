#ifndef FILEEXPLORERTAB_H
#define FILEEXPLORERTAB_H

#include "sortfilterproxymodel.h"

#include <QFileSystemModel>
#include <QWidget>

namespace Ui {
class FileExplorerTab;
}

class FileExplorerTab : public QWidget {
    Q_OBJECT

  public:
    explicit FileExplorerTab(QWidget* parent = nullptr);
    ~FileExplorerTab() override;

    void setCurrentFile(const QString& path);

    void setSortingOrder(Qt::SortOrder order);
    void setSortingMode(SortFilterProxyModel::SortType type);

  private:
     QString getPathFromModelIndex(const QModelIndex &index) const;

  signals:
    void fileActivated(const QString& path);

    void fileOpenRequested(const QString& path);
    void fileAppendRequested(const QString& path);
    void filesOpenRequest(const QStringList& dltPaths);
    void filesAppendRequest(const QStringList& mf4AndPcapPaths);
  private slots:
    void on_exploreView_activated(const QModelIndex &index);
    void on_exploreView_customContextMenuRequested(QPoint pos);
    void on_comboBoxExplorerSortType_currentIndexChanged(int index);

    void on_comboBoxExplorerSortOrder_currentIndexChanged(int index);


  private:
    Ui::FileExplorerTab* ui;

    QFileSystemModel* m_fsModel;
    SortFilterProxyModel * m_fsSortProxyModel;
};

#endif // FILEEXPLORERTAB_H
