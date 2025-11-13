#include "ThemeManager.h"

#include "settings/SettingManager/SettingsManager.h"

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{
    SettingsManager settingsManager{};
    // retrieve theme from user preference
    m_currentTheme = settingsManager.loadSettings().theme;

    // Install THIS ThemeManager instance as an event filter on the QApplication object.
    // This allows its eventFilter() method to intercept events sent to qApp.
    qApp->installEventFilter(this); // CRUCIAL: Install self as filter on qApp

    qDebug() << "Theme: " << m_currentTheme;
    // Set initial theme based on system or preference
    setAppTheme(m_currentTheme); // default theme is Dark
}

ThemeManager& ThemeManager::instance()
{
    static ThemeManager manager;
    return manager;
}

void ThemeManager::updateStyleSheet(const AppTheme theme, const QString& stylePath)
{
    if (QFile styleFile(stylePath); styleFile.open(QFile::ReadOnly | QFile::Text))
    {
        const QString style = styleFile.readAll();
        qApp->setStyleSheet(style);
        styleFile.close();
        m_currentTheme = theme;
        qDebug() << "Application theme set to:" << (theme == Dark ? "Dark" : "Light");
        emit themeChanged(m_currentTheme);
    }
    else
    {
        qWarning() << "Failed to load stylesheet:" << stylePath;
    }
}

void ThemeManager::setAppTheme(const AppTheme theme)
{
    // Important: Only apply stylesheet if it's a different theme
    if (m_currentTheme == theme && !qApp->styleSheet().isEmpty())
    {
        return;
    }

    QString stylePath;
    switch (theme)
    {
    case Light:
        stylePath = ":/styles/light_theme.qss";
        m_currentTheme = Light;
        break;
    case Dark:
        stylePath = ":/styles/dark_theme.qss";
        m_currentTheme = Dark;
        break;
    case SystemDefault:
        const AppTheme detectedTheme = getThemeFromPalette(qApp->palette());
        stylePath = detectedTheme == Light ? ":/styles/light_theme.qss" : ":/styles/dark_theme.qss";
        m_currentTheme = detectedTheme;
        updateStyleSheet(detectedTheme, stylePath);
        break;
    }

    if (theme != SystemDefault) updateStyleSheet(theme, stylePath);
}

bool ThemeManager::eventFilter(QObject* watched, QEvent* event)
{
    // Check if the event is for the QApplication object itself
    if (watched == qApp && event->type() == QEvent::ApplicationPaletteChange)
    {
        qDebug() << "Event filter caught QEvent::ApplicationPaletteChange!";

        SettingsManager settings{};

        if (const auto currentTheme = settings.loadSettings().theme; currentTheme != SystemDefault)
        {
            return false;
        }

        if (const AppTheme detectedTheme = getThemeFromPalette(qApp->palette()); detectedTheme != m_currentTheme)
        {
            qDebug() << "System palette changed. Detected new theme:" << (
                detectedTheme == Dark ? "Dark" : "Light");
            setAppTheme(detectedTheme); // Apply the detected theme
        }
        return true; // Event handled, stop propagation
    }
    // For other events or if not watching qApp, pass to base class eventFilter
    return QObject::eventFilter(watched, event);
}

// Helper function to determine theme from palette (remains the same)
AppTheme ThemeManager::getThemeFromPalette(const QPalette& palette)
{
    const QColor windowColor = palette.color(QPalette::Window);
    const double luminance = (0.2126 * windowColor.red() +
        0.7152 * windowColor.green() +
        0.0722 * windowColor.blue()) / 255.0;

    if (constexpr double DARK_THEME_LUMINANCE_THRESHOLD = 0.3; luminance < DARK_THEME_LUMINANCE_THRESHOLD)
    {
        return Dark;
    }
    return Light;
}
