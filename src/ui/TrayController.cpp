#include "ui/TrayController.h"

#include "core/TimeUtils.h"
#include "core/TrackerService.h"
#include "ui/StatusItem.h"

#include <QAction>
#include <QBrush>
#include <QIcon>
#include <QMenu>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QSystemTrayIcon>

namespace {

constexpr int kRecentTaskLimit = 5;

QIcon makeIcon(bool running) {
    constexpr qreal dpr = 2.0;
    constexpr int size = 22;

    QPixmap pixmap(QSize(size, size) * dpr);
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);

#ifdef Q_OS_MACOS
    const QColor color = Qt::black;
#else
    const QColor color = running ? QColor(46, 158, 68) : QColor(128, 128, 128);
#endif

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color, 1.8));
    painter.setBrush(running ? QBrush(color) : Qt::NoBrush);
    painter.drawEllipse(QRectF(3, 3, size - 6, size - 6));
    painter.end();

    QIcon icon(pixmap);
#ifdef Q_OS_MACOS
    icon.setIsMask(true);
#endif
    return icon;
}

} // namespace

TrayController::TrayController(tt::TrackerService &service, std::unique_ptr<StatusItem> titleItem,
                               QObject *parent)
    : QObject(parent), m_service(service), m_titleItem(std::move(titleItem)) {
    m_tray = new QSystemTrayIcon(this);
    m_menu = new QMenu;
    m_tray->setContextMenu(m_menu);

    m_status = m_menu->addAction(QString());
    m_status->setObjectName(QStringLiteral("trayStatus"));
    m_status->setEnabled(false);

    m_stop = m_menu->addAction(tr("Stop"));
    m_stop->setObjectName(QStringLiteral("trayStop"));
    connect(m_stop, &QAction::triggered, this, [this] { m_service.stop(); });

    m_menu->addSeparator();
    for (int i = 0; i < kRecentTaskLimit; ++i) {
        QAction *recent = m_menu->addAction(QString());
        recent->setObjectName(QStringLiteral("trayRecent%1").arg(i));
        recent->setVisible(false);
        connect(recent, &QAction::triggered, this,
                [this, recent] { m_service.start(recent->data().toLongLong()); });
        m_recent.append(recent);
    }

    m_menu->addSeparator();
    QAction *openTracker = m_menu->addAction(tr("Open Tracker"));
    openTracker->setObjectName(QStringLiteral("trayOpenTracker"));
    connect(openTracker, &QAction::triggered, this, &TrayController::showTrackerRequested);

    QAction *openHistory = m_menu->addAction(tr("Open History"));
    openHistory->setObjectName(QStringLiteral("trayOpenHistory"));
    connect(openHistory, &QAction::triggered, this, &TrayController::showHistoryRequested);

    m_menu->addSeparator();
    QAction *quit = m_menu->addAction(tr("Quit"));
    quit->setObjectName(QStringLiteral("trayQuit"));
    connect(quit, &QAction::triggered, this, &TrayController::quitRequested);

    connect(m_tray, &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::Trigger)
                    emit showTrackerRequested();
            });
    connect(m_menu, &QMenu::aboutToShow, this, &TrayController::refresh);
    connect(&m_service, &tt::TrackerService::sessionStarted, this, &TrayController::refresh);
    connect(&m_service, &tt::TrackerService::sessionStopped, this, &TrayController::refresh);
    connect(&m_service, &tt::TrackerService::dataChanged, this, &TrayController::refresh);

    if (m_titleItem)
        connect(m_titleItem.get(), &StatusItem::clicked, this,
                &TrayController::showTrackerRequested);
    connect(&m_clockTimer, &QTimer::timeout, this, &TrayController::updateTitle);
    m_clockTimer.start(1000);

    refresh();
}

TrayController::~TrayController() {
    delete m_menu;
}

void TrayController::show() {
    m_tray->show();
}

QMenu *TrayController::menu() const {
    return m_menu;
}

QIcon TrayController::icon() const {
    return m_tray->icon();
}

void TrayController::refresh() {
    const auto active = m_service.activeSession();
    const bool running = active.has_value();
    if (running) {
        const auto summary = m_service.taskSummary(active->taskId);
        const QString name = summary ? summary->task.name : QString();
        m_status->setText(
            tr("Running: %1 %2").arg(name, tt::TimeUtils::formatDuration(m_service.elapsed())));
    } else {
        m_status->setText(tr("Not running"));
    }
    m_stop->setEnabled(running);
    m_tray->setIcon(makeIcon(running));
    updateTitle();

    const QList<tt::Task> recent = m_service.recentTasks(kRecentTaskLimit);
    for (int i = 0; i < m_recent.size(); ++i) {
        QAction *action = m_recent.at(i);
        const bool used = i < recent.size();
        action->setVisible(used);
        if (used) {
            action->setText(recent.at(i).name);
            action->setData(recent.at(i).id);
        }
    }
}

void TrayController::updateTitle() {
    if (!m_titleItem)
        return;
    const bool running = m_service.activeSession().has_value();
    m_titleItem->setTitle(running ? tt::TimeUtils::formatDuration(m_service.elapsed()) : QString());
}
