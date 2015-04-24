#include "dlttableview.h"

DltTableView::DltTableView(QWidget *parent) :
    QTableView(parent)
{
}

/*!
    Paints the table on receipt of the given paint event \a event.
*/
void DltTableView::paintEvent(QPaintEvent *event)
{
    if(paintMutex.tryLock())
    {
        QTableView::paintEvent(event);
        paintMutex.unlock();
    }
}

void DltTableView::lock()
{
    paintMutex.lock();
}

void DltTableView::unlock()
{
    paintMutex.unlock();
}
