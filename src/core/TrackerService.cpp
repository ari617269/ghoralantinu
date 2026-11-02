#include "core/TrackerService.h"

#include "core/TimeUtils.h"

#include <QMap>
#include <QTimeZone>
#include <algorithm>

namespace tt {

TrackerService::TrackerService(const Database &db, const IClock &clock, QObject *parent)
    : QObject(parent), m_clock(clock), m_sessions(db), m_tasks(db),
      m_active(m_sessions.findRunning()) {}

std::optional<Task> TrackerService::findOrCreateTask(const QString &name) {
    if (auto existing = m_tasks.findByName(name))
        return existing;
    auto created = m_tasks.create(name, m_clock.now());
    if (created)
        emit dataChanged();
    return created;
}

QList<TaskSummary> TrackerService::taskSummaries() const {
    const qint64 now = m_clock.now();
    QList<TaskSummary> summaries;
    for (const Task &task : m_tasks.list())
        summaries.append(summaryFor(task, now));
    return summaries;
}

QList<Task> TrackerService::recentTasks(int limit) const {
    const QList<Task> active = m_tasks.list();
    QList<Task> recent;
    for (qint64 taskId : m_sessions.recentTaskIds(limit)) {
        for (const Task &task : active) {
            if (task.id == taskId) {
                recent.append(task);
                break;
            }
        }
    }
    return recent;
}

std::optional<TaskSummary> TrackerService::taskSummary(qint64 taskId) const {
    for (const Task &task : m_tasks.list(true)) {
        if (task.id == taskId)
            return summaryFor(task, m_clock.now());
    }
    return std::nullopt;
}

qint64 TrackerService::todayTotal() const {
    const qint64 now = m_clock.now();
    return rangeTotal(TimeUtils::localDayStartUtc(now), TimeUtils::localDayEndUtc(now), now);
}

qint64 TrackerService::weekTotal() const {
    const qint64 now = m_clock.now();
    return rangeTotal(TimeUtils::weekStartUtc(now), TimeUtils::localDayEndUtc(now), now);
}

TaskSummary TrackerService::summaryFor(const Task &task, qint64 nowUtc) const {
    TaskSummary summary;
    summary.task = task;
    summary.todaySecs = m_sessions.totalForTaskInRange(task.id, TimeUtils::localDayStartUtc(nowUtc),
                                                       TimeUtils::localDayEndUtc(nowUtc), nowUtc);
    summary.totalSecs = m_sessions.totalForTaskInRange(task.id, 0, nowUtc, nowUtc);
    summary.running = m_active && m_active->taskId == task.id;
    return summary;
}

qint64 TrackerService::rangeTotal(qint64 startUtc, qint64 endUtc, qint64 nowUtc) const {
    qint64 total = 0;
    const auto totals = m_sessions.dayTotals(startUtc, endUtc, nowUtc);
    for (auto it = totals.cbegin(); it != totals.cend(); ++it)
        total += it.value();
    return total;
}

std::optional<Session> TrackerService::activeSession() const {
    return m_active;
}

qint64 TrackerService::elapsed() const {
    if (!m_active)
        return 0;
    return qMax<qint64>(0, m_clock.now() - m_active->startUtc);
}

bool TrackerService::activeRunningLongerThan(qint64 seconds) const {
    return m_active && elapsed() > seconds;
}

bool TrackerService::isActiveStale() const {
    return activeRunningLongerThan(kStaleSessionSecs);
}

SessionError TrackerService::resolveStaleSession(RecoveryChoice choice, qint64 lastSeenUtc) {
    if (!m_active)
        return SessionError::NotFound;

    switch (choice) {
    case RecoveryChoice::Keep:
        return SessionError::None;
    case RecoveryChoice::StopAtLastSeen: {
        const qint64 endUtc = qMax(m_active->startUtc, qMin(lastSeenUtc, m_clock.now()));
        if (!m_sessions.stop(m_active->id, endUtc))
            return SessionError::StorageFailed;
        m_active.reset();
        emit sessionStopped();
        emit dataChanged();
        return SessionError::None;
    }
    case RecoveryChoice::Discard:
        return deleteSession(m_active->id);
    }
    return SessionError::None;
}

std::optional<Session> TrackerService::start(qint64 taskId) {
    const qint64 now = m_clock.now();
    bool changed = false;

    if (m_active) {
        if (!m_sessions.stop(m_active->id, now))
            return std::nullopt;
        m_active.reset();
        changed = true;
        emit sessionStopped();
    }

    auto session = m_sessions.start(taskId, now, QTimeZone::systemTimeZoneId());
    if (session) {
        m_active = session;
        changed = true;
        emit sessionStarted();
    }
    if (changed)
        emit dataChanged();
    return session;
}

bool TrackerService::stop() {
    if (!m_active || !m_sessions.stop(m_active->id, m_clock.now()))
        return false;
    m_active.reset();
    emit sessionStopped();
    emit dataChanged();
    return true;
}

SessionError TrackerService::addManualEntry(qint64 taskId, qint64 startUtc, qint64 endUtc) {
    if (const SessionError error = validateRange(startUtc, endUtc, std::nullopt);
        error != SessionError::None) {
        return error;
    }
    if (!m_sessions.addClosed(taskId, startUtc, endUtc, QTimeZone::systemTimeZoneId()))
        return SessionError::StorageFailed;
    emit dataChanged();
    return SessionError::None;
}

SessionError TrackerService::editSession(qint64 sessionId, qint64 taskId, qint64 startUtc,
                                         std::optional<qint64> endUtc) {
    const auto current = m_sessions.findById(sessionId);
    if (!current)
        return SessionError::NotFound;

    const bool wasRunning = !current->endUtc.has_value();
    if (wasRunning && endUtc.has_value())
        return SessionError::EndNotAllowed;
    if (!wasRunning && !endUtc.has_value())
        return SessionError::EndNotAllowed;

    if (endUtc) {
        if (const SessionError error = validateRange(startUtc, *endUtc, sessionId);
            error != SessionError::None) {
            return error;
        }
    } else {
        const qint64 now = m_clock.now();
        if (startUtc > now)
            return SessionError::StartInFuture;
        if (m_sessions.hasOverlap(startUtc, now, sessionId))
            return SessionError::Overlap;
    }

    Session updated = *current;
    updated.taskId = taskId;
    updated.startUtc = startUtc;
    updated.endUtc = endUtc;
    if (!m_sessions.update(updated))
        return SessionError::StorageFailed;

    if (m_active && m_active->id == sessionId)
        m_active = updated;
    emit dataChanged();
    return SessionError::None;
}

SessionError TrackerService::deleteSession(qint64 sessionId) {
    if (!m_sessions.findById(sessionId))
        return SessionError::NotFound;
    if (!m_sessions.remove(sessionId))
        return SessionError::StorageFailed;

    if (m_active && m_active->id == sessionId) {
        m_active.reset();
        emit sessionStopped();
    }
    emit dataChanged();
    return SessionError::None;
}

SessionError TrackerService::restoreSession(const Session &session) {
    if (!session.endUtc)
        return SessionError::EndNotAllowed;
    if (const SessionError error = validateRange(session.startUtc, *session.endUtc, std::nullopt);
        error != SessionError::None) {
        return error;
    }
    if (!m_sessions.addClosed(session.taskId, session.startUtc, *session.endUtc, session.tzName))
        return SessionError::StorageFailed;
    emit dataChanged();
    return SessionError::None;
}

std::optional<Session> TrackerService::session(qint64 sessionId) const {
    return m_sessions.findById(sessionId);
}

qint64 TrackerService::nowUtc() const {
    return m_clock.now();
}

QDate TrackerService::todayLocal() const {
    return TimeUtils::localDate(m_clock.now());
}

QList<HistoryDay> TrackerService::history(qint64 rangeStartUtc, qint64 rangeEndUtc) const {
    const qint64 now = m_clock.now();

    QMap<qint64, QString> taskNames;
    for (const Task &task : m_tasks.list(true))
        taskNames.insert(task.id, task.name);

    QMap<QDate, HistoryDay> days;
    const auto totals = m_sessions.dayTotals(rangeStartUtc, rangeEndUtc, now);
    for (auto it = totals.cbegin(); it != totals.cend(); ++it) {
        HistoryDay &day = days[it.key()];
        day.day = it.key();
        day.totalSecs = it.value();
    }

    for (const Session &session : m_sessions.listInRange(rangeStartUtc, rangeEndUtc)) {
        HistoryRow row;
        row.session = session;
        row.taskName = taskNames.value(session.taskId);
        row.running = !session.endUtc.has_value();
        row.durationSecs = qMax<qint64>(0, session.endUtc.value_or(now) - session.startUtc);

        const QDate day = TimeUtils::localDate(qMax(session.startUtc, rangeStartUtc));
        HistoryDay &bucket = days[day];
        bucket.day = day;
        bucket.rows.append(row);
    }

    QList<HistoryDay> result;
    for (auto it = days.cbegin(); it != days.cend(); ++it) {
        HistoryDay day = it.value();
        std::reverse(day.rows.begin(), day.rows.end());
        result.prepend(day);
    }
    return result;
}

SessionError TrackerService::validateRange(qint64 startUtc, qint64 endUtc,
                                           std::optional<qint64> excludeId) const {
    if (endUtc <= startUtc)
        return SessionError::EndNotAfterStart;
    if (m_sessions.hasOverlap(startUtc, endUtc, excludeId))
        return SessionError::Overlap;
    return SessionError::None;
}

} // namespace tt
