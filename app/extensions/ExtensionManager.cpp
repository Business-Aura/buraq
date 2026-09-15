#include "ExtensionManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>
#include <utility>

#include "buraq.h"
#include "editor/LanguageProvider.h"
#include "editor/CppHighlighter.h"
#include "editor/PowerShellHighlighter.h"

ExtensionManager& ExtensionManager::instance()
{
    static ExtensionManager mgr;
    return mgr;
}

ExtensionManager::ExtensionManager() = default;

void ExtensionManager::initialize(buraq::buraq_api* apiContext)
{
    m_apiContext = apiContext;
    reloadExtensions();
}

QString ExtensionManager::getBuiltInExtensionsDir() const
{
    return QDir(QCoreApplication::applicationDirPath()).filePath("extensions");
}

QString ExtensionManager::getUserExtensionsDir() const
{
    if (m_apiContext && !m_apiContext->userDataPath.empty())
    {
        return QDir(QString::fromStdString(m_apiContext->userDataPath.string())).filePath("extensions");
    }

    const QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(appData).filePath(".data/extensions");
}

void ExtensionManager::reloadExtensions()
{
    m_extensions.clear();
    m_extensionToExtensionIdMap.clear();

    // 1. Scan built-in extensions directory (e.g. <app_dir>/extensions/)
    scanDirectory(getBuiltInExtensionsDir(), true);

    // Also scan source tree extensions folder during development if available
    const QString devExtDir = QDir(QCoreApplication::applicationDirPath()).filePath("../../../extensions");
    if (QFileInfo::exists(devExtDir))
    {
        scanDirectory(QDir(devExtDir).canonicalPath(), true);
    }

    // 2. Scan user extensions directory (e.g. %LOCALAPPDATA%/Buraq/.data/extensions/)
    const QString userExtDir = getUserExtensionsDir();
    QDir().mkpath(userExtDir);
    scanDirectory(userExtDir, false);

    qDebug() << "ExtensionManager loaded" << m_extensions.size() << "extensions.";
    emit extensionsChanged();
}

void ExtensionManager::scanDirectory(const QString& directoryPath, const bool isBuiltIn)
{
    const QDir dir(directoryPath);
    if (!dir.exists())
    {
        return;
    }

    const QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const auto& subdir : subdirs)
    {
        const QString manifestPath = subdir.filePath() + "/extension.json";
        if (QFile::exists(manifestPath))
        {
            QFile file(manifestPath);
            if (file.open(QIODevice::ReadOnly))
            {
                const QByteArray content = file.readAll();
                file.close();

                auto manifestOpt = ExtensionManifest::fromJson(content, subdir.absoluteFilePath(), isBuiltIn);
                if (manifestOpt.has_value())
                {
                    ExtensionManifest manifest = manifestOpt.value();
                    m_extensions[manifest.id] = manifest;
                    registerExtensionLanguages(manifest);
                    qDebug() << "Loaded extension:" << manifest.name << "(" << manifest.id << ")"
                             << "v" << manifest.version << (isBuiltIn ? "[Built-in]" : "[User]");
                }
                else
                {
                    qWarning() << "Failed to parse extension manifest at:" << manifestPath;
                }
            }
        }
    }
}

void ExtensionManager::registerExtensionLanguages(const ExtensionManifest& manifest)
{
    for (const auto& lang : manifest.languages)
    {
        // Map extensions to this extension ID for quick runner lookups
        for (const auto& ext : lang.extensions)
        {
            m_extensionToExtensionIdMap[ext.toLower()] = manifest.id;
        }

        // Register syntax highlighter with LanguageRegistry
        std::function<QSyntaxHighlighter*(QTextDocument*)> factory;
        if (lang.highlighter.toLower() == "powershell")
        {
            factory = [](QTextDocument* doc) -> QSyntaxHighlighter* {
                return new PowerShellHighlighter(doc);
            };
        }
        else if (lang.highlighter.toLower() == "cpp")
        {
            factory = [](QTextDocument* doc) -> QSyntaxHighlighter* {
                return new CppHighlighter(doc);
            };
        }

        if (factory)
        {
            auto provider = std::make_shared<GenericLanguageProvider>(lang.name, lang.extensions, factory);
            LanguageRegistry::instance().registerProvider(provider);
        }
    }
}

QList<ExtensionManifest> ExtensionManager::getInstalledExtensions() const
{
    return m_extensions.values();
}

std::optional<ExtensionManifest> ExtensionManager::getExtension(const QString& id) const
{
    if (m_extensions.contains(id))
    {
        return m_extensions.value(id);
    }
    return std::nullopt;
}

