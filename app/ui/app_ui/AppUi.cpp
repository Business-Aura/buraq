// MIT License
//
// Copyright (c)  "2025" Talik A. Kasozi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//
// Created by Talik Kasozi on 2/3/2024.
//

#ifdef _WIN32

#include <windows.h>
#include <shlobj.h> // SHGetKnownFolderPath

#endif
#include "AppUi.h"
#include "AppUi.h"

#include <QTimer>
#include <qcoreapplication.h>
#include <QMouseEvent>
#include <QSqlError>

#include "buraq.h"
#include "Config.h"
#include "PluginManager.h"
#include "Utils.h"
#include "clients/VersionClient/VersionRepository.h"
#include "database/db_conn.h"
#include "dialog/VersionUpdateDialog.h"
#include "frameless_window/FramelessWindow.h"
#include "ManagedProcess/ManagedProcess.h"

AppUi::AppUi(QObject* parent) : QObject(parent)
{
    // Load configuration settings
    Config::singleton();

    // Ensure the singleton (and curl_global_init) is created before threads,
    // though Meyers singleton handles this.
    Network::singleton(); // Initialize network singleton if not already.

    using file_utils::file_log;
    if (!database::db_conn())
    {
        file_log("db_conn() EXIT_FAILURE..");
        // failed to connect to the database
        // return EXIT_FAILURE;
    }

    // Initialize the database:
    if (const QSqlError err = database::init_db(); err.type() != QSqlError::NoError)
    {
        file_log("Error executing initializing db:" + err.text().toStdString());
        // return EXIT_FAILURE;
    }

    // user's home dir should be the default location when the app starts.
    // In the later release, save user's last dir/path
    std::filesystem::current_path(ItoolsNS::get_user_home_directory());

    // Init application views
    initAppLayout();

    // init app's file system
    initAppContext();
}

AppUi::~AppUi()
{
    // When the MyApp object is destroyed, m_bridgeProcess's destructor
    // will be called automatically, terminating the child process.
    delete m_bridgeProcess;
}

void AppUi::showUi() const
{
    m_framelessWindow->show();
}

void AppUi::initAppLayout()
{
    m_framelessWindow = std::make_unique<FramelessWindow>(nullptr);

    // Signals
    connect(this, &AppUi::updateStatusBar, m_framelessWindow.get(), &FramelessWindow::processStatusSlot);
}

void AppUi::initAppContext()
{
    const std::filesystem::path userDataPath = std::filesystem::temp_directory_path() / "Buraq";

    // app search_path for plugins
    const std::filesystem::path searchPath(QCoreApplication::applicationDirPath().toStdString());
    // Add required plugins
    const std::map<std::string, std::string> plugins_{};

    api_context = std::make_unique<buraq::buraq_api>();
    api_context->searchPath = searchPath;
    api_context->plugins = plugins_;
    api_context->userDataPath = userDataPath / ".data";
    api_context->userPath = userDataPath;

    pluginManager = std::make_unique<PluginManager>(api_context.get());

    // TBD
    pluginManager->loadPluginsFromDirectory((searchPath / "plugins").string());

    // Schedule onWindowFullyLoaded to run after current event processing is done
    QTimer::singleShot(10000, this, &AppUi::onWindowFullyLoaded);
}

void AppUi::onWindowFullyLoaded()
{
    initPSLangSupport();

    verifyApplicationVersion();
}

void AppUi::initPSLangSupport()
{
    const std::filesystem::path psLangSupportPath = api_context->searchPath / "PS.Bridge/Buraq.Bridge.exe";

    qDebug() << "PSLang Support: " << psLangSupportPath.string();

    emit updateStatusBar("PSLang Support..", 5000);

    m_bridgeProcess = new ManagedProcess(psLangSupportPath);

    if (!m_bridgeProcess->isRunning())
    {
        std::cerr << "Bridge process failed to start." << std::endl;
        emit updateStatusBar("PowerShell Support Failed.", 5000);
    }

    emit updateStatusBar("PSLang Support Ready", 30000);
}

void AppUi::verifyApplicationVersion()
{
    VersionRepository repo(api_context.get());

    if (const UpdateInfo update_info = repo.main_version_logic(); update_info.isConnFailure == false)
    {
        if (update_info.latestVersion.empty())
        {
            // No version was set
            qDebug() << "app version is empty or already upto date!";
            return;
        }

        VersionUpdateDialog versionUpdater(m_framelessWindow.get());
        versionUpdater.setWindowTitle(
            "A new version " + QString::fromStdString(update_info.latestVersion) + " is available!");
        versionUpdater.setContent(QString::fromStdString(update_info.releaseNotes));

        if (versionUpdater.exec() == QDialog::Accepted)
        {
            const std::filesystem::path installerExe = repo.downloadNewVersion();

            qDebug() << "AppUi.cpp";
            qDebug() << installerExe.string();
            qDebug() << (api_context->searchPath / "updater.exe").string();
            qDebug() << api_context->searchPath.parent_path().string();
            qDebug() << "ENDs AppUi.cpp";
            // Get the path to the AppData\Local folder
            PWSTR pszPath = NULL;
            if (const HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &pszPath); SUCCEEDED(hr))
            {
                // Convert the wide character string to a narrow character string (std::string)
                std::wstring wsPath(pszPath);
                std::string sPath(wsPath.begin(), wsPath.end());

                sPath += "\\Programs\\Buraq";

                // Print the resulting path
                std::cout << "The path is: " << sPath << std::endl;

                launchUpdaterAndExit(
                    api_context->searchPath / "updater.exe", installerExe, sPath
                );

                // Free the memory allocated by SHGetKnownFolderPath
                CoTaskMemFree(pszPath);
            }
            else
            {
                std::cerr << "Failed to get the path." << std::endl;
            }
            emit updateStatusBar("Ready.", 2000);
        }
        else
        {
            // New version was rejected or modal was closed.
            // do nothing
        }
    }
    else if (update_info.isConnFailure)
    {
        qDebug() << "Update failed to connect to github repository.";
        emit updateStatusBar("Failed to get updates. Check connection!", 10000);
    }
}

void AppUi::launchUpdaterAndExit(
    const std::filesystem::path& updaterPath,
    const std::filesystem::path& packagePath,
    const std::filesystem::path& installationPath
)
{
    std::string commandLine = "\"" + updaterPath.string() + "\"";
    commandLine += " \"" + packagePath.string() + "\"";
    commandLine += " \"" + installationPath.string() + "\"";
    commandLine += " " + std::to_string(GetCurrentProcessId()); // Pass current process ID

    qDebug() << "commandLine: " << commandLine;

    //updater.exe setup-x.x.x.dev.exe C:\Users\user\AppData\Local\Programs\Buraq\ 16572
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcessA(NULL, // No module name (use command line)
                       (LPSTR)commandLine.c_str(), // Command line
                       NULL, // Process handle not inheritable
                       NULL, // Thread handle not inheritable
                       FALSE, // Set handle inheritance to FALSE
                       CREATE_NEW_CONSOLE,
                       // Give the updater its own console. Prevents from closing after the main app is closed.
                       NULL, // Use parent's environment block
                       NULL, // Use parent's starting directory
                       &si, // Pointer to STARTUPINFO structure
                       &pi) // Pointer to PROCESS_INFORMATION structure
    )
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        std::cout << "Updater launched. Exiting main application." << std::endl;
        exit(0);
    }
    else
    {
        std::cerr << "Failed to launch updater. Error: " << GetLastError() << std::endl;
    }
}
