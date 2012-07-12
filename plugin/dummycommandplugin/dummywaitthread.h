#ifndef DUMMYWAITTHREAD_H
#define DUMMYWAITTHREAD_H

#include <QThread>

class DummyWaitThread : public QThread
{
    Q_OBJECT
public:
    explicit DummyWaitThread(QObject *parent = 0);
    void run();
    int getProgress();
    void reset();

signals:
    
public slots:
    
private:
    int progress;
};

#endif // DUMMYWAITTHREAD_H
