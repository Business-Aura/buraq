//
// Created by talik on 8/30/2025.
//

#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <qsettings.h>
#include <QGraphicsDropShadowEffect>

#include "Config.h"
#include "Filters/Toolbar/ToolBarEvent.h"
#include "Frame/Frame.h"
#include "settings/SettingManager/SettingsManager.h"
#include "settings/UserSettings.h"
#include "extensions/ExtensionManager.h"

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent),
      settingsManager(new SettingsManager()),
      themeManager(ThemeManager::instance())
{
    setWindowTitle("Settings");
    setWindowIcon(QIcon(":/icons/buraq.png"));
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    m_Frame = new Frame(this, false); // Create the frame, no toolbar needed
    m_Frame->setObjectName("SettingsDialogFrame");
    m_Frame->setAttribute(Qt::WA_TranslucentBackground);
    m_Frame->getTitleLabel()->setText("Settings");
    m_Frame->getTitleLabel()->setStyleSheet("background: transparent; color: #cccccc; font-weight: 600;");

    // Hide toolkit bar, side panels, and bottom panel in SettingsDialog to reclaim space
    m_Frame->getToolKitBar()->hide();
    m_Frame->getLeftSidePanelLayout()->parentWidget()->hide();
    m_Frame->getRightSidePanelLayout()->parentWidget()->hide();
    m_Frame->getBottomPanelLayout()->parentWidget()->hide();

    // --- Main Dialog Layout ---
    // The dialog itself needs a layout. We will add our Frame to this layout.
    QVBoxLayout* dialogLayout = new QVBoxLayout(this);
    dialogLayout->setContentsMargins(14, 14, 14, 14); // Margins for elevation drop shadow
    dialogLayout->addWidget(m_Frame); // Add the frame to the dialog's layout
    setLayout(dialogLayout);

    // Set object name for styling and resize to a professional size
    setObjectName("SettingsDialog");
    resize(808, 588); // 780x560 content + 28px margins for drop shadow

    // Drop shadow effect for elevation and edge differentiation
    auto* shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(28);
    shadowEffect->setColor(QColor(0, 0, 0, 180));
    shadowEffect->setOffset(0, 4);
    m_Frame->setGraphicsEffect(shadowEffect);

    // Load user preferences
    userPreference = SettingsManager::loadSettings();

    // Get the main content area from the Frame to add our settings content
    QWidget* mainContentWidget = m_Frame->getMainContentWidget();
    QVBoxLayout* mainContentLayout = m_Frame->getMainLayout();
    
    // Set 0 top/left/right margins so the tab header and divider line span 100% full width
    mainContentLayout->setContentsMargins(0, 0, 0, 12);
    mainContentLayout->setSpacing(8);

    // --- Main Tab Widget ---
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setObjectName("SettingsTabWidget");

    m_tabWidget->addTab(createAppearancePage(), "Appearance");
    m_tabWidget->addTab(createEditorPage(), "Editor");
    m_tabWidget->addTab(createTerminalPage(), "Terminal");
    m_tabWidget->addTab(createBuildPage(), "Build");
    m_tabWidget->addTab(createExtensionsPage(), "Extensions");
    m_tabWidget->addTab(createAccountPage(), "Account");

    mainContentLayout->addWidget(m_tabWidget, 1);

    // --- Button Box with aligned padding ---
    auto* buttonContainer = new QWidget(this);
    auto* buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(20, 4, 20, 4);
    buttonLayout->setSpacing(0);
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply, this);
    buttonLayout->addWidget(m_buttonBox);
    mainContentLayout->addWidget(buttonContainer, 0);

    // Connect signals
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);

    QPushButton* applyButton = m_buttonBox->button(QDialogButtonBox::Apply);
    if (applyButton) {
        connect(applyButton, &QPushButton::clicked, this, &SettingsDialog::applyChanges);
    }
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::accept()
{
    applyChanges();
    QDialog::accept();
}

void SettingsDialog::applyChanges()
{
    // Read shell settings from UI widgets
    if (m_shellComboBox)
    {
        const int idx = m_shellComboBox->currentIndex();
#if defined(Q_OS_WIN)
        if (idx == 0)        userPreference.shellPath = "powershell.exe";
        else if (idx == 1)   userPreference.shellPath = "cmd.exe";
#elif defined(Q_OS_MAC)
        if (idx == 0)        userPreference.shellPath = "/bin/zsh";
        else if (idx == 1)   userPreference.shellPath = "/bin/bash";
#else
        if (idx == 0)        userPreference.shellPath = "/bin/bash";
        else if (idx == 1)   userPreference.shellPath = "/bin/zsh";
#endif
        else if (m_customShellEdit)
            userPreference.shellPath = m_customShellEdit->text().trimmed();
    }
    if (m_shellArgsEdit)
        userPreference.shellArgs = m_shellArgsEdit->text().trimmed();
    if (m_vcpkgToolchainEdit)
        userPreference.vcpkgToolchainPath = m_vcpkgToolchainEdit->text().trimmed();
    if (m_vcpkgTripletEdit)
        userPreference.vcpkgTargetTriplet = m_vcpkgTripletEdit->text().trimmed();

    SettingsManager::saveSettings(userPreference);
    themeManager.setAppTheme(userPreference.theme);
    emit applySettingChanges();
}

