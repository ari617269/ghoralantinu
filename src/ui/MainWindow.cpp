#include "ui/MainWindow.h"

#include "core/Settings.h"
#include "core/TimeUtils.h"
#include "core/Version.h"
#include "ui/HistoryPage.h"
#include "ui/SettingsDialog.h"
#include "ui/TrackerPage.h"

#include <QAction>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(tt::TrackerService &service, tt::Settings &settings, bool hideOnClose,
                       QWidget *parent)
    : QMainWindow(parent), m_settings(settings), m_hideOnClose(hideOnClose) {
    setWindowTitle(QStringLiteral("Time Tracker %1").arg(tt::versionString()));
    resize(800, 600);

    m_stack = new QStackedWidget;
    m_stack->setObjectName(QStringLiteral("pageStack"));
    m_stack->addWidget(new TrackerPage(service));
    m_stack->addWidget(new HistoryPage(service));

    auto *trackerButton = new QPushButton(tr("Tracker"));
    auto *historyButton = new QPushButton(tr("History"));
    trackerButton->setCheckable(true);
    historyButton->setCheckable(true);

    m_nav = new QButtonGroup(this);
    m_nav->setExclusive(true);
    m_nav->addButton(trackerButton, 0);
    m_nav->addButton(historyButton, 1);
    connect(m_nav, &QButtonGroup::idClicked, m_stack, &QStackedWidget::setCurrentIndex);
    trackerButton->setChecked(true);

    auto *nav = new QHBoxLayout;
    nav->addWidget(trackerButton);
    nav->addWidget(historyButton);
    nav->addStretch(1);

    auto *layout = new QVBoxLayout;
    layout->addLayout(nav);
    layout->addWidget(m_stack, 1);

    auto *central = new QWidget(this);
    central->setLayout(layout);
    setCentralWidget(central);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *settingsAction = fileMenu->addAction(tr("Settings…"));
    settingsAction->setMenuRole(QAction::PreferencesRole);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::openSettings);
    fileMenu->addSeparator();
    QAction *quit = fileMenu->addAction(tr("Quit TimeTracker"));
    quit->setShortcut(QKeySequence::Quit);
    quit->setMenuRole(QAction::QuitRole);
    connect(quit, &QAction::triggered, this, &MainWindow::quitRequested);
}

void MainWindow::showTracker() {
    showPage(0);
}

void MainWindow::showHistory() {
    showPage(1);
}

void MainWindow::showPage(int index) {
    m_stack->setCurrentIndex(index);
    m_nav->button(index)->setChecked(true);
    show();
    raise();
    activateWindow();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_hideOnClose) {
        event->ignore();
        hide();
        return;
    }
    event->accept();
    emit quitRequested();
}

void MainWindow::showLongRunningReminder(const QString &taskName, qint64 elapsedSecs) {
    auto *box = new QMessageBox(QMessageBox::Information, tr("Still tracking?"),
                                tr("\"%1\" has been running for %2.")
                                    .arg(taskName, tt::TimeUtils::formatDuration(elapsedSecs)),
                                QMessageBox::Ok, this);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->show();
}

void MainWindow::openSettings() {
    SettingsDialog dialog(m_settings, this);
    dialog.exec();
}
