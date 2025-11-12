//
// Created by talik on 5/29/2025.
//

#ifndef LINE_NUMBER_AREA_WIDGET_H
#define LINE_NUMBER_AREA_WIDGET_H

#include <QWidget>
#include <QFontMetrics> // For calculating text sizes
#include "buraq.h"

class LineNumberAreaWidget final : public QWidget {
Q_OBJECT

public:
	explicit LineNumberAreaWidget(QWidget *parent = nullptr);

	// Call this when the editor's m_state changes to provide data for painting
	void updateEditorState(const buraq::EditorState &state);

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	buraq::EditorState m_editorState{.lineHeight = 19};
// You might want to store font, colors, etc., as members
// or get them from the m_parent EditorMargin if needed.
};

#endif //LINE_NUMBER_AREA_WIDGET_H