void SettingsDialog::setTheme(const int index)
{
    userPreference.theme = static_cast<AppTheme>(index);
}

QWidget* SettingsDialog::createAppearancePage()
{
    QWidget* pageWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(pageWidget);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(12);

    // Theme Selection
    QGroupBox* themeGroup = new QGroupBox("Theme", this);
    QFormLayout* themeLayout = new QFormLayout(themeGroup);

    QComboBox* themeComboBox = new QComboBox(this);
    themeComboBox->addItem("Light");
    themeComboBox->addItem("Dark");
    themeComboBox->addItem("System Default");
    themeComboBox->setCurrentIndex(static_cast<int>(userPreference.theme));

    connect(themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::setTheme);

    themeLayout->addRow("Application Theme:", themeComboBox);
    layout->addWidget(themeGroup);

    // Typography
    QGroupBox* typographyGroup = new QGroupBox("Typography", this);
    QFormLayout* typographyLayout = new QFormLayout(typographyGroup);

    QComboBox* fontComboBox = new QComboBox(this);
    fontComboBox->addItem("Segoe UI");
    fontComboBox->addItem("Consolas");
    fontComboBox->addItem("Arial");
    // fontComboBox->setCurrentText(userPreference.fontFamily);

    QSpinBox* fontSizeSpinBox = new QSpinBox(this);
    fontSizeSpinBox->setRange(8, 72);
    fontSizeSpinBox->setSuffix(" pt");
    fontSizeSpinBox->setValue(userPreference.editorFontSize);

    typographyLayout->addRow("Font:", fontComboBox);
    typographyLayout->addRow("Font Size:", fontSizeSpinBox);
    layout->addWidget(typographyGroup);

    layout->addStretch();
    return pageWidget;
}

QWidget* SettingsDialog::createEditorPage()
{
    QWidget* pageWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(pageWidget);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(12);

    QCheckBox* autoSaveCheckBox = new QCheckBox("Enable Auto-save", this);
    autoSaveCheckBox->setChecked(true);

    QSpinBox* autoSaveInterval = new QSpinBox(this);
    autoSaveInterval->setRange(1, 60);
    autoSaveInterval->setSuffix(" minutes");
    autoSaveInterval->setValue(5);

    layout->addWidget(autoSaveCheckBox);
    layout->addWidget(new QLabel("Auto-save interval:", this));
    layout->addWidget(autoSaveInterval);
    layout->addStretch();

    pageWidget->setLayout(layout);
    return pageWidget;
}

QWidget* SettingsDialog::createAccountPage()
{
    QWidget* pageWidget = new QWidget(this);
    QFormLayout* layout = new QFormLayout(pageWidget);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(12);

    QLineEdit* apiKeyLineEdit = new QLineEdit(this);
    apiKeyLineEdit->setPlaceholderText("Enter your API Key here");
    apiKeyLineEdit->setEchoMode(QLineEdit::Password);

    QPushButton* loginButton = new QPushButton("Connect Account", this);

    layout->addRow("API Key:", apiKeyLineEdit);
    layout->addRow("", loginButton);

    pageWidget->setLayout(layout);
    return pageWidget;
}

