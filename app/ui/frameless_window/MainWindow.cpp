//
// Created by talik on 11/12/2025.
//

#include <QtGlobal>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QSplitter>
#include <QShortcut>
#include <QKeySequence>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QRegularExpression>

#include <QMessageBox>
#include "editor/Editor.h"
#include "ToolBar.h"
#include "MainWindow.h"
#include "CustomDrawer.h"
#include "Frame/Frame.h"
#include "terminal/TerminalPanel.h"
#include "output_display/OutputDisplay.h"
#include "../../database/db_conn.h"

MainWindow::MainWindow(QWidget* parent) : FramelessWindow(parent)
{
    m_terminalPanel = new TerminalPanel(this);
    m_editor = new Editor(this);
    m_drawer = new CustomDrawer(m_editor);

    const auto main_content_widget = m_Frame->getMainContentWidget();
    const auto contentArea = new QWidget(main_content_widget);
    main_content_widget->layout()->addWidget(contentArea);

    // 1. MAIN LAYOUT: A horizontal layout to separate the left controls from the main content.
    QHBoxLayout* mainLayout = new QHBoxLayout(contentArea);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 2. LEFT CONTROL PANEL: A vertical layout for the top and bottom buttons.
    const auto leftPanelLayout = m_Frame->getLeftSidePanelLayout();

    m_folderButton = new QPushButton("🗀", contentArea);
    m_folderButton->setObjectName("folderButton");
    m_folderButton->setFixedSize(35, 35);

    connect(m_folderButton, &QPushButton::clicked, this, &MainWindow::updateDrawer);

    m_outputButton = new QPushButton("❯_", contentArea);
    m_outputButton->setObjectName("outputConsoleButton");
    m_outputButton->setFixedSize(35, 35);

    connect(m_outputButton, &QPushButton::clicked, this, &MainWindow::onShowOutputButtonClicked);

    leftPanelLayout->addWidget(m_folderButton); // Add button to the top
    leftPanelLayout->addStretch(); // Pushes buttons to top and bottom
    leftPanelLayout->addWidget(m_outputButton); // Add button to the bottom

    // 3a. Top Area (Editor + Drawer)
    QWidget* topAreaWidget = new QWidget();
    QHBoxLayout* topAreaLayout = new QHBoxLayout(topAreaWidget);
    topAreaLayout->setSpacing(0);
    topAreaLayout->setContentsMargins(0, 0, 0, 0);

    // 3. MAIN VERTICAL SPLITTER: Separates the top (editor/drawer) from the bottom (output).
    rightSideSplitter = new QSplitter(Qt::Vertical);

    // 4. HORIZONTAL SPLITTER: This will go in the top section of the vertical splitter.
    // It separates the drawer from the editor.
    topAreaSplitter = new QSplitter(Qt::Horizontal);

    // Add the drawer and editor to the HORIZONTAL splitter
    topAreaSplitter->addWidget(m_drawer);
    topAreaSplitter->addWidget(m_editor);
    topAreaSplitter->setSizes({250, 750}); // Initial widths for drawer and editor

    // 5. BOTTOM AREA (Output)

    // 6. ASSEMBLE THE VERTICAL SPLITTER:
    // Add the horizontal splitter (as the top widget) and the terminal panel (as the bottom widget).
    rightSideSplitter->addWidget(topAreaSplitter);
    rightSideSplitter->addWidget(m_terminalPanel);
    rightSideSplitter->setSizes({500, 200}); // Initial heights for top and bottom sections

    // 7. ASSEMBLE THE MAIN LAYOUT
    mainLayout->addWidget(rightSideSplitter, 1); // The '1' stretch factor allows it to expand

    // 8. Connect ToolBar signals
    if (m_toolBar)
    {
        connect(m_toolBar, &ToolBar::newFileTriggered, this, &MainWindow::onNewFileTriggered);
        connect(m_toolBar, &ToolBar::openFileTriggered, this, &MainWindow::onOpenFileTriggered);
        connect(m_toolBar, &ToolBar::saveFileTriggered, this, &MainWindow::onSaveFileTriggered);

        m_toolBar->addBuildMenu();
        connect(m_toolBar, &ToolBar::buildProjectTriggered, this, &MainWindow::onBuildProjectTriggered);
        connect(m_toolBar, &ToolBar::runProjectTriggered, this, &MainWindow::onRunProjectTriggered);
    }

    // 9. Ctrl+` shortcut — toggle terminal panel
    auto* toggleShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_QuoteLeft), this);
    connect(toggleShortcut, &QShortcut::activated, m_terminalPanel, &TerminalPanel::toggle);

    // 10. Configure terminal shell preferences and active workspace path
    m_terminalPanel->setShell(userPreferences.shellPath, userPreferences.shellArgs);
    m_terminalPanel->setProjectDirectory(database::getWorkspacePath());

    // Connect workspace change signal from drawer to terminal panel
    connect(m_drawer, &CustomDrawer::workspaceChanged, m_terminalPanel, &TerminalPanel::setProjectDirectory);
}

