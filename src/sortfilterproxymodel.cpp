#include <QDebug>
#include <QFileSystemModel>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

#include "sortfilterproxymodel.h"
#include "fieldnames.h"
#include "qdltfile.h"

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

// Enables or disables CRLF filtering
void EcuIdFilterProxyModel::setCrlfFilter(bool enabled) {
    crlfFilterEnabled = enabled;
    this->invalidateFilter();
}

// Set the DLT file for direct access
void EcuIdFilterProxyModel::setDltFile(QDltFile* file) {
    dltFile = file;
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

//Determines if a row should be accepted based on ECU filtering or CRLF filtering
bool EcuIdFilterProxyModel::filterAcceptsRow(int row, const QModelIndex& parent) const {
    if (!sourceModel())
        return false;
    
    // If CRLF filtering is enabled, apply it first
    if (crlfFilterEnabled) {
        if (dltFile) {
            QDltMsg msg;
            if (dltFile->getMsg(row, msg)) {
                QString payload = msg.toStringPayload();
                return payload.contains("\r") || payload.contains("\n");
            }
        }
        // Fallback to model-based method if direct access fails
        QModelIndex payloadIndex = sourceModel()->index(row, FieldNames::Payload, parent);
        if (payloadIndex.isValid()) {
            QString payload = sourceModel()->data(payloadIndex).toString();
            return payload.contains("\r") || payload.contains("\n");
        }
        return false;
    }
    
    // Apply ECU filtering only if CRLF is not enabled
    if (!ecu.isEmpty() || !ecuIdList.isEmpty()) {
        if (ecuColumn < 0)
            return false;
            
        QModelIndex index = sourceModel()->index(row, ecuColumn, parent);
        if (!index.isValid())
            return false;
        
        QString value = sourceModel()->data(index).toString().trimmed().toLower();
        
        if (!ecuIdList.isEmpty()) {
            return ecuIdList.contains(value);
        } else if (!ecu.isEmpty()) {
            return value == ecu.trimmed().toLower();
        }
    }
    
    return true;
}



// Override data method to preserve original source indices in Index column
QVariant EcuIdFilterProxyModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || !sourceModel())
        return QVariant();
        
    // For Index column, return the original source model index instead of filtered proxy index
    if (role == Qt::DisplayRole && index.column() == FieldNames::Index) {
        QModelIndex sourceIndex = mapToSource(index);
        if (sourceIndex.isValid()) {
            // Get the actual source model data for the Index column which uses getMsgFilterPos()
            QModelIndex sourceIndexCol = sourceModel()->index(sourceIndex.row(), FieldNames::Index);
            if (sourceIndexCol.isValid()) {
                QVariant originalIndex = sourceModel()->data(sourceIndexCol, role);                
                return originalIndex;
            }
        }
    }
    return QSortFilterProxyModel::data(index, role);
}

// Override lessThan for proper sorting, especially for Index column
bool EcuIdFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    if (!sourceModel() || !left.isValid() || !right.isValid())
        return false;
        
    QModelIndex leftSource = mapToSource(left);
    QModelIndex rightSource = mapToSource(right);
    
    if (!leftSource.isValid() || !rightSource.isValid())
        return false;
    return leftSource.row() < rightSource.row();
}
