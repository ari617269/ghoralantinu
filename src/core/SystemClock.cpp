#include "core/SystemClock.h"

#include <QDateTime>

namespace tt {

qint64 SystemClock::now() const {
    return QDateTime::currentSecsSinceEpoch();
}

} // namespace tt
