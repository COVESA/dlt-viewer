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
    void setStartIndex(unsigned long int startIndex);
    void setEndIndex(unsigned long int endIndex);
    unsigned long int getEndIndex();
private:
    unsigned long int m_startIndex;
    unsigned long int m_endIndex;
};


#endif // INDEXRANGEPROXYMODEL_H
