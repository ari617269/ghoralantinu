#pragma once

#include "core/IClock.h"

namespace tt {

class FakeClock : public IClock {
public:
    explicit FakeClock(qint64 startUtc = 0);

    qint64 now() const override;
    void setNow(qint64 utc);
    void advance(qint64 seconds);

private:
    qint64 m_now;
};

} // namespace tt
