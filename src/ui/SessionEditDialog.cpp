#include "ui/SessionEditDialog.h"

#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLocale>
#include <QVBoxLayout>

SessionEditDialog::SessionEditDialog(const QList<tt::Task> &tasks, const SessionDraft &initial,
                                     QWidget *parent)
    : QDialog(parent), m_sessionId(initial.sessionId) {
    setWindowTitle(initial.sessionId ? tr("Edit session") : tr("Add entry"));

    m_task = new QComboBox(this);
    m_task->setObjectName(QStringLiteral("taskCombo"));
    for (const tt::Task &task : tasks)
        m_task->addItem(task.name, task.id);
    if (m_task->findData(initial.taskId) < 0)
        m_task->addItem(tr("(archived task)"), initial.taskId);
    m_task->setCurrentIndex(m_task->findData(initial.taskId));

    const QString format = QLocale().dateTimeFormat(QLocale::ShortFormat);

    m_start = new QDateTimeEdit(QDateTime::fromSecsSinceEpoch(initial.startUtc), this);
    m_start->setObjectName(QStringLiteral("startEdit"));
    m_start->setCalendarPopup(true);
    m_start->setDisplayFormat(format);

    m_end = new QDateTimeEdit(
        QDateTime::fromSecsSinceEpoch(initial.endUtc.value_or(initial.startUtc)), this);
    m_end->setObjectName(QStringLiteral("endEdit"));
    m_end->setCalendarPopup(true);
    m_end->setDisplayFormat(format);
    m_end->setEnabled(initial.endUtc.has_value());

    auto *form = new QFormLayout;
    form->addRow(tr("Task"), m_task);
    form->addRow(tr("Start"), m_start);
    form->addRow(tr("End"), m_end);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

SessionDraft SessionEditDialog::draft() const {
    SessionDraft draft;
    draft.sessionId = m_sessionId;
    draft.taskId = m_task->currentData().toLongLong();
    draft.startUtc = m_start->dateTime().toSecsSinceEpoch();
    if (m_end->isEnabled())
        draft.endUtc = m_end->dateTime().toSecsSinceEpoch();
    return draft;
}
