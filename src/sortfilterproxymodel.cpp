#include <QDebug>
#include <QFileSystemModel>
#include <QDir>

#include "sortfilterproxymodel.h"

SortFilterProxyModel::SortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool SortFilterProxyModel::lessThan(const QModelIndex &left,
                                      const QModelIndex &right) const
{
    QFileSystemModel* model     = static_cast<QFileSystemModel*>(sourceModel());
    QVariant          leftData  = model->data(left);
    QVariant          rightData = model->data(right);
    bool left_isdir             = QDir(model->filePath(left)).exists();
    bool right_isdir            = QDir(model->filePath(right)).exists();

    return (left_isdir == right_isdir)?
        (QString::localeAwareCompare(leftData.toString(), rightData.toString()) > 0):
        (left_isdir < right_isdir);

}

