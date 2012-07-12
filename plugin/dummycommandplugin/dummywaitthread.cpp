#include "dummywaitthread.h"

DummyWaitThread::DummyWaitThread(QObject *parent) :
    QThread(parent)
{
}

void DummyWaitThread::run()
{
    progress = 0;
    while(true)
    {
        QThread::msleep(100);
        progress++;
    }
}

int DummyWaitThread::getProgress()
{
    return progress;

}

void DummyWaitThread::reset()
{
    progress = 0;
}
