#include "Toolbox.h"
#include "GameData.h"
#include "Patch.h" // Include for infinite jump

#include <QPushButton>
#include <QComboBox>
#include <QTimer>
#include <LunarTear++.h>
#include <QInputDialog>
#include <QSlider>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QIntValidator>
#include <vector>
#include <Callbacks.h>
#include "MapView.h"

namespace {
    // Predefined speed levels for a non-linear slider
    const std::vector<float> speedLevels = {
        0.1f, 0.25f, 0.5f, 0.75f,
        1.0f, // Default
        1.25f, 1.5f, 2.0f, 3.0f, 5.0f, 10.0f, 20.0f, 50.0f
    };
}

Toolbox::Toolbox(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    applyStyling();
    setupConnections();

    m_stateUpdateTimer = new QTimer(this);
    connect(m_stateUpdateTimer, &QTimer::timeout, this, &Toolbox::updateButtonStates);
    m_stateUpdateTimer->start(300); 
}

void Toolbox::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop);

    auto playerTogglesGroup = new QGroupBox();
    auto playerTogglesLayout = new QHBoxLayout(playerTogglesGroup);
    m_invincibleButton = new QPushButton("Invincibility");
    m_infiniteJumpButton = new QPushButton("Infinite Jump");
    m_infiniteJumpButton->setCheckable(true); 
    m_invincibleButton->setCheckable(true);
    playerTogglesLayout->addWidget(m_invincibleButton);
    playerTogglesLayout->addWidget(m_infiniteJumpButton);
    mainLayout->addWidget(playerTogglesGroup);

    auto statCheatsGroup = new QGroupBox();
    auto statCheatsLayout = new QHBoxLayout(statCheatsGroup);
    m_maxHpMpButton = new QPushButton("Max HP & MP");
    m_maxStatsButton = new QPushButton("Max Stats");
    statCheatsLayout->addWidget(m_maxHpMpButton);
    statCheatsLayout->addWidget(m_maxStatsButton);
    mainLayout->addWidget(statCheatsGroup);

    auto worldGroup = new QGroupBox();
    auto worldLayout = new QGridLayout(worldGroup);
    m_gameSpeedSlider = new QSlider(Qt::Horizontal);
    m_gameSpeedSlider->setRange(0, speedLevels.size() - 1);
    m_gameSpeedSlider->setValue(4); // Default to 1.0f
    m_gameSpeedLabel = new QLabel("Game Speed: 1.0x");
    m_playerSpeedSlider = new QSlider(Qt::Horizontal);
    m_playerSpeedSlider->setRange(0, speedLevels.size() - 1);
    m_playerSpeedSlider->setValue(4); // Default to 1.0f
    m_playerSpeedLabel = new QLabel("Player Speed: 1.0x");
    m_restartPhaseButton = new QPushButton("Restart Phase");
    m_restartRestorePosButton = new QPushButton("Restart + Restore Pos");
    worldLayout->addWidget(m_gameSpeedLabel, 0, 0);
    worldLayout->addWidget(m_gameSpeedSlider, 0, 1);
    worldLayout->addWidget(m_playerSpeedLabel, 1, 0);
    worldLayout->addWidget(m_playerSpeedSlider, 1, 1);
    worldLayout->addWidget(m_restartPhaseButton, 2, 0);
    worldLayout->addWidget(m_restartRestorePosButton, 2, 1);
    mainLayout->addWidget(worldGroup);

    auto progressionGroup = new QGroupBox();
    auto progressionLayout = new QHBoxLayout(progressionGroup);
    m_maxMoneyButton = new QPushButton("Max Money");
    m_maxItemsButton = new QPushButton("Max Items");
    m_changeLevelButton = new QPushButton("Set Level...");
    progressionLayout->addWidget(m_maxMoneyButton);
    progressionLayout->addWidget(m_maxItemsButton);
    progressionLayout->addWidget(m_changeLevelButton);
    mainLayout->addWidget(progressionGroup);

    auto characterGroup = new QGroupBox();
    auto characterLayout = new QHBoxLayout(characterGroup);
    characterLayout->addWidget(new QLabel("Override Player Character:"));
    m_playerCharacterComboBox = new QComboBox();
    m_playerCharacterComboBox->addItem("Off", -1);
    m_playerCharacterComboBox->addItem("Prologue", 0);
    m_playerCharacterComboBox->addItem("Young", 1);
    m_playerCharacterComboBox->addItem("Adult", 2);
    m_playerCharacterComboBox->addItem("Father", 3);
    m_playerCharacterComboBox->addItem("Kaine", 4);

    auto comboBox = m_playerCharacterComboBox;
    registerPostLoadCallback([comboBox]() {
        int idx = comboBox->currentIndex();
        if (comboBox->itemData(idx).toInt() == -1) return;
        QString command = QString("_SetPlayerModel(%1)").arg(comboBox->itemData(idx).toInt());
        LunarTear::Get().QueuePhaseScriptExecution(command.toStdString());
     });


    m_spawnKaineButton = new QPushButton("Spawn Kaine");
    m_spawnEmilButton = new QPushButton("Spawn Emil");
    characterLayout->addWidget(m_playerCharacterComboBox);
    characterLayout->addStretch(1);
    characterLayout->addWidget(m_spawnKaineButton);
    characterLayout->addWidget(m_spawnEmilButton);
    mainLayout->addWidget(characterGroup);

    auto teleportGroup = new QGroupBox("Manual Teleport");
    auto teleportLayout = new QHBoxLayout(teleportGroup);
    m_getCoordsButton = new QPushButton("Get Current");
    m_teleportXEdit = new QLineEdit();
    m_teleportYEdit = new QLineEdit();
    m_teleportZEdit = new QLineEdit();
    m_teleportXEdit->setPlaceholderText("X");
    m_teleportYEdit->setPlaceholderText("Y");
    m_teleportZEdit->setPlaceholderText("Z");
    m_teleportXEdit->setValidator(new QDoubleValidator());
    m_teleportYEdit->setValidator(new QDoubleValidator());
    m_teleportZEdit->setValidator(new QDoubleValidator());
    m_teleportCoordsButton = new QPushButton("Teleport to Coords");
    teleportLayout->addWidget(m_getCoordsButton);
    teleportLayout->addWidget(m_teleportXEdit);
    teleportLayout->addWidget(m_teleportYEdit);
    teleportLayout->addWidget(m_teleportZEdit);
    teleportLayout->addWidget(m_teleportCoordsButton);
    mainLayout->addWidget(teleportGroup);

    mainLayout->addStretch(1);
    updateButtonStates(); 
}

