#include "ui/TaskTableModel.h"

#include "core/TimeUtils.h"

#include <QColor>

namespace {

bool sameTaskOrder(const QList<tt::TaskSummary> &current, const QList<tt::TaskSummary> &next) {
    if (current.size() != next.size())
        return false;
    for (qsizetype i = 0; i < current.size(); ++i) {
        if (current.at(i).task.id != next.at(i).task.id)
            return false;
    }
    return true;
}

} // namespace

TaskTableModel::TaskTableModel(QObject *parent) : QAbstractTableModel(parent) {}

void TaskTableModel::setSummaries(const QList<tt::TaskSummary> &summaries) {
    if (sameTaskOrder(m_rows, summaries)) {
        m_rows = summaries;
        if (!m_rows.isEmpty())
            emit dataChanged(index(0, NameColumn), index(rowCount() - 1, ToggleColumn));
        return;
    }
    beginResetModel();
    m_rows = summaries;
    endResetModel();
}

void TaskTableModel::updateSummary(int row, const tt::TaskSummary &summary) {
    if (row < 0 || row >= m_rows.size())
        return;
    m_rows[row] = summary;
    emit dataChanged(index(row, TodayColumn), index(row, TotalColumn));
}

int TaskTableModel::rowForTask(qint64 taskId) const {
    for (int row = 0; row < m_rows.size(); ++row) {
        if (m_rows.at(row).task.id == taskId)
            return row;
    }
    return -1;
}

std::optional<tt::TaskSummary> TaskTableModel::summaryAt(int row) const {
    if (row < 0 || row >= m_rows.size())
        return std::nullopt;
    return m_rows.at(row);
}

int TaskTableModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
}

int TaskTableModel::columnCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : static_cast<int>(ColumnCount);
}

QVariant TaskTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_rows.size())
        return {};

    const tt::TaskSummary &row = m_rows.at(index.row());

    if (role == Qt::BackgroundRole && row.running)
        return QColor(61, 133, 198, 70);
    if (role == Qt::TextAlignmentRole &&
        (index.column() == TodayColumn || index.column() == TotalColumn))
        return QVariant(Qt::AlignRight | Qt::AlignVCenter);
    if (role != Qt::DisplayRole)
        return {};

    switch (index.column()) {
    case NameColumn:
        return row.task.name;
    case TodayColumn:
        return tt::TimeUtils::formatDuration(row.todaySecs);
    case TotalColumn:
        return tt::TimeUtils::formatDuration(row.totalSecs);
    case ToggleColumn:
        return row.running ? QStringLiteral("■") : QStringLiteral("▶");
    default:
        return {};
    }
}

QVariant TaskTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};
    switch (section) {
    case NameColumn:
        return QStringLiteral("Name");
    case TodayColumn:
        return QStringLiteral("Today");
    case TotalColumn:
        return QStringLiteral("Total");
    default:
        return {};
    }
}
