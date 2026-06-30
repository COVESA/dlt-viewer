#include "crlfindexviewmodel.h"

#include "qdltfile.h"

CrlfIndexViewModel::CrlfIndexViewModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void CrlfIndexViewModel::setSourceModel(QAbstractTableModel *sourceModel)
{
    beginResetModel();
    m_sourceModel = sourceModel;
    endResetModel();
}

void CrlfIndexViewModel::setDltFile(QDltFile *dltFile)
{
    m_dltFile = dltFile;
}

void CrlfIndexViewModel::setSourceRowRefs(QVector<int> rowRefs)
{
    beginResetModel();
    m_sourceRowRefs = std::move(rowRefs);
    endResetModel();
}

void CrlfIndexViewModel::clear()
{
    setSourceRowRefs({});
}

int CrlfIndexViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_sourceRowRefs.size();
}

int CrlfIndexViewModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_sourceModel)
        return 0;

    return m_sourceModel->columnCount();
}

QVariant CrlfIndexViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_sourceModel || index.row() < 0 || index.row() >= m_sourceRowRefs.size())
        return QVariant();

    const int sourceRow = m_sourceRowRefs.at(index.row());
    if (role == Qt::UserRole)
    {
        if (!m_dltFile)
            return QVariant();

        return m_dltFile->getMsgFilterPos(sourceRow);
    }

    if (role == (Qt::UserRole + 1))
    {
        return sourceRow;
    }

    const QModelIndex sourceIndex = m_sourceModel->index(sourceRow, index.column());
    if (!sourceIndex.isValid())
        return QVariant();

    return m_sourceModel->data(sourceIndex, role);
}

QVariant CrlfIndexViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!m_sourceModel)
        return QVariant();

    return m_sourceModel->headerData(section, orientation, role);
}
