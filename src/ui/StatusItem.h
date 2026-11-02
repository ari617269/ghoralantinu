#pragma once

#include <QObject>
#include <QString>
#include <memory>

class StatusItem : public QObject {
    Q_OBJECT

public:
    explicit StatusItem(QObject *parent = nullptr) : QObject(parent) {}

    virtual void setTitle(const QString &title) = 0;

signals:
    void clicked();
};

// Returns nullptr on platforms without a menu bar text item.
std::unique_ptr<StatusItem> createStatusItem();
