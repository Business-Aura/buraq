//
// Created by talik on 8/30/2025.
//

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

#include "settings/UserSettings.h"
#include "Filters/ThemeManager/ThemeManager.h" // Needed for ThemeManager reference

// Forward declarations for Qt classes to speed up compilation
class QTabWidget;
class QDialogButtonBox;
class QListWidget;
class QStackedWidget;
class QComboBox;
class QLineEdit;
class SettingsManager;
class Frame;

class SettingsDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;

public slots:
    void accept() override;

signals:
    void applySettingChanges();

private slots:
    // Slot to handle saving the settings
    void applyChanges();
    void setTheme(const int index);

private:
    Frame* m_Frame;

    SettingsManager* settingsManager;
    UserSettings userPreference;
    ThemeManager &themeManager;
    // Helper functions to create each page of the settings dialog
    QWidget* createAppearancePage();
    QWidget* createEditorPage();
    QWidget* createAccountPage();
    QWidget* createTerminalPage();

    // Shell setting UI refs (to read back on apply)
    QComboBox* m_shellComboBox    = nullptr;
    QLineEdit* m_shellArgsEdit    = nullptr;
    QLineEdit* m_customShellEdit  = nullptr;

    // Main UI elements
    QTabWidget* m_tabWidget;
    QDialogButtonBox* m_buttonBox;
};

#endif // SETTINGSDIALOG_H
