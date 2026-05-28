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
#include <qsettings.h>

#include "Config.h"
#include "Filters/Toolbar/ToolBarEvent.h"
#include "Frame/Frame.h"
#include "settings/SettingManager/SettingsManager.h"
#include "settings/UserSettings.h"

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
    m_Frame->getTitleLabel()->setText("Settings");

    // Hide side panels and bottom panel in SettingsDialog to reclaim space
    m_Frame->getLeftSidePanelLayout()->parentWidget()->hide();
    m_Frame->getRightSidePanelLayout()->parentWidget()->hide();
    m_Frame->getBottomPanelLayout()->parentWidget()->hide();

    // --- Main Dialog Layout ---
    // The dialog itself needs a layout. We will add our Frame to this layout.
    QVBoxLayout* dialogLayout = new QVBoxLayout(this);
    dialogLayout->setContentsMargins(0, 0, 0, 0);
    dialogLayout->addWidget(m_Frame); // Add the frame to the dialog's layout
    setLayout(dialogLayout);

    // Set object name for styling and resize to a professional size
    setObjectName("SettingsDialog");
    resize(700, 550);

    // Load user preferences
    userPreference = SettingsManager::loadSettings();

    // Get the main content area from the Frame to add our settings content
    QWidget* mainContentWidget = m_Frame->getMainContentWidget();
    QVBoxLayout* mainContentLayout = m_Frame->getMainLayout();
    
    // Set nice margins and spacing for the settings contents
    mainContentLayout->setContentsMargins(15, 15, 15, 15);
    mainContentLayout->setSpacing(15);

    // --- Main Tab Widget ---
    m_tabWidget = new QTabWidget(this);

    m_tabWidget->addTab(createAppearancePage(), "Appearance");
    m_tabWidget->addTab(createEditorPage(), "Editor");
    m_tabWidget->addTab(createAccountPage(), "Account");

    mainContentLayout->addWidget(m_tabWidget);

    // --- Button Box ---
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply, this);
    mainContentLayout->addWidget(m_buttonBox);

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
    SettingsManager::saveSettings(userPreference);
    themeManager.setAppTheme(userPreference.theme);
}

void SettingsDialog::setTheme(const int index)
{
    userPreference.theme = static_cast<AppTheme>(index);
}

QWidget* SettingsDialog::createAppearancePage()
{
    QWidget* pageWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(pageWidget);

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

    QLineEdit* apiKeyLineEdit = new QLineEdit(this);
    apiKeyLineEdit->setPlaceholderText("Enter your API Key here");
    apiKeyLineEdit->setEchoMode(QLineEdit::Password);

    QPushButton* loginButton = new QPushButton("Connect Account", this);

    layout->addRow("API Key:", apiKeyLineEdit);
    layout->addRow("", loginButton);

    pageWidget->setLayout(layout);
    return pageWidget;
}
