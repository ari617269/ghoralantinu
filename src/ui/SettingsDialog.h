#pragma once

#include <QDialog>

namespace tt {
class Settings;
}

class QSpinBox;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(tt::Settings &settings, QWidget *parent = nullptr);

    void accept() override;

private:
    tt::Settings &m_settings;
    QSpinBox *m_reminderHours = nullptr;
};
