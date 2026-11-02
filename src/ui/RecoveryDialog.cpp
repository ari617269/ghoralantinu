#include "ui/RecoveryDialog.h"

#include "core/TimeUtils.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

RecoveryDialog::RecoveryDialog(const QString &taskName, qint64 elapsedSecs, QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(tr("Running timer found"));

    auto *message = new QLabel(
        tr("\"%1\" has been running for %2. It may have been left running while the computer "
           "was asleep or the app was closed. What should happen to this session?")
            .arg(taskName, tt::TimeUtils::formatDuration(elapsedSecs)),
        this);
    message->setWordWrap(true);

    auto *keep = new QPushButton(tr("Keep running"), this);
    keep->setObjectName(QStringLiteral("recoveryKeep"));
    auto *stop = new QPushButton(tr("Stop at last seen time"), this);
    stop->setObjectName(QStringLiteral("recoveryStopAtLastSeen"));
    auto *discard = new QPushButton(tr("Discard"), this);
    discard->setObjectName(QStringLiteral("recoveryDiscard"));

    const auto choose = [this](tt::RecoveryChoice choice) {
        return [this, choice] {
            m_choice = choice;
            accept();
        };
    };
    connect(keep, &QPushButton::clicked, this, choose(tt::RecoveryChoice::Keep));
    connect(stop, &QPushButton::clicked, this, choose(tt::RecoveryChoice::StopAtLastSeen));
    connect(discard, &QPushButton::clicked, this, choose(tt::RecoveryChoice::Discard));

    auto *buttons = new QHBoxLayout;
    buttons->addStretch(1);
    buttons->addWidget(keep);
    buttons->addWidget(stop);
    buttons->addWidget(discard);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(message);
    layout->addLayout(buttons);
}

tt::RecoveryChoice RecoveryDialog::choice() const {
    return m_choice;
}
