#pragma once

#include <QSettings>
#include <QtGlobal>
#include <optional>

namespace tt {

class Settings {
public:
    static constexpr int kDefaultReminderHours = 8;
    static constexpr int kMinReminderHours = 1;
    static constexpr int kMaxReminderHours = 24;

    explicit Settings(QSettings &store);

    int reminderHours() const;
    void setReminderHours(int hours);

    std::optional<qint64> lastSeenUtc() const;
    void setLastSeenUtc(qint64 utc);

private:
    QSettings &m_store;
};

} // namespace tt
