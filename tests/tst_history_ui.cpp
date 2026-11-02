#include "core/Database.h"
#include "core/FakeClock.h"
#include "core/SessionRepository.h"
#include "core/TaskRepository.h"
#include "core/TimeUtils.h"
#include "core/TrackerService.h"
#include "ui/HistoryPage.h"
#include "ui/SessionEditDialog.h"

#include <QDate>
#include <QDateEdit>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QTest>
#include <QTimeZone>
#include <QTreeView>
#include <memory>

namespace {

qint64 localEpoch(int year, int month, int day, int hour = 0, int minute = 0) {
    return QDateTime(QDate(year, month, day), QTime(hour, minute), QTimeZone::systemTimeZone())
        .toSecsSinceEpoch();
}

} // namespace

class TstHistoryUi : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<tt::Database> m_db;
    std::unique_ptr<tt::FakeClock> m_clock;
    std::unique_ptr<tt::TrackerService> m_service;
    std::unique_ptr<HistoryPage> m_page;

    template <typename T> T *find(const char *name) const {
        return m_page->findChild<T *>(QLatin1String(name));
    }

    qint64 addClosed(qint64 taskId, qint64 start, qint64 end) {
        tt::SessionRepository sessions(*m_db);
        const qint64 id = sessions.start(taskId, start, QStringLiteral("UTC")).value().id;
        sessions.stop(id, end);
        return id;
    }

    void showPage() {
        m_page = std::make_unique<HistoryPage>(*m_service);
        m_page->resize(900, 600);
        m_page->show();
        QVERIFY(QTest::qWaitForWindowExposed(m_page.get()));
    }

