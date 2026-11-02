#include "ui/HistoryPage.h"

#include "core/CsvExport.h"
#include "core/TimeUtils.h"

#include <QDateEdit>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

namespace {

QString errorText(tt::SessionError error) {
    switch (error) {
    case tt::SessionError::None:
        return {};
    case tt::SessionError::NotFound:
        return QObject::tr("That session no longer exists.");
    case tt::SessionError::EndNotAllowed:
        return QObject::tr("The end time of a running session can't be changed here.");
    case tt::SessionError::EndNotAfterStart:
        return QObject::tr("The end time must be after the start time.");
    case tt::SessionError::StartInFuture:
        return QObject::tr("The start time can't be in the future.");
    case tt::SessionError::Overlap:
        return QObject::tr("This time overlaps another session.");
    case tt::SessionError::StorageFailed:
        return QObject::tr("The change could not be saved.");
    }
    return {};
}

QString timeRangeText(const tt::HistoryRow &row) {
    const QLocale locale;
    const QString start =
        locale.toString(QDateTime::fromSecsSinceEpoch(row.session.startUtc).toLocalTime().time(),
                        QLocale::ShortFormat);
    if (row.running)
        return QObject::tr("%1 – running").arg(start);
    const QString end =
        locale.toString(QDateTime::fromSecsSinceEpoch(*row.session.endUtc).toLocalTime().time(),
                        QLocale::ShortFormat);
    return QObject::tr("%1 – %2").arg(start, end);
}

} // namespace

HistoryPage::HistoryPage(tt::TrackerService &service, QWidget *parent)
    : QWidget(parent), m_service(service) {
    const QDate today = m_service.todayLocal();

    m_from = new QDateEdit(today.addDays(-6), this);
    m_from->setObjectName(QStringLiteral("fromDateEdit"));
    m_from->setCalendarPopup(true);
    m_to = new QDateEdit(today, this);
    m_to->setObjectName(QStringLiteral("toDateEdit"));
    m_to->setCalendarPopup(true);

    m_addButton = new QPushButton(tr("Add entry…"), this);
    m_addButton->setObjectName(QStringLiteral("addButton"));
    m_editButton = new QPushButton(tr("Edit…"), this);
    m_editButton->setObjectName(QStringLiteral("editButton"));
    m_deleteButton = new QPushButton(tr("Delete"), this);
    m_deleteButton->setObjectName(QStringLiteral("deleteButton"));
    m_undoButton = new QPushButton(tr("Undo delete"), this);
    m_undoButton->setObjectName(QStringLiteral("undoButton"));
    m_exportButton = new QPushButton(tr("Export CSV…"), this);
    m_exportButton->setObjectName(QStringLiteral("exportButton"));

    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels({tr("Time"), tr("Duration"), tr("Task")});

    m_view = new QTreeView(this);
    m_view->setObjectName(QStringLiteral("historyView"));
    m_view->setModel(m_model);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    auto *filter = new QHBoxLayout;
    filter->addWidget(new QLabel(tr("From"), this));
    filter->addWidget(m_from);
    filter->addWidget(new QLabel(tr("To"), this));
    filter->addWidget(m_to);
    filter->addStretch(1);
    filter->addWidget(m_addButton);
    filter->addWidget(m_editButton);
    filter->addWidget(m_deleteButton);
    filter->addWidget(m_undoButton);
    filter->addWidget(m_exportButton);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(filter);
    layout->addWidget(m_view, 1);

    connect(m_from, &QDateEdit::dateChanged, this, &HistoryPage::reload);
    connect(m_to, &QDateEdit::dateChanged, this, &HistoryPage::reload);
    connect(&m_service, &tt::TrackerService::dataChanged, this, &HistoryPage::reload);
    connect(m_addButton, &QPushButton::clicked, this, &HistoryPage::addEntry);
    connect(m_editButton, &QPushButton::clicked, this, &HistoryPage::editSelected);
    connect(m_deleteButton, &QPushButton::clicked, this, &HistoryPage::deleteSelected);
    connect(m_undoButton, &QPushButton::clicked, this, &HistoryPage::undoDelete);
    connect(m_exportButton, &QPushButton::clicked, this, &HistoryPage::exportToFile);
    connect(m_view->selectionModel(), &QItemSelectionModel::currentChanged, this,
            &HistoryPage::updateButtons);
    connect(m_view, &QTreeView::doubleClicked, this, &HistoryPage::editSelected);

    auto *undoShortcut = new QShortcut(QKeySequence::Undo, this);
    connect(undoShortcut, &QShortcut::activated, this, &HistoryPage::undoDelete);

    reload();
}

tt::SessionError HistoryPage::saveDraft(const SessionDraft &draft) {
    if (draft.sessionId)
        return m_service.editSession(*draft.sessionId, draft.taskId, draft.startUtc, draft.endUtc);
    if (!draft.endUtc)
        return tt::SessionError::EndNotAllowed;
    return m_service.addManualEntry(draft.taskId, draft.startUtc, *draft.endUtc);
}

