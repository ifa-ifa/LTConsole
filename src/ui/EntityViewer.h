#pragma once

#include <QWidget>
#include <QTableWidgetItem>
#include <QTimer>

class QTableWidget;
class QPushButton;

struct EntityInfo {
    uintptr_t pActor;
    int actorId;
    QString name;
    QString rtti;
    int hp;
    float x, y, z;
};

class EntityViewer : public QWidget
{
    Q_OBJECT

public:
    explicit EntityViewer(QWidget* parent = nullptr);

private slots:
    void refreshEntityList();
    void onTeleportToPlayerClicked();
    void onTeleportPlayerToEntityClicked();
    void onTableSelectionChanged();
    void updateButtonStates();

private:
    void setupUi();
    void applyStyling();
    void setupConnections();

    QTableWidget* m_entityTable;

    QPushButton* m_teleportToPlayerButton;
    QPushButton* m_teleportPlayerToEntityButton;

    QTimer* m_autoRefreshTimer;
    QTimer* m_stateUpdateTimer;

    std::vector<EntityInfo> m_entities;
};