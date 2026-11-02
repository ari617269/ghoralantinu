#pragma once

#include "core/Database.h"
#include "core/IClock.h"
#include "core/Models.h"
#include "core/SessionRepository.h"
#include "core/TaskRepository.h"

#include <QDate>
#include <QList>
#include <QObject>
#include <optional>

namespace tt {

enum class SessionError {
    None,
    NotFound,
    EndNotAllowed,
    EndNotAfterStart,
    StartInFuture,
    Overlap,
    StorageFailed,
};

enum class RecoveryChoice {
    Keep,
    StopAtLastSeen,
    Discard,
};

struct TaskSummary {
    Task task;
    qint64 todaySecs = 0;
    qint64 totalSecs = 0;
    bool running = false;
};

struct HistoryRow {
    Session session;
    QString taskName;
    qint64 durationSecs = 0;
    bool running = false;
};

struct HistoryDay {
    QDate day;
    qint64 totalSecs = 0;
    QList<HistoryRow> rows;
};

class TrackerService : public QObject {
    Q_OBJECT

public:
    static constexpr qint64 kStaleSessionSecs = 12 * 3600;

    TrackerService(const Database &db, const IClock &clock, QObject *parent = nullptr);

    std::optional<Session> activeSession() const;
    qint64 elapsed() const;
    bool activeRunningLongerThan(qint64 seconds) const;
    bool isActiveStale() const;

    std::optional<Session> start(qint64 taskId);
    SessionError resolveStaleSession(RecoveryChoice choice, qint64 lastSeenUtc);
    bool stop();

    SessionError addManualEntry(qint64 taskId, qint64 startUtc, qint64 endUtc);
    SessionError editSession(qint64 sessionId, qint64 taskId, qint64 startUtc,
                             std::optional<qint64> endUtc);
    SessionError deleteSession(qint64 sessionId);
    SessionError restoreSession(const Session &session);
    std::optional<Session> session(qint64 sessionId) const;

    std::optional<Task> findOrCreateTask(const QString &name);
    QList<TaskSummary> taskSummaries() const;
    std::optional<TaskSummary> taskSummary(qint64 taskId) const;
    QList<Task> recentTasks(int limit) const;
    qint64 todayTotal() const;
    qint64 weekTotal() const;
    QDate todayLocal() const;
    qint64 nowUtc() const;
    QList<HistoryDay> history(qint64 rangeStartUtc, qint64 rangeEndUtc) const;

signals:
    void sessionStarted();
    void sessionStopped();
    void dataChanged();

private:
    SessionError validateRange(qint64 startUtc, qint64 endUtc,
                               std::optional<qint64> excludeId) const;
    TaskSummary summaryFor(const Task &task, qint64 nowUtc) const;
    qint64 rangeTotal(qint64 startUtc, qint64 endUtc, qint64 nowUtc) const;

    const IClock &m_clock;
    SessionRepository m_sessions;
    TaskRepository m_tasks;
    std::optional<Session> m_active;
};

} // namespace tt
