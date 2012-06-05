#include "qextwineventnotifier_p.h"
#include <QtTest/QtTest>
#include <QtCore/QTimer>

class tst_QextWinEventNotifier : public QObject
{
    Q_OBJECT

public:
    tst_QextWinEventNotifier(){}
    ~tst_QextWinEventNotifier(){}

    void initTestCase(){}
    void cleanupTestCase(){}
    void init(){}
    void cleanup(){}

protected slots:
    void simple_activated();
    void simple_timerSet();

private slots:
    void empty();
    void simple();

private:
    HANDLE simpleHEvent;
    int simpleActivated;
};

void tst_QextWinEventNotifier::simple_activated()
{
    ++simpleActivated;
    ResetEvent((HANDLE)simpleHEvent);
    QTestEventLoop::instance().exitLoop();
}

void tst_QextWinEventNotifier::simple_timerSet()
{
    SetEvent((HANDLE)simpleHEvent);
}

void tst_QextWinEventNotifier::empty()
{
    QextWinEventNotifier n1, n2, n3;
    n1.setEnabled(true);
    n2.setEnabled(true);
    n3.setEnabled(true);
    n1.setEnabled(false);
}

void tst_QextWinEventNotifier::simple()
{
    simpleHEvent = CreateEvent(0, TRUE, FALSE, 0);
    QVERIFY(simpleHEvent);

    QextWinEventNotifier n(simpleHEvent);
    QObject::connect(&n, SIGNAL(activated(HANDLE)), this, SLOT(simple_activated()));

    simpleActivated = 0;
    SetEvent((HANDLE)simpleHEvent);
    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout())
        QFAIL("Timed out");
    QVERIFY(simpleActivated == 1);

    simpleActivated = 0;
    QTimer::singleShot(3000, this, SLOT(simple_timerSet()));
    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout())
        QFAIL("Timed out");
    QVERIFY(simpleActivated == 1);
}

QTEST_MAIN(tst_QextWinEventNotifier)

#include "tst_qextwineventnotifier.moc"
