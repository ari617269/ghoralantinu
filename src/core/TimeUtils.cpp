#include "core/TimeUtils.h"

#include <QDateTime>
#include <QLocale>
#include <QTime>
#include <QTimeZone>

namespace tt {

QDate TimeUtils::localDate(qint64 epochSecs) {
    return QDateTime::fromSecsSinceEpoch(epochSecs, QTimeZone::systemTimeZone())
        .toLocalTime()
        .date();
}

qint64 TimeUtils::localMidnightUtc(const QDate &day) {
    return QDateTime(day, QTime(0, 0), QTimeZone::systemTimeZone()).toSecsSinceEpoch();
}

qint64 TimeUtils::localDayStartUtc(qint64 epochSecs) {
    return localMidnightUtc(localDate(epochSecs));
}

qint64 TimeUtils::localDayEndUtc(qint64 epochSecs) {
    return localMidnightUtc(localDate(epochSecs).addDays(1));
}

qint64 TimeUtils::weekStartUtc(qint64 epochSecs) {
    const QDate day = localDate(epochSecs);
    const int firstDay = QLocale().firstDayOfWeek();
    const int offset = (day.dayOfWeek() - firstDay + 7) % 7;
    return localMidnightUtc(day.addDays(-offset));
}

QList<DaySegment> TimeUtils::splitByLocalDay(qint64 startUtc, qint64 endUtc) {
    QList<DaySegment> segments;
    qint64 cursor = startUtc;
    while (cursor < endUtc) {
        const qint64 dayEnd = localDayEndUtc(cursor);
        const qint64 segmentEnd = qMin(dayEnd, endUtc);
        segments.append({cursor, segmentEnd, localDate(cursor)});
        cursor = segmentEnd;
    }
    return segments;
}

QString TimeUtils::formatDuration(qint64 seconds) {
    const qint64 hours = seconds / 3600;
    const qint64 minutes = (seconds % 3600) / 60;
    const qint64 secs = seconds % 60;
    return QStringLiteral("%1:%2:%3")
        .arg(hours)
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(secs, 2, 10, QLatin1Char('0'));
}

} // namespace tt
