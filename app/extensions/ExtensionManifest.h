#ifndef EXTENSION_MANIFEST_H
#define EXTENSION_MANIFEST_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <optional>

struct ExtensionLanguage
{
    QString id;
    QString name;
    QStringList extensions;
    QString highlighter; // e.g. "powershell", "cpp", "generic"
    QString icon;        // e.g. ":/icons/languages/powershell.svg"
};

struct ExtensionRunner
{
    QString command;
    QString windowsCommand;
    QString workingDirectory;
    QString action; // e.g. "build_menu" or empty for terminal command
    QString description;
};

struct ExtensionManifest
{
    QString id;
    QString name;
    QString version;
    QString description;
    QString author;
    QString icon;
    QList<ExtensionLanguage> languages;
    ExtensionRunner runner;
    QString directoryPath;
    bool isBuiltIn = false;
    bool isEnabled = true;

    static std::optional<ExtensionManifest> fromJson(const QByteArray& json, const QString& dirPath, bool isBuiltIn = false)
    {
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(json, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject())
        {
            return std::nullopt;
        }

        const QJsonObject obj = doc.object();
        ExtensionManifest manifest;
        manifest.id = obj.value("id").toString();
        manifest.name = obj.value("name").toString();
        manifest.version = obj.value("version").toString("1.0.0");
        manifest.description = obj.value("description").toString();
        manifest.author = obj.value("author").toString("Unknown");
        manifest.icon = obj.value("icon").toString();
        manifest.directoryPath = dirPath;
        manifest.isBuiltIn = isBuiltIn;
        manifest.isEnabled = true;

        if (manifest.id.isEmpty() || manifest.name.isEmpty())
        {
            return std::nullopt;
        }

        // Parse languages
        const QJsonArray langs = obj.value("languages").toArray();
        for (const auto& langVal : langs)
        {
            if (langVal.isObject())
            {
                const QJsonObject langObj = langVal.toObject();
                ExtensionLanguage lang;
                lang.id = langObj.value("id").toString();
                lang.name = langObj.value("name").toString(lang.id);
                lang.highlighter = langObj.value("highlighter").toString();
                lang.icon = langObj.value("icon").toString();

                const QJsonArray extArr = langObj.value("extensions").toArray();
                for (const auto& extVal : extArr)
                {
                    QString ext = extVal.toString().trimmed();
                    if (ext.startsWith('.'))
                    {
                        ext = ext.mid(1);
                    }
                    if (!ext.isEmpty())
                    {
                        lang.extensions.append(ext.toLower());
                    }
                }
                manifest.languages.append(lang);
            }
        }

        // Parse runner
        if (obj.contains("runner") && obj.value("runner").isObject())
        {
            const QJsonObject runnerObj = obj.value("runner").toObject();
            manifest.runner.command = runnerObj.value("command").toString();
            manifest.runner.windowsCommand = runnerObj.value("windowsCommand").toString();
            manifest.runner.workingDirectory = runnerObj.value("workingDirectory").toString();
            manifest.runner.action = runnerObj.value("action").toString();
            manifest.runner.description = runnerObj.value("description").toString();
        }

        return manifest;
    }
};

#endif // EXTENSION_MANIFEST_H
