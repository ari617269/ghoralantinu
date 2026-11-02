#include "core/Database.h"

#include <QSqlQuery>
#include <QUuid>

namespace tt {

namespace {

const QList<QStringList> kMigrations = {
    {
        QStringLiteral("CREATE TABLE tasks ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "name TEXT NOT NULL UNIQUE, "
                       "created_utc INTEGER NOT NULL, "
                       "archived INTEGER NOT NULL DEFAULT 0)"),
        QStringLiteral("CREATE TABLE sessions ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "task_id INTEGER NOT NULL REFERENCES tasks(id), "
                       "start_utc INTEGER NOT NULL, "
                       "end_utc INTEGER, "
                       "tz_name TEXT NOT NULL)"),
        QStringLiteral("CREATE INDEX sessions_start_idx ON sessions(start_utc)"),
        QStringLiteral("CREATE INDEX sessions_task_idx ON sessions(task_id)"),
    },
};

}

Database::Database(const QString &path)
    : m_connectionName(QUuid::createUuid().toString(QUuid::WithoutBraces)) {
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    db.setDatabaseName(path);
    m_ready = db.open() && enableForeignKeys() && migrate();
}

Database::~Database() {
    {
        QSqlDatabase db = connection();
        db.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool Database::isReady() const {
    return m_ready;
}

QSqlDatabase Database::connection() const {
    return QSqlDatabase::database(m_connectionName, false);
}

int Database::schemaVersion() const {
    QSqlQuery query(connection());
    if (!query.exec(QStringLiteral("SELECT COALESCE(MAX(version), 0) FROM schema_version")) ||
        !query.next()) {
        return 0;
    }
    return query.value(0).toInt();
}

bool Database::enableForeignKeys() {
    QSqlQuery query(connection());
    return query.exec(QStringLiteral("PRAGMA foreign_keys = ON"));
}

bool Database::migrate() {
    QSqlDatabase db = connection();
    QSqlQuery query(db);
    if (!query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS schema_version ("
                                   "version INTEGER NOT NULL)"))) {
        return false;
    }

    const int current = schemaVersion();
    for (int index = current; index < kMigrations.size(); ++index) {
        if (!db.transaction())
            return false;
        bool ok = true;
        for (const QString &statement : kMigrations.at(index)) {
            QSqlQuery step(db);
            if (!step.exec(statement)) {
                ok = false;
                break;
            }
        }
        if (ok) {
            QSqlQuery record(db);
            ok = record.prepare(QStringLiteral("INSERT INTO schema_version (version) VALUES (?)"));
            if (ok) {
                record.addBindValue(index + 1);
                ok = record.exec();
            }
        }
        if (!ok) {
            db.rollback();
            return false;
        }
        if (!db.commit())
            return false;
    }
    return true;
}

} // namespace tt
