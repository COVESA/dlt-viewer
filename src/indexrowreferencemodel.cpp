#include "indexrowreferencemodel.h"

IndexRowReferenceModel::IndexRowReferenceModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void IndexRowReferenceModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    beginResetModel();
    sourceModel = newSourceModel;
    sourceRows.clear();
    endResetModel();
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
