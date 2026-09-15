#ifndef EXTENSION_MANAGER_H
#define EXTENSION_MANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include <QIcon>
#include <memory>
#include <optional>

#include "ExtensionManifest.h"

namespace buraq
{
    struct buraq_api;
}

class ExtensionManager final : public QObject
{
    Q_OBJECT

public:
    static ExtensionManager& instance();

    void initialize(buraq::buraq_api* apiContext = nullptr);
    void reloadExtensions();

    [[nodiscard]] QList<ExtensionManifest> getInstalledExtensions() const;
    [[nodiscard]] std::optional<ExtensionManifest> getExtension(const QString& id) const;

    [[nodiscard]] QString getRunCommand(const QString& filePath, const QString& activeShell) const;
    [[nodiscard]] QString getActionForFile(const QString& filePath) const;

    [[nodiscard]] QIcon getIconForFile(const QString& filePath) const;
    [[nodiscard]] QIcon getIconForExtension(const QString& extension) const;

    bool installFromFolder(const QString& sourceFolderPath, QString& errorMessage);
    bool uninstallExtension(const QString& extensionId, QString& errorMessage);

    [[nodiscard]] QString getUserExtensionsDir() const;
    [[nodiscard]] QString getBuiltInExtensionsDir() const;

signals:
    void extensionsChanged();
    void extensionInstalled(const QString& id);
    void extensionUninstalled(const QString& id);

private:
    ExtensionManager();
    ~ExtensionManager() override = default;
    ExtensionManager(const ExtensionManager&) = delete;
    ExtensionManager& operator=(const ExtensionManager&) = delete;

    void scanDirectory(const QString& directoryPath, bool isBuiltIn);
    void registerExtensionLanguages(const ExtensionManifest& manifest);

    buraq::buraq_api* m_apiContext{nullptr};
    QMap<QString, ExtensionManifest> m_extensions;
    QMap<QString, QString> m_extensionToExtensionIdMap; // Maps file extension (e.g. "ps1") -> extension id
    QMap<QString, QString> m_extensionToIconPathMap;    // Maps file extension (e.g. "ps1") -> icon resource/path
    mutable QMap<QString, QIcon> m_iconCache;
};

#endif // EXTENSION_MANAGER_H
