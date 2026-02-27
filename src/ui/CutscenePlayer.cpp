#include "CutscenePlayer.h"
#include "LunarTear++.h"
#include "Callbacks.h"
#include "GameData.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

namespace {
    struct Scene {
        QString label;
        QString phase;
        QString script;
    };

    const QList<Scene> scenes = {
        {
            "Intro (Library)",
            "A_CENTER_LIBRARY_01",
            R"(
                _Wait(0)
                LIB_EventStart2(0, "EID_0300_a0035_End", 0, 0, 1, "MA_EID_0300_a0035", 1, 0)
            )"
        },
        {
            "Yonah's Cough",
            "A_CENTER_LIBRARY_01",
            R"lua(EID_2000_a0100_Continue)lua"
        },

        
    };
}

CutscenePlayer::CutscenePlayer(QWidget* parent) : QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto searchBox = new QLineEdit();
    searchBox->setPlaceholderText("Search...");
    mainLayout->addWidget(searchBox);

    auto scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true); 
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto scrollContent = new QWidget();
    auto contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setAlignment(Qt::AlignTop); 
    contentLayout->setSpacing(2); 

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

    this->setStyleSheet(R"(
        QLineEdit { background: #21252b; color: #abb2bf; border: 1px solid #2c313a; padding: 5px; }
        QScrollArea { background: transparent; }
        QWidget { background: transparent; }
        QPushButton { 
            background: #3a3f4b; color: #abb2bf; border: 1px solid #2c313a; 
            text-align: left; padding: 8px; border-radius: 4px; 
        }
        QPushButton:hover { background: #4b5162; color: white; }
        QPushButton:pressed { background: #61afef; color: black; }
    )");

    for (const auto& scene : scenes) {
        if (scene.label.isEmpty()) continue;

        auto btn = new QPushButton(scene.label);


        connect(btn, &QPushButton::clicked, this, [scene]() {
            if (!GameData::instance().isGameActive()) return;

            QString currentPhase = GameData::instance().getCurrentPhase();

            if (scene.phase == currentPhase) {
                LunarTear::Get().QueuePhaseScriptExecution(R"(
                        _DebugPrint("Injeced")
                        dsfsdfsdfds = function()
                        _DebugPrint("Ran")
                            LIB_EventLoad("EID_2000_a0100", "MA_EID_2000_a0100")

                               LIB_EventStart(0, "EID_2000_a0100_End", 0, 1, 1, "MA_EID_2000_a0100", 1, 0)

                        end
                                        )", [](const LuaResult) {
                        LunarTear::Get().QueuePhaseScriptCall("dsfsdfsdfds");
                    });



            }
            else {
                QString cmd = QString("_ChangeMap('%1', 0)").arg(scene.phase);
                LunarTear::Get().QueuePhaseScriptExecution(cmd.toStdString());

                enqueuePostLoadTask([script = scene.script]() {
                    //LunarTear::Get().QueuePhaseScriptCall(script.toStdString());
                    
                    
                    LunarTear::Get().QueuePhaseScriptExecution(R"(
                        dsfsdfsdfds = function()
                            LIB_EventLoad("EID_2000_a0100", "MA_EID_2000_a0100")

                               LIB_EventStart(0, "EID_2000_a0100_End", 0, 1, 1, "MA_EID_2000_a0100", 1, 0)

                        end
                                        )", [](const LuaResult) {
                            LunarTear::Get().QueuePhaseScriptCall("dsfsdfsdfds");
                   });


                   
                    
                    
                    
                    
                });
            }
            });

        contentLayout->addWidget(btn);
    }

    connect(searchBox, &QLineEdit::textChanged, this, [contentLayout](const QString& text) {
        for (int i = 0; i < contentLayout->count(); ++i) {
            QWidget* widget = contentLayout->itemAt(i)->widget();
            if (auto btn = qobject_cast<QPushButton*>(widget)) {
                bool match = btn->text().contains(text, Qt::CaseInsensitive);
                btn->setVisible(match);
            }
        }
        });
}