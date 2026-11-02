#include "core/SessionRepository.h"

#include "core/TimeUtils.h"

#include <QSqlQuery>

namespace tt {

namespace {

const QString kSessionColumns =
    QStringLiteral("SELECT id, task_id, start_utc, end_utc, tz_name FROM sessions");

Session sessionFromQuery(const QSqlQuery &query) {
    Session session;
    session.id = query.value(0).toLongLong();
    session.taskId = query.value(1).toLongLong();
    session.startUtc = query.value(2).toLongLong();
    if (!query.value(3).isNull())
        session.endUtc = query.value(3).toLongLong();
    session.tzName = query.value(4).toString();
    return session;
}

} // namespace

SessionRepository::SessionRepository(const Database &db) : m_db(db.connection()) {}

std::optional<Session> SessionRepository::start(qint64 taskId, qint64 startUtc,
                                                const QString &tzName) {
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("INSERT INTO sessions (task_id, start_utc, tz_name) "
                                 "VALUES (?, ?, ?)"));
    query.addBindValue(taskId);
    query.addBindValue(startUtc);
    query.addBindValue(tzName);
    if (!query.exec())
        return std::nullopt;

    Session session;
    session.id = query.lastInsertId().toLongLong();
    session.taskId = taskId;
    session.startUtc = startUtc;
    session.tzName = tzName;
    return session;
}

std::optional<Session> SessionRepository::addClosed(qint64 taskId, qint64 startUtc, qint64 endUtc,
                                                    const QString &tzName) {
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("INSERT INTO sessions (task_id, start_utc, end_utc, tz_name) "
                                 "VALUES (?, ?, ?, ?)"));
    query.addBindValue(taskId);
    query.addBindValue(startUtc);
    query.addBindValue(endUtc);
    query.addBindValue(tzName);
    if (!query.exec())
        return std::nullopt;

    Session session;
    session.id = query.lastInsertId().toLongLong();
    session.taskId = taskId;
    session.startUtc = startUtc;
    session.endUtc = endUtc;
    session.tzName = tzName;
    return session;
}

bool SessionRepository::stop(qint64 id, qint64 endUtc) {
    QSqlQuery query(m_db);
    query.prepare(
        QStringLiteral("UPDATE sessions SET end_utc = ? WHERE id = ? AND end_utc IS NULL"));
    query.addBindValue(endUtc);
    query.addBindValue(id);
    return query.exec() && query.numRowsAffected() == 1;
}

std::optional<Session> SessionRepository::findById(qint64 id) const {
    QSqlQuery query(m_db);
    query.prepare(kSessionColumns + QStringLiteral(" WHERE id = ?"));
    query.addBindValue(id);
    if (!query.exec() || !query.next())
        return std::nullopt;
    return sessionFromQuery(query);
}

std::optional<Session> SessionRepository::findRunning() const {
    QSqlQuery query(m_db);
    if (!query.exec(kSessionColumns + QStringLiteral(" WHERE end_utc IS NULL "
                                                     "ORDER BY start_utc DESC LIMIT 1")) ||
        !query.next()) {
        return std::nullopt;
    }
    return sessionFromQuery(query);
}

QList<Session> SessionRepository::listInRange(qint64 rangeStartUtc, qint64 rangeEndUtc) const {
    QList<Session> sessions;
    QSqlQuery query(m_db);
    query.prepare(kSessionColumns + QStringLiteral(" WHERE start_utc < ? "
                                                   "AND (end_utc IS NULL OR end_utc > ?) "
                                                   "ORDER BY start_utc"));
    query.addBindValue(rangeEndUtc);
    query.addBindValue(rangeStartUtc);
    if (!query.exec())
        return sessions;
    while (query.next())
        sessions.append(sessionFromQuery(query));
    return sessions;
}

QList<qint64> SessionRepository::recentTaskIds(int limit) const {
    QList<qint64> ids;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT task_id FROM sessions GROUP BY task_id "
                                 "ORDER BY MAX(start_utc) DESC LIMIT ?"));
    query.addBindValue(limit);
    if (!query.exec())
        return ids;
    while (query.next())
        ids.append(query.value(0).toLongLong());
    return ids;
}

bool SessionRepository::hasOverlap(qint64 startUtc, qint64 endUtc,
                                   std::optional<qint64> excludeId) const {
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM sessions WHERE start_utc < ? "
                                 "AND (end_utc IS NULL OR end_utc > ?) AND id != ?"));
    query.addBindValue(endUtc);
    query.addBindValue(startUtc);
    query.addBindValue(excludeId.value_or(0));
    return query.exec() && query.next() && query.value(0).toLongLong() > 0;
}

bool SessionRepository::update(const Session &session) {
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE sessions SET task_id = ?, start_utc = ?, end_utc = ?, "
                                 "tz_name = ? WHERE id = ?"));
    query.addBindValue(session.taskId);
    query.addBindValue(session.startUtc);
    query.addBindValue(session.endUtc.has_value() ? QVariant(*session.endUtc) : QVariant());
    query.addBindValue(session.tzName);
    query.addBindValue(session.id);
    return query.exec() && query.numRowsAffected() == 1;
}

bool SessionRepository::remove(qint64 id) {
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("DELETE FROM sessions WHERE id = ?"));
    query.addBindValue(id);
    return query.exec() && query.numRowsAffected() == 1;
}

qint64 SessionRepository::totalForTaskInRange(qint64 taskId, qint64 rangeStartUtc,
                                              qint64 rangeEndUtc, qint64 nowUtc) const {
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT COALESCE(SUM(MAX(0, MIN(COALESCE(end_utc, ?), ?) - MAX(start_utc, ?))), 0) "
        "FROM sessions WHERE task_id = ? AND start_utc < ? "
        "AND (end_utc IS NULL OR end_utc > ?)"));
    query.addBindValue(nowUtc);
    query.addBindValue(rangeEndUtc);
    query.addBindValue(rangeStartUtc);
    query.addBindValue(taskId);
    query.addBindValue(rangeEndUtc);
    query.addBindValue(rangeStartUtc);
    if (!query.exec() || !query.next())
        return 0;
    return query.value(0).toLongLong();
}

QMap<QDate, qint64> SessionRepository::dayTotals(qint64 rangeStartUtc, qint64 rangeEndUtc,
                                                 qint64 nowUtc) const {
    QMap<QDate, qint64> totals;
    for (const Session &session : listInRange(rangeStartUtc, rangeEndUtc)) {
        const qint64 start = qMax(session.startUtc, rangeStartUtc);
        const qint64 end = qMin(session.endUtc.value_or(nowUtc), rangeEndUtc);
        for (const DaySegment &segment : TimeUtils::splitByLocalDay(start, end))
            totals[segment.localDay] += segment.endUtc - segment.startUtc;
    }
    return totals;
}

} // namespace tt
