#pragma once

#include "core/Database.h"
#include "core/Models.h"

#include <QList>
#include <QSqlDatabase>
#include <optional>

namespace tt {

class TaskRepository {
public:
    explicit TaskRepository(const Database &db);

    std::optional<Task> create(const QString &name, qint64 createdUtc);
    std::optional<Task> findByName(const QString &name) const;
    QList<Task> list(bool includeArchived = false) const;
    bool rename(qint64 id, const QString &name);
    bool setArchived(qint64 id, bool archived);

private:
    QSqlDatabase m_db;
};

} // namespace tt
