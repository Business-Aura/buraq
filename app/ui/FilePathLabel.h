//
// Created by talik on 3/12/2024.
//

#ifndef IT_TOOLS_FILEPATHLABEL_H
#define IT_TOOLS_FILEPATHLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QStyle>
#include <QMenu>
#include <QAction>
#include <QPoint>
#include <QHBoxLayout>
#include <QPushButton>
#include <utility>
#include "CustomDrawer.h"
#include "CustomLabel.h"

class FilePathLabel final : public CustomLabel
{
    Q_OBJECT

signals:
    void removeRequested(const QString& path);

public slots:
    void activeLabel()
    {
        isActive = true;
        setStyleSheet("FilePathLabel { background-color: #3e3e42; color: white; padding: 5px; padding-right: 25px; border-radius: 3px; }");
    }

    void reset()
    {
        isActive = false;
        setStyleSheet("FilePathLabel { background-color: transparent; color: #cccccc; padding: 5px; padding-right: 25px; }");
    }

private slots:
    void showContextMenu(const QPoint& pos)
    {
        QMenu contextMenu(this);
        QAction* removeAction = contextMenu.addAction("Remove from Workspace");
        connect(removeAction, &QAction::triggered, this, [this]() {
            emit removeRequested(filePath);
        });
        contextMenu.exec(mapToGlobal(pos));
    }

public:
    explicit FilePathLabel(QString filePath, QWidget* parent = nullptr)
        : CustomLabel(parent), isActive(false), filePath(std::move(filePath))
    {
        setMouseTracking(true);
        reset();
        setMinimumHeight(30);

        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(5, 0, 5, 0);
        layout->setSpacing(0);

        auto* removeBtn = new QPushButton("✕", this);
        removeBtn->setFixedSize(20, 20);
        removeBtn->setFlat(true);
        removeBtn->setCursor(Qt::PointingHandCursor);
        removeBtn->setStyleSheet("QPushButton { color: #888; border: none; font-weight: bold; background: transparent; } "
                                 "QPushButton:hover { color: #ff5555; }");
        
        connect(removeBtn, &QPushButton::clicked, this, [this]() {
            emit removeRequested(getFilePath());
        });

        layout->addStretch();
        layout->addWidget(removeBtn);

        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, &QWidget::customContextMenuRequested, this, &FilePathLabel::showContextMenu);
    }

    ~FilePathLabel() override = default;

    QString getFilePath()
    {
        return filePath;
    };

private:
    bool isActive;
    QString filePath;
};

#endif //IT_TOOLS_FILEPATHLABEL_H
