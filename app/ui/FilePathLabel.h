//
// Created by talik on 3/12/2024.
//

#ifndef IT_TOOLS_FILEPATHLABEL_H
#define IT_TOOLS_FILEPATHLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QStyle>
#include <utility>
#include "CustomDrawer.h"
#include "CustomLabel.h"

class FilePathLabel final : public CustomLabel
{
    Q_OBJECT

public slots:
    void activeLabel()
    {
        isActive = true;
        setStyleSheet("background-color: #3e3e42; color: white; padding: 5px; border-radius: 3px;");
    }

    void reset()
    {
        isActive = false;
        setStyleSheet("background-color: transparent; color: #cccccc; padding: 5px;");
    }

public:
    explicit FilePathLabel(QString filePath, QWidget* parent = nullptr)
        : CustomLabel(parent), isActive(false), filePath(std::move(filePath))
    {
        setMouseTracking(true);
        reset();
        setMinimumHeight(30);
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
