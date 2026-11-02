#include "core/FakeClock.h"

namespace tt {

FakeClock::FakeClock(qint64 startUtc) : m_now(startUtc) {}

qint64 FakeClock::now() const {
    return m_now;
}

void FakeClock::setNow(qint64 utc) {
    m_now = utc;
}

void FakeClock::advance(qint64 seconds) {
    m_now += seconds;
}

} // namespace tt
