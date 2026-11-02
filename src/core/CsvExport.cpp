#include "core/CsvExport.h"

#include <QDateTime>
#include <QStringList>

namespace tt {

namespace {

QString csvField(const QString &value) {
    if (!value.contains(QLatin1Char(',')) && !value.contains(QLatin1Char('"')) &&
        !value.contains(QLatin1Char('\n')) && !value.contains(QLatin1Char('\r'))) {
        return value;
    }
    QString escaped = value;
    escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QLatin1Char('"') + escaped + QLatin1Char('"');
}

QString localIsoTime(qint64 utc) {
    return QDateTime::fromSecsSinceEpoch(utc).toLocalTime().toString(Qt::ISODate);
}

} // namespace

QString historyToCsv(const QList<HistoryDay> &days) {
    QStringList lines{QStringLiteral("date,start,end,duration_seconds,task")};
    for (const HistoryDay &day : days) {
        for (const HistoryRow &row : day.rows) {
            const QString end = row.running ? QString() : localIsoTime(*row.session.endUtc);
            lines.append(QStringList{day.day.toString(Qt::ISODate),
                                     localIsoTime(row.session.startUtc), end,
                                     QString::number(row.durationSecs), csvField(row.taskName)}
                             .join(QLatin1Char(',')));
        }
    }
    return lines.join(QLatin1Char('\n')) + QLatin1Char('\n');
}

} // namespace tt
