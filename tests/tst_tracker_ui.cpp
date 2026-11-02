#include "core/Database.h"
#include "core/FakeClock.h"
#include "core/TrackerService.h"
#include "ui/TrackerPage.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QTest>
#include <memory>

class TstTrackerUi : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<tt::Database> m_db;
    std::unique_ptr<tt::FakeClock> m_clock;
    std::unique_ptr<tt::TrackerService> m_service;
    std::unique_ptr<TrackerPage> m_page;

    QLineEdit *newTaskEdit() const {
        return m_page->findChild<QLineEdit *>(QStringLiteral("newTaskEdit"));
    }
    QTableView *taskView() const {
        return m_page->findChild<QTableView *>(QStringLiteral("taskView"));
    }
    QLabel *label(const char *name) const {
        return m_page->findChild<QLabel *>(QLatin1String(name));
    }

    void showPage() {
        m_page->resize(800, 600);
        m_page->show();
        QVERIFY(QTest::qWaitForWindowExposed(m_page.get()));
    }

private slots:
    void init() {
        m_db = std::make_unique<tt::Database>(QStringLiteral(":memory:"));
        QVERIFY(m_db->isReady());
        m_clock = std::make_unique<tt::FakeClock>(1800000000);
        m_service = std::make_unique<tt::TrackerService>(*m_db, *m_clock);
        m_page = std::make_unique<TrackerPage>(*m_service);
    }

    void cleanup() {
        m_page.reset();
        m_service.reset();
        m_clock.reset();
        m_db.reset();
    }

    void newTaskFieldCreatesAndStartsTask() {
        showPage();
        QTest::keyClicks(newTaskEdit(), QStringLiteral("Design"));
        QTest::keyClick(newTaskEdit(), Qt::Key_Return);

        const auto active = m_service->activeSession();
        QVERIFY(active.has_value());
        QCOMPARE(m_service->taskSummary(active->taskId)->task.name, QStringLiteral("Design"));
        QCOMPARE(taskView()->model()->rowCount(), 1);
        QCOMPARE(newTaskEdit()->text(), QString());
    }

    void clickingToggleCellStartsAndStopsTask() {
        const qint64 task = m_service->findOrCreateTask(QStringLiteral("Review"))->id;
        showPage();
        QTableView *view = taskView();
        const QRect cell = view->visualRect(view->model()->index(0, TaskTableModel::ToggleColumn));

        QTest::mouseClick(view->viewport(), Qt::LeftButton, {}, cell.center());
        QVERIFY(m_service->activeSession().has_value());
        QCOMPARE(m_service->activeSession()->taskId, task);

        QTest::mouseClick(view->viewport(), Qt::LeftButton, {}, cell.center());
        QVERIFY(!m_service->activeSession().has_value());
    }

    void spaceTogglesSelectedRow() {
        m_service->findOrCreateTask(QStringLiteral("Review"));
        showPage();
        taskView()->setFocus();
        taskView()->selectRow(0);

        QTest::keyClick(taskView(), Qt::Key_Space);
        QVERIFY(m_service->activeSession().has_value());

        QTest::keyClick(taskView(), Qt::Key_Space);
        QVERIFY(!m_service->activeSession().has_value());
    }

    void ctrlNFocusesNewTaskField() {
        showPage();
        taskView()->setFocus();
        QTest::keyClick(m_page.get(), Qt::Key_N, Qt::ControlModifier);
        QVERIFY(QTest::qWaitFor([this] { return newTaskEdit()->hasFocus(); }));
    }

    void ctrlPeriodStopsRunningTask() {
        const qint64 task = m_service->findOrCreateTask(QStringLiteral("Review"))->id;
        m_service->start(task);
        showPage();

        QTest::keyClick(m_page.get(), Qt::Key_Period, Qt::ControlModifier);
        QVERIFY(!m_service->activeSession().has_value());
    }

    void elapsedLabelFollowsClockOnTimerTick() {
        const qint64 task = m_service->findOrCreateTask(QStringLiteral("Review"))->id;
        m_service->start(task);
        m_clock->advance(3661);
        showPage();

        QVERIFY(QTest::qWaitFor(
            [this] { return label("elapsedLabel")->text() == QStringLiteral("1:01:01"); }));
    }

    void footerShowsTodayTotalAfterManualEntry() {
        const qint64 task = m_service->findOrCreateTask(QStringLiteral("Review"))->id;
        const qint64 now = m_clock->now();
        QVERIFY(m_service->addManualEntry(task, now - 300, now) == tt::SessionError::None);
        showPage();
        QVERIFY(label("todayTotalLabel")->text().contains(QStringLiteral("0:05:00")));
    }
};

QTEST_MAIN(TstTrackerUi)
#include "tst_tracker_ui.moc"
