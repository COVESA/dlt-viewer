#include "indexrangeproxymodel.h"
#include "fieldnames.h"
#include<QDebug>

IndexRangeProxyModel::IndexRangeProxyModel(QObject *parent): QSortFilterProxyModel(parent),
    m_startIndex(0), m_endIndex(0)
{
}

void IndexRangeProxyModel::setStartIndex(int startIndex){
    if(m_startIndex != startIndex)
        m_startIndex = startIndex;
    //invalidateFilter();
}

void IndexRangeProxyModel::setEndIndex(int endIndex){
    if(m_endIndex != endIndex)
        m_endIndex = endIndex;
    invalidateFilter();
    //this->invalidate();
}

bool IndexRangeProxyModel::filterAcceptsRow(int source_row,
                                  const QModelIndex &source_parent) const{

    QModelIndex index = sourceModel()->index(source_row,
                                              FieldNames::Index, source_parent);

    if(m_endIndex > m_startIndex ){

    if(sourceModel()->data(index).toInt() < m_startIndex ||
            sourceModel()->data(index).toInt() > m_endIndex)
        return false;
    }
    return true;
}

QVariant IndexRangeProxyModel::headerData(int section, Qt::Orientation orientation,
                                int role) const {
    return sourceModel()->headerData(section, orientation,
                                     role);
}
