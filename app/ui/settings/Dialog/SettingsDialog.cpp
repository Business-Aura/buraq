//
// Created by talik on 8/30/2025.
//

#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
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
      m_titleBar(std::make_unique<QWidget>(this)),
      m_Frame(std::make_unique<Frame>(this)),
      settingsManager(new SettingsManager()),
      themeManager(ThemeManager::instance())
{
    setWindowTitle("Settings");
    // Set the m_window flag to remove the default frame
    this->setWindowFlags(Qt::FramelessWindowHint);
    // allow the m_window to be transparent if you have rounded corners.
    this->setAttribute(Qt::WA_TranslucentBackground);

    // Load user preferences
    userPreference = settingsManager->loadSettings();

    // Create a central widget to hold the main layout.
    // QMainWindow requires a central widget to manage content.
    if (const auto mainLayout = m_Frame->getMainLayout(); mainLayout)
    {
        // --- Main Tab Widget ---
        m_tabWidget = new QTabWidget(this);
        m_tabWidget->addTab(createAppearancePage(), "Appearance");
        m_tabWidget->addTab(createEditorPage(), "Editor");
        m_tabWidget->addTab(createAccountPage(), "Account");

        // --- Bottom Button Box (OK, Cancel, Apply) ---
        m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply,
                                           this);
        m_buttonBox->button(QDialogButtonBox::Ok)->setObjectName("OkButtonBox");
        m_buttonBox->button(QDialogButtonBox::Apply)->setObjectName("ApplyButtonBox");
        m_buttonBox->button(QDialogButtonBox::Cancel)->setObjectName("CancelButtonBox");

        connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
        connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
        connect(m_buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this,
                &SettingsDialog::applyChanges);

        // --- Main Layout ---
        mainLayout->addWidget(m_tabWidget);
        mainLayout->addWidget(m_buttonBox);
    }

    const auto windowConfig = Config::singleton().getWindow();
    // Set an initial size
    resize(windowConfig->normalSize, windowConfig->minHeight);
    hide();
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::applyChanges() const
{
    settingsManager->saveSettings(userPreference);

    // update the UI
    themeManager.setAppTheme(userPreference.theme);
}

void SettingsDialog::setTheme(const int index)
{
    if (index >= 0 && index < 3)
    {
        qDebug() << "ThemeManager::loadSettings index = " << index;
        userPreference.theme = static_cast<AppTheme>(index);
    }
}

QWidget* SettingsDialog::createAppearancePage()
{
    QWidget* pageWidget = new QWidget(this);
    QFormLayout* layout = new QFormLayout(pageWidget);

    // --- Theme Selection ---
    QGroupBox* themeGroup = new QGroupBox("Theme", this);
    QFormLayout* themeLayout = new QFormLayout(themeGroup);

    QComboBox* themeComboBox = new QComboBox(this);
    themeComboBox->setObjectName("themeComboBox"); // For retrieving the value later
    themeComboBox->addItems({"Light", "Dark", "System Default"});
    themeLayout->addRow(new QLabel("Application Theme:", this), themeComboBox);
    qDebug() << "userPreference.theme: " << userPreference.theme;
    themeComboBox->setCurrentIndex(userPreference.theme);
    connect(themeComboBox, &QComboBox::currentIndexChanged, this, &SettingsDialog::setTheme);

    layout->addWidget(themeGroup);

    // --- Font Selection ---
    QGroupBox* fontGroup = new QGroupBox("Typography", this);
    QFormLayout* fontLayout = new QFormLayout(fontGroup);

    QComboBox* fontComboBox = new QComboBox(this);
    fontComboBox->addItems({"Segoe UI", "Roboto", "Helvetica"});
    QSpinBox* fontSizeSpinBox = new QSpinBox(this);
    fontSizeSpinBox->setRange(8, 20);
    fontSizeSpinBox->setValue(10);
    fontSizeSpinBox->setSuffix(" pt");

    fontLayout->addRow(new QLabel("Font:", this), fontComboBox);
    fontLayout->addRow(new QLabel("Font Size:", this), fontSizeSpinBox);

    layout->addWidget(fontGroup);

    pageWidget->setLayout(layout);
    return pageWidget;
}

QWidget* SettingsDialog::createEditorPage()
{
    QWidget* pageWidget = new QWidget(this);
    QFormLayout* layout = new QFormLayout(pageWidget);
    layout->setSpacing(15);

    QSpinBox* tabSizeSpinBox = new QSpinBox(this);
    tabSizeSpinBox->setValue(4);

    QCheckBox* wordWrapCheckBox = new QCheckBox("Enable word wrap", this);
    wordWrapCheckBox->setChecked(true);

    QCheckBox* autoIndentCheckBox = new QCheckBox("Enable auto-indentation", this);
    autoIndentCheckBox->setChecked(true);

    QCheckBox* showLineNumbersCheckBox = new QCheckBox("Show line numbers", this);
    showLineNumbersCheckBox->setChecked(true);

    layout->addRow("Tab Size:", tabSizeSpinBox);
    layout->addRow(wordWrapCheckBox);
    layout->addRow(autoIndentCheckBox);
    layout->addRow(showLineNumbersCheckBox);

    pageWidget->setLayout(layout);
    return pageWidget;
}

QWidget* SettingsDialog::createAccountPage()
{
    QWidget* pageWidget = new QWidget(this);
    QFormLayout* layout = new QFormLayout(pageWidget);

    QLineEdit* apiKeyLineEdit = new QLineEdit(this);
    apiKeyLineEdit->setDisabled(true);
    apiKeyLineEdit->setPlaceholderText("Enter your API key");
    apiKeyLineEdit->setEchoMode(QLineEdit::Password);

    QPushButton* loginButton = new QPushButton("Connect Account", this);

    layout->addRow("API Key:", apiKeyLineEdit);
    layout->addRow("", loginButton);

    pageWidget->setLayout(layout);
    return pageWidget;
}
