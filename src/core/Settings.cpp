#include "core/Settings.h"

#include <QtGlobal>

namespace tt {

namespace {

const QString kReminderHoursKey = QStringLiteral("reminder/hours");
const QString kLastSeenKey = QStringLiteral("session/lastSeenUtc");

} // namespace

Settings::Settings(QSettings &store) : m_store(store) {}

int Settings::reminderHours() const {
    return m_store.value(kReminderHoursKey, kDefaultReminderHours).toInt();
}

void Settings::setReminderHours(int hours) {
    m_store.setValue(kReminderHoursKey, qBound(kMinReminderHours, hours, kMaxReminderHours));
}

std::optional<qint64> Settings::lastSeenUtc() const {
    if (!m_store.contains(kLastSeenKey))
        return std::nullopt;
    return m_store.value(kLastSeenKey).toLongLong();
}

void Settings::setLastSeenUtc(qint64 utc) {
    m_store.setValue(kLastSeenKey, utc);
}

} // namespace tt
