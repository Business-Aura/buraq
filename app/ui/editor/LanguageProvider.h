#ifndef LANGUAGE_PROVIDER_H
#define LANGUAGE_PROVIDER_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QTextDocument>
#include <QSyntaxHighlighter>
#include <memory>

#include "CppHighlighter.h"
#include "PowerShellHighlighter.h"

// 1. The Abstract Language Provider Interface
class ILanguageProvider {
public:
    virtual ~ILanguageProvider() = default;
    virtual QString getLanguageName() const = 0;
    virtual QStringList getSupportedExtensions() const = 0;
    virtual QSyntaxHighlighter* createHighlighter(QTextDocument* document) const = 0;
};

// 2. Concrete Implementations
class GenericLanguageProvider : public ILanguageProvider {
public:
    GenericLanguageProvider(QString name, QStringList extensions, std::function<QSyntaxHighlighter*(QTextDocument*)> factory)
        : m_name(std::move(name)), m_extensions(std::move(extensions)), m_factory(std::move(factory)) {}

    QString getLanguageName() const override { return m_name; }
    QStringList getSupportedExtensions() const override { return m_extensions; }
    QSyntaxHighlighter* createHighlighter(QTextDocument* document) const override {
        if (m_factory) {
            return m_factory(document);
        }
        return nullptr;
    }

private:
    QString m_name;
    QStringList m_extensions;
    std::function<QSyntaxHighlighter*(QTextDocument*)> m_factory;
};

class CppLanguageProvider : public ILanguageProvider {
public:
    QString getLanguageName() const override { return "C++"; }
    QStringList getSupportedExtensions() const override { return {"cpp", "h", "hpp", "cxx", "hxx", "c"}; }
    QSyntaxHighlighter* createHighlighter(QTextDocument* document) const override {
        return new CppHighlighter(document);
    }
};

class PowerShellLanguageProvider : public ILanguageProvider {
public:
    QString getLanguageName() const override { return "PowerShell"; }
    QStringList getSupportedExtensions() const override { return {"ps1", "psm1", "psd1"}; }
    QSyntaxHighlighter* createHighlighter(QTextDocument* document) const override {
        return new PowerShellHighlighter(document);
    }
};


// 3. The Language Registry
class LanguageRegistry {
public:
    static LanguageRegistry& instance() {
        static LanguageRegistry reg;
        return reg;
    }

    void registerProvider(std::shared_ptr<ILanguageProvider> provider) {
        if (!provider) return;
        const QStringList extensions = provider->getSupportedExtensions();
        for (const QString& ext : extensions) {
            m_extensionMap[ext.toLower()] = provider;
        }
    }

    void unregisterProviderForExtensions(const QStringList& extensions) {
        for (const QString& ext : extensions) {
            m_extensionMap.remove(ext.toLower());
        }
    }

    ILanguageProvider* getProviderForExtension(const QString& extension) const {
        const QString lowerExt = extension.toLower();
        if (m_extensionMap.contains(lowerExt)) {
            return m_extensionMap[lowerExt].get();
        }
        return nullptr;
    }

private:
    LanguageRegistry() {
        // Register default built-in providers
        registerProvider(std::make_unique<CppLanguageProvider>());
        registerProvider(std::make_unique<PowerShellLanguageProvider>());
    }
    ~LanguageRegistry() = default;
    LanguageRegistry(const LanguageRegistry&) = delete;
    LanguageRegistry& operator=(const LanguageRegistry&) = delete;

    QMap<QString, std::shared_ptr<ILanguageProvider>> m_extensionMap;
};

#endif // LANGUAGE_PROVIDER_H
