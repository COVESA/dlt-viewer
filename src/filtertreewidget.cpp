#include "filtertreewidget.h"
#include <QDropEvent>
#include <QMimeData>

FilterTreeWidget::FilterTreeWidget(QObject *parent) :
    QTreeWidget(qobject_cast<QWidget *>(parent))
{
}

void FilterTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void FilterTreeWidget::dropEvent(QDropEvent *event)
{
    QStringList types = event->mimeData()->formats();
    for(int i=0;i < types.size(); i++)
    {
        /* QT advertises our filteritems with this MIME identifier.
         * We use this to identify when a filter item was dropped,
         * instead of some other random data. */
        if(types[i] == "application/x-qabstractitemmodeldatalist")
        {
            QTreeWidget::dropEvent(event);
            emit filterItemDropped();
            event->accept();
            break;
         }
    }
}
