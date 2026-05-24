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
		    emit clicked();
		    return true;
        }
	}
	return QLabel::event(event);
}