#pragma once

#include <QWidget>
#include <QTableWidgetItem>
#include <QTimer>

class QTableWidget;

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
    void updateWidgetState();
    void onContextMenuRequested(const QPoint& pos);

private:
    void setupUi();
    void applyStyling();
    void setupConnections();

    QTableWidget* m_entityTable;

    QTimer* m_autoRefreshTimer;
    QTimer* m_stateUpdateTimer;

    std::vector<EntityInfo> m_entities;
};