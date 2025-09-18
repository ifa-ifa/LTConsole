#include <QMainWindow>
#include "MainWindow.h"
#include "Terminal.h"
#include "Atlas.h"
#include "LuaConsoleManager.h"
#include <QApplication>
#include <QSettings>
#include <QPushButton>
#include "Inspector.h"
#include "Toolbox.h" 
#include "util/AtlasImporter.h"
#include "InfoWidget.h"
#include "EntityViewer.h"
#include "LunarTear++.h"
#include <iostream>

constexpr int Mainwindow_Version = 1;

MainWindow::MainWindow() {

    const QString bgColor = "#332834";
    const QString textColor = "#abb2bf";
    const QString borderColor = "#2c313a";
    const QString highlightColor = "#61afef";
    const QString secondaryBgColor = "#21252b";
    const QString disabledColor = "#5c6370";
    const QString checkedHoverColor = "#72b8f2";

    QString styleSheet = QString(R"(
        QWidget { background-color: %1; color: %2; font-family: 'Consolas', 'Courier New', monospace; font-size: 14px; }
        QDockWidget::title {
            background-color: #3a3f4b;
            padding: 5px;
            border: 1px solid %3;
        }
        QGroupBox { border: 1px solid %3; border-radius: 4px; margin-top: 1ex; }
        QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 3px; }
        QLineEdit, QComboBox, QSpinBox { background-color: %5; border: 1px solid %3; border-radius: 4px; padding: 4px; }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView { background-color: %5; border: 1px solid %3; selection-background-color: %4; }
        QPushButton { background-color: #3a3f4b; border: 1px solid %3; border-radius: 4px; padding: 3px 8px; }
        QPushButton:hover { background-color: #4b5162; }
        QPushButton:pressed { background-color: #404552; }
        QPushButton:disabled { background-color: %3; color: %6; }
        QPushButton:checked { background-color: %4; color: %5; border-color: %4; }
        QPushButton:checked { background-color: %4; color: %5; border-color: %4; }
        QPushButton:checked:hover { background-color: %7; border-color: %7; }
        QSlider::groove:horizontal { border: 1px solid %3; height: 4px; background: %5; margin: 2px 0; border-radius: 2px; }
        QSlider::handle:horizontal { background: %4; border: 1px solid %4; width: 14px; margin: -6px 0; border-radius: 7px; }
    )").arg(bgColor, textColor, borderColor, highlightColor, secondaryBgColor, disabledColor, checkedHoverColor);
    this->setStyleSheet(styleSheet);

    setCentralWidget(nullptr);
    
    Atlas* atlas = new Atlas();
    Toolbox* toolbox = new Toolbox();
    EntityViewer* entityViewer = new EntityViewer();
    InfoWidget* infoWidget = new InfoWidget();
    Inspector* inspector = new Inspector();
	Terminal* terminal = new Terminal();

    uint64_t termId = LuaConsoleManager::instance().registerTerminal(terminal);
   
    connect(terminal, &QObject::destroyed, [termId]() {
        LuaConsoleManager::instance().unregisterTerminal(termId);
        });
   
    connect(terminal, &Terminal::commandEntered, [termId](const QString& cmd) {
        LuaConsoleManager::instance().enqueueCommand(termId, cmd);
        });


    terminalDock = new QDockWidget("Terminal", this);
    terminalDock->setObjectName("TerminalDock");
    terminalDock->setWidget(terminal);

    atlasDock = new QDockWidget("Atlas", this);
    atlasDock->setObjectName("AtlasDock");
    atlasDock->setWidget(atlas);

    toolboxDock = new QDockWidget("Toolbox", this);
    toolboxDock->setObjectName("ToolboxDock");
    toolboxDock->setWidget(toolbox);

    entityViewerDock = new QDockWidget("Entity Viewer", this);
    entityViewerDock->setObjectName("EntityViewerDock");
    entityViewerDock->setWidget(entityViewer);

    inspectorDock = new QDockWidget("Inspector", this);
    inspectorDock->setObjectName("InspectorDock");
    inspectorDock->setWidget(inspector);

    infoWidgetDock = new QDockWidget("Info", this);
    infoWidgetDock->setObjectName("InfoDock");
    infoWidgetDock->setWidget(infoWidget);

    addDockWidget(Qt::BottomDockWidgetArea, terminalDock);
    addDockWidget(Qt::LeftDockWidgetArea, atlasDock);
    addDockWidget(Qt::RightDockWidgetArea, toolboxDock);
    addDockWidget(Qt::RightDockWidgetArea, entityViewerDock);
    tabifyDockWidget(toolboxDock, entityViewerDock);
    addDockWidget(Qt::RightDockWidgetArea, infoWidgetDock);
    tabifyDockWidget(entityViewerDock, inspectorDock);

    this->setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

    QMenu* windowMenu = menuBar()->addMenu(tr("&View"));

    windowMenu->addAction(terminalDock->toggleViewAction());
    windowMenu->addAction(atlasDock->toggleViewAction());
    windowMenu->addAction(toolboxDock->toggleViewAction());
    windowMenu->addAction(entityViewerDock->toggleViewAction());
    windowMenu->addAction(inspectorDock->toggleViewAction());
    windowMenu->addAction(infoWidgetDock->toggleViewAction());

    QString modBasePath = QString::fromStdString(LunarTear::Get().GetModDirectory("LTCon"));

    QString settingsPath = modBasePath + "/qt.ini";
    QSettings settings(settingsPath, QSettings::IniFormat);
    this->restoreState(settings.value("DOCK_LOCATIONS").toByteArray(), Mainwindow_Version);

    m_saveStateTimer = new QTimer(this);
    connect(m_saveStateTimer, &QTimer::timeout, this, &MainWindow::saveUiState);
    m_saveStateTimer->start(5000);

    this->show();
}

void MainWindow::saveUiState() {
    QString modBasePath = QString::fromStdString(LunarTear::Get().GetModDirectory("LTCon"));
    QString settingsPath = modBasePath + "/qt.ini";
    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.setValue("DOCK_LOCATIONS", saveState(Mainwindow_Version));
}

void MainWindow::closeEvent(QCloseEvent* event) {

    QString modBasePath = QString::fromStdString(LunarTear::Get().GetModDirectory("LTCon"));
    QString settingsPath = modBasePath + "/qt.ini";
    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.setValue("DOCK_LOCATIONS", saveState(Mainwindow_Version));

    QMainWindow::closeEvent(event); 
}