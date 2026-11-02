#pragma once

#include "core/TrackerService.h"
#include "ui/SessionEditDialog.h"

#include <QWidget>
#include <optional>
#include <utility>

class QDateEdit;
class QPushButton;
class QStandardItemModel;
class QTreeView;

class HistoryPage : public QWidget {
    Q_OBJECT

public:
    explicit HistoryPage(tt::TrackerService &service, QWidget *parent = nullptr);

    tt::SessionError saveDraft(const SessionDraft &draft);
    bool exportCsv(const QString &path);

private:
    void reload();
    void updateButtons();
    std::optional<qint64> selectedSessionId() const;
    std::pair<qint64, qint64> currentRange() const;
    void addEntry();
    void editSelected();
    void deleteSelected();
    void undoDelete();
    void exportToFile();
    void runDialog(const SessionDraft &initial);

    tt::TrackerService &m_service;
    QDateEdit *m_from = nullptr;
    QDateEdit *m_to = nullptr;
    QStandardItemModel *m_model = nullptr;
    QTreeView *m_view = nullptr;
    QPushButton *m_addButton = nullptr;
    QPushButton *m_editButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
    QPushButton *m_undoButton = nullptr;
    QPushButton *m_exportButton = nullptr;
    std::optional<tt::Session> m_undoSession;
};
