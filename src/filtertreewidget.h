#ifndef FILTERTREEWIDGET_H
#define FILTERTREEWIDGET_H

#include <QTreeWidget>
#include <QPushButton>

class FilterTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit FilterTreeWidget(QObject *parent = 0);
    void setFilterButton(QPushButton *btn);
    void dragMoveEvent(QDragMoveEvent *event);

signals:
    
public slots:
    
private:
    QPushButton *filterButton;
};

#endif // FILTERTREEWIDGET_H
