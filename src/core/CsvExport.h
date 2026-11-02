#pragma once

#include "core/TrackerService.h"

#include <QList>
#include <QString>

namespace tt {

QString historyToCsv(const QList<HistoryDay> &days);

}
