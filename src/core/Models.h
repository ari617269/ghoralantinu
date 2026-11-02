#pragma once

#include <QString>
#include <QtGlobal>
#include <optional>

namespace tt {

struct Task {
    qint64 id = 0;
    QString name;
    qint64 createdUtc = 0;
    bool archived = false;
};

struct Session {
    qint64 id = 0;
    qint64 taskId = 0;
    qint64 startUtc = 0;
    std::optional<qint64> endUtc;
    QString tzName;
};

} // namespace tt
