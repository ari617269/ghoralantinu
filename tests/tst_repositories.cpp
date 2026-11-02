#include "core/Database.h"
#include "core/SessionRepository.h"
#include "core/TaskRepository.h"

#include <QDate>
#include <QDateTime>
#include <QTemporaryDir>
#include <QTest>
#include <QTimeZone>
#include <memory>

namespace {

qint64 localEpoch(int year, int month, int day, int hour = 0, int minute = 0, int second = 0) {
    return QDateTime(QDate(year, month, day), QTime(hour, minute, second),
                     QTimeZone::systemTimeZone())
        .toSecsSinceEpoch();
}

} // namespace

class TstRepositories : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<tt::Database> m_db;

    tt::Task makeTask(tt::TaskRepository &tasks, const QString &name) {
        return tasks.create(name, 1000).value();
    }

    tt::Session addClosedSession(tt::SessionRepository &sessions, qint64 taskId, qint64 start,
                                 qint64 end) {
        const qint64 id = sessions.start(taskId, start, QStringLiteral("UTC")).value().id;
        sessions.stop(id, end);
        return sessions.findById(id).value();
    }

private slots:
    void init() {
        m_db = std::make_unique<tt::Database>(QStringLiteral(":memory:"));
        QVERIFY(m_db->isReady());
    }

    void cleanup() { m_db.reset(); }

    void databaseOpensInMemoryAtCurrentSchemaVersion() { QCOMPARE(m_db->schemaVersion(), 1); }

    void databaseReopenKeepsDataAndSchema() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("timetracker.db"));

        {
            tt::Database db(path);
            QVERIFY(db.isReady());
            tt::TaskRepository tasks(db);
            QVERIFY(tasks.create(QStringLiteral("Design"), 1000).has_value());
        }
        {
            tt::Database db(path);
            QVERIFY(db.isReady());
            QCOMPARE(db.schemaVersion(), 1);
            tt::TaskRepository tasks(db);
            QCOMPARE(tasks.list().size(), qsizetype(1));
        }
    }

    void taskCreateReturnsStoredTask() {
        tt::TaskRepository tasks(*m_db);
        auto task = tasks.create(QStringLiteral("Design"), 1000);
        QVERIFY(task.has_value());
        QVERIFY(task->id > 0);
        QCOMPARE(task->name, QStringLiteral("Design"));
        QCOMPARE(task->createdUtc, qint64(1000));
        QVERIFY(!task->archived);
    }

    void taskCreateRejectsDuplicateName() {
        tt::TaskRepository tasks(*m_db);
        QVERIFY(tasks.create(QStringLiteral("Design"), 1000).has_value());
        QVERIFY(!tasks.create(QStringLiteral("Design"), 2000).has_value());
    }

    void taskFindByNameReturnsMatchOrNothing() {
        tt::TaskRepository tasks(*m_db);
        const auto created = makeTask(tasks, QStringLiteral("Design"));

        auto found = tasks.findByName(QStringLiteral("Design"));
        QVERIFY(found.has_value());
        QCOMPARE(found->id, created.id);
        QVERIFY(!tasks.findByName(QStringLiteral("Missing")).has_value());
    }

    void taskListOrdersByNameAndHidesArchivedByDefault() {
        tt::TaskRepository tasks(*m_db);
        makeTask(tasks, QStringLiteral("Zeta"));
        makeTask(tasks, QStringLiteral("Alpha"));
        const auto mid = makeTask(tasks, QStringLiteral("Mid"));
        QVERIFY(tasks.setArchived(mid.id, true));

        const auto active = tasks.list();
        QCOMPARE(active.size(), qsizetype(2));
        QCOMPARE(active.at(0).name, QStringLiteral("Alpha"));
        QCOMPARE(active.at(1).name, QStringLiteral("Zeta"));

        const auto all = tasks.list(true);
        QCOMPARE(all.size(), qsizetype(3));
        QCOMPARE(all.at(1).name, QStringLiteral("Mid"));
        QVERIFY(all.at(1).archived);
    }

    void taskUnarchiveRestoresToActiveList() {
        tt::TaskRepository tasks(*m_db);
        const auto task = makeTask(tasks, QStringLiteral("Design"));
        QVERIFY(tasks.setArchived(task.id, true));
        QVERIFY(tasks.setArchived(task.id, false));
        QCOMPARE(tasks.list().size(), qsizetype(1));
    }

    void taskRenameUpdatesName() {
        tt::TaskRepository tasks(*m_db);
        const auto task = makeTask(tasks, QStringLiteral("Design"));
        QVERIFY(tasks.rename(task.id, QStringLiteral("Renamed")));
        QVERIFY(tasks.findByName(QStringLiteral("Renamed")).has_value());
        QVERIFY(!tasks.findByName(QStringLiteral("Design")).has_value());
    }

    void taskRenameRejectsConflictingName() {
        tt::TaskRepository tasks(*m_db);
        makeTask(tasks, QStringLiteral("Design"));
        const auto other = makeTask(tasks, QStringLiteral("Review"));
        QVERIFY(!tasks.rename(other.id, QStringLiteral("Design")));
        QVERIFY(tasks.findByName(QStringLiteral("Review")).has_value());
    }

    void sessionStartCreatesRunningSession() {
        tt::TaskRepository tasks(*m_db);
        tt::SessionRepository sessions(*m_db);
        const auto task = makeTask(tasks, QStringLiteral("Design"));

        auto started = sessions.start(task.id, 100, QStringLiteral("Europe/Berlin"));
        QVERIFY(started.has_value());
        QVERIFY(!started->endUtc.has_value());
        QCOMPARE(started->tzName, QStringLiteral("Europe/Berlin"));

        auto running = sessions.findRunning();
        QVERIFY(running.has_value());
        QCOMPARE(running->id, started->id);
    }

    void sessionStopClosesRunningSession() {
        tt::TaskRepository tasks(*m_db);
        tt::SessionRepository sessions(*m_db);
        const auto task = makeTask(tasks, QStringLiteral("Design"));
        const auto started = sessions.start(task.id, 100, QStringLiteral("UTC"));
        QVERIFY(started.has_value());

        QVERIFY(sessions.stop(started->id, 200));
        QVERIFY(!sessions.findRunning().has_value());
        auto closed = sessions.findById(started->id);
        QVERIFY(closed.has_value());
        QCOMPARE(closed->endUtc.value_or(-1), qint64(200));
    }

    void sessionStopFailsWhenNotRunning() {
        tt::TaskRepository tasks(*m_db);
        tt::SessionRepository sessions(*m_db);
        const auto task = makeTask(tasks, QStringLiteral("Design"));
        const auto closed = addClosedSession(sessions, task.id, 100, 200);
        QVERIFY(!sessions.stop(closed.id, 300));
    }

    void sessionListInRangeReturnsOverlappingSessionsOnly() {
        tt::TaskRepository tasks(*m_db);
        tt::SessionRepository sessions(*m_db);
        const auto task = makeTask(tasks, QStringLiteral("Design"));
        const auto a = addClosedSession(sessions, task.id, 0, 100);
        const auto b = addClosedSession(sessions, task.id, 150, 250);
        addClosedSession(sessions, task.id, 300, 400);

        const auto hits = sessions.listInRange(90, 200);
        QCOMPARE(hits.size(), qsizetype(2));
        QCOMPARE(hits.at(0).id, a.id);
        QCOMPARE(hits.at(1).id, b.id);

        QVERIFY(sessions.listInRange(400, 500).isEmpty());
    }

    void sessionListInRangeIncludesRunningSession() {
        tt::TaskRepository tasks(*m_db);
        tt::SessionRepository sessions(*m_db);
        const auto task = makeTask(tasks, QStringLiteral("Design"));
        QVERIFY(sessions.start(task.id, 100, QStringLiteral("UTC")).has_value());
        QCOMPARE(sessions.listInRange(500, 600).size(), qsizetype(1));
    }

    void sessionOverlapDetectsIntersectingRanges() {
        tt::TaskRepository tasks(*m_db);
        tt::SessionRepository sessions(*m_db);
        const auto task = makeTask(tasks, QStringLiteral("Design"));
        const auto existing = addClosedSession(sessions, task.id, 100, 200);

        QVERIFY(sessions.hasOverlap(150, 250, std::nullopt));
        QVERIFY(sessions.hasOverlap(0, 101, std::nullopt));
        QVERIFY(!sessions.hasOverlap(200, 300, std::nullopt));
        QVERIFY(!sessions.hasOverlap(0, 100, std::nullopt));
        QVERIFY(!sessions.hasOverlap(150, 250, existing.id));
    }

    void sessionOverlapTreatsRunningSessionAsOpenEnded() {
        tt::TaskRepository tasks(*m_db);
        tt::SessionRepository sessions(*m_db);
        const auto task = makeTask(tasks, QStringLiteral("Design"));
        QVERIFY(sessions.start(task.id, 100, QStringLiteral("UTC")).has_value());
        QVERIFY(sessions.hasOverlap(500, 600, std::nullopt));
        QVERIFY(!sessions.hasOverlap(0, 100, std::nullopt));
    }

    void sessionUpdateChangesFields() {
        tt::TaskRepository tasks(*m_db);
        tt::SessionRepository sessions(*m_db);
        const auto task = makeTask(tasks, QStringLiteral("Design"));
        auto session = addClosedSession(sessions, task.id, 100, 200);

        session.startUtc = 50;
        session.endUtc = 300;
        session.tzName = QStringLiteral("Europe/Berlin");
        QVERIFY(sessions.update(session));

        const auto stored = sessions.findById(session.id);
        QVERIFY(stored.has_value());
        QCOMPARE(stored->startUtc, qint64(50));
        QCOMPARE(stored->endUtc.value_or(-1), qint64(300));
        QCOMPARE(stored->tzName, QStringLiteral("Europe/Berlin"));
    }

    void sessionRemoveDeletesSession() {
        tt::TaskRepository tasks(*m_db);
        tt::SessionRepository sessions(*m_db);
        const auto task = makeTask(tasks, QStringLiteral("Design"));
        const auto session = addClosedSession(sessions, task.id, 100, 200);
        QVERIFY(sessions.remove(session.id));
        QVERIFY(!sessions.findById(session.id).has_value());
    }

    void totalForTaskCountsClosedAndRunningSessionsClipped() {
        tt::TaskRepository tasks(*m_db);
        tt::SessionRepository sessions(*m_db);
        const auto task = makeTask(tasks, QStringLiteral("Design"));
        addClosedSession(sessions, task.id, 0, 3600);
        QVERIFY(sessions.start(task.id, 7200, QStringLiteral("UTC")).has_value());

        QCOMPARE(sessions.totalForTaskInRange(task.id, 0, 10000, 9000), qint64(5400));
        QCOMPARE(sessions.totalForTaskInRange(task.id, 1800, 7500, 9000), qint64(2100));
    }

    void totalForTaskIgnoresOtherTasks() {
        tt::TaskRepository tasks(*m_db);
        tt::SessionRepository sessions(*m_db);
        const auto design = makeTask(tasks, QStringLiteral("Design"));
        const auto review = makeTask(tasks, QStringLiteral("Review"));
        addClosedSession(sessions, design.id, 0, 100);
        addClosedSession(sessions, review.id, 0, 900);
        QCOMPARE(sessions.totalForTaskInRange(design.id, 0, 1000, 1000), qint64(100));
    }

    void dayTotalsSplitSessionAtLocalMidnight() {
        tt::TaskRepository tasks(*m_db);
        tt::SessionRepository sessions(*m_db);
        const auto task = makeTask(tasks, QStringLiteral("Design"));
        addClosedSession(sessions, task.id, localEpoch(2026, 6, 15, 22),
                         localEpoch(2026, 6, 16, 2));

        const auto totals = sessions.dayTotals(localEpoch(2026, 6, 15), localEpoch(2026, 6, 17),
                                               localEpoch(2026, 6, 17));
        QCOMPARE(totals.size(), qsizetype(2));
        QCOMPARE(totals.value(QDate(2026, 6, 15)), qint64(7200));
        QCOMPARE(totals.value(QDate(2026, 6, 16)), qint64(7200));
    }

    void dayTotalsCountRunningSessionUpToNow() {
        tt::TaskRepository tasks(*m_db);
        tt::SessionRepository sessions(*m_db);
        const auto task = makeTask(tasks, QStringLiteral("Design"));
        QVERIFY(sessions.start(task.id, localEpoch(2026, 6, 15, 22), QStringLiteral("UTC"))
                    .has_value());

        const auto totals = sessions.dayTotals(localEpoch(2026, 6, 15), localEpoch(2026, 6, 16),
                                               localEpoch(2026, 6, 15, 23));
        QCOMPARE(totals.value(QDate(2026, 6, 15)), qint64(3600));
    }
};

QTEST_GUILESS_MAIN(TstRepositories)
#include "tst_repositories.moc"
