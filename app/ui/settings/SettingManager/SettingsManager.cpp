//
// Created by talik on 8/30/2025.
//

#include "SettingsManager.h"

#include <QSettings>
#include <QVariant>

void SettingsManager::saveSettings(const UserSettings& settings)
{
    // QSettings will automatically use the organization and application name set in main.cpp
    QSettings qsettings;

    // Use groups to keep settings organized
    qsettings.beginGroup("Preferences");

    // Save each member of the struct
    qsettings.setValue("theme", settings.theme);
    qsettings.setValue("windowSize", settings.windowSize);
    qsettings.setValue("windowPosition", settings.windowPosition);
    qsettings.setValue("wordWrap", settings.wordWrapEnabled);
    qsettings.setValue("editorFontSize", settings.editorFontSize);
    qsettings.setValue("shellPath", settings.shellPath);
    qsettings.setValue("shellArgs", settings.shellArgs);

    qsettings.endGroup();
}

// This function safely converts an integer to a Theme enum.
// It returns an empty optional if the integer is not a valid theme.
std::optional<AppTheme> SettingsManager::intToTheme(const int value) {
    // Check if the integer value corresponds to a known enum member
    switch (value) {
    case static_cast<int>(Light):
        return Light;
    case static_cast<int>(Dark):
        return Dark;
    case static_cast<int>(SystemDefault):
        return SystemDefault;
    default:
        // The integer is not a valid theme, return an empty optional
        return std::nullopt;
    }
}

UserSettings SettingsManager::loadSettings()
{
    QSettings qsettings;
    UserSettings settings; // Create a default-initialized struct

    qsettings.beginGroup("Preferences");

    // Read each value, providing a default from our struct
    // The QVariant::fromValue is used to ensure type safety
    try
    {
        // 1. Read the theme setting as an integer.
        const int themeAsInt = qsettings.value("theme", settings.theme).toInt();

        // 2. Use the safe casting function.
        // 3. Check if the conversion was successful before applying it.
        if (const std::optional<AppTheme> loadedTheme = intToTheme(themeAsInt); loadedTheme.has_value()) {
            settings.theme = loadedTheme.value();
        }
        settings.windowSize = qsettings.value("windowSize", QVariant::fromValue(settings.windowSize)).toSize();
        settings.windowPosition = qsettings.value("windowPosition", QVariant::fromValue(settings.windowPosition)).toPoint();
        settings.wordWrapEnabled = qsettings.value("wordWrap", QVariant::fromValue(settings.wordWrapEnabled)).toBool();
        settings.editorFontSize = qsettings.value("editorFontSize", QVariant::fromValue(settings.editorFontSize)).toInt();
        settings.shellPath = qsettings.value("shellPath", settings.shellPath).toString();
        settings.shellArgs = qsettings.value("shellArgs", settings.shellArgs).toString();
    }
    catch (...)
    {
        qDebug() << "SettingsManager::loadSettings failed";
    }

    qsettings.endGroup();

    return settings;
}