QWidget* SettingsDialog::createTerminalPage()
{
    QWidget* pageWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(pageWidget);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(12);

    // ── Shell Group ──
    QGroupBox* shellGroup = new QGroupBox("Shell", this);
    QFormLayout* shellLayout = new QFormLayout(shellGroup);

    m_shellComboBox = new QComboBox(this);
#if defined(Q_OS_WIN)
    m_shellComboBox->addItem("PowerShell  (powershell.exe)");
    m_shellComboBox->addItem("Command Prompt  (cmd.exe)");
    m_shellComboBox->addItem("Custom...");

    // Pre-select based on saved setting
    const QString saved = userPreference.shellPath.toLower();
    if (saved.contains("cmd"))           m_shellComboBox->setCurrentIndex(1);
    else if (!saved.contains("powershell") && !saved.contains("pwsh"))
                                          m_shellComboBox->setCurrentIndex(2);
    else                                  m_shellComboBox->setCurrentIndex(0);
#elif defined(Q_OS_MAC)
    m_shellComboBox->addItem("Zsh  (/bin/zsh)");
    m_shellComboBox->addItem("Bash  (/bin/bash)");
    m_shellComboBox->addItem("Custom...");

    // Pre-select based on saved setting
    const QString saved = userPreference.shellPath.toLower();
    if (saved.contains("bash"))          m_shellComboBox->setCurrentIndex(1);
    else if (!saved.contains("zsh"))     m_shellComboBox->setCurrentIndex(2);
    else                                 m_shellComboBox->setCurrentIndex(0);
#else
    m_shellComboBox->addItem("Bash  (/bin/bash)");
    m_shellComboBox->addItem("Zsh  (/bin/zsh)");
    m_shellComboBox->addItem("Custom...");

    // Pre-select based on saved setting
    const QString saved = userPreference.shellPath.toLower();
    if (saved.contains("zsh"))           m_shellComboBox->setCurrentIndex(1);
    else if (!saved.contains("bash"))    m_shellComboBox->setCurrentIndex(2);
    else                                 m_shellComboBox->setCurrentIndex(0);
#endif

    m_customShellEdit = new QLineEdit(this);
    m_customShellEdit->setPlaceholderText("e.g. C:\\Program Files\\Git\\bin\\bash.exe");
    m_customShellEdit->setText(userPreference.shellPath);

    // Show/hide custom path field based on combo selection
    auto updateCustomVisibility = [this](int idx) {
        m_customShellEdit->setVisible(idx == 2);
    };
    updateCustomVisibility(m_shellComboBox->currentIndex());
    connect(m_shellComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [updateCustomVisibility](int idx){ updateCustomVisibility(idx); });

    shellLayout->addRow("Shell:", m_shellComboBox);
    shellLayout->addRow("Custom path:", m_customShellEdit);
    layout->addWidget(shellGroup);

    // ── Shell Arguments Group ──
    QGroupBox* argsGroup = new QGroupBox("Startup Arguments", this);
    QFormLayout* argsLayout = new QFormLayout(argsGroup);

    m_shellArgsEdit = new QLineEdit(this);
    m_shellArgsEdit->setPlaceholderText("-NoExit -NoLogo");
    m_shellArgsEdit->setText(userPreference.shellArgs);

    argsLayout->addRow("Arguments:", m_shellArgsEdit);
    layout->addWidget(argsGroup);

    layout->addStretch();
    pageWidget->setLayout(layout);
    return pageWidget;
}

QWidget* SettingsDialog::createBuildPage()
{
    QWidget* pageWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(pageWidget);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(12);

    QGroupBox* cmakeGroup = new QGroupBox("CMake Settings", this);
    QFormLayout* cmakeLayout = new QFormLayout(cmakeGroup);

    m_vcpkgToolchainEdit = new QLineEdit(this);
    m_vcpkgToolchainEdit->setPlaceholderText("e.g. C:\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake");
    m_vcpkgToolchainEdit->setText(userPreference.vcpkgToolchainPath);

    m_vcpkgTripletEdit = new QLineEdit(this);
    m_vcpkgTripletEdit->setPlaceholderText("e.g. x64-mingw-dynamic  (leave empty for auto-detection)");
    m_vcpkgTripletEdit->setText(userPreference.vcpkgTargetTriplet);

    cmakeLayout->addRow("CMake Toolchain File:", m_vcpkgToolchainEdit);
    cmakeLayout->addRow("Vcpkg Target Triplet:", m_vcpkgTripletEdit);
    layout->addWidget(cmakeGroup);

    layout->addStretch();
    pageWidget->setLayout(layout);
    return pageWidget;
}

