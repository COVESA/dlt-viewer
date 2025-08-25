#include <QDebug>
#include <QFileSystemModel>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

#include "sortfilterproxymodel.h"

SortFilterProxyModel::SortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void SortFilterProxyModel::changeSortingType(SortType type)
{
    sort_type = type;
    // invalidate current filter
    this->invalidate();
    // call sort again
    this->sort(0, this->sortOrder());
}

void SortFilterProxyModel::changeSortingOrder(Qt::SortOrder order)
{
    // invalidate current filter
    this->invalidate();
    // call sort again
    this->sort(0, order);
}

bool SortFilterProxyModel::lessThan(const QModelIndex &left,
                                      const QModelIndex &right) const
{
    bool ret_val                = false;
    QFileSystemModel* model     = static_cast<QFileSystemModel*>(sourceModel());
    QVariant          leftData  = model->data(left);
    QVariant          rightData = model->data(right);
    bool left_isdir             = QDir(model->filePath(left)).exists();
    bool right_isdir            = QDir(model->filePath(right)).exists();

    if (left_isdir == right_isdir)
    {
        switch (sort_type)
        {
        case TIMESTAMP:
        {
            QDateTime l_date = QFileInfo(model->filePath(left)).lastModified();
            QDateTime r_date = QFileInfo(model->filePath(right)).lastModified();

            ret_val = l_date < r_date;
        }
            break;
        case ALPHABETICALLY:
        default:
            /* Default sorting type */
            ret_val = QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
            break;
        }
    }
    else
    {   /* Directories always on top of the list */
        if (Qt::AscendingOrder == this->sortOrder())
            ret_val = left_isdir > right_isdir;
        else
            ret_val = left_isdir < right_isdir;
    }

    return ret_val;
}

EcuIdFilterProxyModel::EcuIdFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

// Sets a single ECU ID for filtering
void EcuIdFilterProxyModel::setEcuId(const QString& ecuId) {
    ecu = ecuId;
    this->invalidateFilter();
}

// Sets a list of ECU IDs for filtering
void EcuIdFilterProxyModel::setEcuIdList(const QSet<QString>& ids) {
    ecuIdList.clear();
    for (const QString& id : ids)
        ecuIdList.insert(id.trimmed().toLower());
    invalidateFilter();
    sort(-1);
}

// Sets the column index for ECU filtering
void EcuIdFilterProxyModel::setEcuColumn(int column) {
    ecuColumn = column;
    this->invalidateFilter();
}

// Determines if a row should be accepted based on ECU filtering
bool EcuIdFilterProxyModel::filterAcceptsRow(int row, const QModelIndex& parent) const {
    if (!sourceModel() || ecuColumn < 0)
        return false;
    QModelIndex index = sourceModel()->index(row, ecuColumn, parent);
    if (!index.isValid())
        return false;
    QString value = sourceModel()->data(index).toString().trimmed().toLower();
    if (ecu.isEmpty() && ecuIdList.isEmpty())
        return true;
    if (!ecuIdList.isEmpty()) {
        return ecuIdList.contains(value);
    }
    if (!ecu.isEmpty()) {
        return value == ecu.trimmed().toLower();
    }
    return false;
}
