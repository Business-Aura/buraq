#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QObject>
#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QPalette>
#include <QEvent>

enum AppTheme {
    Light,
    Dark,
    SystemDefault,
};

class ThemeManager final : public QObject
{
    Q_OBJECT
public:
    static ThemeManager& instance();
    void updateStyleSheet(AppTheme theme, const QString& stylePath);

    void setAppTheme(AppTheme theme);
    AppTheme currentTheme() const { return m_currentTheme; }
    static AppTheme getThemeFromPalette(const QPalette &palette);

    signals:
        void themeChanged(AppTheme theme);

protected:
    // This is the method that will be called when an event is filtered.
    // It's part of the QEventFilter interface.
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    explicit ThemeManager(QObject *parent = nullptr);
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    AppTheme m_currentTheme;
};

#endif // THEME_MANAGER_H