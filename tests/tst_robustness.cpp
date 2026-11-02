#include "core/CsvExport.h"
#include "core/Settings.h"
#include "core/TimeUtils.h"

#include <QDate>
#include <QDateTime>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

class TstRobustness : public QObject {
    Q_OBJECT

private slots:
    void settingsDefaultsWhenUnset() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QSettings store(dir.filePath(QStringLiteral("test.ini")), QSettings::IniFormat);
        tt::Settings settings(store);
        QCOMPARE(settings.reminderHours(), 8);
        QVERIFY(!settings.lastSeenUtc().has_value());
    }

    void settingsPersistReminderHours() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("test.ini"));
        {
            QSettings store(path, QSettings::IniFormat);
            tt::Settings settings(store);
            settings.setReminderHours(3);
        }
        QSettings store(path, QSettings::IniFormat);
        QCOMPARE(tt::Settings(store).reminderHours(), 3);
    }

    void settingsClampReminderHoursToRange() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QSettings store(dir.filePath(QStringLiteral("test.ini")), QSettings::IniFormat);
        tt::Settings settings(store);
        settings.setReminderHours(0);
        QCOMPARE(settings.reminderHours(), 1);
        settings.setReminderHours(100);
        QCOMPARE(settings.reminderHours(), 24);
    }

    void settingsRoundTripLastSeen() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QSettings store(dir.filePath(QStringLiteral("test.ini")), QSettings::IniFormat);
        tt::Settings settings(store);
        settings.setLastSeenUtc(1234567);
        QCOMPARE(settings.lastSeenUtc().value_or(-1), qint64(1234567));
    }

    void csvHasHeaderAndRowPerSession() {
        tt::HistoryDay day;
        day.day = QDate(2027, 1, 15);
        day.totalSecs = 3600;
        tt::HistoryRow row;
        row.session.id = 1;
        row.session.taskId = 1;
        row.session.startUtc = 1800000000;
        row.session.endUtc = 1800003600;
        row.session.tzName = QStringLiteral("UTC");
        row.taskName = QStringLiteral("Design");
        row.durationSecs = 3600;
        day.rows.append(row);

        const QString csv = tt::historyToCsv({day});
        const QStringList lines = csv.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        QCOMPARE(lines.size(), 2);
        QCOMPARE(lines.at(0), QStringLiteral("date,start,end,duration_seconds,task"));

        const QString start =
            QDateTime::fromSecsSinceEpoch(1800000000).toLocalTime().toString(Qt::ISODate);
        const QString end =
            QDateTime::fromSecsSinceEpoch(1800003600).toLocalTime().toString(Qt::ISODate);
        QCOMPARE(lines.at(1), QStringLiteral("2027-01-15,%1,%2,3600,Design").arg(start, end));
    }

    void csvQuotesFieldsWithDelimitersAndQuotes() {
        tt::HistoryDay day;
        day.day = QDate(2027, 1, 15);
        tt::HistoryRow row;
        row.session.startUtc = 1800000000;
        row.session.endUtc = 1800000060;
        row.taskName = QStringLiteral("Design, \"v2\"");
        row.durationSecs = 60;
        day.rows.append(row);

        const QString csv = tt::historyToCsv({day});
        QVERIFY(csv.contains(QStringLiteral(",\"Design, \"\"v2\"\"\"\n")));
    }

    void csvLeavesEndEmptyForRunningSession() {
        tt::HistoryDay day;
        day.day = QDate(2027, 1, 15);
        tt::HistoryRow row;
        row.session.startUtc = 1800000000;
        row.taskName = QStringLiteral("Design");
        row.durationSecs = 120;
        row.running = true;
        day.rows.append(row);

        const QStringList lines =
            tt::historyToCsv({day}).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        QCOMPARE(lines.size(), 2);
        QVERIFY(lines.at(1).contains(QStringLiteral(",,120,Design")));
    }
};

QTEST_GUILESS_MAIN(TstRobustness)
#include "tst_robustness.moc"