MainWindow::~MainWindow() = default;

bool MainWindow::maybeSave()
{
    if (!m_editor || !m_editor->isDirty()) return true;

    const QMessageBox::StandardButton ret = QMessageBox::warning(this, "Buraq Editor",
                                "The document has been modified.\nDo you want to save your changes?",
                                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save)
    {
        return m_editor->saveFile();
    }

    if (ret == QMessageBox::Cancel)
    {
        return false;
    }

    return true;
}

void MainWindow::onNewFileTriggered()
{
    if (m_editor)
    {
        m_editor->clear();
        processStatusSlot("New file created.");
    }
}

void MainWindow::onOpenFileTriggered()
{
    if (m_drawer)
    {
        m_drawer->onAddFolderButtonClicked();
    }
}

void MainWindow::onSaveFileTriggered()
{
    if (m_editor)
    {
        m_editor->saveFile();
    }
}

void MainWindow::processResultSlot(const int exitCode, const QString& output, const QString& error) const
{
    if (m_terminalPanel == nullptr) return;

    m_terminalPanel->log(output, error);
    m_terminalPanel->showOutputTab();

    processStatusSlot(exitCode == 0
        ? (error.isEmpty() ? "Completed!" : "Completed with errors.")
        : "Process failed!");
}

void MainWindow::processStatusSlot(const QString& message, const int timeout) const
{
    if (m_statusBar)
    {
        m_statusBar->showMessage(message, timeout);
    }
}

void MainWindow::onShowOutputButtonClicked() const
{
    qDebug() << "Toggle bottom panel";
    if (m_terminalPanel)
        m_terminalPanel->toggle();
}

Editor* MainWindow::getEditor() const
{
    return m_editor;
}

OutputDisplay* MainWindow::outputDisplay() const
{
    return m_terminalPanel ? m_terminalPanel->outputDisplay() : nullptr;
}

void MainWindow::updateDrawer() const
{
    qDebug() << "Open or close Drawer";
    if (m_drawer == nullptr) return;

    if (m_drawer->isHidden())
    {
        m_drawer->show();
    }
    else
    {
        m_drawer->hide();
    }
}

void MainWindow::onApplySettingChanges()
{
    FramelessWindow::onApplySettingChanges();
    if (m_terminalPanel)
    {
        m_terminalPanel->setShell(userPreferences.shellPath, userPreferences.shellArgs);
    }
}

void MainWindow::onBuildProjectTriggered()
{
    buildOrRunProject(true);
}

void MainWindow::onRunProjectTriggered()
{
    buildOrRunProject(false);
}

QString MainWindow::findCMakeExecutable(const QString& checkDir, const QString& buildDir, const QString& baseName) const
{
    QStringList pathsToTry;
    pathsToTry << checkDir + "/" + buildDir + "/build";
    pathsToTry << checkDir + "/" + buildDir + "/app";
    pathsToTry << checkDir + "/" + buildDir;

    QStringList namesToTry;
    namesToTry << baseName;
    namesToTry << "buraq";

    for (const QString& dirPath : pathsToTry)
    {
        for (const QString& name : namesToTry)
        {
#if defined(Q_OS_WIN)
            QString exePath = dirPath + "/" + name + ".exe";
#else
            QString exePath = dirPath + "/" + name;
#endif
            if (QFileInfo::exists(exePath) && QFileInfo(exePath).isFile())
            {
                return QDir::toNativeSeparators(exePath);
            }
        }
    }

    for (const QString& dirPath : pathsToTry)
    {
        QDir dir(dirPath);
        if (dir.exists())
        {
#if defined(Q_OS_WIN)
            QStringList filters;
            filters << "*.exe";
            QStringList exes = dir.entryList(filters, QDir::Files);
            for (const QString& exe : exes)
            {
                if (exe.toLower() != "updater.exe")
                {
                    return QDir::toNativeSeparators(dirPath + "/" + exe);
                }
            }
#else
            QFileInfoList list = dir.entryInfoList(QDir::Files);
            for (const QFileInfo& fi : list)
            {
                if (fi.isExecutable())
                {
                    return QDir::toNativeSeparators(fi.absoluteFilePath());
                }
            }
#endif
        }
    }

#if defined(Q_OS_WIN)
    return QDir::toNativeSeparators(checkDir + "/" + buildDir + "/build/" + baseName + ".exe");
#else
    return QDir::toNativeSeparators(checkDir + "/" + buildDir + "/build/" + baseName);
#endif
}

