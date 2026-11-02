#pragma once

#include <QIcon>
#include <QList>
#include <QObject>
#include <QTimer>
#include <memory>

namespace tt {
class TrackerService;
}

class QAction;
class QMenu;
class QSystemTrayIcon;
class StatusItem;

class TrayController : public QObject {
    Q_OBJECT

public:
    explicit TrayController(tt::TrackerService &service,
                            std::unique_ptr<StatusItem> titleItem = nullptr,
                            QObject *parent = nullptr);
    ~TrayController() override;

    void show();
    QMenu *menu() const;
    QIcon icon() const;

signals:
    void showTrackerRequested();
    void showHistoryRequested();
    void quitRequested();

private:
    void refresh();
    void updateTitle();

    tt::TrackerService &m_service;
    QSystemTrayIcon *m_tray = nullptr;
    QMenu *m_menu = nullptr;
    QAction *m_status = nullptr;
    QAction *m_stop = nullptr;
    QList<QAction *> m_recent;
    std::unique_ptr<StatusItem> m_titleItem;
    QTimer m_clockTimer;
};
