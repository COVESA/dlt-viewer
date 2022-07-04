#ifndef PLUGINTREEWIDGET_H
#define PLUGINTREEWIDGET_H

#include <QTreeWidget>

class PluginTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit PluginTreeWidget(QObject *parent = 0);

    void sortAccordingPriority(const QStringList& prio_list);
    void raisePluginPriority(int index);
    void decreasePluginPriority(int index);

    void dropEvent(QDropEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);

signals:
    void pluginOrderChanged(const QString& name, int);
public slots:

private:
    bool setPluginPriority(const QString& name, unsigned int prio);

};

#endif // PLUGINTREEWIDGET_H
