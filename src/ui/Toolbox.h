#pragma once
#include <QWidget>

class QPushButton;
class QSlider;
class QLabel;
class QLineEdit;
class QComboBox; 
class QTimer;

class Toolbox : public QWidget
{
    Q_OBJECT

public:
    explicit Toolbox(QWidget* parent = nullptr);

private slots:
    void onInvincibleClicked(bool checked);
    void onInfiniteJumpToggled(bool checked);

    void onMaxHpMpClicked();
    void onMaxStatsClicked();

    void onGameSpeedChanged(int value);
    void onPlayerSpeedChanged(int value);
    void onRestartPhaseClicked();
    void onRestartRestorePosClicked();

    void onMaxMoneyClicked();
    void onMaxItemsClicked();
    void onChangeLevelClicked();

    void onSpawnKaineClicked();
    void onSpawnEmilClicked();

    void onGetCoordsClicked(); 
    void onTeleportToCoordsClicked();

    void updateButtonStates();

private:
    void setupUi();
    void setupConnections();
    void applyStyling();

    QPushButton* m_invincibleButton;
    QPushButton* m_infiniteJumpButton;

    QPushButton* m_maxHpMpButton;
    QPushButton* m_maxStatsButton;

    QSlider* m_gameSpeedSlider;
    QLabel* m_gameSpeedLabel;
    QSlider* m_playerSpeedSlider;
    QLabel* m_playerSpeedLabel;
    QPushButton* m_restartPhaseButton;
    QPushButton* m_restartRestorePosButton;

    QPushButton* m_maxMoneyButton;
    QPushButton* m_maxItemsButton;
    QPushButton* m_changeLevelButton;

    QComboBox* m_playerCharacterComboBox;
    QPushButton* m_spawnKaineButton;
    QPushButton* m_spawnEmilButton;

    QPushButton* m_getCoordsButton;
    QLineEdit* m_teleportXEdit;
    QLineEdit* m_teleportYEdit;
    QLineEdit* m_teleportZEdit;
    QPushButton* m_teleportCoordsButton;

    QTimer* m_stateUpdateTimer;
    bool m_isInfiniteJumpPatched = false;
};



