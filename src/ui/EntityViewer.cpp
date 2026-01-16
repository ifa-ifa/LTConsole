#include "EntityViewer.h"
#include "GameData.h"
#include <LunarTear++.h>

#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QScrollBar>
#include <QMessageBox>
#include <QVector3D>
#include <QMenu>


#pragma pack(push, 1)
struct CParamSet {
    void** vtable;
    char padding1[3 * 8];
    int cparamsetforvehcile_id;
    char padding2[89 * 8];
    int health;
    char padding3[6 * 8];
    float x_pos;
    float y_pos;
    float z_pos;
};

struct cActor {
    void* vtable;
    cActor* prev_actor_primary;
    cActor* next_actor_primary;
    cActor* prev_actor_secondary;
    cActor* next_actor_secondary;
    char padding1[0x1B0];
    int actor_id;
    char padding2[0x14174];
    CParamSet* cparams;
};

struct ActorListController {
    cActor* head;
    cActor* tail;
    int64_t count;
};

struct ActorListControllerPair {
    ActorListController primary_list;
    ActorListController secondary_list;
};
#pragma pack(pop)


namespace {
    constexpr uintptr_t MANAGER_ADDRESS_OFFSET = 0x2ca6e00;
    constexpr uintptr_t RTTI_NAME_OFFSET = 0x10;

    QString getRttiName(void* vtable) {
        if (!vtable) return "N/A";
        try {
            uintptr_t rtti_col_ptr = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(vtable) - 8);
            if (!rtti_col_ptr) return "N/A";

            uintptr_t baseAddr = LunarTear::Get().Game().GetProcessBaseAddress();
            uint32_t hierarchy_desc_rva = *reinterpret_cast<uint32_t*>(rtti_col_ptr + 0x10);
            uintptr_t hierarchy_desc_addr = baseAddr + hierarchy_desc_rva;
            uint32_t num_base_classes = *reinterpret_cast<uint32_t*>(hierarchy_desc_addr + 0x08);
            if (num_base_classes == 0) return "N/A";

            uint32_t base_class_array_rva = *reinterpret_cast<uint32_t*>(hierarchy_desc_addr + 0x0C);
            uintptr_t base_class_array_addr = baseAddr + base_class_array_rva;
            uint32_t desc_rva = *reinterpret_cast<uint32_t*>(base_class_array_addr);
            uintptr_t type_desc_rva_ptr = baseAddr + desc_rva;
            uint32_t type_desc_rva = *reinterpret_cast<uint32_t*>(type_desc_rva_ptr);
            uintptr_t type_desc_addr = baseAddr + type_desc_rva;

            char name_buffer[256];
            memcpy(name_buffer, reinterpret_cast<void*>(type_desc_addr + RTTI_NAME_OFFSET), 255);
            name_buffer[255] = '\0';

            return QString(name_buffer).split("@@")[0].remove(".?AV");

        }
        catch (...) {
            return "Parse Error";
        }
    }
}

EntityViewer::EntityViewer(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    applyStyling();
    setupConnections();
    updateWidgetState();

    m_autoRefreshTimer = new QTimer(this);
    connect(m_autoRefreshTimer, &QTimer::timeout, this, &EntityViewer::refreshEntityList);
    m_autoRefreshTimer->start(500);

    m_stateUpdateTimer = new QTimer(this);
    connect(m_stateUpdateTimer, &QTimer::timeout, this, &EntityViewer::updateWidgetState);
    m_stateUpdateTimer->start(300);
}

void EntityViewer::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    m_entityTable = new QTableWidget();

    m_entityTable->setColumnCount(6);
    m_entityTable->setHorizontalHeaderLabels({ "Address", "RTTI", "Actor ID", "HP", "Position", "Distance" });

    m_entityTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_entityTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_entityTable->verticalHeader()->setVisible(false);
    m_entityTable->setContextMenuPolicy(Qt::CustomContextMenu);

    QHeaderView* header = m_entityTable->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setStretchLastSection(true);

    mainLayout->addWidget(m_entityTable);
}

