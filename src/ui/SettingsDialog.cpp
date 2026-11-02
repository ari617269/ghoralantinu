#include "ui/SettingsDialog.h"

#include "core/Settings.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(tt::Settings &settings, QWidget *parent)
    : QDialog(parent), m_settings(settings) {
    setWindowTitle(tr("Settings"));

    m_reminderHours = new QSpinBox(this);
    m_reminderHours->setObjectName(QStringLiteral("reminderHoursSpin"));
    m_reminderHours->setRange(tt::Settings::kMinReminderHours, tt::Settings::kMaxReminderHours);
    m_reminderHours->setSuffix(tr(" h"));
    m_reminderHours->setValue(m_settings.reminderHours());

    auto *form = new QFormLayout;
    form->addRow(tr("Remind me when a timer has run for"), m_reminderHours);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

void SettingsDialog::accept() {
    m_settings.setReminderHours(m_reminderHours->value());
    QDialog::accept();
}
