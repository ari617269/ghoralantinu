#include "core/FakeClock.h"
#include "core/SystemClock.h"

#include <QDateTime>
#include <QTest>

class TstClock : public QObject {
    Q_OBJECT

private slots:
    void fakeClockReturnsInitialTime() {
        tt::FakeClock clock(1000);
        QCOMPARE(clock.now(), 1000);
    }

    void fakeClockAdvancesByGivenSeconds() {
        tt::FakeClock clock(1000);
        clock.advance(5);
        QCOMPARE(clock.now(), 1005);
    }

    void fakeClockSetNowReplacesTime() {
        tt::FakeClock clock(1000);
        clock.setNow(42);
        QCOMPARE(clock.now(), 42);
    }

    void systemClockIsWithinCurrentWallTime() {
        const qint64 before = QDateTime::currentSecsSinceEpoch();
        tt::SystemClock clock;
        const qint64 now = clock.now();
        const qint64 after = QDateTime::currentSecsSinceEpoch();
        QVERIFY(now >= before);
        QVERIFY(now <= after);
    }
};

QTEST_APPLESS_MAIN(TstClock)
#include "tst_clock.moc"
