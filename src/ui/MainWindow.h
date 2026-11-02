#pragma once

#include <QMainWindow>

namespace tt {
class Settings;
class TrackerService;
} // namespace tt

class QButtonGroup;
class QStackedWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(tt::TrackerService &service, tt::Settings &settings, bool hideOnClose,
               QWidget *parent = nullptr);

    void showTracker();
    void showHistory();

public slots:
    void showLongRunningReminder(const QString &taskName, qint64 elapsedSecs);

signals:
    void quitRequested();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void showPage(int index);
    void openSettings();

    tt::Settings &m_settings;
    QStackedWidget *m_stack = nullptr;
    QButtonGroup *m_nav = nullptr;
    bool m_hideOnClose = false;
};
