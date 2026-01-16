#include "GameData.h"
#include <LunarTear++.h>
#include <mutex>
#include "Callbacks.h"
#include <cmath>


constexpr float PI = 3.1415926535f;

struct CameraMatrix {
    float m[4][4];
};

namespace {

    const CameraMatrix* getCameraMatrix() {
        uintptr_t camManagerAddr = LunarTear::Get().Game().GetProcessBaseAddress() + 0x32e7a00;
        if (!camManagerAddr) return nullptr;
        return (const CameraMatrix*)(camManagerAddr + 0x240);
    }
}

GameData& GameData::instance()
{
    static GameData s_instance;
    return s_instance;
}


bool GameData::isGameActive()
{

    try {
        return LunarTear::Get().Game().GetActorPlayable() != nullptr;
    }
    catch (const LunarTearUninitializedError& e) {
        return false;
    }
}

std::span<replicant::raw::RawWeaponBody*> GameData::getWeaponSpecs() {
	auto weaponSpecBase = reinterpret_cast<replicant::raw::RawWeaponBody**>(LunarTear::Get().Game().GetProcessBaseAddress() + 0x47c2670);

    return std::span<replicant::raw::RawWeaponBody*>(
        weaponSpecBase,
        64
	);

}



QString GameData::getCurrentPhase()
{
    PlayerSaveData* saveData = LunarTear::Get().Game().GetPlayerSaveData();
    if (saveData) {
        return QString::fromUtf8(saveData->current_phase, strnlen(saveData->current_phase, sizeof(saveData->current_phase)));
    }
    return QString();
}

QVector3D GameData::getPlayerPosition()
{
    ActorPlayable* actor = LunarTear::Get().Game().GetActorPlayable();
    if (actor) {
        return QVector3D(actor->posX, actor->posY, actor->posZ);
    }
    return QVector3D();
}

float GameData::getPlayerRotationY()
{
    ActorPlayable* actor = LunarTear::Get().Game().GetActorPlayable();
    if (!actor) return 0.0f;
    float forwardX = actor->rotationVal3;
    float forwardZ = actor->rotationVal4;
    if (forwardX == 0.0f && forwardZ == 0.0f) return 0.0f;
    return atan2(forwardX, forwardZ) * (180.0f / PI);
}



void GameData::teleportToPoint(const QString& mapName, const QVector3D& pos, float rotY)
{
    if (mapName == getCurrentPhase()) {
        setPlayerPosition(pos, rotY);
        return;
    }

    LunarTear::Get().Log(LT_LOG_INFO) << mapName.toStdString();
    QString command = QString("_ChangeMap('%1', 0)").arg(mapName);
    LunarTear::Get().QueuePhaseScriptExecution(command.toStdString());
    enqueuePostStartTask([this, pos, rotY]() {
        setPlayerPosition(pos, rotY);
    });
    
}

void GameData::setPlayerPosition(const QVector3D& pos, float rotY)
{
    ActorPlayable* actor = LunarTear::Get().Game().GetActorPlayable();
    if (!actor) return;
    actor->posX = pos.x();
    actor->posY = pos.y();
    actor->posZ = pos.z();
    float radians = rotY * (PI / 180.0f);
    actor->rotationVal3 = sin(radians);
    actor->rotationVal4 = cos(radians);
}


float GameData::getCameraYaw()
{
    const CameraMatrix* matrix = getCameraMatrix();
    if (!matrix) return 0.0f;

    // The "forward" vector is the third column of the camera's transformation matrix.
    // We can treat the 4x4 matrix as a flat array of 16 floats.
    // The forward vector's components are at indices 8, 9, and 10.
    const float* matrixData = &(matrix->m[0][0]);
    float forwardX = matrixData[8];
    float forwardZ = matrixData[10];



    return atan2f(forwardX, forwardZ);
}

// NEW FUNCTION IMPLEMENTATION
float GameData::getCameraPitch()
{
    const CameraMatrix* matrix = getCameraMatrix();
    if (!matrix) return 0.0f;

    // Extract all three components of the forward vector (3rd column).
    const float* matrixData = &(matrix->m[0][0]);
    float forwardX = matrixData[8];
    float forwardY = matrixData[9];
    float forwardZ = matrixData[10];

    // Calculate the length of the forward vector on the horizontal (X-Z) plane.
    float horizontalDistance = sqrtf(forwardX * forwardX + forwardZ * forwardZ);

    // Calculate pitch using the standard formula, which matches the reversed code.
    // This gives the angle of the camera looking up or down.
    return atan2f(-forwardY, horizontalDistance);
}




void GameData::setInvincible(bool enabled)
{
    QString command = QString("_SetUniqueActorInvincible(-1, %1)").arg(enabled ? 500000 : 0);
    LunarTear::Get().QueuePhaseScriptExecution(command.toStdString());
}

void GameData::setMaxHpMp()
{
    CPlayerParam* params = LunarTear::Get().Game().GetPlayerParam();
    PlayerSaveData* save = LunarTear::Get().Game().GetPlayerSaveData();

    if (params) {
        params->maxHP = 1000000000;
        params->maxMP = 1000000000;

        save->current_hp = 1000000000;
        save->current_mp = 1000000000;
    }
}

void GameData::setMaxStats()
{
    CPlayerParam* params = LunarTear::Get().Game().GetPlayerParam();
    if (params) {
        params->attack_stat = 100000000;
        params->magickAttack_stat = 100000000;
        params->defense_stat = 100000000;
        params->magickDefense_stat = 100000000;
    }
}

void GameData::setGameSpeed(float factor)
{
    QString command = QString("_SetGameSpeed(%1)").arg(factor);
    LunarTear::Get().QueuePhaseScriptExecution(command.toStdString());
}

void GameData::setPlayerSpeed(float factor)
{
    QString command = QString("_SetPlayerSpeed(%1)").arg(factor);
    LunarTear::Get().QueuePhaseScriptExecution(command.toStdString());
}

void GameData::restartPhase(bool quick)
{
    if (quick) {
        LunarTear::Get().QueuePhaseScriptExecution("_RestartPhaseQuick()");
    }
    else {
        LunarTear::Get().QueuePhaseScriptExecution("_RestartPhase()");
    }
}

void GameData::setMaxMoney()
{
    LunarTear::Get().Game().GetPlayerSaveData()->gold = std::numeric_limits<int>::max();
}

void GameData::setMaxItems()
{
    LunarTear::Get().QueuePhaseScriptExecution(R"(

    for itemId = 0, 767 do
        _AddItemNum(itemId, 99999) 
    end

)");

}

void GameData::setPlayerLevel(int level)
{
    QString command = QString("_SetUniqueActorLevel(-1, %1)").arg(level);
    LunarTear::Get().QueuePhaseScriptExecution(command.toStdString());
}

void GameData::setPlayerCharacter(int id)
{
    QString command = QString("_SetPlayerModel(%1)").arg(id);
    LunarTear::Get().QueuePhaseScriptExecution(command.toStdString());
}
