//
// Created by talik on 8/30/2025.
//

#ifndef USERSETTINGS_H
#define USERSETTINGS_H

#include <QString>
#include <QSize>
#include <QPoint>

#include "Filters/ThemeManager/ThemeManager.h"

struct SettingsDialogPreference
{
    QSize windowSize = QSize(1200, 720);
};

// A simple struct to hold all application preferences
struct UserSettings
{
    AppTheme theme = Dark;
    QSize windowSize = QSize(1200, 720);
    QPoint windowPosition = QPoint(100, 100);
    bool wordWrapEnabled = true;
    int editorFontSize = 11;
    SettingsDialogPreference settingsDialog;
    QString buildConfiguration = "Debug";
    QString vcpkgToolchainPath = "";
    QString vcpkgTargetTriplet = "";

    // Terminal settings
#if defined(Q_OS_WIN)
    QString shellPath = "powershell.exe";
    QString shellArgs = "-NoExit -NoLogo";
#elif defined(Q_OS_MAC)
    QString shellPath = "/bin/zsh";
    QString shellArgs = "-l";
#else
    QString shellPath = "/bin/bash";
    QString shellArgs = "";
#endif
};

#endif // USERSETTINGS_H
