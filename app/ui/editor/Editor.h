//
// Created by talik on 3/2/2024.
//

#ifndef IT_TOOLS_EDITOR_H2
#define IT_TOOLS_EDITOR_H2

#include <QPlainTextEdit>
#include <QWidget>
#include <QFile>
#include <QTimer>
#include <QRegularExpression>
#include <QStack>
#include <QSyntaxHighlighter>

#include "EditorMargin.h"
#include "buraq.h"
#include "LanguageProvider.h"

class BuraqTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    using QPlainTextEdit::QPlainTextEdit;
    
    // Expose protected methods needed for line number rendering
    using QPlainTextEdit::firstVisibleBlock;
    using QPlainTextEdit::blockBoundingGeometry;
    using QPlainTextEdit::blockBoundingRect;
    using QPlainTextEdit::contentOffset;
};

class Editor final : public QWidget
{
    Q_OBJECT

signals:
    void statusUpdate(QString status, int timeout = 10000);
    void readyToSaveEvent();
    void lineNumberAreaPaintEventSignal(const buraq::EditorState& state);

public:
    explicit Editor(QWidget* window = nullptr);
    ~Editor() override = default;

    void openAndParseFile(const QString& filePath, QFile::OpenModeFlag modeFlag = QFile::OpenModeFlag::ReadOnly);
    [[nodiscard]] QString currentFile() const { return m_currentFile; }
    [[nodiscard]] QString toPlainText() const { return m_plainTextEdit->toPlainText(); }
    [[nodiscard]] QString selectedText() const { return m_plainTextEdit->textCursor().selectedText(); }
    void setPlainText(const QString& text) {
        m_plainTextEdit->setPlainText(text);
        m_isDirty = false;
    }
    void clear() {
        m_plainTextEdit->clear();
        m_currentFile.clear();
        m_isDirty = false;
        setHighlighterForFile(""); // Clear highlighter
    }

    [[nodiscard]] bool isDirty() const { return m_isDirty; }

    bool saveFile();
    void autoSave();
    void highlightCurrentLine();

private slots:
    void onTextChanged();

private:
    void setHighlighterForFile(const QString& filePath);
    void setupSignals();

    BuraqTextEdit* m_plainTextEdit;
    EditorMargin* m_editorMargin;
    QWidget* m_window;
    QStack<QString> m_history;
    QString m_currentFile;
    QTimer m_autoSaveTimer;
    buraq::EditorState m_state;
    QSyntaxHighlighter* m_highlighter = nullptr; // Will be managed by the QPlainTextEdit
    bool m_isDirty = false;
};

#endif //IT_TOOLS_EDITOR_H2
