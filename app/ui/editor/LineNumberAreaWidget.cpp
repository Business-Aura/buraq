//
// Created by talik on 5/29/2025.
//

#include "LineNumberAreaWidget.h"
#include <QPainter>
#include <QPalette>
#include <QTextBlock>
#include "EditorMargin.h"
#include "Editor.h"

LineNumberAreaWidget::LineNumberAreaWidget(QWidget *parent) : QWidget(parent) {
	setMinimumWidth(5);
}

void LineNumberAreaWidget::updateEditorState(const buraq::EditorState &state) {
	Q_UNUSED(state);
	update();
}

void LineNumberAreaWidget::paintEvent(QPaintEvent *event) {
	auto margin = qobject_cast<EditorMargin*>(parentWidget());
	if (!margin) return;
	auto editor = margin->getEditor();
	if (!editor) return;

	QPainter painter(this);
	
	// Draw background matching editor/margin theme background
	QColor bgColor = palette().color(QPalette::Window);
	painter.fillRect(event->rect(), bgColor);

	// Match editor font
	painter.setFont(editor->font());

	// Get vertical offset of the editor viewport relative to the editor widget
	QPoint viewportOffset = editor->viewport()->mapTo(editor, QPoint(0, 0));
	int vOffset = viewportOffset.y();

	QTextBlock block = editor->firstVisibleBlock();
	int blockNumber = block.blockNumber();
	
	// Calculate top and bottom coordinates of the block mapped to our widget
	int top = qRound(editor->blockBoundingGeometry(block).translated(editor->contentOffset()).top()) + vOffset;
	int bottom = top + qRound(editor->blockBoundingRect(block).height());

	int cursorBlockNumber = editor->textCursor().blockNumber();

	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QRect lineAreaRect(0, top, width(), bottom - top);

			// Active line highlight in the margin (matches the editor's highlight height and position)
			if (blockNumber == cursorBlockNumber) {
				QColor highlightColor = QColor(0, 188, 212, 25); // Subtle cyan
				painter.fillRect(lineAreaRect, highlightColor);
			}

			QString numberText = QString::number(blockNumber + 1);
			int textPaddingRight = 8;
			QRect textDrawingRect = lineAreaRect;
			textDrawingRect.setRight(lineAreaRect.right() - textPaddingRight);

			// Distinct colors: cyan for active line, gray for inactive lines
			QColor textColor = palette().color(QPalette::WindowText);
			if (blockNumber == cursorBlockNumber) {
				textColor = QColor("#00BCD4"); // Cyan accent
			} else {
				// Make inactive line numbers softer/semi-transparent
				textColor = QColor(textColor.red(), textColor.green(), textColor.blue(), 140);
			}
			painter.setPen(textColor);
			painter.drawText(textDrawingRect, Qt::AlignRight | Qt::AlignVCenter, numberText);
		}

		block = block.next();
		top = bottom;
		bottom = top + qRound(editor->blockBoundingRect(block).height());
		++blockNumber;
	}
}
