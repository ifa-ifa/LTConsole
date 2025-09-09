#pragma once
#include <QVector3D>
#include <QString>
#include "LunarTear.h"


class GameData
{
public:
    static GameData& instance();

    bool isGameActive();
    QString getCurrentPhase();
    QVector3D getPlayerPosition();
    float getPlayerRotationY();

    float getCameraYaw();
    float getCameraPitch();

    void teleportToPoint(const QString& mapName, const QVector3D& pos, float rotY);
    void setPlayerPosition(const QVector3D& pos, float rotY);

    void setInvincible(bool enabled);
    void setMaxHpMp();
    void setMaxStats();
    void setGameSpeed(float factor);
    void setPlayerSpeed(float factor);
    void restartPhase(bool quick);
    void setMaxMoney();
    void setMaxItems();
    void setPlayerLevel(int level);
    void setPlayerCharacter(int id);

private:


    GameData() = default;
    ~GameData() = default;
    GameData(const GameData&) = delete;
    GameData& operator=(const GameData&) = delete;

};