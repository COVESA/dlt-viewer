#ifndef PLUGINTREEWIDGET_H
#define PLUGINTREEWIDGET_H

#include <QTreeWidget>

class PluginTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit PluginTreeWidget(QObject *parent = 0);
    void dropEvent(QDropEvent *event);

signals:
    void pluginOrderChanged(const QString& name, int);
public slots:

};

#endif // PLUGINTREEWIDGET_H