QString ExtensionManager::getRunCommand(const QString& filePath, const QString& activeShell) const
{
    if (filePath.isEmpty()) return {};

    const QFileInfo fileInfo(filePath);
    const QString ext = fileInfo.suffix().toLower();

    if (!m_extensionToExtensionIdMap.contains(ext))
    {
        return {};
    }

    const QString extensionId = m_extensionToExtensionIdMap.value(ext);
    if (!m_extensions.contains(extensionId))
    {
        return {};
    }

    const ExtensionManifest& manifest = m_extensions[extensionId];
    const ExtensionRunner& runner = manifest.runner;

    QString templateCmd;
    const QString shellLower = activeShell.toLower();

#if defined(Q_OS_WIN)
    if (!runner.windowsCommand.isEmpty() && (shellLower.contains("powershell") || shellLower.contains("pwsh")))
    {
        templateCmd = runner.windowsCommand;
    }
    else
#endif
    {
        templateCmd = runner.command;
    }

    if (templateCmd.isEmpty())
    {
        return {};
    }

    const QString nativeFilePath = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
    const QString nativeFileDir = QDir::toNativeSeparators(fileInfo.absolutePath());

    templateCmd.replace("${file}", nativeFilePath);
    templateCmd.replace("${fileDir}", nativeFileDir);
    templateCmd.replace("${fileName}", fileInfo.fileName());
    templateCmd.replace("${fileBaseName}", fileInfo.baseName());

    return templateCmd;
}

QString ExtensionManager::getActionForFile(const QString& filePath) const
{
    if (filePath.isEmpty()) return {};

    const QFileInfo fileInfo(filePath);
    const QString ext = fileInfo.suffix().toLower();

    if (!m_extensionToExtensionIdMap.contains(ext))
    {
        return {};
    }

    const QString extensionId = m_extensionToExtensionIdMap.value(ext);
    if (!m_extensions.contains(extensionId))
    {
        return {};
    }

    return m_extensions[extensionId].runner.action;
}

static bool copyDirectoryRecursively(const QString& source, const QString& destination)
{
    const QDir srcDir(source);
    if (!srcDir.exists()) return false;

    QDir().mkpath(destination);
    const QDir destDir(destination);

    for (const auto& file : srcDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
    {
        const QString destFilePath = destDir.filePath(file.fileName());
        if (QFile::exists(destFilePath))
        {
            QFile::remove(destFilePath);
        }
        if (!QFile::copy(file.absoluteFilePath(), destFilePath))
        {
            return false;
        }
    }

    for (const auto& subDir : srcDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        const QString destSubDirPath = destDir.filePath(subDir.fileName());
        if (!copyDirectoryRecursively(subDir.absoluteFilePath(), destSubDirPath))
        {
            return false;
        }
    }

    return true;
}

bool ExtensionManager::installFromFolder(const QString& sourceFolderPath, QString& errorMessage)
{
    const QDir sourceDir(sourceFolderPath);
    if (!sourceDir.exists())
    {
        errorMessage = "Source directory does not exist: " + sourceFolderPath;
        return false;
    }

    const QString manifestPath = sourceDir.filePath("extension.json");
    if (!QFile::exists(manifestPath))
    {
        errorMessage = "extension.json manifest not found in: " + sourceFolderPath;
        return false;
    }

    QFile file(manifestPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        errorMessage = "Could not open extension.json for reading.";
        return false;
    }

    const QByteArray content = file.readAll();
    file.close();

    const auto manifestOpt = ExtensionManifest::fromJson(content, sourceFolderPath, false);
    if (!manifestOpt.has_value())
    {
        errorMessage = "Invalid extension.json format. Missing 'id' or 'name'.";
        return false;
    }

    const ExtensionManifest& manifest = manifestOpt.value();
    const QString targetDir = QDir(getUserExtensionsDir()).filePath(manifest.id);

    if (QDir(targetDir).exists())
    {
        QDir(targetDir).removeRecursively();
    }

    if (!copyDirectoryRecursively(sourceFolderPath, targetDir))
    {
        errorMessage = "Failed to copy extension files to user extensions directory.";
        return false;
    }

    reloadExtensions();
    emit extensionInstalled(manifest.id);
    return true;
}

bool ExtensionManager::uninstallExtension(const QString& extensionId, QString& errorMessage)
{
    if (!m_extensions.contains(extensionId))
    {
        errorMessage = "Extension '" + extensionId + "' is not installed.";
        return false;
    }

    const ExtensionManifest& manifest = m_extensions[extensionId];
    if (manifest.isBuiltIn)
    {
        errorMessage = "Cannot uninstall built-in extension '" + manifest.name + "'.";
        return false;
    }

    QDir targetDir(manifest.directoryPath);
    if (!targetDir.removeRecursively())
    {
        errorMessage = "Failed to remove directory: " + manifest.directoryPath;
        return false;
    }

    reloadExtensions();
    emit extensionUninstalled(extensionId);
    return true;
}
