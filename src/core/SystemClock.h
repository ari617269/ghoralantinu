#pragma once

#include "core/IClock.h"

namespace tt {

class SystemClock : public IClock {
public:
    qint64 now() const override;
};

} // namespace tt
