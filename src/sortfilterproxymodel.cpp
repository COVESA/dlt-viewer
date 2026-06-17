#include <QDebug>
#include <QFileSystemModel>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

#include "sortfilterproxymodel.h"
#include "fieldnames.h"

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
    this->invalidate();
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

void EcuIdFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (this->sourceModel())
    {
        disconnect(this->sourceModel(), nullptr, this, nullptr);
    }

    QSortFilterProxyModel::setSourceModel(sourceModel);

    acceptedRows.clear();
    indexedRowCount = 0;
    indexReady = false;
    knownSourceRowCount = sourceModel ? sourceModel->rowCount() : 0;

    if (!sourceModel)
    {
        return;
    }

    connect(sourceModel, &QAbstractItemModel::layoutChanged,
            this, &EcuIdFilterProxyModel::onSourceLayoutChanged);
    connect(sourceModel, &QAbstractItemModel::modelReset,
            this, &EcuIdFilterProxyModel::onSourceModelReset);
    connect(sourceModel, &QAbstractItemModel::rowsInserted, this,
            [this](const QModelIndex &, int first, int last) {
                if(first <= last)
                {
                    buildAcceptedRowsIncremental(first, last);
                    knownSourceRowCount = this->sourceModel() ? this->sourceModel()->rowCount() : 0;
                    updateFilter();
                }
            });
}

void EcuIdFilterProxyModel::updateFilter()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
    endFilterChange();
#else
    invalidateFilter();
#endif
}

// Sets a single ECU ID for filtering
void EcuIdFilterProxyModel::setEcuId(const QString& ecuId) {
    ecu = ecuId;
    acceptedRows.clear();
    indexedRowCount = 0;
    indexReady = false;
    updateFilter();
}



// Sets a list of ECU IDs for filtering
void EcuIdFilterProxyModel::setEcuIdList(const QSet<QString>& ids) {
    ecuIdList.clear();
    for (const QString& id : ids)
        ecuIdList.insert(id.trimmed().toLower());
    acceptedRows.clear();
    indexedRowCount = 0;
    indexReady = false;
    updateFilter();
    sort(-1);
}

// Sets the column index for ECU filtering
void EcuIdFilterProxyModel::setEcuColumn(int column) {
    ecuColumn = column;
    acceptedRows.clear();
    indexedRowCount = 0;
    indexReady = false;
    updateFilter();
}

//Determines if a row should be accepted based on ECU filtering
bool EcuIdFilterProxyModel::filterAcceptsRow(int row, const QModelIndex& parent) const {
    if (!sourceModel())
        return false;

    if (ecu.isEmpty() && ecuIdList.isEmpty())
        return true;

    if (!indexReady)
    {
        const_cast<EcuIdFilterProxyModel*>(this)->rebuildAcceptedRowsIndex();
    }
    else if (row >= indexedRowCount)
    {
        const_cast<EcuIdFilterProxyModel*>(this)->buildAcceptedRowsIncremental(indexedRowCount, row);
    }

    Q_UNUSED(parent);
    return acceptedRows.contains(row);
}

bool EcuIdFilterProxyModel::rowMatchesCurrentEcuFilter(int row, const QModelIndex &parent) const
{
    if (!sourceModel() || ecuColumn < 0)
        return false;

    QModelIndex index = sourceModel()->index(row, ecuColumn, parent);
    if (!index.isValid())
        return false;

    const QString value = sourceModel()->data(index).toString().trimmed().toLower();

    if (!ecuIdList.isEmpty())
    {
        return ecuIdList.contains(value);
    }

    if (!ecu.isEmpty())
    {
        return value == ecu.trimmed().toLower();
    }

    return true;
}

void EcuIdFilterProxyModel::rebuildAcceptedRowsIndex()
{
    acceptedRows.clear();
    indexedRowCount = 0;

    if (!sourceModel())
    {
        indexReady = true;
        knownSourceRowCount = 0;
        return;
    }

    const int totalRows = sourceModel()->rowCount();
    buildAcceptedRowsIncremental(0, totalRows - 1);
    knownSourceRowCount = totalRows;
    indexReady = true;
}

void EcuIdFilterProxyModel::buildAcceptedRowsIncremental(int fromRow, int toRow)
{
    if (!sourceModel() || fromRow > toRow)
        return;

    const int totalRows = sourceModel()->rowCount();
    if (totalRows <= 0)
    {
        indexedRowCount = 0;
        return;
    }

    const int start = qMax(0, fromRow);
    const int end = qMin(toRow, totalRows - 1);
    if (start > end)
    {
        indexedRowCount = qMax(indexedRowCount, totalRows);
        return;
    }

    for (int row = start; row <= end; ++row)
    {
        if (rowMatchesCurrentEcuFilter(row, QModelIndex()))
        {
            acceptedRows.insert(row);
        }
    }

    indexedRowCount = qMax(indexedRowCount, end + 1);
}

void EcuIdFilterProxyModel::onSourceLayoutChanged()
{
    if (!sourceModel())
    {
        acceptedRows.clear();
        indexedRowCount = 0;
        knownSourceRowCount = 0;
        indexReady = false;
        return;
    }

    const int currentRows = sourceModel()->rowCount();
    if (!indexReady)
    {
        rebuildAcceptedRowsIndex();
        updateFilter();
        return;
    }

    if (currentRows < knownSourceRowCount)
    {
        rebuildAcceptedRowsIndex();
        updateFilter();
        return;
    }

    if (currentRows > knownSourceRowCount)
    {
        buildAcceptedRowsIncremental(knownSourceRowCount, currentRows - 1);
        knownSourceRowCount = currentRows;
        updateFilter();
    }
}

void EcuIdFilterProxyModel::onSourceModelReset()
{
    rebuildAcceptedRowsIndex();
    updateFilter();
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