QWidget* SettingsDialog::createExtensionsPage()
{
    QWidget* pageWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(pageWidget);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(12);

    // Top Action Bar
    QHBoxLayout* actionLayout = new QHBoxLayout();

    QPushButton* installBtn = new QPushButton("Install from Folder...", this);
    installBtn->setToolTip("Install an extension folder containing extension.json");

    QPushButton* openDirBtn = new QPushButton("Open Extensions Folder", this);
    openDirBtn->setToolTip("Open the user extensions directory in system file explorer");

    QPushButton* reloadBtn = new QPushButton("Reload", this);
    reloadBtn->setToolTip("Reload all extensions from disk");

    actionLayout->addWidget(installBtn);
    actionLayout->addWidget(openDirBtn);
    actionLayout->addWidget(reloadBtn);
    actionLayout->addStretch();

    layout->addLayout(actionLayout);

    // Table of Extensions
    m_extensionsTable = new QTableWidget(this);
    m_extensionsTable->setColumnCount(6);
    m_extensionsTable->setHorizontalHeaderLabels({"Extension", "Version", "Scope", "File Types", "Description", "Action"});
    m_extensionsTable->horizontalHeader()->setStretchLastSection(false);
    m_extensionsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_extensionsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_extensionsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_extensionsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_extensionsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_extensionsTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_extensionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_extensionsTable->setAlternatingRowColors(true);
    m_extensionsTable->verticalHeader()->setVisible(false);

    layout->addWidget(m_extensionsTable);

    // Connections
    connect(installBtn, &QPushButton::clicked, this, [this]() {
        const QString dir = QFileDialog::getExistingDirectory(this, "Select Extension Directory (containing extension.json)", QString(), QFileDialog::ShowDirsOnly);
        if (!dir.isEmpty()) {
            QString error;
            if (ExtensionManager::instance().installFromFolder(dir, error)) {
                QMessageBox::information(this, "Extension Installed", "Extension installed successfully!");
                populateExtensionsTable();
            } else {
                QMessageBox::warning(this, "Installation Failed", error);
            }
        }
    });

    connect(openDirBtn, &QPushButton::clicked, this, []() {
        const QString path = ExtensionManager::instance().getUserExtensionsDir();
        QDir().mkpath(path);
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });

    connect(reloadBtn, &QPushButton::clicked, this, [this]() {
        ExtensionManager::instance().reloadExtensions();
        populateExtensionsTable();
    });

    connect(&ExtensionManager::instance(), &ExtensionManager::extensionsChanged, this, &SettingsDialog::populateExtensionsTable);

    // Initial populate
    populateExtensionsTable();

    pageWidget->setLayout(layout);
    return pageWidget;
}

void SettingsDialog::populateExtensionsTable()
{
    if (!m_extensionsTable) return;

    m_extensionsTable->setRowCount(0);
    const auto extensions = ExtensionManager::instance().getInstalledExtensions();

    for (int row = 0; row < extensions.size(); ++row) {
        const auto& ext = extensions[row];
        m_extensionsTable->insertRow(row);

        // Name & ID
        auto* nameItem = new QTableWidgetItem(QString("%1\n(%2)").arg(ext.name, ext.id));
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        m_extensionsTable->setItem(row, 0, nameItem);

        // Version
        auto* verItem = new QTableWidgetItem(ext.version);
        verItem->setFlags(verItem->flags() & ~Qt::ItemIsEditable);
        verItem->setTextAlignment(Qt::AlignCenter);
        m_extensionsTable->setItem(row, 1, verItem);

        // Scope
        auto* typeItem = new QTableWidgetItem(ext.isBuiltIn ? "Built-in" : "User");
        typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
        typeItem->setTextAlignment(Qt::AlignCenter);
        m_extensionsTable->setItem(row, 2, typeItem);

        // File Types
        QStringList extList;
        for (const auto& lang : ext.languages) {
            for (const auto& fext : lang.extensions) {
                extList << "." + fext;
            }
        }
        auto* extsItem = new QTableWidgetItem(extList.join(", "));
        extsItem->setFlags(extsItem->flags() & ~Qt::ItemIsEditable);
        m_extensionsTable->setItem(row, 3, extsItem);

        // Description
        auto* descItem = new QTableWidgetItem(ext.description);
        descItem->setFlags(descItem->flags() & ~Qt::ItemIsEditable);
        m_extensionsTable->setItem(row, 4, descItem);

        // Actions
        if (!ext.isBuiltIn) {
            QPushButton* uninstallBtn = new QPushButton("Uninstall", this);
            QString extId = ext.id;
            QString extName = ext.name;
            connect(uninstallBtn, &QPushButton::clicked, this, [this, extId, extName]() {
                auto reply = QMessageBox::question(this, "Uninstall Extension",
                    QString("Are you sure you want to uninstall '%1'?").arg(extName),
                    QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    QString error;
                    if (ExtensionManager::instance().uninstallExtension(extId, error)) {
                        populateExtensionsTable();
                    } else {
                        QMessageBox::warning(this, "Uninstall Failed", error);
                    }
                }
            });
            m_extensionsTable->setCellWidget(row, 5, uninstallBtn);
        } else {
            auto* lockItem = new QTableWidgetItem("System");
            lockItem->setFlags(lockItem->flags() & ~Qt::ItemIsEditable);
            lockItem->setTextAlignment(Qt::AlignCenter);
            lockItem->setForeground(QBrush(QColor("#888888")));
            m_extensionsTable->setItem(row, 5, lockItem);
        }
    }

    m_extensionsTable->resizeRowsToContents();
}

