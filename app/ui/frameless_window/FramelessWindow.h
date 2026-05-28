//
// Created by talik on 5/29/2025.
//

#ifndef FRAMELESS_WINDOW_H
#define FRAMELESS_WINDOW_H

#include <qevent.h>
#include <QMainWindow>
#include <QWidget>

#include "settings/UserSettings.h"
#include "settings/SettingManager/SettingsManager.h"

namespace buraq
{
    struct buraq_api;
}

class QPushButton; // Forward declaration
class QStatusBar;
class QSplitter;
class QGridLayout;
class QPoint;
class QHBoxLayout;
class QVBoxLayout;
class CustomDrawer;
class PluginManager;
class OutputDisplay;
class Editor;
class EditorMargin;
class ToolBar;
class ThemeManager;
class Frame;

class FramelessWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FramelessWindow(QWidget* parent);
    ~FramelessWindow() override;

    [[nodiscard]] Editor* getEditor() const;
    [[nodiscard]] PluginManager* getLangPluginManager() const;

protected:
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;

public slots:
    void processStatusSlot(const QString&, int timeout = 5000) const;
    void closeWindowSlot();
    void showMaximizeOrRestoreSlot();

protected:
    virtual bool maybeSave() { return true; }
    Frame* m_Frame;
    QStatusBar* m_statusBar{};
    ToolBar* m_toolBar;
    std::unique_ptr<buraq::buraq_api> api_context;

    // buttons
    QPushButton* m_settingsButton;
    QPushButton* m_minimizeButton;
    QPushButton* m_maximizeButton;
    QPushButton* m_closeButton;

    // splitters
     QSplitter* rightSideSplitter;
     QSplitter* topAreaSplitter;

    UserSettings userPreferences;

private:
    void setupTitleBar();
    ThemeManager& themeManager;
    PluginManager* pluginManager;

protected:
    void resizeEvent(QResizeEvent *event) override {
        emit windowResize(event->size());
        QMainWindow::resizeEvent(event);
    }
signals:
    void closeApp();
    void windowResize(QSize size);
};

#endif //FRAMELESS_WINDOW_H