void EntityViewer::applyStyling()
{
    const QString bgColor = "#282c34";
    const QString textColor = "#abb2bf";
    const QString borderColor = "#2c313a";
    const QString highlightColor = "#61afef";
    const QString secondaryBgColor = "#21252b";

    QString styleSheet = QString(R"(
        QWidget { background-color: %1; color: %2; font-family: 'Consolas', 'Courier New', monospace;  }
        QTableWidget { background-color: %5; border: 1px solid %3; gridline-color: %3; }
        QTableWidget::item:selected { background-color: %4; color: %5; }
        QHeaderView::section { background-color: %1; border: 1px solid %3; padding: 4px; }
        QMenu { background-color: #3a3f4b; border: 1px solid %3; }
        QMenu::item:selected { background-color: %4; }
        
    )").arg(bgColor, textColor, borderColor, highlightColor, secondaryBgColor);
    this->setStyleSheet(styleSheet);
}

void EntityViewer::setupConnections()
{
    connect(m_entityTable, &QTableWidget::customContextMenuRequested, this, &EntityViewer::onContextMenuRequested);
}

void EntityViewer::refreshEntityList()
{
    if (!GameData::instance().isGameActive()) {
        m_entityTable->setRowCount(0);
        return;
    }

    uintptr_t selectedActorAddress = 0;
    int scrollPosition = 0;
    int currentRow = m_entityTable->currentRow();

    if (currentRow >= 0 && currentRow < m_entities.size()) {
        selectedActorAddress = m_entities[currentRow].pActor;
        scrollPosition = m_entityTable->verticalScrollBar()->value();
    }

    m_entityTable->blockSignals(true);
    m_entityTable->setSortingEnabled(false);

    m_entities.clear();

    uintptr_t baseAddr = LunarTear::Get().Game().GetProcessBaseAddress();
    auto* pManager = reinterpret_cast<ActorListControllerPair*>(baseAddr + MANAGER_ADDRESS_OFFSET);
    ActorListController* list = &pManager->secondary_list;

    cActor* currentActor = list->head;
    if (currentActor) {
        QVector3D playerPos = GameData::instance().getPlayerPosition();
        do {
            EntityInfo info;
            info.pActor = reinterpret_cast<uintptr_t>(currentActor);
            info.actorId = currentActor->actor_id;
            info.rtti = getRttiName(currentActor->vtable);

            //if (currentActor->cparams) {
            //    info.hp = currentActor->cparams->health;
            //    info.x = currentActor->cparams->x_pos;
            //    info.y = currentActor->cparams->y_pos;
            //    info.z = currentActor->cparams->z_pos;
            //}
            //else {
                info.hp = -1;
                info.x = info.y = info.z = 0.0f;
            //}
            m_entities.push_back(info);

            currentActor = currentActor->next_actor_secondary;
        } while (currentActor && currentActor != list->head);
    }



    m_entityTable->setRowCount(m_entities.size());
    QVector3D playerPos = GameData::instance().getPlayerPosition();
    for (int i = 0; i < m_entities.size(); ++i) {
        const auto& info = m_entities[i];
        QVector3D entityPos(info.x, info.y, info.z);
        float distance = playerPos.distanceToPoint(entityPos);

        m_entityTable->setItem(i, 0, new QTableWidgetItem(QString("0x%1").arg(info.pActor, 0, 16)));
        m_entityTable->setItem(i, 1, new QTableWidgetItem(info.rtti));
        m_entityTable->setItem(i, 2, new QTableWidgetItem(QString::number(info.actorId)));
        m_entityTable->setItem(i, 3, new QTableWidgetItem(info.hp == -1 ? "N/A" : QString::number(info.hp)));
        QString posString = QString("%1, %2, %3")
            .arg(info.x, 0, 'f', 2)
            .arg(info.y, 0, 'f', 2)
            .arg(info.z, 0, 'f', 2);
        m_entityTable->setItem(i, 4, new QTableWidgetItem(posString));
        m_entityTable->setItem(i, 5, new QTableWidgetItem(QString::number(distance, 'f', 2)));
    }

    int newRowToSelect = -1;
    if (selectedActorAddress != 0) {
        for (size_t i = 0; i < m_entities.size(); ++i) {
            if (m_entities[i].pActor == selectedActorAddress) {
                newRowToSelect = i;
                break;
            }
        }
    }

    if (newRowToSelect != -1) {
        m_entityTable->selectRow(newRowToSelect);
        m_entityTable->verticalScrollBar()->setValue(scrollPosition);
    }

    m_entityTable->setSortingEnabled(true);
    m_entityTable->blockSignals(false);
}

void EntityViewer::onTeleportToPlayerClicked()
{
    if (m_entityTable->selectedItems().isEmpty()) return;

    int selectedRow = m_entityTable->currentRow();
    const auto& entityInfo = m_entities[selectedRow];

    auto* actor = reinterpret_cast<cActor*>(entityInfo.pActor);
    if (actor && actor->cparams) {
        QVector3D playerPos = GameData::instance().getPlayerPosition();
        actor->cparams->x_pos = playerPos.x();
        actor->cparams->y_pos = playerPos.y();
        actor->cparams->z_pos = playerPos.z();
    }
}

void EntityViewer::onTeleportPlayerToEntityClicked()
{
    if (m_entityTable->selectedItems().isEmpty()) return;

    int selectedRow = m_entityTable->currentRow();
    const auto& entityInfo = m_entities[selectedRow];

    QVector3D entityPos(entityInfo.x, entityInfo.y, entityInfo.z);
    float currentRot = GameData::instance().getPlayerRotationY();
    GameData::instance().setPlayerPosition(entityPos, currentRot);
}

void EntityViewer::onContextMenuRequested(const QPoint& pos)
{
    QTableWidgetItem* item = m_entityTable->itemAt(pos);
    if (!item) {
        return;
    }

    QMenu contextMenu;
    QAction* teleportToPlayerAction = contextMenu.addAction("Teleport to Me");
    QAction* teleportPlayerToEntityAction = contextMenu.addAction("Teleport Me to It");

    connect(teleportToPlayerAction, &QAction::triggered, this, &EntityViewer::onTeleportToPlayerClicked);
    connect(teleportPlayerToEntityAction, &QAction::triggered, this, &EntityViewer::onTeleportPlayerToEntityClicked);

    contextMenu.exec(m_entityTable->viewport()->mapToGlobal(pos));
}

void EntityViewer::updateWidgetState()
{
    bool isGameActive = GameData::instance().isGameActive();
    this->setEnabled(isGameActive);
}