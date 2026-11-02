#pragma once

#include "core/TrackerService.h"
#include "ui/TaskTableModel.h"

#include <QTimer>
#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QTableView;

class TrackerPage : public QWidget {
    Q_OBJECT

public:
    explicit TrackerPage(tt::TrackerService &service, QWidget *parent = nullptr);

private:
    void reload();
    void refreshClock();
    void updateHeader();
    void updateFooter();
    void toggleRow(int row);
    void startFromNewTaskField();
    void stopActive();

    tt::TrackerService &m_service;
    TaskTableModel *m_model = nullptr;
    QLabel *m_activeName = nullptr;
    QLabel *m_elapsed = nullptr;
    QPushButton *m_stop = nullptr;
    QLineEdit *m_newTask = nullptr;
    QTableView *m_view = nullptr;
    QLabel *m_today = nullptr;
    QLabel *m_week = nullptr;
    QTimer m_timer;
};
