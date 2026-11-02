#pragma once

#include "core/Database.h"
#include "core/Models.h"

#include <QDate>
#include <QList>
#include <QMap>
#include <QSqlDatabase>
#include <optional>

namespace tt {

class SessionRepository {
public:
    explicit SessionRepository(const Database &db);

    std::optional<Session> start(qint64 taskId, qint64 startUtc, const QString &tzName);
    std::optional<Session> addClosed(qint64 taskId, qint64 startUtc, qint64 endUtc,
                                     const QString &tzName);
    bool stop(qint64 id, qint64 endUtc);
    std::optional<Session> findById(qint64 id) const;
    std::optional<Session> findRunning() const;
    QList<Session> listInRange(qint64 rangeStartUtc, qint64 rangeEndUtc) const;
    QList<qint64> recentTaskIds(int limit) const;
    bool hasOverlap(qint64 startUtc, qint64 endUtc, std::optional<qint64> excludeId) const;
    bool update(const Session &session);
    bool remove(qint64 id);

    qint64 totalForTaskInRange(qint64 taskId, qint64 rangeStartUtc, qint64 rangeEndUtc,
                               qint64 nowUtc) const;
    QMap<QDate, qint64> dayTotals(qint64 rangeStartUtc, qint64 rangeEndUtc, qint64 nowUtc) const;

private:
    QSqlDatabase m_db;
};

} // namespace tt
