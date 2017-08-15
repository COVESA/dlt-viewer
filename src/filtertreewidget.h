#ifndef FILTERTREEWIDGET_H
#define FILTERTREEWIDGET_H

#include <QTreeWidget>
#include <QPushButton>

class FilterTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit FilterTreeWidget(QObject *parent = 0);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
signals:
    void filterItemDropped();
public slots:

};

#endif // FILTERTREEWIDGET_H
