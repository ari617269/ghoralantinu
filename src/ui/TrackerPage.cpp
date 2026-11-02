#include "ui/TrackerPage.h"

#include "core/TimeUtils.h"

#include <QFontDatabase>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QShortcut>
#include <QTableView>
#include <QVBoxLayout>

TrackerPage::TrackerPage(tt::TrackerService &service, QWidget *parent)
    : QWidget(parent), m_service(service), m_model(new TaskTableModel(this)) {
    const QFont monospace = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    m_activeName = new QLabel(this);
    m_activeName->setObjectName(QStringLiteral("activeNameLabel"));

    m_elapsed = new QLabel(this);
    m_elapsed->setObjectName(QStringLiteral("elapsedLabel"));
    m_elapsed->setFont(monospace);

    m_stop = new QPushButton(tr("Stop"), this);
    m_stop->setObjectName(QStringLiteral("stopButton"));

    QHBoxLayout *header = new QHBoxLayout;
    header->addWidget(m_activeName, 1);
    header->addWidget(m_elapsed);
    header->addWidget(m_stop);

    m_newTask = new QLineEdit(this);
    m_newTask->setObjectName(QStringLiteral("newTaskEdit"));
    m_newTask->setPlaceholderText(tr("New task…"));

    m_view = new QTableView(this);
    m_view->setObjectName(QStringLiteral("taskView"));
    m_view->setModel(m_model);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->verticalHeader()->hide();
    m_view->horizontalHeader()->setSectionResizeMode(TaskTableModel::NameColumn,
                                                     QHeaderView::Stretch);
    m_view->horizontalHeader()->setSectionResizeMode(TaskTableModel::ToggleColumn,
                                                     QHeaderView::ResizeToContents);

    m_today = new QLabel(this);
    m_today->setObjectName(QStringLiteral("todayTotalLabel"));
    m_week = new QLabel(this);
    m_week->setObjectName(QStringLiteral("weekTotalLabel"));

    QHBoxLayout *footer = new QHBoxLayout;
    footer->addWidget(m_today);
    footer->addStretch(1);
    footer->addWidget(m_week);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(header);
    layout->addWidget(m_newTask);
    layout->addWidget(m_view, 1);
    layout->addLayout(footer);

    connect(m_newTask, &QLineEdit::returnPressed, this, &TrackerPage::startFromNewTaskField);
    connect(m_stop, &QPushButton::clicked, this, &TrackerPage::stopActive);
    connect(m_view, &QTableView::clicked, this, [this](const QModelIndex &index) {
        if (index.column() == TaskTableModel::ToggleColumn)
            toggleRow(index.row());
    });

    auto *spaceShortcut = new QShortcut(QKeySequence(Qt::Key_Space), m_view);
    spaceShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(spaceShortcut, &QShortcut::activated, this,
            [this] { toggleRow(m_view->currentIndex().row()); });

    auto *newShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this);
    connect(newShortcut, &QShortcut::activated, m_newTask, qOverload<>(&QWidget::setFocus));

    auto *stopShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Period), this);
    connect(stopShortcut, &QShortcut::activated, this, &TrackerPage::stopActive);

    connect(&m_service, &tt::TrackerService::dataChanged, this, &TrackerPage::reload);
    connect(&m_timer, &QTimer::timeout, this, &TrackerPage::refreshClock);
    m_timer.start(1000);

    reload();
}

void TrackerPage::reload() {
    m_model->setSummaries(m_service.taskSummaries());
    updateHeader();
    updateFooter();
}

void TrackerPage::refreshClock() {
    const auto active = m_service.activeSession();
    if (active) {
        const int row = m_model->rowForTask(active->taskId);
        const auto summary = m_service.taskSummary(active->taskId);
        if (row >= 0 && summary)
            m_model->updateSummary(row, *summary);
    }
    updateHeader();
    updateFooter();
}

void TrackerPage::updateHeader() {
    const auto active = m_service.activeSession();
    if (active) {
        const auto summary = m_service.taskSummary(active->taskId);
        m_activeName->setText(summary ? summary->task.name : QString());
        m_elapsed->setText(tt::TimeUtils::formatDuration(m_service.elapsed()));
    } else {
        m_activeName->setText(tr("Not running"));
        m_elapsed->setText(tt::TimeUtils::formatDuration(0));
    }
    m_stop->setEnabled(active.has_value());
}

void TrackerPage::updateFooter() {
    m_today->setText(tr("Today %1").arg(tt::TimeUtils::formatDuration(m_service.todayTotal())));
    m_week->setText(tr("This week %1").arg(tt::TimeUtils::formatDuration(m_service.weekTotal())));
}

void TrackerPage::toggleRow(int row) {
    const auto summary = m_model->summaryAt(row);
    if (!summary)
        return;

    const auto active = m_service.activeSession();
    if (active && active->taskId == summary->task.id)
        m_service.stop();
    else
        m_service.start(summary->task.id);
}

void TrackerPage::startFromNewTaskField() {
    const QString name = m_newTask->text().trimmed();
    if (name.isEmpty())
        return;

    const auto task = m_service.findOrCreateTask(name);
    if (!task)
        return;
    m_service.start(task->id);
    m_newTask->clear();
}

void TrackerPage::stopActive() {
    m_service.stop();
}
