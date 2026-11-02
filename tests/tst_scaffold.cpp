#include "core/Version.h"

#include <QTest>

class TstScaffold : public QObject {
    Q_OBJECT

private slots:
    void versionIsNonEmpty() { QVERIFY(!tt::versionString().isEmpty()); }
};

QTEST_APPLESS_MAIN(TstScaffold)
#include "tst_scaffold.moc"
