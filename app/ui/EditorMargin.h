//
// Created by talik on 3/2/2024.
//

#ifndef EDITOR_MARGIN_H
#define EDITOR_MARGIN_H

#include <QWidget>
#include <QPainter>
#include <QPlainTextEdit>

#include "editor/CodeRunner.h"
#include "CommonWidget.h"
#include "editor/LineNumberAreaWidget.h"

class BuraqTextEdit;

class EditorMargin final : public CommonWidget {
	Q_OBJECT

public slots:
	void updateState(const buraq::EditorState &newState) const;
	void onEditorScrolled();
	void updateMarginWidth(const buraq::EditorState& state) const;

public:

	explicit EditorMargin(QWidget *windowPtr, QWidget *parent = nullptr);
	void setEditor(BuraqTextEdit *editor) { m_editor = editor; }
	BuraqTextEdit* getEditor() const { return m_editor; }
	~EditorMargin() override = default;

private:
	QWidget *windowPtr;
	CodeRunner* codeRunner;
	LineNumberAreaWidget* line_numbers_widget;
	BuraqTextEdit *m_editor{}; // Pointer to the associated editor

	void setupSignals() const override;
	int lineNumberAreaWidth();
};

#endif //EDITOR_MARGIN_H
