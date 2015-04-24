#ifndef DLTTABLEVIEW_H
#define DLTTABLEVIEW_H

#include <QTableView>
#include <QMutex>

/**
 * @brief QTableView to wrap the paintEvent
 * There was heap corruption from paintEvent, when
 * enabling filters. Root cause was not yet found.
 * This is a workaround to disable painting while updating filters.
 */
class DltTableView : public QTableView
{
    Q_OBJECT
public:
    explicit DltTableView(QWidget *parent = 0);
    void lock();
    void unlock();
private:
    QMutex paintMutex;

protected:
    void paintEvent(QPaintEvent *e);

signals:
    
public slots:
    
};

#endif // DLTTABLEVIEW_H
