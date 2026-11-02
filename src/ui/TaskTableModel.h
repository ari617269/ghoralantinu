#pragma once

#include "core/TrackerService.h"

#include <QAbstractTableModel>
#include <QList>

class TaskTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column { NameColumn, TodayColumn, TotalColumn, ToggleColumn, ColumnCount };

    explicit TaskTableModel(QObject *parent = nullptr);

    void setSummaries(const QList<tt::TaskSummary> &summaries);
    void updateSummary(int row, const tt::TaskSummary &summary);
    int rowForTask(qint64 taskId) const;
    std::optional<tt::TaskSummary> summaryAt(int row) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    QList<tt::TaskSummary> m_rows;
};
