#include "core/Database.h"
#include "core/FakeClock.h"
#include "core/Settings.h"
#include "core/TaskRepository.h"
#include "core/TrackerService.h"
#include "ui/HistoryPage.h"
#include "ui/RecoveryDialog.h"
#include "ui/SessionMonitor.h"
#include "ui/SettingsDialog.h"

#include <QDateTime>
#include <QFile>
#include <QPushButton>
#include <QSignalSpy>
#include <QSpinBox>
#include <QTemporaryDir>
#include <QTest>
#include <QTimeZone>
#include <memory>

namespace {

qint64 localEpoch(int year, int month, int day, int hour = 0) {
    return QDateTime(QDate(year, month, day), QTime(hour, 0), QTimeZone::systemTimeZone())
        .toSecsSinceEpoch();
}

} // namespace

class TstRobustnessUi : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<tt::Database> m_db;
    std::unique_ptr<tt::FakeClock> m_clock;
    std::unique_ptr<tt::TrackerService> m_service;
    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<QSettings> m_store;
    std::unique_ptr<tt::Settings> m_settings;

private slots:
    void init() {
        m_db = std::make_unique<tt::Database>(QStringLiteral(":memory:"));
        QVERIFY(m_db->isReady());
        m_clock = std::make_unique<tt::FakeClock>(localEpoch(2027, 1, 15, 12));
        m_service = std::make_unique<tt::TrackerService>(*m_db, *m_clock);
        m_dir = std::make_unique<QTemporaryDir>();
        QVERIFY(m_dir->isValid());
        m_store = std::make_unique<QSettings>(m_dir->filePath(QStringLiteral("test.ini")),
                                              QSettings::IniFormat);
        m_settings = std::make_unique<tt::Settings>(*m_store);
    }

    void cleanup() {
        m_settings.reset();
        m_store.reset();
        m_dir.reset();
        m_service.reset();
        m_clock.reset();
        m_db.reset();
    }

    void recoveryKeepChoiceIsDefaultAndSelectable() {
        RecoveryDialog dialog(QStringLiteral("Design"), 13 * 3600);
        QCOMPARE(dialog.choice(), tt::RecoveryChoice::Keep);
        dialog.findChild<QPushButton *>(QStringLiteral("recoveryKeep"))->click();
        QCOMPARE(dialog.choice(), tt::RecoveryChoice::Keep);
    }

    void recoveryStopAtLastSeenChoice() {
        RecoveryDialog dialog(QStringLiteral("Design"), 13 * 3600);
        dialog.findChild<QPushButton *>(QStringLiteral("recoveryStopAtLastSeen"))->click();
        QCOMPARE(dialog.choice(), tt::RecoveryChoice::StopAtLastSeen);
    }

    void recoveryDiscardChoice() {
        RecoveryDialog dialog(QStringLiteral("Design"), 13 * 3600);
        dialog.findChild<QPushButton *>(QStringLiteral("recoveryDiscard"))->click();
        QCOMPARE(dialog.choice(), tt::RecoveryChoice::Discard);
    }

    void settingsDialogSavesReminderHours() {
        SettingsDialog dialog(*m_settings);
        dialog.findChild<QSpinBox *>(QStringLiteral("reminderHoursSpin"))->setValue(3);
        dialog.accept();
        QCOMPARE(m_settings->reminderHours(), 3);
    }

    void settingsDialogStartsWithCurrentValue() {
        m_settings->setReminderHours(5);
        SettingsDialog dialog(*m_settings);
        QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("reminderHoursSpin"))->value(), 5);
    }

    void monitorRemindsOnceAfterThreshold() {
        SessionMonitor monitor(*m_service, *m_settings);
        QSignalSpy reminders(&monitor, &SessionMonitor::reminderDue);

        const qint64 task = tt::TaskRepository(*m_db).create(QStringLiteral("Design"), 0)->id;
        m_service->start(task);

        m_clock->advance(8 * 3600 - 1);
        monitor.tick();
        QCOMPARE(reminders.count(), 0);

        m_clock->advance(1);
        monitor.tick();
        QCOMPARE(reminders.count(), 1);
        QCOMPARE(reminders.at(0).at(0).toString(), QStringLiteral("Design"));

        monitor.tick();
        QCOMPARE(reminders.count(), 1);
    }

    void monitorRemindsAgainForNewSession() {
        SessionMonitor monitor(*m_service, *m_settings);
        QSignalSpy reminders(&monitor, &SessionMonitor::reminderDue);
        const qint64 task = tt::TaskRepository(*m_db).create(QStringLiteral("Design"), 0)->id;

        m_service->start(task);
        m_clock->advance(8 * 3600);
        monitor.tick();
        m_service->start(task);
        m_clock->advance(8 * 3600);
        monitor.tick();
        QCOMPARE(reminders.count(), 2);
    }

    void monitorUsesConfiguredReminderHours() {
        m_settings->setReminderHours(2);
        SessionMonitor monitor(*m_service, *m_settings);
        QSignalSpy reminders(&monitor, &SessionMonitor::reminderDue);
        const qint64 task = tt::TaskRepository(*m_db).create(QStringLiteral("Design"), 0)->id;
        m_service->start(task);
        m_clock->advance(2 * 3600);
        monitor.tick();
        QCOMPARE(reminders.count(), 1);
    }

    void monitorRecordsHeartbeat() {
        SessionMonitor monitor(*m_service, *m_settings);
        monitor.tick();
        QCOMPARE(m_settings->lastSeenUtc().value_or(-1), m_clock->now());
    }

    void historyExportWritesRowsInRange() {
        const qint64 task = tt::TaskRepository(*m_db).create(QStringLiteral("Design"), 0)->id;
        tt::SessionRepository(*m_db).addClosed(task, localEpoch(2027, 1, 15, 9),
                                               localEpoch(2027, 1, 15, 10), QStringLiteral("UTC"));
        HistoryPage page(*m_service);

        const QString path = m_dir->filePath(QStringLiteral("export.csv"));
        QVERIFY(page.exportCsv(path));

        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
        const QString content = QString::fromUtf8(file.readAll());
        QVERIFY(content.startsWith(QStringLiteral("date,start,end,duration_seconds,task\n")));
        QVERIFY(content.contains(QStringLiteral("2027-01-15")));
        QVERIFY(content.contains(QStringLiteral(",3600,Design")));
    }
};

QTEST_MAIN(TstRobustnessUi)
#include "tst_robustness_ui.moc"
