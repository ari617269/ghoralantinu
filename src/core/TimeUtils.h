#pragma once

#include <QDate>
#include <QList>
#include <QString>
#include <QtGlobal>

namespace tt {

struct DaySegment {
    qint64 startUtc;
    qint64 endUtc;
    QDate localDay;
};

class TimeUtils {
public:
    static QDate localDate(qint64 epochSecs);
    static qint64 localMidnightUtc(const QDate &day);
    static qint64 localDayStartUtc(qint64 epochSecs);
    static qint64 localDayEndUtc(qint64 epochSecs);
    static qint64 weekStartUtc(qint64 epochSecs);
    static QList<DaySegment> splitByLocalDay(qint64 startUtc, qint64 endUtc);
    static QString formatDuration(qint64 seconds);
};

} // namespace tt
