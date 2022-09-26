#ifndef INDEXRANGEPROXYMODEL_H
#define INDEXRANGEPROXYMODEL_H

#include <QSortFilterProxyModel>

class IndexRangeProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    IndexRangeProxyModel(QObject* parent = 0);
    bool filterAcceptsRow(int source_row,
                          const QModelIndex &source_parent) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const;

public slots:
    void setStartIndex(int startIndex);
    void setEndIndex(int endIndex);
private:
    int m_startIndex;
    int m_endIndex;
};


#endif // INDEXRANGEPROXYMODEL_H