void Toolbox::applyStyling()
{
    const QString bgColor = "#282c34";
    const QString textColor = "#abb2bf";
    const QString borderColor = "#2c313a";
    const QString highlightColor = "#61afef";
    const QString secondaryBgColor = "#21252b";
    const QString disabledColor = "#5c6370";
    const QString checkedHoverColor = "#72b8f2";

    QString styleSheet = QString(R"(
        QWidget { background-color: %1; color: %2; font-family: 'Consolas', 'Courier New', monospace; font-size: 14px; }
        QGroupBox { border: 1px solid %3; border-radius: 4px; margin-top: 1ex; }
        QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 3px; }
        QLineEdit, QComboBox, QSpinBox { background-color: %5; border: 1px solid %3; border-radius: 4px; padding: 4px; }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView { background-color: %5; border: 1px solid %3; selection-background-color: %4; }
        QPushButton { background-color: #3a3f4b; border: 1px solid %3; border-radius: 4px; padding: 5px 12px; }
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
}

void Toolbox::setupConnections()
{
    connect(m_invincibleButton, &QPushButton::toggled, this, &Toolbox::onInvincibleClicked);
    connect(m_infiniteJumpButton, &QPushButton::toggled, this, &Toolbox::onInfiniteJumpToggled);
    connect(m_maxHpMpButton, &QPushButton::clicked, this, &Toolbox::onMaxHpMpClicked);
    connect(m_maxStatsButton, &QPushButton::clicked, this, &Toolbox::onMaxStatsClicked);

    connect(m_gameSpeedSlider, &QSlider::valueChanged, this, &Toolbox::onGameSpeedChanged);
    connect(m_playerSpeedSlider, &QSlider::valueChanged, this, &Toolbox::onPlayerSpeedChanged);
    connect(m_restartPhaseButton, &QPushButton::clicked, this, &Toolbox::onRestartPhaseClicked);
    connect(m_restartRestorePosButton, &QPushButton::clicked, this, &Toolbox::onRestartRestorePosClicked);

    connect(m_maxMoneyButton, &QPushButton::clicked, this, &Toolbox::onMaxMoneyClicked);
    connect(m_maxItemsButton, &QPushButton::clicked, this, &Toolbox::onMaxItemsClicked);
    connect(m_changeLevelButton, &QPushButton::clicked, this, &Toolbox::onChangeLevelClicked);

    connect(m_spawnKaineButton, &QPushButton::clicked, this, &Toolbox::onSpawnKaineClicked);
    connect(m_spawnEmilButton, &QPushButton::clicked, this, &Toolbox::onSpawnEmilClicked);

    connect(m_getCoordsButton, &QPushButton::clicked, this, &Toolbox::onGetCoordsClicked);
    connect(m_teleportCoordsButton, &QPushButton::clicked, this, &Toolbox::onTeleportToCoordsClicked);
}


void Toolbox::onInvincibleClicked(bool checked) { GameData::instance().setInvincible(checked); }
void Toolbox::onInfiniteJumpToggled(bool checked)
{
    if (checked) {
        PatchInfiniteJump();
    }
    else {
        UnpatchInfiniteJump();
    }
    m_isInfiniteJumpPatched = checked;
}

void Toolbox::onMaxHpMpClicked() { GameData::instance().setMaxHpMp(); }
void Toolbox::onMaxStatsClicked() { GameData::instance().setMaxStats(); }

void Toolbox::onGameSpeedChanged(int value)
{
    if (value >= 0 && value < speedLevels.size()) {
        float speed = speedLevels[value];
        m_gameSpeedLabel->setText(QString("Game Speed: %1x").arg(speed, 0, 'f', 2));
        GameData::instance().setGameSpeed(speed);
    }
}

void Toolbox::onPlayerSpeedChanged(int value)
{
    if (value >= 0 && value < speedLevels.size()) {
        float speed = speedLevels[value];
        m_playerSpeedLabel->setText(QString("Player Speed: %1x").arg(speed, 0, 'f', 2));
        GameData::instance().setPlayerSpeed(speed);
    }
}

void Toolbox::onRestartPhaseClicked() { GameData::instance().restartPhase(false); }
void Toolbox::onRestartRestorePosClicked() { 
    

    QVector3D pos = GameData::instance().getPlayerPosition();
    float rotY = GameData::instance().getPlayerRotationY();

    LunarTear::Get().QueuePhaseScriptExecution("_RestartPhase()");
    enqueuePostStartTask([this, pos, rotY]() {

        GameData::instance().setPlayerPosition(pos, rotY);

        });

}

void Toolbox::onMaxMoneyClicked() { GameData::instance().setMaxMoney(); }
void Toolbox::onMaxItemsClicked() { GameData::instance().setMaxItems(); }
void Toolbox::onChangeLevelClicked()
{
    bool ok;
    int level = QInputDialog::getInt(this, "Set Level", "Enter new level:", 1, 1, std::numeric_limits<int>::max(), 1, &ok);
    if (ok) {
        GameData::instance().setPlayerLevel(level);
    }
}


void Toolbox::onSpawnKaineClicked() { LunarTear::Get().QueuePhaseScriptExecution("_ActivateUniqueActor(-2); _EndUniqueActorScriptMode(-2);"); }
void Toolbox::onSpawnEmilClicked() { LunarTear::Get().QueuePhaseScriptExecution("_ActivateUniqueActor(-3); _EndUniqueActorScriptMode(-3);"); }

void Toolbox::onGetCoordsClicked()
{
    if (!GameData::instance().isGameActive()) {
        m_teleportXEdit->clear();
        m_teleportYEdit->clear();
        m_teleportZEdit->clear();
        return;
    }
    QVector3D pos = GameData::instance().getPlayerPosition();
    m_teleportXEdit->setText(QString::number(pos.x()));
    m_teleportYEdit->setText(QString::number(pos.y()));
    m_teleportZEdit->setText(QString::number(pos.z()));
}

void Toolbox::onTeleportToCoordsClicked()
{
    if (m_teleportXEdit->text().isEmpty() || m_teleportYEdit->text().isEmpty() || m_teleportZEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Teleport", "All coordinate fields must be filled.");
        return;
    }
    QVector3D pos(
        m_teleportXEdit->text().toFloat(),
        m_teleportYEdit->text().toFloat(),
        m_teleportZEdit->text().toFloat()
    );
    float rot = GameData::instance().getPlayerRotationY();
    GameData::instance().setPlayerPosition(pos, rot);
}

void Toolbox::updateButtonStates()
{
    bool isActive = GameData::instance().isGameActive();
    this->setEnabled(isActive);

    if (isActive) {
        m_infiniteJumpButton->setChecked(m_isInfiniteJumpPatched);
    }
}