QString MainWindow::findMakefileExecutable(const QString& checkDir, const QString& baseName) const
{
    QStringList pathsToTry;
    pathsToTry << checkDir;

    QStringList namesToTry;
    namesToTry << baseName;
    namesToTry << "buraq";

    for (const QString& dirPath : pathsToTry)
    {
        for (const QString& name : namesToTry)
        {
#if defined(Q_OS_WIN)
            QString exePath = dirPath + "/" + name + ".exe";
#else
            QString exePath = dirPath + "/" + name;
#endif
            if (QFileInfo::exists(exePath) && QFileInfo(exePath).isFile())
            {
                return QDir::toNativeSeparators(exePath);
            }
        }
    }

#if defined(Q_OS_WIN)
    return QDir::toNativeSeparators(checkDir + "/" + baseName + ".exe");
#else
    return QDir::toNativeSeparators(checkDir + "/" + baseName);
#endif
}

void MainWindow::buildOrRunProject(bool buildOnly)
{
    if (!m_editor)
    {
        return;
    }

    m_editor->saveFile();

    QString filePath = m_editor->currentFile();
    if (filePath.isEmpty())
    {
        processStatusSlot("Cannot build: No active file.");
        return;
    }

    QFileInfo fileInfo(filePath);
    QString ext = fileInfo.suffix().toLower();
    QString dir = QDir::toNativeSeparators(fileInfo.absolutePath());
    QString fileName = fileInfo.fileName();
    QString baseName = fileInfo.baseName();

    QString workspaceDir = database::getWorkspacePath();
    bool hasCMake = QFileInfo::exists(fileInfo.absolutePath() + "/CMakeLists.txt") ||
                    (!workspaceDir.isEmpty() && QFileInfo::exists(workspaceDir + "/CMakeLists.txt"));

    bool hasMakefile = QFileInfo::exists(fileInfo.absolutePath() + "/Makefile") ||
                       QFileInfo::exists(fileInfo.absolutePath() + "/makefile") ||
                       (!workspaceDir.isEmpty() && (QFileInfo::exists(workspaceDir + "/Makefile") || QFileInfo::exists(workspaceDir + "/makefile")));

    bool isCppSource = (ext == "cpp" || ext == "cxx" || ext == "cc" || ext == "c");
    bool isCppHeader = (ext == "h" || ext == "hpp" || ext == "hxx");

    if (!hasCMake && !hasMakefile && !isCppSource)
    {
        if (isCppHeader)
            processStatusSlot("Cannot compile a header file directly. Open a source file or configure CMake/Makefile.");
        else
            processStatusSlot("Active file is not a C/C++ source file.");
        return;
    }

    TerminalPanel* termPanel = terminalPanel();
    if (!termPanel)
    {
        return;
    }

    const UserSettings& prefs = getUserPreferences();
    QString shell = prefs.shellPath.toLower();

    // Query active build configuration (Debug or Release)
    QString config = "Debug";
    if (m_toolBar)
    {
        config = m_toolBar->activeBuildConfig();
    }

    bool isCmd = false;
    bool isBash = false;
    bool isPs = false;

#if defined(Q_OS_WIN)
    if (shell.contains("cmd"))
    {
        isCmd = true;
    }
    else if (shell.contains("bash") || shell.contains("sh") || shell.contains("zsh") || shell.contains("msys") || shell.contains("git"))
    {
        isBash = true;
    }
    else
    {
        isPs = true; // Default to PowerShell on Windows
    }
#else
    isBash = true; // Unix standard is POSIX/Bash
#endif

    QString buildCmd;
    QString runCmd;

    QString compiler = (ext == "c") ? "gcc" : "g++";

    if (hasCMake)
    {
        QString checkDir = workspaceDir.isEmpty() ? fileInfo.absolutePath() : workspaceDir;
        
        QString buildDir = (config == "Release") ? "build-release" : "build-debug";
        bool needsConfigure = true;

        QStringList candidateDirs;
        if (config == "Release")
        {
            candidateDirs = {
                "cmake-build-release-mingw",
                "cmake-build-release",
                "build-release",
                "build",
                "out"
            };
        }
        else
        {
            candidateDirs = {
                "cmake-build-debug-mingw",
                "cmake-build-debug",
                "build-debug",
                "build",
                "out"
            };
        }

        for (const QString& candidate : candidateDirs)
        {
            QString candidatePath = checkDir + "/" + candidate;
            if (QFileInfo::exists(candidatePath + "/CMakeCache.txt"))
            {
                // Check if build files actually exist (e.g. build.ninja, Makefile, makefile, or Visual Studio project/sln)
                bool hasBuildFile = QFileInfo::exists(candidatePath + "/build.ninja") ||
                                    QFileInfo::exists(candidatePath + "/Makefile") ||
                                    QFileInfo::exists(candidatePath + "/makefile") ||
                                    !QDir(candidatePath).entryList(QStringList() << "*.sln" << "*.vcxproj", QDir::Files).isEmpty();

                if (hasBuildFile)
                {
                    buildDir = candidate;
                    needsConfigure = false;

                    // Check if cached vcpkg triplet in CMakeCache.txt matches our desired one.
                    // If they differ, force re-configure.
                    QString targetTriplet = prefs.vcpkgTargetTriplet;
                    if (targetTriplet.isEmpty())
                        targetTriplet = detectVcpkgTriplet();

                    if (!targetTriplet.isEmpty())
                    {
                        QFile cacheFile(candidatePath + "/CMakeCache.txt");
                        if (cacheFile.open(QIODevice::ReadOnly | QIODevice::Text))
                        {
                            QString content = cacheFile.readAll();
                            cacheFile.close();
                            
                            QRegularExpression rx("VCPKG_TARGET_TRIPLET:[^=]*=([^\\r\\n]*)");
                            auto match = rx.match(content);
                            if (match.hasMatch())
                            {
                                QString cachedTriplet = match.captured(1).trimmed();
                                if (cachedTriplet != targetTriplet)
                                {
                                    needsConfigure = true;
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }

        if (needsConfigure)
        {
            QString buildTypeArg = QString("-DCMAKE_BUILD_TYPE=%1").arg(config);
            QString toolchainArg;
            
            // Prioritize user settings configuration
            QString vcpkgPath = prefs.vcpkgToolchainPath;
            
            if (vcpkgPath.isEmpty())
            {
                // Fall back to auto-detection lookup
                QString envVcpkgRoot = qgetenv("VCPKG_ROOT");
                if (!envVcpkgRoot.isEmpty() && QFileInfo::exists(envVcpkgRoot + "/scripts/buildsystems/vcpkg.cmake"))
                {
                    vcpkgPath = QDir::toNativeSeparators(envVcpkgRoot + "/scripts/buildsystems/vcpkg.cmake");
                }
                else if (QFileInfo::exists("C:/vcpkg/scripts/buildsystems/vcpkg.cmake"))
                {
                    vcpkgPath = QDir::toNativeSeparators("C:/vcpkg/scripts/buildsystems/vcpkg.cmake");
                }
                else if (QFileInfo::exists("/usr/local/share/vcpkg/scripts/buildsystems/vcpkg.cmake"))
                {
                    vcpkgPath = "/usr/local/share/vcpkg/scripts/buildsystems/vcpkg.cmake";
                }
                else if (QFileInfo::exists(QDir::homePath() + "/vcpkg/scripts/buildsystems/vcpkg.cmake"))
                {
                    vcpkgPath = QDir::toNativeSeparators(QDir::homePath() + "/vcpkg/scripts/buildsystems/vcpkg.cmake");
                }
            }

            if (!vcpkgPath.isEmpty())
            {
                toolchainArg = QString(" -DCMAKE_TOOLCHAIN_FILE=\"%1\"").arg(isBash ? QDir::fromNativeSeparators(vcpkgPath) : QDir::toNativeSeparators(vcpkgPath));
            }

            QString triplet = prefs.vcpkgTargetTriplet;
            if (triplet.isEmpty())
            {
                triplet = detectVcpkgTriplet();
            }
            QString tripletArg;
            if (!triplet.isEmpty())
            {
                tripletArg = QString(" -DVCPKG_TARGET_TRIPLET=%1").arg(triplet);
            }

            QString generatorArg;
#if defined(Q_OS_WIN)
            if (triplet == "x64-mingw-dynamic")
            {
                generatorArg = " -G \"MinGW Makefiles\"";
            }
#endif

            if (isCmd)
                buildCmd = QString("cmake -B %1 %2%3%4%5 < NUL && cmake --build %1 < NUL").arg(buildDir, buildTypeArg, toolchainArg, tripletArg, generatorArg);
            else if (isPs)
                buildCmd = QString("$null | cmake -B %1 %2%3%4%5; if ($?) { $null | cmake --build %1 }").arg(buildDir, buildTypeArg, toolchainArg, tripletArg, generatorArg);
            else // isBash / POSIX
                buildCmd = QString("cmake -B %1 %2%3%4%5 && cmake --build %1").arg(buildDir, buildTypeArg, toolchainArg, tripletArg, generatorArg);
        }
        else
        {
            QString buildPath = checkDir + "/" + buildDir;
            if (isCmd)
                buildCmd = QString("cmake --build \"%1\" < NUL").arg(QDir::toNativeSeparators(buildPath));
            else if (isPs)
                buildCmd = QString("$null | cmake --build \"%1\"").arg(QDir::toNativeSeparators(buildPath));
            else // isBash
                buildCmd = QString("cmake --build \"%1\"").arg(buildPath);
        }

        if (!buildOnly)
        {
            runCmd = findCMakeExecutable(checkDir, buildDir, baseName);
        }
    }
    else if (hasMakefile)
    {
        QString checkDir = workspaceDir.isEmpty() ? fileInfo.absolutePath() : workspaceDir;
        if (isCmd)
            buildCmd = QString("cd /d \"%1\" && mingw32-make < NUL").arg(QDir::toNativeSeparators(checkDir));
        else if (isPs)
            buildCmd = QString("cd \"%1\"; $null | mingw32-make").arg(QDir::toNativeSeparators(checkDir));
        else // isBash
        {
#if defined(Q_OS_WIN)
            buildCmd = QString("cd \"%1\" && mingw32-make").arg(checkDir);
#else
            buildCmd = QString("cd \"%1\" && make").arg(checkDir);
#endif
        }

        if (!buildOnly)
        {
            runCmd = findMakefileExecutable(checkDir, baseName);
        }
    }
    else
    {
        // Single file compilation
        QString flags = (config == "Release") ? "-O3" : "-g";
        QString outputName = (config == "Release") ? (baseName + "-release") : (baseName + "-debug");
        
        if (isCmd)
        {
            buildCmd = QString("cd /d \"%1\" && %2 \"%3\" %4 -o \"%5.exe\"").arg(dir, compiler, fileName, flags, outputName);
            if (!buildOnly)
                runCmd = QDir::toNativeSeparators(dir + "/" + outputName + ".exe");
        }
        else if (isPs)
        {
            buildCmd = QString("cd \"%1\"; %2 \"%3\" %4 -o \"%5.exe\"").arg(QDir::toNativeSeparators(dir), compiler, fileName, flags, outputName);
            if (!buildOnly)
                runCmd = QDir::toNativeSeparators(dir + "/" + outputName + ".exe");
        }
        else // isBash
        {
#if defined(Q_OS_WIN)
            buildCmd = QString("cd \"%1\" && %2 \"%3\" %4 -o \"%5.exe\"").arg(dir, compiler, fileName, flags, outputName);
            if (!buildOnly)
                runCmd = dir + "/" + outputName + ".exe";
#else
            buildCmd = QString("cd \"%1\" && %2 \"%3\" %4 -o \"%5\"").arg(dir, compiler, fileName, flags, outputName);
            if (!buildOnly)
                runCmd = dir + "/" + outputName;
#endif
        }
    }

    QString finalCmd;
    if (buildOnly || runCmd.isEmpty())
    {
        finalCmd = buildCmd;
    }
    else
    {
        if (isCmd)
            finalCmd = QString("%1 && \"%2\"").arg(buildCmd, runCmd);
        else if (isPs)
            finalCmd = QString("%1; if ($?) { & \"%2\" }").arg(buildCmd, runCmd);
        else // isBash
            finalCmd = QString("%1 && \"%2\"").arg(buildCmd, runCmd);
    }

    if (!finalCmd.isEmpty())
    {
        processStatusSlot(buildOnly ? QString("Building project (%1)...").arg(config) : QString("Building and running project (%1)...").arg(config));
        termPanel->executeCommand(finalCmd);
    }
}

QString MainWindow::detectVcpkgTriplet() const
{
#if defined(Q_OS_WIN)
    // On Windows, check if we are using MinGW.
    // If gcc/g++ or any folder in the PATH contains mingw, we default to x64-mingw-dynamic.
    QString path = qgetenv("PATH");
    QStringList paths = path.split(';');
    for (const QString& p : paths)
    {
        if (QFileInfo::exists(p + "/g++.exe") || QFileInfo::exists(p + "/gcc.exe") || p.toLower().contains("mingw"))
        {
            return "x64-mingw-dynamic";
        }
    }
    return "x64-windows";
#else
    return ""; // On macOS/Linux, vcpkg will automatically resolve to standard host triplets.
#endif
}
