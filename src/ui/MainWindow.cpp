#include <QMainWindow>
#include "MainWindow.h"
#include "Terminal.h"
#include "Atlas.h"
#include "LuaConsoleManager.h"
#include <QApplication>
#include <QPushButton>
#include "Toolbox.h" 
#include "util/AtlasImporter.h"
#include "LunarTear++.h"
#include <iostream>

MainWindow::MainWindow() {
	this->resize(700, 700);
    
    //QString modBasePath = QString::fromStdString(LunarTear::Get().GetModDirectory("LTCon"));
    //QString inputPath = modBasePath + "/resource/tables";
    //QString outputPath = modBasePath + "/resource/atlas";
    //AtlasImporter importer;
    //importer.run(inputPath, outputPath);

	Terminal* terminal = new Terminal();
	terminal->show();
    uint64_t termId = LuaConsoleManager::instance().registerTerminal(terminal);
   
    connect(terminal, &QObject::destroyed, [termId]() {
        LuaConsoleManager::instance().unregisterTerminal(termId);
        });
   
    connect(terminal, &Terminal::commandEntered, [termId](const QString& cmd) {
        LuaConsoleManager::instance().enqueueCommand(termId, cmd);
        });


    Atlas* atlas = new Atlas();
    atlas->show();

    Toolbox* toolbox = new Toolbox();
    toolbox->show();
}