#include "core/Database.h"
#include "core/FakeClock.h"
#include "core/SessionRepository.h"
#include "core/TaskRepository.h"
#include "core/TimeUtils.h"
#include "core/TrackerService.h"

#include <QDate>
#include <QDateTime>
#include <QLocale>
#include <QSignalSpy>
#include <QTest>
#include <QTimeZone>
#include <memory>

using tt::SessionError;

namespace {

constexpr qint64 kBase = 1800000000;

qint64 localNoon(int year, int month, int day) {
    return QDateTime(QDate(year, month, day), QTime(12, 0), QTimeZone::systemTimeZone())
        .toSecsSinceEpoch();
}

int code(SessionError error) {
    return static_cast<int>(error);
}

} // namespace

class TstTrackerService : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<tt::Database> m_db;
    std::unique_ptr<tt::FakeClock> m_clock;
    std::unique_ptr<tt::TrackerService> m_service;

    qint64 makeTask(const QString &name) {
        return tt::TaskRepository(*m_db).create(name, kBase).value().id;
    }

    qint64 addClosed(qint64 taskId, qint64 start, qint64 end) {
        tt::SessionRepository sessions(*m_db);
        const qint64 id = sessions.start(taskId, start, QStringLiteral("UTC")).value().id;
        sessions.stop(id, end);
        return id;
    }

    void recreateService() { m_service = std::make_unique<tt::TrackerService>(*m_db, *m_clock); }

