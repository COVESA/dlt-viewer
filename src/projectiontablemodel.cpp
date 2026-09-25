#include "projectiontablemodel.h"

#include <algorithm>
#include <iterator>

ProjectionTableModel::ProjectionTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void ProjectionTableModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (m_sourceModel == sourceModel)
        return;

    beginResetModel();
    reconnectSourceSignals(sourceModel);
    endResetModel();
}

QAbstractItemModel *ProjectionTableModel::sourceModel() const
{
    return m_sourceModel;
}

void ProjectionTableModel::setProjectionRows(const std::vector<int> &rows)
{
    beginResetModel();
    m_projectionRows = rows;
    endResetModel();
}

void ProjectionTableModel::clearProjection()
{
    setProjectionRows(std::vector<int>());
}

int ProjectionTableModel::sourceRowForRow(int row) const
{
    if (row < 0 || row >= static_cast<int>(m_projectionRows.size()))
        return -1;

    return m_projectionRows.at(static_cast<std::size_t>(row));
}

int ProjectionTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(m_projectionRows.size());
}

int ProjectionTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_sourceModel)
        return 0;

    return m_sourceModel->columnCount();
}

QVariant ProjectionTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_sourceModel)
        return QVariant();

    const int sourceRow = sourceRowForRow(index.row());
    if (sourceRow < 0)
        return QVariant();

    const QModelIndex sourceIndex = m_sourceModel->index(sourceRow, index.column());
    if (!sourceIndex.isValid())
        return QVariant();

    return m_sourceModel->data(sourceIndex, role);
}

QVariant ProjectionTableModel::headerData(int section,
                                           Qt::Orientation orientation,
                                           int role) const
{
    if (!m_sourceModel)
        return QVariant();

    return m_sourceModel->headerData(section, orientation, role);
}

void ProjectionTableModel::onSourceDataChanged(const QModelIndex &topLeft,
                                                const QModelIndex &bottomRight,
                                                const QVector<int> &roles)
{
    if (!m_sourceModel || topLeft.parent().isValid() || bottomRight.parent().isValid())
        return;

    // m_projectionRows holds source-row indices in ascending order (every
    // setProjectionRows() caller builds/sorts it that way), so the affected
    // projected range can be found with a binary search instead of scanning
    // every projected row on every source dataChanged signal.
    const auto begin = m_projectionRows.cbegin();
    const auto end = m_projectionRows.cend();
    const auto lower = std::lower_bound(begin, end, topLeft.row());
    if (lower == end || *lower > bottomRight.row())
        return;

    const auto upper = std::upper_bound(lower, end, bottomRight.row());

    const int firstProjectedRow = static_cast<int>(std::distance(begin, lower));
    const int lastProjectedRow = static_cast<int>(std::distance(begin, upper)) - 1;

    const QModelIndex projectedTopLeft = index(firstProjectedRow, topLeft.column());
    const QModelIndex projectedBottomRight = index(lastProjectedRow, bottomRight.column());
    emit dataChanged(projectedTopLeft, projectedBottomRight, roles);
}

void ProjectionTableModel::onSourceHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    emit headerDataChanged(orientation, first, last);
}

void ProjectionTableModel::onSourceModelReset()
{
    // Row indices were computed against the old structure; keeping them would map to wrong/shifted rows.
    beginResetModel();
    m_projectionRows.clear();
    endResetModel();
}

void ProjectionTableModel::onSourceLayoutChanged()
{
    // Row indices were computed against the old structure; keeping them would map to wrong/shifted rows.
    beginResetModel();
    m_projectionRows.clear();
    endResetModel();
}

void ProjectionTableModel::reconnectSourceSignals(QAbstractItemModel *sourceModel)
{
    if (m_sourceModel)
    m_sourceModel->disconnect(this);

    m_sourceModel = sourceModel;

    if (!m_sourceModel)
        return;

    connect(m_sourceModel,
            &QAbstractItemModel::dataChanged,
            this,
            &ProjectionTableModel::onSourceDataChanged);
    connect(m_sourceModel,
            &QAbstractItemModel::headerDataChanged,
            this,
            &ProjectionTableModel::onSourceHeaderDataChanged);
    connect(m_sourceModel,
            &QAbstractItemModel::modelReset,
            this,
            &ProjectionTableModel::onSourceModelReset);
    connect(m_sourceModel,
            &QAbstractItemModel::layoutChanged,
            this,
            &ProjectionTableModel::onSourceLayoutChanged);
    connect(m_sourceModel,
            &QAbstractItemModel::rowsInserted,
            this,
            &ProjectionTableModel::onSourceModelReset);
    connect(m_sourceModel,
            &QAbstractItemModel::rowsRemoved,
            this,
            &ProjectionTableModel::onSourceModelReset);
    // QPointer nulls m_sourceModel automatically, but views still need a reset notification.
    connect(m_sourceModel,
            &QObject::destroyed,
            this,
            &ProjectionTableModel::onSourceModelReset);
}
