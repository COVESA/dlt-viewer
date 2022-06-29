#include "plugintreewidget.h"
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>

PluginTreeWidget::PluginTreeWidget(QObject *parent) :
    QTreeWidget(qobject_cast<QWidget *>(parent))
{
    setDragDropMode(QAbstractItemView::InternalMove);
}

void PluginTreeWidget::dropEvent(QDropEvent *event)
{
    QTreeWidgetItem* item = this->currentItem();
    QTreeWidget::dropEvent(event);
    this->setCurrentItem(item);

    emit pluginOrderChanged(item->text(0), this->indexOfTopLevelItem(item));
}

