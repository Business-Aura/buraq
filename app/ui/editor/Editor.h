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

#include "EditorMargin.h"
#include "buraq.h"
#include "Highlighter.h"

class Editor final : public QWidget
{
    Q_OBJECT

protected:
    // void focusInEvent(QFocusEvent *e) override;

    void keyPressEvent(QKeyEvent* e) override;

    void keyReleaseEvent(QKeyEvent* e) override;

    void mousePressEvent(QMouseEvent* e) override;

signals:
    void statusUpdate(QString status, int timeout = 10000);

    void readyToSaveEvent();

    void syntaxtHighlightingEvent();

    void inlineSyntaxtHighlightingEvent();

    void lineNumberAreaPaintEventSignal(const buraq::EditorState& state);

public:
    explicit Editor(QWidget* window = nullptr);

    ~Editor() override = default;

    void openAndParseFile(const QString& filePath, QFile::OpenModeFlag modeFlag = QFile::OpenModeFlag::ReadOnly);
    // Add forwarding methods if external code calls QPlainTextEdit methods on Editor
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
    }

    [[nodiscard]] bool isDirty() const { return m_isDirty; }

    void saveFile();

    void autoSave();

private slots:
    void highlightCurrentLine();

    void documentSyntaxHighlighting();

    void inlineSyntaxHighlighting();

private:
    std::unique_ptr<QPlainTextEdit> m_plainTextEdit; // FIX: Internal QPlainTextEdit
    std::unique_ptr<EditorMargin> m_editorMargin; // Your margin widget
    QWidget* m_window;
    QStack<QString> m_history;
    QString m_currentFile;
    QString m_previousText;
    QTimer m_autoSaveTimer;
    buraq::EditorState m_state;
    std::unique_ptr<buraq::Highlighter> m_highlighter;
    bool m_isDirty = false;

    static QString convertTextToHtml(QString&);
    static QString convertRhsTextToHtml(const QString&);

    void setupSignals();
};

#endif //IT_TOOLS_EDITOR_H2