private slots:
    void init() {
        m_db = std::make_unique<tt::Database>(QStringLiteral(":memory:"));
        QVERIFY(m_db->isReady());
        m_clock = std::make_unique<tt::FakeClock>(localEpoch(2027, 1, 15, 12));
        m_service = std::make_unique<tt::TrackerService>(*m_db, *m_clock);
        m_page = std::make_unique<HistoryPage>(*m_service);
    }

    void cleanup() {
        m_page.reset();
        m_service.reset();
        m_clock.reset();
        m_db.reset();
    }

    void dateFiltersDefaultToLastSevenDays() {
        QCOMPARE(find<QDateEdit>("fromDateEdit")->date(), QDate(2027, 1, 9));
        QCOMPARE(find<QDateEdit>("toDateEdit")->date(), QDate(2027, 1, 15));
    }

    void groupsEntriesUnderDayHeadersWithTotals() {
        const qint64 task = tt::TaskRepository(*m_db).create(QStringLiteral("Design"), 0)->id;
        addClosed(task, localEpoch(2027, 1, 14, 9), localEpoch(2027, 1, 14, 9, 30));
        addClosed(task, localEpoch(2027, 1, 15, 9), localEpoch(2027, 1, 15, 10));
        m_clock->setNow(localEpoch(2027, 1, 15, 12));
        showPage();

        QTreeView *view = find<QTreeView>("historyView");
        QCOMPARE(view->model()->rowCount(), 2);
        QCOMPARE(view->model()->index(0, 1).data().toString(), QStringLiteral("1:00:00"));
        QCOMPARE(view->model()->index(1, 1).data().toString(), QStringLiteral("0:30:00"));
        QCOMPARE(view->model()->rowCount(view->model()->index(0, 0)), 1);
    }

    void saveDraftRejectsOverlapAndLeavesSessionUnchanged() {
        const qint64 task = tt::TaskRepository(*m_db).create(QStringLiteral("Design"), 0)->id;
        const qint64 first =
            addClosed(task, localEpoch(2027, 1, 15, 9), localEpoch(2027, 1, 15, 10));
        const qint64 second =
            addClosed(task, localEpoch(2027, 1, 15, 11), localEpoch(2027, 1, 15, 12));

        SessionDraft draft;
        draft.sessionId = second;
        draft.taskId = task;
        draft.startUtc = localEpoch(2027, 1, 15, 9, 30);
        draft.endUtc = localEpoch(2027, 1, 15, 10, 30);
        QCOMPARE(static_cast<int>(m_page->saveDraft(draft)),
                 static_cast<int>(tt::SessionError::Overlap));

        const auto stored = tt::SessionRepository(*m_db).findById(second);
        QCOMPARE(stored->startUtc, localEpoch(2027, 1, 15, 11));
        QCOMPARE(stored->endUtc.value_or(-1), localEpoch(2027, 1, 15, 12));
        QVERIFY(tt::SessionRepository(*m_db).findById(first).has_value());
    }

    void saveDraftRejectsEndBeforeStart() {
        const qint64 task = tt::TaskRepository(*m_db).create(QStringLiteral("Design"), 0)->id;
        const qint64 id = addClosed(task, localEpoch(2027, 1, 15, 9), localEpoch(2027, 1, 15, 10));

        SessionDraft draft;
        draft.sessionId = id;
        draft.taskId = task;
        draft.startUtc = localEpoch(2027, 1, 15, 10);
        draft.endUtc = localEpoch(2027, 1, 15, 9);
        QCOMPARE(static_cast<int>(m_page->saveDraft(draft)),
                 static_cast<int>(tt::SessionError::EndNotAfterStart));
    }

    void saveDraftAppliesValidEdit() {
        const qint64 task = tt::TaskRepository(*m_db).create(QStringLiteral("Design"), 0)->id;
        const qint64 id = addClosed(task, localEpoch(2027, 1, 15, 9), localEpoch(2027, 1, 15, 10));

        SessionDraft draft;
        draft.sessionId = id;
        draft.taskId = task;
        draft.startUtc = localEpoch(2027, 1, 15, 8);
        draft.endUtc = localEpoch(2027, 1, 15, 8, 45);
        QCOMPARE(static_cast<int>(m_page->saveDraft(draft)),
                 static_cast<int>(tt::SessionError::None));
        QCOMPARE(tt::SessionRepository(*m_db).findById(id)->startUtc, localEpoch(2027, 1, 15, 8));
    }

    void saveDraftRejectsSettingEndOnRunningSession() {
        const qint64 task = tt::TaskRepository(*m_db).create(QStringLiteral("Design"), 0)->id;
        const qint64 id = tt::SessionRepository(*m_db)
                              .start(task, localEpoch(2027, 1, 15, 11), QStringLiteral("UTC"))
                              ->id;

        SessionDraft draft;
        draft.sessionId = id;
        draft.taskId = task;
        draft.startUtc = localEpoch(2027, 1, 15, 11);
        draft.endUtc = localEpoch(2027, 1, 15, 11, 30);
        QCOMPARE(static_cast<int>(m_page->saveDraft(draft)),
                 static_cast<int>(tt::SessionError::EndNotAllowed));
    }

    void deleteThenUndoRestoresSelectedSession() {
        const qint64 task = tt::TaskRepository(*m_db).create(QStringLiteral("Design"), 0)->id;
        addClosed(task, localEpoch(2027, 1, 15, 9), localEpoch(2027, 1, 15, 10));
        showPage();

        QTreeView *view = find<QTreeView>("historyView");
        view->expandAll();
        view->setCurrentIndex(view->model()->index(0, 0, view->model()->index(0, 0)));
        QTest::mouseClick(find<QPushButton>("deleteButton"), Qt::LeftButton);
        QVERIFY(tt::SessionRepository(*m_db).listInRange(0, m_clock->now()).isEmpty());

        QTest::mouseClick(find<QPushButton>("undoButton"), Qt::LeftButton);
        QCOMPARE(tt::SessionRepository(*m_db).listInRange(0, m_clock->now()).size(), qsizetype(1));
    }

    void runningSessionAllowsEditButNotDelete() {
        const qint64 task = tt::TaskRepository(*m_db).create(QStringLiteral("Design"), 0)->id;
        m_service->start(task);
        showPage();

        QTreeView *view = find<QTreeView>("historyView");
        view->expandAll();
        view->setCurrentIndex(view->model()->index(0, 0, view->model()->index(0, 0)));
        QVERIFY(find<QPushButton>("editButton")->isEnabled());
        QVERIFY(!find<QPushButton>("deleteButton")->isEnabled());
    }

    void editDialogConvertsLocalTimeToUtc() {
        const qint64 task = tt::TaskRepository(*m_db).create(QStringLiteral("Design"), 0)->id;
        const qint64 id = addClosed(task, localEpoch(2027, 1, 15, 9), localEpoch(2027, 1, 15, 10));

        SessionDraft initial;
        initial.sessionId = id;
        initial.taskId = task;
        initial.startUtc = localEpoch(2027, 1, 15, 9);
        initial.endUtc = localEpoch(2027, 1, 15, 10);
        SessionEditDialog dialog({tt::Task{task, QStringLiteral("Design"), 0, false}}, initial,
                                 nullptr);

        dialog.findChild<QDateTimeEdit *>(QStringLiteral("startEdit"))
            ->setDateTime(QDateTime(QDate(2027, 1, 15), QTime(8, 15), QTimeZone::systemTimeZone()));
        const SessionDraft draft = dialog.draft();
        QCOMPARE(draft.startUtc, localEpoch(2027, 1, 15, 8, 15));
        QCOMPARE(draft.endUtc.value_or(-1), localEpoch(2027, 1, 15, 10));
    }

    void editDialogDisablesEndForRunningSession() {
        SessionDraft initial;
        initial.taskId = 1;
        initial.startUtc = localEpoch(2027, 1, 15, 9);
        SessionEditDialog dialog({tt::Task{1, QStringLiteral("Design"), 0, false}}, initial,
                                 nullptr);
        QVERIFY(!dialog.findChild<QDateTimeEdit *>(QStringLiteral("endEdit"))->isEnabled());
        QVERIFY(!dialog.draft().endUtc.has_value());
    }
};

QTEST_MAIN(TstHistoryUi)
#include "tst_history_ui.moc"