bool HistoryPage::exportCsv(const QString &path) {
    const auto [rangeStart, rangeEnd] = currentRange();
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    const QByteArray csv = tt::historyToCsv(m_service.history(rangeStart, rangeEnd)).toUtf8();
    return file.write(csv) == csv.size();
}

std::pair<qint64, qint64> HistoryPage::currentRange() const {
    QDate to = m_to->date();
    if (to < m_from->date())
        to = m_from->date();
    return {tt::TimeUtils::localMidnightUtc(m_from->date()),
            tt::TimeUtils::localMidnightUtc(to.addDays(1))};
}

void HistoryPage::reload() {
    const auto [rangeStart, rangeEnd] = currentRange();

    m_model->removeRows(0, m_model->rowCount());
    for (const tt::HistoryDay &day : m_service.history(rangeStart, rangeEnd)) {
        auto *dayTitle = new QStandardItem(QLocale().toString(day.day, QLocale::LongFormat));
        QFont bold = dayTitle->font();
        bold.setBold(true);
        dayTitle->setFont(bold);
        auto *dayTotal = new QStandardItem(tt::TimeUtils::formatDuration(day.totalSecs));
        auto *dayTask = new QStandardItem;
        const QList<QStandardItem *> dayItems{dayTitle, dayTotal, dayTask};
        for (QStandardItem *item : dayItems)
            item->setEditable(false);
        m_model->appendRow(dayItems);

        for (const tt::HistoryRow &row : day.rows) {
            auto *time = new QStandardItem(timeRangeText(row));
            time->setData(row.session.id, Qt::UserRole);
            auto *duration = new QStandardItem(tt::TimeUtils::formatDuration(row.durationSecs));
            auto *task = new QStandardItem(row.taskName);
            const QList<QStandardItem *> rowItems{time, duration, task};
            for (QStandardItem *item : rowItems)
                item->setEditable(false);
            dayTitle->appendRow(rowItems);
        }
    }

    m_view->expandAll();
    updateButtons();
}

void HistoryPage::updateButtons() {
    const auto id = selectedSessionId();
    const auto session = id ? m_service.session(*id) : std::nullopt;
    m_editButton->setEnabled(session.has_value());
    m_deleteButton->setEnabled(session.has_value() && session->endUtc.has_value());
    m_undoButton->setEnabled(m_undoSession.has_value());
}

std::optional<qint64> HistoryPage::selectedSessionId() const {
    const QModelIndex current = m_view->selectionModel()->currentIndex();
    if (!current.isValid())
        return std::nullopt;
    const QVariant id = current.siblingAtColumn(0).data(Qt::UserRole);
    if (!id.isValid())
        return std::nullopt;
    return id.toLongLong();
}

void HistoryPage::addEntry() {
    const QList<tt::TaskSummary> summaries = m_service.taskSummaries();
    if (summaries.isEmpty()) {
        QMessageBox::information(this, tr("Add entry"),
                                 tr("Create a task on the Tracker screen first."));
        return;
    }

    const qint64 now = m_service.nowUtc();
    SessionDraft draft;
    draft.taskId = summaries.first().task.id;
    draft.startUtc = now - 3600;
    draft.endUtc = now;
    runDialog(draft);
}

void HistoryPage::editSelected() {
    const auto id = selectedSessionId();
    const auto session = id ? m_service.session(*id) : std::nullopt;
    if (!session)
        return;

    SessionDraft draft;
    draft.sessionId = session->id;
    draft.taskId = session->taskId;
    draft.startUtc = session->startUtc;
    draft.endUtc = session->endUtc;
    runDialog(draft);
}

void HistoryPage::deleteSelected() {
    const auto id = selectedSessionId();
    const auto session = id ? m_service.session(*id) : std::nullopt;
    if (!session || !session->endUtc)
        return;

    if (m_service.deleteSession(session->id) == tt::SessionError::None)
        m_undoSession = session;
    updateButtons();
}

void HistoryPage::undoDelete() {
    if (!m_undoSession)
        return;

    const tt::SessionError error = m_service.restoreSession(*m_undoSession);
    if (error == tt::SessionError::None) {
        m_undoSession.reset();
    } else {
        QMessageBox::warning(this, tr("Undo"), errorText(error));
    }
    updateButtons();
}

void HistoryPage::exportToFile() {
    const QString path =
        QFileDialog::getSaveFileName(this, tr("Export CSV"), QString(), tr("CSV files (*.csv)"));
    if (path.isEmpty())
        return;
    if (!exportCsv(path))
        QMessageBox::warning(this, tr("Export CSV"), tr("The file could not be written."));
}

void HistoryPage::runDialog(const SessionDraft &initial) {
    QList<tt::Task> tasks;
    for (const tt::TaskSummary &summary : m_service.taskSummaries())
        tasks.append(summary.task);

    SessionEditDialog dialog(tasks, initial, this);
    while (dialog.exec() == QDialog::Accepted) {
        const tt::SessionError error = saveDraft(dialog.draft());
        if (error == tt::SessionError::None)
            return;
        QMessageBox::warning(this, tr("Invalid entry"), errorText(error));
    }
}
