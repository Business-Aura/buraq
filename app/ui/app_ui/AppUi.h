//
// Created by Talik Kasozi on 2/3/2024.
//
#ifndef APP_UI_H
#define APP_UI_H

#include <filesystem>
#include <QObject>

namespace buraq
{
    struct buraq_api;
}

class Minion;
class QMainWindow;
class QMouseEvent;
class QThread;
class EditorMargin;
class ManagedProcess;
class FramelessWindow;
class PluginManager;
class ToolBar;

class AppUi final : public QObject
{
    Q_OBJECT

private slots:
    void onWindowFullyLoaded();

signals:
    void updateStatusBar(const QString&, int timeOut);

public:
    explicit AppUi(QObject* parent = nullptr);

    // The std::unique_ptr members will automatically
    // delete their managed objects when AppUi is destroyed.
    // No manual delete, no manual nullptr.
    ~AppUi() override;
    void showUi() const;

    [[nodiscard]] buraq::buraq_api* get_api_context() const { return api_context.get(); };

private:
    std::unique_ptr<PluginManager> pluginManager;
    std::unique_ptr<buraq::buraq_api> api_context;
    std::unique_ptr<FramelessWindow> m_framelessWindow;

    // For running background services
    ManagedProcess* m_bridgeProcess{};

    QThread *m_workerThread{};
    Minion *m_minion{};

    void initPSLangSupport();
    void verifyApplicationVersion();
    void initAppLayout();
    void initAppContext();
    static void launchUpdaterAndExit(
        const std::filesystem::path& updaterPath,
        const std::filesystem::path& packagePath,
        const std::filesystem::path& installationPath
    );
};


#endif // APP_UI_H
