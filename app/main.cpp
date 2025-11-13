//
// Created by Talik Kasozi on 2/3/2024.
//

#include "ui/Filters/ThemeManager/ThemeManager.h"
#include "app_ui/AppUi.h"

#include <QApplication>
#include <QMainWindow>
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include <QFrame>
#include <QPushButton>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Set these before creating any QSettings objects
    QCoreApplication::setOrganizationName("BizAura.app Inc");
    QCoreApplication::setApplicationName("Buraq Editor");

    const AppUi appUi{};
    appUi.showUi();

    return QApplication::exec();
}



// Helper function to create styled placeholder widgets for the frame
QWidget* createStyledPanel(const QString& text, const QString& color) {
    QLabel *panel = new QLabel(text);
    panel->setAlignment(Qt::AlignCenter);
    panel->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    panel->setStyleSheet(QString("background-color: %1; border: 1px solid #a0a0a0;").arg(color));
    return panel;
}

int frame_sample(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMainWindow window;
    window.setWindowTitle("Frame Layout (Top, Bottom, Left, Right)");

    // 1. Create a main container widget. This will be the single central widget.
    QWidget *frameContainer = new QWidget();

    // 2. Create the grid layout that the container will use.
    QGridLayout *gridLayout = new QGridLayout(frameContainer);
    gridLayout->setContentsMargins(0, 0, 0, 0); // No margins for a seamless frame
    gridLayout->setSpacing(0);                  // No spacing between frame parts

    // 3. Create the widgets for each section of the frame.
    // In your app, these would be your custom QWidget-based classes.
    QWidget *topBar = createStyledPanel("Top Bar (Title)", "#d0d0e0");
    topBar->setFixedHeight(35);

    QWidget *leftPanel = createStyledPanel("Left\nPanel", "#e0d8d0");
    leftPanel->setFixedWidth(100);

    QWidget *centerWidget = createStyledPanel("Central Widget", "#ffffff");

    QWidget *rightPanel = createStyledPanel("Right\nPanel", "#e0d8d0");
    rightPanel->setFixedWidth(100);

    QWidget *bottomBar = createStyledPanel("Bottom Bar (Status)", "#d0d0e0");
    bottomBar->setFixedHeight(25);

    // 4. Add the widgets to the grid layout at specific row/column positions.
    // The format is: addWidget(widget, row, column, rowSpan, columnSpan)
    gridLayout->addWidget(topBar,        0, 0, 1, 3); // Row 0, Col 0, spans 1 row, 3 columns
    gridLayout->addWidget(leftPanel,     1, 0, 1, 1); // Row 1, Col 0
    gridLayout->addWidget(centerWidget,  1, 1, 1, 1); // Row 1, Col 1
    gridLayout->addWidget(rightPanel,    1, 2, 1, 1); // Row 1, Col 2
    gridLayout->addWidget(bottomBar,     2, 0, 1, 3); // Row 2, Col 0, spans 1 row, 3 columns

    // 5. Set stretch factors to control resizing behavior.
    // We want the central row (1) and central column (1) to expand.
    gridLayout->setRowStretch(1, 1);    // Make row 1 stretchable
    gridLayout->setColumnStretch(1, 1); // Make column 1 stretchable

    // Set other rows/columns to have 0 stretch so they don't expand.
    gridLayout->setRowStretch(0, 0);
    gridLayout->setRowStretch(2, 0);
    gridLayout->setColumnStretch(0, 0);
    gridLayout->setColumnStretch(2, 0);

    // 6. Set the container as the central widget of the main window.
    window.setCentralWidget(frameContainer);

    window.resize(800, 600);
    window.show();

    return app.exec();
}


