#include "core/Database.h"
#include "core/FakeClock.h"
#include "core/Settings.h"
#include "core/TaskRepository.h"
#include "core/TrackerService.h"
#include "ui/MainWindow.h"
#include "ui/StatusItem.h"
#include "ui/TrayController.h"

#include <QAction>
#include <QMenu>
#include <QSettings>
#include <QSignalSpy>
#include <QStackedWidget>
#include <QTemporaryDir>
#include <QTest>
#include <memory>

class FakeStatusItem : public StatusItem {
    Q_OBJECT

public:
    void setTitle(const QString &title) override { m_title = title; }
    QString title() const { return m_title; }

private:
    QString m_title;
};

class TstTrayUi : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<tt::Database> m_db;
    std::unique_ptr<tt::FakeClock> m_clock;
    std::unique_ptr<tt::TrackerService> m_service;
    std::unique_ptr<TrayController> m_tray;
    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<QSettings> m_store;
    std::unique_ptr<tt::Settings> m_settings;
    FakeStatusItem *m_titleItem = nullptr;

    QAction *action(const char *name) const {
        return m_tray->menu()->findChild<QAction *>(QLatin1String(name));
    }

    qint64 makeTask(const QString &name) {
        return tt::TaskRepository(*m_db).create(name, 0).value().id;
    }

private slots:
    void init() {
        m_db = std::make_unique<tt::Database>(QStringLiteral(":memory:"));
        QVERIFY(m_db->isReady());
        m_clock = std::make_unique<tt::FakeClock>(1800000000);
        m_service = std::make_unique<tt::TrackerService>(*m_db, *m_clock);
        auto titleItem = std::make_unique<FakeStatusItem>();
        m_titleItem = titleItem.get();
        m_tray = std::make_unique<TrayController>(*m_service, std::move(titleItem));
        m_dir = std::make_unique<QTemporaryDir>();
        m_store = std::make_unique<QSettings>(m_dir->filePath(QStringLiteral("test.ini")),
                                              QSettings::IniFormat);
        m_settings = std::make_unique<tt::Settings>(*m_store);
    }

    void cleanup() {
        m_settings.reset();
        m_store.reset();
        m_dir.reset();
        m_tray.reset();
        m_titleItem = nullptr;
        m_service.reset();
        m_clock.reset();
        m_db.reset();
    }

    void idleMenuShowsNotRunningAndDisablesStop() {
        QCOMPARE(action("trayStatus")->text(), QStringLiteral("Not running"));
        QVERIFY(!action("trayStop")->isEnabled());
    }

    void startingTaskUpdatesStatusAndEnablesStop() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        m_service->start(task);

        QCOMPARE(action("trayStatus")->text(), QStringLiteral("Running: Design 0:00:00"));
        QVERIFY(action("trayStop")->isEnabled());
    }

    void stopActionStopsRunningTask() {
        m_service->start(makeTask(QStringLiteral("Design")));
        action("trayStop")->trigger();

        QVERIFY(!m_service->activeSession().has_value());
        QCOMPARE(action("trayStatus")->text(), QStringLiteral("Not running"));
    }

    void recentTaskActionStartsThatTask() {
        const qint64 alpha = makeTask(QStringLiteral("Alpha"));
        const qint64 beta = makeTask(QStringLiteral("Beta"));
        m_service->start(alpha);
        m_clock->advance(10);
        m_service->start(beta);

        QCOMPARE(action("trayRecent0")->text(), QStringLiteral("Beta"));
        QVERIFY(action("trayRecent0")->isVisible());
        QCOMPARE(action("trayRecent1")->text(), QStringLiteral("Alpha"));
        QVERIFY(!action("trayRecent2")->isVisible());

        action("trayRecent1")->trigger();
        QCOMPARE(m_service->activeSession()->taskId, alpha);
    }

    void openActionsEmitRequests() {
        QSignalSpy tracker(m_tray.get(), &TrayController::showTrackerRequested);
        QSignalSpy history(m_tray.get(), &TrayController::showHistoryRequested);
        action("trayOpenTracker")->trigger();
        action("trayOpenHistory")->trigger();
        QCOMPARE(tracker.count(), 1);
        QCOMPARE(history.count(), 1);
    }

    void quitActionEmitsQuitRequested() {
        QSignalSpy quit(m_tray.get(), &TrayController::quitRequested);
        action("trayQuit")->trigger();
        QCOMPARE(quit.count(), 1);
    }

    void iconChangesWhenTimerStarts() {
        const qint64 before = m_tray->icon().cacheKey();
        m_service->start(makeTask(QStringLiteral("Design")));
        QVERIFY(m_tray->icon().cacheKey() != before);
    }

    void windowCloseHidesWhenTrayIsAvailable() {
        MainWindow window(*m_service, *m_settings, true);
        window.show();
        QVERIFY(window.isVisible());
        window.close();
        QVERIFY(!window.isVisible());
    }

    void windowCloseQuitsWhenNoTray() {
        MainWindow window(*m_service, *m_settings, false);
        QSignalSpy quit(&window, &MainWindow::quitRequested);
        window.show();
        window.close();
        QCOMPARE(quit.count(), 1);
    }

    void windowPagesSwitchFromTrayRequests() {
        MainWindow window(*m_service, *m_settings, true);
        auto *stack = window.findChild<QStackedWidget *>(QStringLiteral("pageStack"));
        QVERIFY(stack);

        window.showHistory();
        QCOMPARE(stack->currentIndex(), 1);
        window.showTracker();
        QCOMPARE(stack->currentIndex(), 0);
    }
    void statusTitleShowsElapsedTimeWhileRunning() {
        m_service->start(makeTask(QStringLiteral("Design")));
        QCOMPARE(m_titleItem->title(), QStringLiteral("0:00:00"));

        m_clock->advance(3661);
        QVERIFY(QTest::qWaitFor(
            [this] { return m_titleItem->title() == QStringLiteral("1:01:01"); }, 3000));
    }

    void statusTitleIsEmptyWhenIdle() {
        m_service->start(makeTask(QStringLiteral("Design")));
        m_service->stop();
        QVERIFY(m_titleItem->title().isEmpty());
    }

    void clickingStatusTitleOpensTracker() {
        QSignalSpy tracker(m_tray.get(), &TrayController::showTrackerRequested);
        m_titleItem->clicked();
        QCOMPARE(tracker.count(), 1);
    }
};

QTEST_MAIN(TstTrayUi)
#include "tst_tray_ui.moc"
