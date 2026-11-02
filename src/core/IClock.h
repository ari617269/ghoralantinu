#pragma once

#include <QtGlobal>

namespace tt {

class IClock {
public:
    virtual ~IClock() = default;
    virtual qint64 now() const = 0;
};

} // namespace tt
