/****************************************************************************
** Copyright (c) 2000-2003 Wayne Roth
** Copyright (c) 2004-2007 Stefan Sander
** Copyright (c) 2007 Michal Policht
** Copyright (c) 2008 Brandon Fosdick
** Copyright (c) 2009-2010 Liam Staskawicz
** Copyright (c) 2011 Debao Zhang
** All right reserved.
** Web: http://code.google.com/p/qextserialport/
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
** LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
** OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
** WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
****************************************************************************/

#include "qextwineventnotifier_p.h"
#include <QtCore/QThread>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QEvent>
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>

class QextWinEventNotifierPrivate
{
    Q_DECLARE_PUBLIC(QextWinEventNotifier)
public:
    QextWinEventNotifierPrivate(HANDLE hEvent, QextWinEventNotifier * q)
        :handleToEvent(hEvent), enabled(false), q_ptr(q)
    {}

    HANDLE handleToEvent;
    bool enabled;
private:
    QextWinEventNotifier * q_ptr;
};

/*
  \internal

  \class QextWinEventNotifierThread

  This class works more or less like an EventDispatcher.

  The api function WaitForMultipleObjects() is used in the new thread
  to wait for the  registered handle.
*/
class QextWinEventNotifierThread:public QThread
{
public:
    explicit QextWinEventNotifierThread(QObject * parent=0);
    ~QextWinEventNotifierThread();
    void stop();
    bool registerEventNotifier(QextWinEventNotifier * notifier);
    void unregisterEventNotifier(QextWinEventNotifier * notifier);
protected:
    void run();
private:
    HANDLE hStopEvent; //stop thread when this event signaled.
    HANDLE hUpdateEvent; //make sure eventlist updated.
    QMutex mutex;
    QList<QextWinEventNotifier *> winEventNotifierList;
};

Q_GLOBAL_STATIC(QextWinEventNotifierThread, notifierThread)

QextWinEventNotifierThread::QextWinEventNotifierThread(QObject * parent)
    :QThread(parent)
{
    hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    hUpdateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    start();
}

QextWinEventNotifierThread::~QextWinEventNotifierThread()
{
    if (isRunning())
        stop();
    CloseHandle(hStopEvent);
    CloseHandle(hUpdateEvent);
}

void QextWinEventNotifierThread::stop()
{
    {
        QMutexLocker locker(&mutex);
        SetEvent(hStopEvent);
    }
    wait();    /// Is this an good idea?
}

bool QextWinEventNotifierThread::registerEventNotifier(QextWinEventNotifier *notifier)
{
    QMutexLocker locker(&mutex);
    if (!notifier) {
        QESP_WARNING("QextWinEventNotifier: Internal error");
        return false;
    }
    if (winEventNotifierList.contains(notifier))
        return true;
    if (winEventNotifierList.count() >= MAXIMUM_WAIT_OBJECTS - 3) {
        QESP_WARNING("QextWinEventNotifier: Cannot have more than %d enabled at one time", MAXIMUM_WAIT_OBJECTS - 3);
        return false;
    }
    winEventNotifierList.append(notifier);
    SetEvent(hUpdateEvent);
    return true;
}

void QextWinEventNotifierThread::unregisterEventNotifier(QextWinEventNotifier *notifier)
{
    QMutexLocker locker(&mutex);
    if (!notifier) {
        QESP_WARNING("QextWinEventNotifier: Internal error");
        return;
    }

    int idx = winEventNotifierList.indexOf(notifier);
    if (idx != -1) {
        winEventNotifierList.takeAt(idx);
        SetEvent(hUpdateEvent);
    }
}

void QextWinEventNotifierThread::run()
{
    forever{
        HANDLE pHandles[MAXIMUM_WAIT_OBJECTS - 1];
        DWORD nCount = 0;
        {
            QMutexLocker locker(&mutex);
            nCount = winEventNotifierList.count();
            for (int i=0; i<(int)nCount; ++i)
                pHandles[i] = winEventNotifierList.at(i)->handle();
            pHandles[nCount] = hUpdateEvent;
            pHandles[nCount+1] = hStopEvent;
        }
        DWORD ret = WaitForMultipleObjects(nCount+2, pHandles, FALSE, INFINITE);
        if (ret >= WAIT_OBJECT_0 && ret < WAIT_OBJECT_0 + nCount) {
            QEvent *evt = new QEvent(QEvent::User);
            QMutexLocker locker(&mutex);
            ResetEvent(pHandles[ret-WAIT_OBJECT_0]);
            QObject * notifier = winEventNotifierList[ret - WAIT_OBJECT_0];
            QCoreApplication::postEvent(notifier, evt);
        }
        else if (ret == WAIT_OBJECT_0 + nCount) {
            //ResetEvent(hUpdateEvent);
        }
        else if (ret == WAIT_OBJECT_0 + nCount + 1) {
            //qDebug()<<"quit...";
            return;
        }
    }
}

/*!
    \internal
    \class QextWinEventNotifier
    \brief The QextWinEventNotifier class provides support for the Windows Wait functions.

    The QextWinEventNotifier class makes it possible to use the wait
    functions on windows in a asynchronous manner. With this class
    you can register a HANDLE to an event and get notification when
    that event becomes signalled.

    \bold Note: If it is a manual reset event ,it will be reset before
    the notification. This is different from QWinEventNotifier.

    \bold Note: All the registered handles will be waited under a new thread.
    This is different from QWinEventNotifier whose event handle will be waited
    in its affinal thread.
*/

QextWinEventNotifier::QextWinEventNotifier(QObject *parent)
    : QObject(parent), d_ptr(new QextWinEventNotifierPrivate(0, this))
{}

QextWinEventNotifier::QextWinEventNotifier(HANDLE hEvent, QObject *parent)
 : QObject(parent), d_ptr(new QextWinEventNotifierPrivate(hEvent, this))
{
    setEnabled(true);
}

QextWinEventNotifier::~QextWinEventNotifier()
{
    setEnabled(false);
}

void QextWinEventNotifier::setHandle(HANDLE hEvent)
{
    setEnabled(false);
    Q_D(QextWinEventNotifier);
    d->handleToEvent = hEvent;
}

HANDLE  QextWinEventNotifier::handle() const
{
    return d_func()->handleToEvent;
}

bool QextWinEventNotifier::isEnabled() const
{
    return d_func()->enabled;
}

void QextWinEventNotifier::setEnabled(bool enable)
{
    Q_D(QextWinEventNotifier);

    if (d->enabled == enable)
        return;
    d->enabled = enable;

    if (d->enabled)
        notifierThread()->registerEventNotifier(this);
    else
        notifierThread()->unregisterEventNotifier(this);
}

bool QextWinEventNotifier::event(QEvent * e)
{
    Q_D(QextWinEventNotifier);
    QObject::event(e);
    if (e->type() == QEvent::User) {
        emit activated(d->handleToEvent);
        return true;
    }
    return false;
}

