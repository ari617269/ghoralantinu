#include "ui/SessionMonitor.h"

#include "core/Settings.h"
#include "core/TrackerService.h"

namespace {

constexpr int kTickIntervalMs = 30000;

}

SessionMonitor::SessionMonitor(tt::TrackerService &service, tt::Settings &settings, QObject *parent)
    : QObject(parent), m_service(service), m_settings(settings) {
    connect(&m_timer, &QTimer::timeout, this, &SessionMonitor::tick);
    m_timer.start(kTickIntervalMs);
}

void SessionMonitor::tick() {
    m_settings.setLastSeenUtc(m_service.nowUtc());

    const auto active = m_service.activeSession();
    if (!active || m_remindedSessionId == active->id)
        return;
    if (m_service.elapsed() < m_settings.reminderHours() * 3600LL)
        return;

    m_remindedSessionId = active->id;
    const auto summary = m_service.taskSummary(active->taskId);
    emit reminderDue(summary ? summary->task.name : QString(), m_service.elapsed());
}
