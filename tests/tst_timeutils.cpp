#include "core/TimeUtils.h"

#include <QDate>
#include <QDateTime>
#include <QLocale>
#include <QTest>
#include <QTimeZone>

namespace {

qint64 localEpoch(int year, int month, int day, int hour = 0, int minute = 0, int second = 0) {
    return QDateTime(QDate(year, month, day), QTime(hour, minute, second),
                     QTimeZone::systemTimeZone())
        .toSecsSinceEpoch();
}

bool systemZoneIs(const char *zoneId) {
    return QTimeZone::systemTimeZoneId() == zoneId;
}

} // namespace

class TstTimeUtils : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() { QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates)); }

    void dayStartIsLocalMidnightForAfternoonTime() {
        const qint64 epoch = localEpoch(2026, 6, 15, 14, 30);
        QCOMPARE(tt::TimeUtils::localDayStartUtc(epoch), localEpoch(2026, 6, 15));
    }

    void dayStartOfExactMidnightIsItself() {
        const qint64 midnight = localEpoch(2026, 6, 15);
        QCOMPARE(tt::TimeUtils::localDayStartUtc(midnight), midnight);
    }

    void dayEndIsNextLocalMidnight() {
        const qint64 epoch = localEpoch(2026, 6, 15, 14, 30);
        QCOMPARE(tt::TimeUtils::localDayEndUtc(epoch), localEpoch(2026, 6, 16));
    }

    void dayEndOfLastSecondIsNextMidnight() {
        const qint64 epoch = localEpoch(2026, 6, 15, 23, 59, 59);
        QCOMPARE(tt::TimeUtils::localDayEndUtc(epoch), localEpoch(2026, 6, 16));
    }

    void dayEndOfExactMidnightBelongsToNextDay() {
        const qint64 midnight = localEpoch(2026, 6, 16);
        QCOMPARE(tt::TimeUtils::localDayStartUtc(midnight), midnight);
        QCOMPARE(tt::TimeUtils::localDayEndUtc(midnight), localEpoch(2026, 6, 17));
    }

    void springForwardDayIsTwentyThreeHoursNewYork() {
        if (!systemZoneIs("America/New_York"))
            QSKIP("requires TZ=America/New_York");
        const qint64 start = tt::TimeUtils::localDayStartUtc(localEpoch(2026, 3, 8, 12));
        QCOMPARE(start, qint64(1772946000)); // 2026-03-08T00:00 EST == 05:00 UTC
        QCOMPARE(tt::TimeUtils::localDayEndUtc(start) - start, qint64(23 * 3600));
    }

    void fallBackDayIsTwentyFiveHoursNewYork() {
        if (!systemZoneIs("America/New_York"))
            QSKIP("requires TZ=America/New_York");
        const qint64 start = tt::TimeUtils::localDayStartUtc(localEpoch(2026, 11, 1, 12));
        QCOMPARE(tt::TimeUtils::localDayEndUtc(start) - start, qint64(25 * 3600));
    }

    void springForwardDayIsTwentyThreeHoursBerlin() {
        if (!systemZoneIs("Europe/Berlin"))
            QSKIP("requires TZ=Europe/Berlin");
        const qint64 start = tt::TimeUtils::localDayStartUtc(localEpoch(2026, 3, 29, 12));
        QCOMPARE(tt::TimeUtils::localDayEndUtc(start) - start, qint64(23 * 3600));
    }

    void fallBackDayIsTwentyFiveHoursBerlin() {
        if (!systemZoneIs("Europe/Berlin"))
            QSKIP("requires TZ=Europe/Berlin");
        const qint64 start = tt::TimeUtils::localDayStartUtc(localEpoch(2026, 10, 25, 12));
        QCOMPARE(tt::TimeUtils::localDayEndUtc(start) - start, qint64(25 * 3600));
    }

    void splitSingleDayReturnsOneSegment() {
        const qint64 start = localEpoch(2026, 6, 15, 9);
        const qint64 end = localEpoch(2026, 6, 15, 17);
        const auto segments = tt::TimeUtils::splitByLocalDay(start, end);
        QCOMPARE(segments.size(), 1);
        QCOMPARE(segments.at(0).startUtc, start);
        QCOMPARE(segments.at(0).endUtc, end);
        QCOMPARE(segments.at(0).localDay, QDate(2026, 6, 15));
    }

    void splitMidnightCrossingAtLocalMidnight() {
        const qint64 start = localEpoch(2026, 6, 15, 22);
        const qint64 end = localEpoch(2026, 6, 16, 2);
        const auto segments = tt::TimeUtils::splitByLocalDay(start, end);
        QCOMPARE(segments.size(), 2);

        QCOMPARE(segments.at(0).startUtc, start);
        QCOMPARE(segments.at(0).endUtc, localEpoch(2026, 6, 16));
        QCOMPARE(segments.at(0).localDay, QDate(2026, 6, 15));

        QCOMPARE(segments.at(1).startUtc, localEpoch(2026, 6, 16));
        QCOMPARE(segments.at(1).endUtc, end);
        QCOMPARE(segments.at(1).localDay, QDate(2026, 6, 16));
    }

    void splitSpansThreeDaysContiguously() {
        const qint64 start = localEpoch(2026, 6, 15, 10);
        const qint64 end = localEpoch(2026, 6, 17, 8);
        const auto segments = tt::TimeUtils::splitByLocalDay(start, end);
        QCOMPARE(segments.size(), 3);

        QCOMPARE(segments.at(0).startUtc, start);
        QCOMPARE(segments.at(2).endUtc, end);
        for (qsizetype i = 0; i + 1 < segments.size(); ++i)
            QCOMPARE(segments.at(i).endUtc, segments.at(i + 1).startUtc);

        qint64 total = 0;
        for (const auto &segment : segments)
            total += segment.endUtc - segment.startUtc;
        QCOMPARE(total, end - start);
    }

    void splitAcrossSpringForwardUsesRealLengthOfDay() {
        if (!systemZoneIs("America/New_York"))
            QSKIP("requires TZ=America/New_York");
        const qint64 start = localEpoch(2026, 3, 7, 12);
        const qint64 end = localEpoch(2026, 3, 9, 12);
        const auto segments = tt::TimeUtils::splitByLocalDay(start, end);
        QCOMPARE(segments.size(), 3);
        QCOMPARE(segments.at(0).endUtc - segments.at(0).startUtc, qint64(12 * 3600));
        QCOMPARE(segments.at(1).endUtc - segments.at(1).startUtc, qint64(23 * 3600));
        QCOMPARE(segments.at(2).endUtc - segments.at(2).startUtc, qint64(12 * 3600));
        QCOMPARE(segments.at(1).localDay, QDate(2026, 3, 8));
    }

    void splitReturnsEmptyWhenEndNotAfterStart() {
        const qint64 start = localEpoch(2026, 6, 15, 10);
        QVERIFY(tt::TimeUtils::splitByLocalDay(start, start).isEmpty());
        QVERIFY(tt::TimeUtils::splitByLocalDay(start, start - 1).isEmpty());
    }

    void formatDurationShowsHoursMinutesSeconds() {
        QCOMPARE(tt::TimeUtils::formatDuration(0), QStringLiteral("0:00:00"));
        QCOMPARE(tt::TimeUtils::formatDuration(59), QStringLiteral("0:00:59"));
        QCOMPARE(tt::TimeUtils::formatDuration(3600), QStringLiteral("1:00:00"));
        QCOMPARE(tt::TimeUtils::formatDuration(3661), QStringLiteral("1:01:01"));
        QCOMPARE(tt::TimeUtils::formatDuration(36000), QStringLiteral("10:00:00"));
    }

    void formatDurationKeepsCountingPastTwentyFourHours() {
        QCOMPARE(tt::TimeUtils::formatDuration(86405), QStringLiteral("24:00:05"));
    }

    void weekStartIsLocalSundayUnderUsLocale() {
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
        const qint64 wednesday = localEpoch(2026, 6, 17, 15);
        QCOMPARE(tt::TimeUtils::weekStartUtc(wednesday), localEpoch(2026, 6, 14));
    }

    void weekStartIsLocalMondayUnderUkLocale() {
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedKingdom));
        const qint64 wednesday = localEpoch(2026, 6, 17, 15);
        QCOMPARE(tt::TimeUtils::weekStartUtc(wednesday), localEpoch(2026, 6, 15));
        const qint64 sunday = localEpoch(2026, 6, 14, 9);
        QCOMPARE(tt::TimeUtils::weekStartUtc(sunday), localEpoch(2026, 6, 8));
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }
};

QTEST_APPLESS_MAIN(TstTimeUtils)
#include "tst_timeutils.moc"
