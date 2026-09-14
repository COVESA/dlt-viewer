#include "indexrowreferencemodel.h"

IndexRowReferenceModel::IndexRowReferenceModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void IndexRowReferenceModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    if(sourceModel)
    {
        disconnect(sourceModel, nullptr, this, nullptr);
    }

    beginResetModel();
    sourceModel = newSourceModel;
    sourceRows.clear();
    endResetModel();

    if(sourceModel)
    {
        connect(sourceModel, &QAbstractItemModel::dataChanged,
                this, &IndexRowReferenceModel::onSourceDataChanged,
                Qt::UniqueConnection);
        connect(sourceModel, &QAbstractItemModel::headerDataChanged,
                this, &IndexRowReferenceModel::onSourceHeaderDataChanged,
                Qt::UniqueConnection);
    }
}

void IndexRowReferenceModel::setRowReferences(const QVector<int> &rows)
{
    beginResetModel();
    sourceRows = rows;
    endResetModel();
}

int IndexRowReferenceModel::sourceRowAt(int row) const
{
    if(row < 0 || row >= sourceRows.size())
    {
        return -1;
    }

    return sourceRows[row];
}

const QVector<int>& IndexRowReferenceModel::rowReferences() const
{
    return sourceRows;
}

int IndexRowReferenceModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        return 0;
    }

    return sourceRows.size();
}

int IndexRowReferenceModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid() || !sourceModel)
    {
        return 0;
    }

    return sourceModel->columnCount();
}

QVariant IndexRowReferenceModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || !sourceModel)
    {
        return QVariant();
    }

    const int sourceRow = sourceRowAt(index.row());
    if(sourceRow < 0 || sourceRow >= sourceModel->rowCount())
    {
        return QVariant();
    }

    const QModelIndex sourceIndex = sourceModel->index(sourceRow, index.column());
    if(!sourceIndex.isValid())
    {
        return QVariant();
    }

    if(role == Qt::UserRole)
    {
        return sourceRow;
    }

    return sourceModel->data(sourceIndex, role);
}

QVariant IndexRowReferenceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(!sourceModel)
    {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    return sourceModel->headerData(section, orientation, role);
}

void IndexRowReferenceModel::onSourceDataChanged(const QModelIndex &topLeft,
                                                 const QModelIndex &bottomRight,
                                                 const QVector<int> &roles)
{
    if(!sourceModel || sourceRows.isEmpty())
    {
        return;
    }

    const int columnTotal = columnCount();
    if(columnTotal <= 0)
    {
        return;
    }

    const int sourceFirstRow = topLeft.row();
    const int sourceLastRow = bottomRight.row();
    const int firstProxyColumn = qBound(0, topLeft.column(), columnTotal - 1);
    const int lastProxyColumn = qBound(0, bottomRight.column(), columnTotal - 1);

    int runStart = -1;
    for(int proxyRow = 0; proxyRow < sourceRows.size(); ++proxyRow)
    {
        const int mappedSourceRow = sourceRows[proxyRow];
        const bool inChangedRange = (mappedSourceRow >= sourceFirstRow && mappedSourceRow <= sourceLastRow);

        if(inChangedRange)
        {
            if(runStart < 0)
            {
                runStart = proxyRow;
            }
            continue;
        }

        if(runStart >= 0)
        {
            emit dataChanged(index(runStart, firstProxyColumn),
                             index(proxyRow - 1, lastProxyColumn),
                             roles);
            runStart = -1;
        }
    }

    if(runStart >= 0)
    {
        emit dataChanged(index(runStart, firstProxyColumn),
                         index(sourceRows.size() - 1, lastProxyColumn),
                         roles);
    }
}

void IndexRowReferenceModel::onSourceHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    emit headerDataChanged(orientation, first, last);
}
