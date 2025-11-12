//
// Created by talik on 3/2/2024.
//

#include <QGridLayout>
#include <QTextBlock>
#include <QString>
#include <QMouseEvent>
#include <QTextStream>  // For text files
#include <QProcess>

#include "Editor.h"

#include <qscrollbar.h>

#include "EditorMargin.h"
#include "app_ui/AppUi.h"
#include "frameless_window/FramelessWindow.h"

#define string_equals(keyText, key) \
(std::equal(keyText.begin(), keyText.end(), key));

// Capture keywords in a given line
static QRegularExpression keywordsRegex(
    QStringLiteral(
        "^\\s*\\b(echo|ls|ps|Write-Output|Get-ChildItem|Connect-SPOService|"
        "Get-SPOsite|Set-SPOUser|Install-Module)\\b(.*)$"),
    QRegularExpression::CaseInsensitiveOption);

// Capture comments at the start of in a given line
static QRegularExpression commentsRegex(QStringLiteral("^\\s*(#\\w*)"));

// Capture string literals in a given line
static QRegularExpression doubleQuotesLiteralRegex(QStringLiteral("\"(.*?)\""));

// Capture variables in a given line
static QRegularExpression variablesRegex(
    QStringLiteral("^\\s*(\\$\\w+\\s*)=(.*?)$"),
    QRegularExpression::CaseInsensitiveOption);

/**
 *
 * @param window The pointer to the main app.
 */
Editor::Editor(QWidget* window)
    : QWidget(window), // FIX: Editor now inherits QWidget
      m_window(window)
{
    setObjectName("Editor");

    // 1. Create the actual QPlainTextEdit instance as a member.
    // Parent it to 'this' (Editor) so Qt manages its memory.
    m_plainTextEdit = std::make_unique<QPlainTextEdit>(this);
    // Set an object name for the internal QPlainTextEdit for specific QSS styling if needed
    m_plainTextEdit->setObjectName("InternalPlainTextEdit"); // Useful for QSS specific to the text area

    // 2. Create the EditorMargin instance.
    // Parent it to 'this' (Editor).
    m_editorMargin = std::make_unique<EditorMargin>(window, this);

    // FIX: Set the editor for the margin so it can synchronize
    m_editorMargin->setEditor(m_plainTextEdit.get());

    // 3. Set up the main horizontal layout for the Editor container.
    const auto main_layout = new QHBoxLayout(this); // 'this' (Editor) takes ownership of the layout
    main_layout->setSpacing(0); // No spacing between margin and editor
    main_layout->setContentsMargins(0, 0, 0, 0); // No margins around the layout

    // 5. Apply QPlainTextEdit specific properties to the *member* m_plainTextEdit
    m_plainTextEdit->setFrameShape(QFrame::NoFrame);
    m_plainTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_plainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 6. Forward properties/methods to the internal QPlainTextEdit
    {
        // update place holder text
        QFile file(":/test/temp.ps1");
        file.open(QFile::OpenModeFlag::ReadOnly);

        const QString fileContent = QString::fromLatin1(file.readAll());
        m_plainTextEdit->setPlaceholderText(fileContent); // FIX: Call on m_plainTextEdit

        file.close();
    }

    // 7. Setup communication between internal widgets (e.g., scroll synchronization)
    // You'll need to define setupSignals() to connect the scrollbars etc.
    // Example connections (assuming EditorMargin has onEditorScrolled and updateMarginWidth slots)
    connect(m_plainTextEdit->verticalScrollBar(), &QScrollBar::valueChanged,
            m_editorMargin.get(), &EditorMargin::onEditorScrolled);
    connect(m_plainTextEdit->document(), &QTextDocument::contentsChange, m_editorMargin.get(), [this]()
    {
        m_editorMargin.get()->updateMarginWidth(m_state);
    }); // Also update on text changes

    // Call your existing setupSignals() if it does more than just this.
    setupSignals();

    // 4. Add widgets to the layout.
    // The order determines left-to-right placement.
    // Use release() to transfer ownership from unique_ptr to the layout.
    main_layout->addWidget(m_editorMargin.get()); // Add margin to the left
    main_layout->addWidget(m_plainTextEdit.get()); // Add QPlainTextEdit to the right

    // Any other signals from Editor that need to come from m_plainTextEdit
    // would be connected here, and then Editor would re-emit them.
    // Example: connect(m_plainTextEdit.get(), &QPlainTextEdit::textChanged, this, &Editor::textChanged);
}

