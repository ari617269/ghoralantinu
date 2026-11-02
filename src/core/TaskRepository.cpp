#include "core/TaskRepository.h"

#include <QSqlQuery>

namespace tt {

namespace {

Task taskFromQuery(const QSqlQuery &query) {
    Task task;
    task.id = query.value(0).toLongLong();
    task.name = query.value(1).toString();
    task.createdUtc = query.value(2).toLongLong();
    task.archived = query.value(3).toBool();
    return task;
}

} // namespace

TaskRepository::TaskRepository(const Database &db) : m_db(db.connection()) {}

std::optional<Task> TaskRepository::create(const QString &name, qint64 createdUtc) {
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("INSERT INTO tasks (name, created_utc) VALUES (?, ?)"));
    query.addBindValue(name);
    query.addBindValue(createdUtc);
    if (!query.exec())
        return std::nullopt;

    Task task;
    task.id = query.lastInsertId().toLongLong();
    task.name = name;
    task.createdUtc = createdUtc;
    return task;
}

std::optional<Task> TaskRepository::findByName(const QString &name) const {
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT id, name, created_utc, archived FROM tasks "
                                 "WHERE name = ?"));
    query.addBindValue(name);
    if (!query.exec() || !query.next())
        return std::nullopt;
    return taskFromQuery(query);
}

QList<Task> TaskRepository::list(bool includeArchived) const {
    QString sql = QStringLiteral("SELECT id, name, created_utc, archived FROM tasks");
    if (!includeArchived)
        sql += QStringLiteral(" WHERE archived = 0");
    sql += QStringLiteral(" ORDER BY name");

    QList<Task> tasks;
    QSqlQuery query(m_db);
    if (!query.exec(sql))
        return tasks;
    while (query.next())
        tasks.append(taskFromQuery(query));
    return tasks;
}

bool TaskRepository::rename(qint64 id, const QString &name) {
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE tasks SET name = ? WHERE id = ?"));
    query.addBindValue(name);
    query.addBindValue(id);
    return query.exec() && query.numRowsAffected() == 1;
}

bool TaskRepository::setArchived(qint64 id, bool archived) {
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE tasks SET archived = ? WHERE id = ?"));
    query.addBindValue(archived ? 1 : 0);
    query.addBindValue(id);
    return query.exec() && query.numRowsAffected() == 1;
}

} // namespace tt
