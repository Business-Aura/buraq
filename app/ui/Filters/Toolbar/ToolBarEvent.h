//
// Created by talik on 2/7/2024.
//

#ifndef IT_TOOLS_WINDOW_EVENT_FILTER_H
#define IT_TOOLS_WINDOW_EVENT_FILTER_H

#include <QMainWindow>
#include <QMetaObject>
#include <iostream>

class ToolBarEvent final : public QObject {
	Q_OBJECT

public:
	explicit ToolBarEvent(QObject *parent = nullptr);
	~ToolBarEvent() override;

signals:
	void showMinimized();
	void showMaximizedOrRestore();
	void dragWindow(QMouseEvent* event);
protected:
	// This is the method that will be called when an event is filtered.
	// It's part of the QEventFilter interface.
	bool eventFilter(QObject *watched, QEvent *event) override;

private:
	QPoint dragStartPos;
	bool isMouseBtnPressed;

	// bool m_resizing = false;
	// QPoint m_dragPosition;
	// Qt::Edges m_resizeEdges;
	// int m_resizeMargin = 5; // The pixel margin to detect resizing
};


#endif //IT_TOOLS_WINDOW_EVENT_FILTER_H
