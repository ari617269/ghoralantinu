#pragma once

#include <QObject>
#include <QTimer>
#include <optional>

namespace tt {
class Settings;
class TrackerService;
} // namespace tt

class SessionMonitor : public QObject {
    Q_OBJECT

public:
    SessionMonitor(tt::TrackerService &service, tt::Settings &settings, QObject *parent = nullptr);

    void tick();

signals:
    void reminderDue(const QString &taskName, qint64 elapsedSecs);

private:
    tt::TrackerService &m_service;
    tt::Settings &m_settings;
    QTimer m_timer;
    std::optional<qint64> m_remindedSessionId;
};
