#pragma once

#include <QSqlDatabase>
#include <QString>

namespace tt {

class Database {
public:
    explicit Database(const QString &path);
    ~Database();

    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    bool isReady() const;
    int schemaVersion() const;
    QSqlDatabase connection() const;

private:
    bool enableForeignKeys();
    bool migrate();

    QString m_connectionName;
    bool m_ready = false;
};

} // namespace tt