private slots:
    void init() {
        m_db = std::make_unique<tt::Database>(QStringLiteral(":memory:"));
        QVERIFY(m_db->isReady());
        m_clock = std::make_unique<tt::FakeClock>(kBase);
        recreateService();
    }

    void cleanup() {
        m_service.reset();
        m_clock.reset();
        m_db.reset();
    }

    void startCreatesRunningSessionAndEmits() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        QSignalSpy started(m_service.get(), &tt::TrackerService::sessionStarted);

        const auto session = m_service->start(task);
        QVERIFY(session.has_value());
        QCOMPARE(started.count(), 1);
        QVERIFY(m_service->activeSession().has_value());
        QCOMPARE(m_service->activeSession()->taskId, task);
        QVERIFY(tt::SessionRepository(*m_db).findRunning().has_value());
    }

    void startFailsForUnknownTask() {
        QVERIFY(!m_service->start(9999).has_value());
        QVERIFY(!m_service->activeSession().has_value());
    }

    void elapsedFollowsClockWithoutAccumulatingTicks() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        QCOMPARE(m_service->elapsed(), qint64(0));

        m_service->start(task);
        m_clock->advance(90);
        QCOMPARE(m_service->elapsed(), qint64(90));
        m_clock->advance(10);
        QCOMPARE(m_service->elapsed(), qint64(100));
    }

    void startAutoStopsActiveSession() {
        const qint64 design = makeTask(QStringLiteral("Design"));
        const qint64 review = makeTask(QStringLiteral("Review"));
        QSignalSpy started(m_service.get(), &tt::TrackerService::sessionStarted);
        QSignalSpy stopped(m_service.get(), &tt::TrackerService::sessionStopped);

        const auto first = m_service->start(design);
        QVERIFY(first.has_value());
        m_clock->advance(60);
        const auto second = m_service->start(review);
        QVERIFY(second.has_value());

        QCOMPARE(started.count(), 2);
        QCOMPARE(stopped.count(), 1);
        QCOMPARE(m_service->activeSession()->id, second->id);

        const auto closed = tt::SessionRepository(*m_db).findById(first->id);
        QVERIFY(closed.has_value());
        QCOMPARE(closed->endUtc.value_or(-1), kBase + 60);
    }

    void startingSameTaskRestartsIt() {
        const qint64 design = makeTask(QStringLiteral("Design"));
        const auto first = m_service->start(design);
        m_clock->advance(30);
        const auto second = m_service->start(design);
        QVERIFY(second.has_value());
        QVERIFY(second->id != first->id);
        QCOMPARE(m_service->elapsed(), qint64(0));
    }

    void stopClosesActiveSessionAndEmits() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const auto session = m_service->start(task);
        m_clock->advance(30);

        QSignalSpy stopped(m_service.get(), &tt::TrackerService::sessionStopped);
        QVERIFY(m_service->stop());
        QCOMPARE(stopped.count(), 1);
        QVERIFY(!m_service->activeSession().has_value());
        QVERIFY(!tt::SessionRepository(*m_db).findRunning().has_value());

        const auto stored = tt::SessionRepository(*m_db).findById(session->id);
        QCOMPARE(stored->endUtc.value_or(-1), kBase + 30);
    }

    void stopWithoutActiveSessionReturnsFalseSilently() {
        QSignalSpy stopped(m_service.get(), &tt::TrackerService::sessionStopped);
        QSignalSpy changed(m_service.get(), &tt::TrackerService::dataChanged);
        QVERIFY(!m_service->stop());
        QCOMPARE(stopped.count(), 0);
        QCOMPARE(changed.count(), 0);
    }

    void dataChangedEmittedForStartStopAndEdits() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        QSignalSpy changed(m_service.get(), &tt::TrackerService::dataChanged);

        m_service->start(task);
        QCOMPARE(changed.count(), 1);
        m_service->stop();
        QCOMPARE(changed.count(), 2);
        m_service->addManualEntry(task, kBase + 100, kBase + 200);
        QCOMPARE(changed.count(), 3);
    }

    void restoresRunningSessionOnStartup() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        tt::SessionRepository(*m_db).start(task, kBase, QStringLiteral("UTC"));
        m_clock->advance(500);

        recreateService();
        QVERIFY(m_service->activeSession().has_value());
        QCOMPARE(m_service->activeSession()->taskId, task);
        QCOMPARE(m_service->elapsed(), qint64(500));
    }

    void staleFlagTriggersAfterTwelveHours() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        m_service->start(task);

        m_clock->advance(tt::TrackerService::kStaleSessionSecs);
        QVERIFY(!m_service->isActiveStale());
        m_clock->advance(1);
        QVERIFY(m_service->isActiveStale());
    }

    void addManualEntryStoresClosedSession() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        QCOMPARE(code(m_service->addManualEntry(task, kBase, kBase + 3600)),
                 code(SessionError::None));

        const auto sessions = tt::SessionRepository(*m_db).listInRange(kBase, kBase + 7200);
        QCOMPARE(sessions.size(), qsizetype(1));
        QCOMPARE(sessions.at(0).endUtc.value_or(-1), kBase + 3600);
    }

    void manualEntryRejectsEndNotAfterStart() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        QCOMPARE(code(m_service->addManualEntry(task, kBase, kBase)),
                 code(SessionError::EndNotAfterStart));
        QCOMPARE(code(m_service->addManualEntry(task, kBase, kBase - 1)),
                 code(SessionError::EndNotAfterStart));
    }

    void manualEntryRejectsOverlapButAllowsTouching() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        addClosed(task, kBase + 100, kBase + 200);

        QCOMPARE(code(m_service->addManualEntry(task, kBase + 150, kBase + 250)),
                 code(SessionError::Overlap));
        QCOMPARE(code(m_service->addManualEntry(task, kBase + 200, kBase + 300)),
                 code(SessionError::None));
    }

    void manualEntryRejectsOverlapWithRunningSession() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        m_service->start(task);
        QCOMPARE(code(m_service->addManualEntry(task, kBase - 100, kBase + 10)),
                 code(SessionError::Overlap));
        QCOMPARE(code(m_service->addManualEntry(task, kBase - 100, kBase)),
                 code(SessionError::None));
    }

    void editSessionChangesTaskAndRange() {
        const qint64 design = makeTask(QStringLiteral("Design"));
        const qint64 review = makeTask(QStringLiteral("Review"));
        const qint64 id = addClosed(design, kBase, kBase + 100);

        QCOMPARE(code(m_service->editSession(id, review, kBase + 10, kBase + 50)),
                 code(SessionError::None));
        const auto stored = tt::SessionRepository(*m_db).findById(id);
        QCOMPARE(stored->taskId, review);
        QCOMPARE(stored->startUtc, kBase + 10);
        QCOMPARE(stored->endUtc.value_or(-1), kBase + 50);
    }

    void editSessionMayKeepItsOwnRange() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const qint64 id = addClosed(task, kBase, kBase + 100);
        QCOMPARE(code(m_service->editSession(id, task, kBase, kBase + 100)),
                 code(SessionError::None));
    }

    void editSessionRejectsOverlapAndInvalidRange() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        addClosed(task, kBase, kBase + 100);
        const qint64 second = addClosed(task, kBase + 200, kBase + 300);

        QCOMPARE(code(m_service->editSession(second, task, kBase + 50, kBase + 150)),
                 code(SessionError::Overlap));
        QCOMPARE(code(m_service->editSession(second, task, kBase + 250, kBase + 250)),
                 code(SessionError::EndNotAfterStart));
    }

    void editSessionReportsMissingSession() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        QCOMPARE(code(m_service->editSession(9999, task, kBase, kBase + 1)),
                 code(SessionError::NotFound));
    }

    void editRunningSessionRejectsSettingEnd() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const auto running = m_service->start(task);
        QCOMPARE(code(m_service->editSession(running->id, task, kBase, kBase + 10)),
                 code(SessionError::EndNotAllowed));
    }

    void editClosedSessionRejectsReopening() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const qint64 id = addClosed(task, kBase, kBase + 100);
        QCOMPARE(code(m_service->editSession(id, task, kBase, std::nullopt)),
                 code(SessionError::EndNotAllowed));
    }

    void editRunningSessionStartUpdatesActiveSession() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const auto running = m_service->start(task);
        m_clock->advance(100);

        QCOMPARE(code(m_service->editSession(running->id, task, kBase - 50, std::nullopt)),
                 code(SessionError::None));
        QCOMPARE(m_service->activeSession()->startUtc, kBase - 50);
        QCOMPARE(m_service->elapsed(), qint64(150));
    }

    void editRunningSessionRejectsStartInFuture() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const auto running = m_service->start(task);
        QCOMPARE(code(m_service->editSession(running->id, task, kBase + 1, std::nullopt)),
                 code(SessionError::StartInFuture));
    }

    void deleteSessionRemovesIt() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const qint64 id = addClosed(task, kBase, kBase + 100);
        QSignalSpy changed(m_service.get(), &tt::TrackerService::dataChanged);

        QCOMPARE(code(m_service->deleteSession(id)), code(SessionError::None));
        QVERIFY(!tt::SessionRepository(*m_db).findById(id).has_value());
        QCOMPARE(changed.count(), 1);
    }

    void deleteRunningSessionClearsActive() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const auto running = m_service->start(task);
        QSignalSpy stopped(m_service.get(), &tt::TrackerService::sessionStopped);

        QCOMPARE(code(m_service->deleteSession(running->id)), code(SessionError::None));
        QVERIFY(!m_service->activeSession().has_value());
        QCOMPARE(stopped.count(), 1);
    }

    void deleteSessionReportsMissingSession() {
        QCOMPARE(code(m_service->deleteSession(9999)), code(SessionError::NotFound));
    }
    void findOrCreateTaskReusesExistingName() {
        QSignalSpy changed(m_service.get(), &tt::TrackerService::dataChanged);
        const auto first = m_service->findOrCreateTask(QStringLiteral("Design"));
        const auto again = m_service->findOrCreateTask(QStringLiteral("Design"));
        QVERIFY(first.has_value());
        QVERIFY(again.has_value());
        QCOMPARE(again->id, first->id);
        QCOMPARE(tt::TaskRepository(*m_db).list().size(), qsizetype(1));
        QCOMPARE(changed.count(), 1);
    }

    void taskSummariesReportTodayTotalAndRunningState() {
        const qint64 noon = localNoon(2027, 1, 15);
        const qint64 design = makeTask(QStringLiteral("Design"));
        const qint64 review = makeTask(QStringLiteral("Review"));
        addClosed(design, noon - 600, noon - 300);

        m_clock->setNow(noon - 100);
        m_service->start(review);
        m_clock->setNow(noon);

        const auto summaries = m_service->taskSummaries();
        QCOMPARE(summaries.size(), qsizetype(2));
        QCOMPARE(summaries.at(0).task.name, QStringLiteral("Design"));
        QCOMPARE(summaries.at(0).todaySecs, qint64(300));
        QCOMPARE(summaries.at(0).totalSecs, qint64(300));
        QVERIFY(!summaries.at(0).running);
        QCOMPARE(summaries.at(1).task.name, QStringLiteral("Review"));
        QCOMPARE(summaries.at(1).todaySecs, qint64(100));
        QCOMPARE(summaries.at(1).totalSecs, qint64(100));
        QVERIFY(summaries.at(1).running);

        QCOMPARE(m_service->taskSummary(review)->todaySecs, qint64(100));
        QVERIFY(!m_service->taskSummary(9999).has_value());
    }

    void todayAndWeekTotalsUseLocalBoundaries() {
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
        const qint64 now = localNoon(2027, 1, 15);
        m_clock->setNow(now);
        const qint64 task = makeTask(QStringLiteral("Design"));

        addClosed(task, localNoon(2027, 1, 9), localNoon(2027, 1, 9) + 3600);
        addClosed(task, localNoon(2027, 1, 14), localNoon(2027, 1, 14) + 3600);
        addClosed(task, now - 300, now);

        QCOMPARE(m_service->todayTotal(), qint64(300));
        QCOMPARE(m_service->weekTotal(), qint64(3900));
    }
    void historyGroupsByLocalStartDayNewestFirst() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const qint64 jan14 = localNoon(2027, 1, 14);
        const qint64 first = addClosed(task, jan14, jan14 + 1800);
        const qint64 second = addClosed(task, jan14 + 3600, jan14 + 5400);
        addClosed(task, localNoon(2027, 1, 15), localNoon(2027, 1, 15) + 3600);
        m_clock->setNow(localNoon(2027, 1, 15) + 7200);

        const auto days = m_service->history(tt::TimeUtils::localMidnightUtc(QDate(2027, 1, 13)),
                                             tt::TimeUtils::localMidnightUtc(QDate(2027, 1, 16)));
        QCOMPARE(days.size(), qsizetype(2));
        QCOMPARE(days.at(0).day, QDate(2027, 1, 15));
        QCOMPARE(days.at(0).totalSecs, qint64(3600));
        QCOMPARE(days.at(0).rows.size(), qsizetype(1));

        QCOMPARE(days.at(1).day, QDate(2027, 1, 14));
        QCOMPARE(days.at(1).totalSecs, qint64(3600));
        QCOMPARE(days.at(1).rows.size(), qsizetype(2));
        QCOMPARE(days.at(1).rows.at(0).session.id, second);
        QCOMPARE(days.at(1).rows.at(1).session.id, first);
        QCOMPARE(days.at(1).rows.at(0).taskName, QStringLiteral("Design"));
    }

    void historySplitsDayTotalsAcrossMidnight() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const qint64 midnight = tt::TimeUtils::localMidnightUtc(QDate(2027, 1, 15));
        addClosed(task, midnight - 7200, midnight + 7200);
        m_clock->setNow(midnight + 7200);

        const auto days = m_service->history(tt::TimeUtils::localMidnightUtc(QDate(2027, 1, 13)),
                                             tt::TimeUtils::localMidnightUtc(QDate(2027, 1, 16)));
        QCOMPARE(days.size(), qsizetype(2));
        QCOMPARE(days.at(0).totalSecs, qint64(7200));
        QCOMPARE(days.at(1).totalSecs, qint64(7200));
        QCOMPARE(days.at(1).rows.size(), qsizetype(1));
        QCOMPARE(days.at(1).rows.at(0).durationSecs, qint64(14400));
    }

    void historyMarksRunningSessionWithElapsedDuration() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        m_clock->setNow(localNoon(2027, 1, 15));
        m_service->start(task);
        m_clock->advance(600);

        const auto days = m_service->history(tt::TimeUtils::localMidnightUtc(QDate(2027, 1, 13)),
                                             tt::TimeUtils::localMidnightUtc(QDate(2027, 1, 16)));
        QCOMPARE(days.size(), qsizetype(1));
        QVERIFY(days.at(0).rows.at(0).running);
        QCOMPARE(days.at(0).rows.at(0).durationSecs, qint64(600));
    }

    void restoreSessionReinsertsDeletedSession() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        QCOMPARE(code(m_service->addManualEntry(task, kBase, kBase + 100)),
                 code(SessionError::None));
        const auto session = tt::SessionRepository(*m_db).listInRange(kBase, kBase + 200).at(0);
        QCOMPARE(code(m_service->deleteSession(session.id)), code(SessionError::None));

        QCOMPARE(code(m_service->restoreSession(session)), code(SessionError::None));
        const auto restored = tt::SessionRepository(*m_db).listInRange(kBase, kBase + 200);
        QCOMPARE(restored.size(), qsizetype(1));
        QCOMPARE(restored.at(0).taskId, task);
        QCOMPARE(restored.at(0).startUtc, kBase);
        QCOMPARE(restored.at(0).endUtc.value_or(-1), kBase + 100);
    }

    void restoreSessionRejectsOverlapWithNewerEntry() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const qint64 id = addClosed(task, kBase, kBase + 100);
        const auto session = tt::SessionRepository(*m_db).findById(id).value();
        QCOMPARE(code(m_service->deleteSession(id)), code(SessionError::None));
        QCOMPARE(code(m_service->addManualEntry(task, kBase + 50, kBase + 150)),
                 code(SessionError::None));

        QCOMPARE(code(m_service->restoreSession(session)), code(SessionError::Overlap));
    }

    void restoreRunningSessionIsRejected() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const auto running = m_service->start(task);
        QCOMPARE(code(m_service->restoreSession(*running)), code(SessionError::EndNotAllowed));
    }

    void sessionLookupReturnsStoredSession() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const qint64 id = addClosed(task, kBase, kBase + 100);
        QVERIFY(m_service->session(id).has_value());
        QVERIFY(!m_service->session(9999).has_value());
    }

    void todayLocalFollowsClock() {
        m_clock->setNow(localNoon(2027, 1, 15));
        QCOMPARE(m_service->todayLocal(), QDate(2027, 1, 15));
    }
    void recentTasksOrderedByLatestStartExcludingArchived() {
        const qint64 alpha = makeTask(QStringLiteral("Alpha"));
        const qint64 beta = makeTask(QStringLiteral("Beta"));
        makeTask(QStringLiteral("Gamma"));
        addClosed(beta, kBase + 100, kBase + 150);
        addClosed(alpha, kBase, kBase + 50);
        addClosed(alpha, kBase + 200, kBase + 250);

        const auto recent = m_service->recentTasks(5);
        QCOMPARE(recent.size(), qsizetype(2));
        QCOMPARE(recent.at(0).id, alpha);
        QCOMPARE(recent.at(1).id, beta);
        QCOMPARE(m_service->recentTasks(1).size(), qsizetype(1));

        QVERIFY(tt::TaskRepository(*m_db).setArchived(alpha, true));
        const auto afterArchive = m_service->recentTasks(5);
        QCOMPARE(afterArchive.size(), qsizetype(1));
        QCOMPARE(afterArchive.at(0).id, beta);
    }
    void resolveStaleKeepLeavesSessionRunning() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        m_service->start(task);
        m_clock->advance(13 * 3600);
        QVERIFY(m_service->isActiveStale());

        QCOMPARE(code(m_service->resolveStaleSession(tt::RecoveryChoice::Keep, kBase)),
                 code(SessionError::None));
        QVERIFY(m_service->activeSession().has_value());
    }

    void resolveStaleStopAtLastSeenClosesAtThatTime() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const qint64 id = m_service->start(task)->id;
        m_clock->advance(13 * 3600);

        QCOMPARE(
            code(m_service->resolveStaleSession(tt::RecoveryChoice::StopAtLastSeen, kBase + 7200)),
            code(SessionError::None));
        QVERIFY(!m_service->activeSession().has_value());
        QCOMPARE(tt::SessionRepository(*m_db).findById(id)->endUtc.value_or(-1), kBase + 7200);
    }

    void resolveStaleStopAtLastSeenClampsToSessionBounds() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const qint64 early = m_service->start(task)->id;
        m_clock->advance(13 * 3600);
        m_service->resolveStaleSession(tt::RecoveryChoice::StopAtLastSeen, kBase - 500);
        QCOMPARE(tt::SessionRepository(*m_db).findById(early)->endUtc.value_or(-1), kBase);

        const qint64 late = m_service->start(task)->id;
        m_clock->advance(13 * 3600);
        m_service->resolveStaleSession(tt::RecoveryChoice::StopAtLastSeen, kBase + 999999999);
        QCOMPARE(tt::SessionRepository(*m_db).findById(late)->endUtc.value_or(-1), m_clock->now());
    }

    void resolveStaleDiscardDeletesSession() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        const qint64 id = m_service->start(task)->id;
        m_clock->advance(13 * 3600);

        QCOMPARE(code(m_service->resolveStaleSession(tt::RecoveryChoice::Discard, kBase)),
                 code(SessionError::None));
        QVERIFY(!m_service->activeSession().has_value());
        QVERIFY(!tt::SessionRepository(*m_db).findById(id).has_value());
    }

    void resolveStaleWithoutActiveSessionIsNotFound() {
        QCOMPARE(code(m_service->resolveStaleSession(tt::RecoveryChoice::Discard, kBase)),
                 code(SessionError::NotFound));
    }

    void longRunningThresholdIsStrict() {
        const qint64 task = makeTask(QStringLiteral("Design"));
        m_service->start(task);
        m_clock->advance(8 * 3600);
        QVERIFY(m_service->activeRunningLongerThan(8 * 3600) == false);
        m_clock->advance(1);
        QVERIFY(m_service->activeRunningLongerThan(8 * 3600));
    }
};

QTEST_GUILESS_MAIN(TstTrackerService)
#include "tst_trackerservice.moc"
