#include "core/Database.h"
#include "core/Settings.h"
#include "core/SystemClock.h"
#include "core/TrackerService.h"
#include "ui/MainWindow.h"
#include "ui/RecoveryDialog.h"
#include "ui/SessionMonitor.h"
#include "ui/StatusItem.h"
#include "ui/TrayController.h"

#include <QApplication>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <memory>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("TimeTracker"));
    app.setApplicationName(QStringLiteral("TimeTracker"));
    app.setQuitOnLastWindowClosed(false);

    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    tt::Database db(QDir(dataDir).filePath(QStringLiteral("timetracker.db")));
    if (!db.isReady())
        return 1;

    QSettings store;
    tt::Settings settings(store);
    tt::SystemClock clock;
    tt::TrackerService service(db, clock);

    if (service.isActiveStale()) {
        const auto active = service.activeSession();
        const auto summary = service.taskSummary(active->taskId);
        RecoveryDialog dialog(summary ? summary->task.name : QString(), service.elapsed());
        dialog.exec();
        service.resolveStaleSession(dialog.choice(),
                                    settings.lastSeenUtc().value_or(active->startUtc));
    }

    // The monitor's first tick overwrites the last-seen time, so it must start after recovery.
    SessionMonitor monitor(service, settings);
    monitor.tick();

    const bool trayAvailable = QSystemTrayIcon::isSystemTrayAvailable();
    MainWindow window(service, settings, trayAvailable);
    QObject::connect(&window, &MainWindow::quitRequested, &app, &QCoreApplication::quit);
    QObject::connect(&monitor, &SessionMonitor::reminderDue, &window,
                     &MainWindow::showLongRunningReminder);

    std::unique_ptr<TrayController> tray;
    if (trayAvailable) {
        tray = std::make_unique<TrayController>(service, createStatusItem());
        QObject::connect(tray.get(), &TrayController::showTrackerRequested, &window,
                         &MainWindow::showTracker);
        QObject::connect(tray.get(), &TrayController::showHistoryRequested, &window,
                         &MainWindow::showHistory);
        QObject::connect(tray.get(), &TrayController::quitRequested, &app, &QCoreApplication::quit);
        tray->show();
    }

    window.show();
    return app.exec();
}
