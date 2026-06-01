//
// Created by talik on 3/2/2024.
//

#include <QGridLayout>
#include <QTextBlock>
#include <QString>
#include <QMouseEvent>
#include <QTextStream>
#include <QTextEdit>
#include <QProcess>
#include <QScrollBar>
#include <QFileInfo>
#include <QFileDialog>
#include <QCoreApplication>

#include "Editor.h"
#include "EditorMargin.h"
#include "app_ui/AppUi.h"
#include "frameless_window/MainWindow.h"
#include "LanguageProvider.h"
#include "../../database/db_conn.h"

Editor::Editor(QWidget* window)
    : QWidget(window),
      m_window(window)
{
    setObjectName("Editor");

    m_plainTextEdit = new BuraqTextEdit(this);
    m_plainTextEdit->setObjectName("InternalPlainTextEdit");

    m_editorMargin = new EditorMargin(window, this);
    m_editorMargin->setEditor(m_plainTextEdit);

    const auto main_layout = new QHBoxLayout(this);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);

    m_plainTextEdit->setFrameShape(QFrame::NoFrame);
    m_plainTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_plainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QFile file(":/test/temp.ps1");
    if (file.open(QFile::OpenModeFlag::ReadOnly)) {
        const QString fileContent = QString::fromLatin1(file.readAll());
        m_plainTextEdit->setPlaceholderText(fileContent);
        file.close();
    }

    main_layout->addWidget(m_editorMargin);
    main_layout->addWidget(m_plainTextEdit);

    setFocusProxy(m_plainTextEdit);

    setupSignals();
    highlightCurrentLine();
}

void Editor::openAndParseFile(const QString& filePath, QFile::OpenModeFlag modeFlag)
{
    if (filePath.isEmpty()) return;

    // Delete/detach old highlighter BEFORE setting the new text
    // to avoid redundant layout calculations and formatting runs.
    setHighlighterForFile("");

    QFile file(filePath);
    if (!file.open(modeFlag)) {
        emit statusUpdate("Failed to open file: " + filePath);
        return;
    }

    QTextStream in(&file);
    const QString fileContent = in.readAll();
    file.close();

    m_currentFile = filePath;
    database::setLastOpenedFilePath(filePath);

    m_plainTextEdit->blockSignals(true);
    setPlainText(fileContent);
    m_plainTextEdit->blockSignals(false);

    highlightCurrentLine();
    m_editorMargin->updateState(m_state);

    // Apply the new highlighter
    setHighlighterForFile(filePath);

    m_isDirty = false;
    emit statusUpdate("File opened: " + filePath);
}

void Editor::setHighlighterForFile(const QString& filePath)
{
    // Delete the old highlighter if it exists
    if (m_highlighter) {
        delete m_highlighter;
        m_highlighter = nullptr;
    }

    if (filePath.isEmpty()) return;

    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix();

    ILanguageProvider* provider = LanguageRegistry::instance().getProviderForExtension(extension);

    if (provider) {
        // The provider creates the highlighter.
        // The QPlainTextEdit's document takes ownership of the highlighter.
        m_highlighter = provider->createHighlighter(m_plainTextEdit->document());
        emit statusUpdate("Syntax highlighting enabled for: " + provider->getLanguageName());
    } else {
        emit statusUpdate("No syntax highlighter found for '." + extension + "' files.");
    }
}

void Editor::setupSignals()
{
    connect(m_plainTextEdit, &QPlainTextEdit::cursorPositionChanged, this, [this](){
        highlightCurrentLine();
        m_editorMargin->updateState(m_state);
    });

    connect(m_plainTextEdit, &QPlainTextEdit::textChanged, this, [this](){
        onTextChanged();
        m_editorMargin->updateState(m_state);
    });

    connect(m_plainTextEdit->verticalScrollBar(), &QScrollBar::valueChanged, m_editorMargin, [this](){
        m_editorMargin->updateState(m_state);
    });

    connect(m_plainTextEdit->document(), &QTextDocument::contentsChange, m_editorMargin, [this]() {
        m_editorMargin->updateState(m_state);
    });

    const auto main_window = dynamic_cast<MainWindow*>(m_window);
    if (main_window) {
        connect(this, &Editor::statusUpdate, main_window, &MainWindow::processStatusSlot);
    }
}

void Editor::onTextChanged()
{
    m_isDirty = true;
    // The auto-save timer could be started here if desired
}

bool Editor::saveFile()
{
    if (m_currentFile.isEmpty()) {
        QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", "All Files (*)");
        if (fileName.isEmpty()) return false;
        m_currentFile = fileName;
        // After saving for the first time, the file type is known, so set the highlighter.
        setHighlighterForFile(m_currentFile);
    }

    QFile file(m_currentFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit statusUpdate("Could not save file: " + file.errorString());
        return false;
    }

    QTextStream out(&file);
    out << toPlainText();
    file.close();

    m_isDirty = false;
    database::setLastOpenedFilePath(m_currentFile);
    emit statusUpdate("File saved: " + m_currentFile, 5000);
    return true;
}

void Editor::autoSave()
{
    if (!m_currentFile.isEmpty() && m_isDirty) {
        saveFile();
    }
}

void Editor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!m_plainTextEdit->isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        // Modern, premium styling: subtle semi-transparent cyan background
        QColor lineColor = QColor(0, 188, 212, 25); 

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = m_plainTextEdit->textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    m_plainTextEdit->setExtraSelections(extraSelections);
}


