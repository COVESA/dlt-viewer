#ifndef PLUGINTREEWIDGET_H
#define PLUGINTREEWIDGET_H

#include <QTreeWidget>

class PluginTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit PluginTreeWidget(QObject *parent = 0);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

signals:
    void filterItemDropped();
public slots:

};

#endif // PLUGINTREEWIDGET_H
