#pragma once

#include "core/Models.h"

#include <QDialog>
#include <QList>
#include <optional>

class QComboBox;
class QDateTimeEdit;

struct SessionDraft {
    std::optional<qint64> sessionId;
    qint64 taskId = 0;
    qint64 startUtc = 0;
    std::optional<qint64> endUtc;
};

class SessionEditDialog : public QDialog {
    Q_OBJECT

public:
    SessionEditDialog(const QList<tt::Task> &tasks, const SessionDraft &initial,
                      QWidget *parent = nullptr);

    SessionDraft draft() const;

private:
    std::optional<qint64> m_sessionId;
    QComboBox *m_task = nullptr;
    QDateTimeEdit *m_start = nullptr;
    QDateTimeEdit *m_end = nullptr;
};
