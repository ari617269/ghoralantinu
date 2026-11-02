#pragma once

#include "core/TrackerService.h"

#include <QDialog>

class RecoveryDialog : public QDialog {
    Q_OBJECT

public:
    RecoveryDialog(const QString &taskName, qint64 elapsedSecs, QWidget *parent = nullptr);

    tt::RecoveryChoice choice() const;

private:
    tt::RecoveryChoice m_choice = tt::RecoveryChoice::Keep;
};
