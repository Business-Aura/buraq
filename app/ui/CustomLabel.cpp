//
// Created by talik on 4/19/2024.
//

#include "CustomLabel.h"

CustomLabel::CustomLabel(QWidget *parent): QLabel(parent) {
	setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
}

bool CustomLabel::event(QEvent *event) {
	if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            // Check if the click is on a child widget (like the close button)
            QWidget* childAtPos = childAt(mouseEvent->pos());
            if (childAtPos && childAtPos != this) {
                // Let the child handle the event
                return QLabel::event(event);
            }

		    emit clicked();
		    return true; // We handled the click on the label itself
        }
	}
	return QLabel::event(event);
}