void Editor::highlightCurrentLine()
{
    if (const auto textEdit = m_plainTextEdit.get(); !textEdit->isReadOnly())
    {
        QList<QTextEdit::ExtraSelection> extraSelections;
        QTextEdit::ExtraSelection selection;

        const QColor lineColor = QColor(Qt::lightGray).lighter(25);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        m_state.hasText = m_currentFile.isEmpty();
        m_state.isBlockValid = false;
        m_state.isSelected = false;
        m_state.blockCount = textEdit->blockCount();

        const auto text_cursor = textEdit->textCursor();
        selection.cursor = text_cursor;
        extraSelections.append(selection);
        m_state.blockNumber = m_state.cursorBlockNumber = text_cursor.blockNumber();
        m_state.lineHeight = fontMetrics().height();

        m_state.currentLineHeight = textEdit->cursorRect().height();

        if (selection.cursor.hasSelection())
        {
            m_state.isSelected = true;
            m_state.selectedBlockNumbers.insert(selection.cursor.block().blockNumber());
        }
        else
        {
            m_state.selectedBlockNumbers.clear();
        }

        emit lineNumberAreaPaintEventSignal(m_state);

        textEdit->setExtraSelections(extraSelections);
    }
}

QString Editor::convertRhsTextToHtml(const QString& qRhsLine)
{
    //qDebug() << qRhsLine;

    QRegularExpressionMatch doubleQuotesLiteralExp = doubleQuotesLiteralRegex.match(qRhsLine);
    if (doubleQuotesLiteralExp.hasMatch())
    {
		//@formatter:off
		return QString("<span style='color:#3eb489'>" +
					   qRhsLine +
					   "</span>");
        //@formatter:on
    }

    return qRhsLine;
}

QString Editor::convertTextToHtml(QString& line)
{
    QString lineBreak("<p> </p>");

    if (line.isEmpty())
    {
        // FIXME what is the best way to add new lines? <br/> adds extra lines
        return lineBreak;
    }

    // Handles comments on if # is found at the beginning of the line
    QRegularExpressionMatch commentAtStartOfLineExp = commentsRegex.match(line);
    if (commentAtStartOfLineExp.hasMatch())
    {
        return "<p style='color:gray'>" + line + "</p>";
    }

    QRegularExpressionMatch keywordsExp = keywordsRegex.match(line);
    if (keywordsExp.hasMatch())
    {
        auto keyword = keywordsExp.captured(1);
        auto rhsTextToHtml = convertRhsTextToHtml(keywordsExp.captured(2));

        //qDebug() << rhsTextToHtml;
        //qDebug() << keywordsExp.capturedTexts();

		//@formatter:off
		return QString("<p>"
				"<span style='color:#FFB76B'>" + keyword + "</span>" +
				rhsTextToHtml +
				"</p>");
        //@formatter:on
    }

    QRegularExpressionMatch variablesRegexExp = variablesRegex.match(line);
    if (variablesRegexExp.hasMatch())
    {
        auto variable = variablesRegexExp.captured(1);
        auto rhsTextToHtml = convertRhsTextToHtml(variablesRegexExp.captured(2));

		//@formatter:off
		return QString("<p>"
					   "<span style='color:#87CEEB'>" + variable + "</span>" +
					   "=" +
					   rhsTextToHtml +
					   "</p>");
        //@formatter:on
    }

    if (std::equal(line.begin(), line.end(), " ")) return lineBreak;

	//@formatter:off
	return QString("<p style='text-decoration: green wavy underline;'>"
					+ line + // TODO underline only the first word
				   "</p>");
    //@formatter:on
}

void Editor::openAndParseFile(const QString& filePath, QFile::OpenModeFlag modeFlag)
{
    if (modeFlag != QFile::ReadOnly)
    {
        this->m_currentFile = filePath;
    }

    try
    {
        QFile file(filePath);
        file.open(modeFlag);

        QString fileContent = QString::fromLatin1(file.readAll());
        file.close(); // close file

        // clear editor before using the function to avoid adding to the previous opened file
        setPlainText(fileContent);

        // highlight syntax
        emit syntaxtHighlightingEvent();
    }
    catch (...)
    {
        qDebug() << "emit exception";
        statusUpdate("File to open file " + filePath);
    }
}

void Editor::setupSignals()
{
    connect(m_plainTextEdit.get(), &QPlainTextEdit::cursorPositionChanged, this, &Editor::highlightCurrentLine);

    // Enables syntaxHighlighting i.e. showing keywords, comments, variables etc.
    connect(this, &Editor::syntaxtHighlightingEvent, this, &Editor::documentSyntaxHighlighting);
    connect(this, &Editor::inlineSyntaxtHighlightingEvent, this, &Editor::inlineSyntaxHighlighting);

    // Enables auto saving the document
    connect(this, &Editor::readyToSaveEvent, this, &Editor::autoSave);

    // Update status bar in AppUI component
    const auto appUi_ = dynamic_cast<FramelessWindow*>(m_window);
    connect(this, &Editor::statusUpdate, appUi_, &FramelessWindow::processStatusSlot);

    // Signal to update status bar in AppUI component for the running process
    connect(this, &Editor::lineNumberAreaPaintEventSignal, m_editorMargin.get(), &EditorMargin::updateState);
}

void Editor::autoSave()

{
    // auto save works only if the file had been saved before
    // therefore m_currentFile should have been set
    if (!m_currentFile.isEmpty())
    {
        // Open the file for writing
        QFile file(m_currentFile);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            emit statusUpdate(file.errorString());
        }

        // Write data to the file (using QTextStream for text files)
        QTextStream out(&file);
        out << toPlainText();

        emit statusUpdate("Auto Saving..", 5000);

        // Close the file (important to flush data to disk)
        file.close();
    }
}

void Editor::keyReleaseEvent(QKeyEvent* e)
{
    // default behaviour
    // QPlainTextEdit::keyReleaseEvent(e);

    QString keyText = e->text();
    // event to signal Auto Save can occur;
    const bool isBackSpaceKey = string_equals(keyText, "\b");
    const bool isEnterKey = string_equals(keyText, "\r");
    const bool isUndo = string_equals(keyText, "\u001A");

    if (keyText.isEmpty()) { return; }

    if (isUndo)
    {
        // revert to previous version of the document
        return;
    }

    if (!isBackSpaceKey and !isEnterKey)
    {
        emit inlineSyntaxtHighlightingEvent();
        return;
    }

    const QString currentText = toPlainText();
    // Analyze the difference between m_previousText and currentText to identify deletions

    if (const auto deletedLength = m_previousText.size() - currentText.size(); isBackSpaceKey && deletedLength > 0)
    {
        emit inlineSyntaxtHighlightingEvent();
    }
    else
    {
        emit readyToSaveEvent();
    }
}

void Editor::mousePressEvent(QMouseEvent* e)
{
    // clear placeholder text since the user is active in the area
    m_plainTextEdit->setPlaceholderText("");

    // QPlainTextEdit::mousePressEvent(e);
}

void Editor::inlineSyntaxHighlighting()
{
    QTextCursor cursor = m_plainTextEdit->textCursor();
    const int position = cursor.position();

    // Create a new text cursor positioned at the beginning of the block
    QTextBlock blockToReplace = cursor.block();
    QTextCursor nextCursor(blockToReplace);
    nextCursor.select(QTextCursor::SelectionType::LineUnderCursor);

    QString text = blockToReplace.text();
    const QString html = convertTextToHtml(text);

    nextCursor.insertHtml(
        "<pre>" +
        html +
        "</pre>"
    );
    nextCursor.setPosition(position);
    m_plainTextEdit->setTextCursor(nextCursor);

    //save after successful syntax highlighting
    emit readyToSaveEvent();
}

void Editor::documentSyntaxHighlighting()
{
    if (QString plainText = toPlainText(); !plainText.isEmpty())
    {
        // creating opening tags for HTML string
        QString html("<pre>");
        for (auto line : plainText.split("\n"))
        {
            // Process the line here
            html.append(convertTextToHtml(line));
        }
        // closing tags for HTML string
        html.append("</pre>");

        // update the UI with the new formatted code.
        m_plainTextEdit->clear();
        m_plainTextEdit->appendHtml(html);
    }

    // save after successful syntax highlighting
    emit readyToSaveEvent();
}

void Editor::keyPressEvent(QKeyEvent* e)
{
    m_previousText = toPlainText();

    // m_plainTextEdit->keyPressEvent(e);
}
