#include "Inspector.h"
#include "GameData.h"
#include <LunarTear++.h>
#include <QVBoxLayout>
#include <QHeaderView>
#include "common/GameStrings.h"
#include <QSpinBox>
#include <QTreeWidgetItemIterator>
#include <QScrollBar>
#include <QApplication>
#include <QLineEdit>
#include <replicant/weapon.h> 

using namespace replicant::raw;
using namespace replicant::weapon;

// --- Robust Field Definitions ---

enum FieldType { TYPE_UINT, TYPE_INT, TYPE_FLOAT, TYPE_UINT8 };

struct FieldDef {
    const char* id;
    const char* name;
    size_t offset;
    FieldType type;
};

// Main body fields - Complete mapping of RawWeaponBody
static const std::vector<FieldDef> kBodyFields = {
    // 0x00 - 0x0F
    { "u00", "UInt32 0x00", offsetof(RawWeaponBody, uint32_0x00), TYPE_UINT },
    { "u04", "UInt32 0x04", offsetof(RawWeaponBody, uint32_0x04), TYPE_UINT },
    { "ptrJpName", "Ptr JP Name (0x08)", offsetof(RawWeaponBody, offestToJPName), TYPE_UINT },
    { "ptrIntName", "Ptr Int Name (0x0C)", offsetof(RawWeaponBody, offsetToInternalWeaponName), TYPE_UINT },

    // 0x10 - 0x2B (IDs)
    { "weaponID", "Weapon ID", offsetof(RawWeaponBody, weaponID), TYPE_UINT },
    { "nameStringID", "Name ID", offsetof(RawWeaponBody, nameStringID), TYPE_UINT },
    { "descStringID", "Desc ID", offsetof(RawWeaponBody, descStringID), TYPE_UINT },
    { "unkStringID1", "Unk Str 1", offsetof(RawWeaponBody, unkStringID1), TYPE_UINT },
    { "unkStringID2", "Unk Str 2", offsetof(RawWeaponBody, unkStringID2), TYPE_UINT },
    { "unkStringID3", "Unk Str 3", offsetof(RawWeaponBody, unkStringID3), TYPE_UINT },
    { "storyStringID", "Story ID", offsetof(RawWeaponBody, storyStringID), TYPE_UINT },

    // 0x2C - 0x3B
    { "u2c", "UInt32 0x2C", offsetof(RawWeaponBody, uint32_0x2C), TYPE_UINT },
    { "u30", "UInt32 0x30", offsetof(RawWeaponBody, uint32_0x30), TYPE_UINT },
    { "u34", "UInt32 0x34", offsetof(RawWeaponBody, uint32_0x34), TYPE_UINT },
    { "listOrder", "List Order", offsetof(RawWeaponBody, listOrder), TYPE_UINT },

    // 0x3C - 0x47
    { "f3c", "Float 0x3C", offsetof(RawWeaponBody, float_0x3C), TYPE_FLOAT },
    { "u40", "UInt32 0x40", offsetof(RawWeaponBody, uint32_0x40), TYPE_UINT },
    { "u44", "UInt32 0x44", offsetof(RawWeaponBody, uint32_0x44), TYPE_UINT },

    // 0x48 - 0x53
    { "f48", "Float 0x48", offsetof(RawWeaponBody, float_0x48), TYPE_FLOAT },
    { "u4c", "UInt32 0x4C", offsetof(RawWeaponBody, uint32_0x4C), TYPE_UINT },
    { "u50", "UInt32 0x50", offsetof(RawWeaponBody, uint32_0x50), TYPE_UINT },

    // 0x54 - 0x5F
    { "heightBack", "Height On Back (0x54)", offsetof(RawWeaponBody, float_0x54), TYPE_FLOAT },
    { "u58", "UInt32 0x58", offsetof(RawWeaponBody, uint32_0x58), TYPE_UINT },
    { "u5c", "UInt32 0x5C", offsetof(RawWeaponBody, uint32_0x5C), TYPE_UINT },

    // 0x60 - 0x6B
    { "f60", "Float 0x60", offsetof(RawWeaponBody, float_0x60), TYPE_FLOAT },
    { "u64", "UInt32 0x64", offsetof(RawWeaponBody, uint32_0x64), TYPE_UINT },
    { "u68", "UInt32 0x68", offsetof(RawWeaponBody, uint32_0x68), TYPE_UINT },

    // 0x6C - 0x77
    { "handDisp", "In Hand Displacement (0x6C)", offsetof(RawWeaponBody, float_0x6C), TYPE_FLOAT },
    { "u70", "UInt32 0x70", offsetof(RawWeaponBody, uint32_0x70), TYPE_UINT },
    { "u74", "UInt32 0x74", offsetof(RawWeaponBody, uint32_0x74), TYPE_UINT },

    // 0x78 - 0x83
    { "f78", "Float 0x78", offsetof(RawWeaponBody, float_0x78), TYPE_FLOAT },
    { "u7c", "UInt32 0x7C", offsetof(RawWeaponBody, uint32_0x7C), TYPE_UINT },
    { "u80", "UInt32 0x80", offsetof(RawWeaponBody, uint32_0x80), TYPE_UINT },

    // 0x84 - 0x93
    { "i84", "Int32 0x84", offsetof(RawWeaponBody, int32_0x84), TYPE_INT },
    { "shopPrice", "Shop Price", offsetof(RawWeaponBody, shopPrice), TYPE_UINT },
    { "knockback", "Knockback %", offsetof(RawWeaponBody, knockbackPercent), TYPE_FLOAT },
    { "u90", "UInt32 0x90", offsetof(RawWeaponBody, uint32_0x90), TYPE_UINT },

    // ... Stats/Recipes in between ...

    // 0x168 - 0x170
    { "u168", "UInt32 0x168", offsetof(RawWeaponBody, uint32_0x168), TYPE_UINT },
    { "b16c", "UInt8 0x16C", offsetof(RawWeaponBody, uint8_0x16C), TYPE_UINT8 },
    { "b16d", "UInt8 0x16D", offsetof(RawWeaponBody, uint8_0x16D), TYPE_UINT8 },
    { "b16e", "UInt8 0x16E", offsetof(RawWeaponBody, uint8_0x16E), TYPE_UINT8 },
    { "b16f", "UInt8 0x16F", offsetof(RawWeaponBody, uint8_0x16F), TYPE_UINT8 },
    { "f170", "Float 0x170", offsetof(RawWeaponBody, float_0x170), TYPE_FLOAT },
};

// Stats sub-struct fields
static const std::vector<FieldDef> kStatsFields = {
    { "attack", "Attack", offsetof(WeaponStats, attack), TYPE_UINT },
    { "magicPower", "Magic Power", offsetof(WeaponStats, magicPower), TYPE_UINT },
    { "guardBreak", "Guard Break", offsetof(WeaponStats, guardBreak), TYPE_UINT },
    { "armourBreak", "Armour Break", offsetof(WeaponStats, armourBreak), TYPE_UINT },
    { "weight", "Weight", offsetof(WeaponStats, weight), TYPE_UINT },
    { "f10", "Float 0x10", offsetof(WeaponStats, float_0x10), TYPE_FLOAT },
    { "f18", "Float 0x18", offsetof(WeaponStats, float_0x18), TYPE_FLOAT },
    { "f1c", "Float 0x1C", offsetof(WeaponStats, float_0x1c), TYPE_FLOAT },
};

// Recipe sub-struct fields
static const std::vector<FieldDef> kRecipeFields = {
    { "cost", "Upgrade Cost", offsetof(WeaponUpgradeRecipe, upgradeCost), TYPE_UINT },
    { "id1", "Ingr 1 ID", offsetof(WeaponUpgradeRecipe, ingredientId1), TYPE_INT },
    { "ct1", "Ingr 1 Count", offsetof(WeaponUpgradeRecipe, ingredientCount1), TYPE_UINT },
    { "id2", "Ingr 2 ID", offsetof(WeaponUpgradeRecipe, ingredientId2), TYPE_INT },
    { "ct2", "Ingr 2 Count", offsetof(WeaponUpgradeRecipe, ingredientCount2), TYPE_UINT },
    { "id3", "Ingr 3 ID", offsetof(WeaponUpgradeRecipe, ingredientId3), TYPE_INT },
    { "ct3", "Ingr 3 Count", offsetof(WeaponUpgradeRecipe, ingredientCount3), TYPE_UINT },
};

static const std::map<QString, size_t> kSubStructOffsets = {
    { "lvl1", offsetof(RawWeaponBody, level1Stats) },
    { "lvl2", offsetof(RawWeaponBody, level2Stats) },
    { "lvl2rcp", offsetof(RawWeaponBody, level2Recipe) },
    { "lvl3", offsetof(RawWeaponBody, level3Stats) },
    { "lvl3rcp", offsetof(RawWeaponBody, level3Recipe) },
    { "lvl4", offsetof(RawWeaponBody, level4Stats) },
    { "lvl4rcp", offsetof(RawWeaponBody, level4Recipe) }
};

// --- Implementation ---

void safeStringCopy(char* dest, const QString& src, size_t destSize) {
    QByteArray utf8 = src.toUtf8();
    strncpy(dest, utf8.constData(), destSize - 1);
    dest[destSize - 1] = '\0';
}

Inspector::Inspector(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    applyStyling();
    setupConnections();
    updateWidgetState();

    m_autoRefreshTimer = new QTimer(this);
    connect(m_autoRefreshTimer, &QTimer::timeout, this, &Inspector::refreshData);
    m_autoRefreshTimer->start(1000);

    m_stateUpdateTimer = new QTimer(this);
    connect(m_stateUpdateTimer, &QTimer::timeout, this, &Inspector::updateWidgetState);
    m_stateUpdateTimer->start(300);
}

void Inspector::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);

    m_searchBox = new QLineEdit();
    m_searchBox->setPlaceholderText("Filter properties...");

    m_treeWidget = new QTreeWidget();
    m_treeWidget->setColumnCount(2);
    m_treeWidget->setHeaderLabels({ "Property", "Value" });
    m_treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_treeWidget->header()->setSectionResizeMode(1, QHeaderView::Stretch);

    mainLayout->addWidget(m_searchBox);
    mainLayout->addWidget(m_treeWidget);
}

void Inspector::applyStyling()
{
    const QString bgColor = "#282c34";
    const QString textColor = "#abb2bf";
    const QString borderColor = "#2c313a";
    const QString highlightColor = "#61afef";
    const QString secondaryBgColor = "#21252b";

    QString styleSheet = QString(R"(
        QWidget { background-color: %1; color: %2; font-family: 'Consolas', 'Courier New', monospace; }
        QLineEdit { background-color: %5; border: 1px solid %3; border-radius: 4px; padding: 4px; }
        QTreeWidget { background-color: %5; border: 1px solid %3; }
        QTreeWidget::item:selected { background-color: %4; color: %5; }
        QTreeWidget::item { padding: 3px; }
        QHeaderView::section { background-color: %1; border: 1px solid %3; padding: 4px; }
    )").arg(bgColor, textColor, borderColor, highlightColor, secondaryBgColor);
    this->setStyleSheet(styleSheet);
}

void Inspector::setupConnections()
{
    connect(m_treeWidget, &QTreeWidget::itemChanged, this, &Inspector::onItemChanged);
    connect(m_searchBox, &QLineEdit::textChanged, this, &Inspector::onSearchChanged);
}

void Inspector::refreshData()
{
    QWidget* focusedWidget = QApplication::focusWidget();
    bool isEditing = (focusedWidget != nullptr) && (focusedWidget->parentWidget() == m_treeWidget->viewport());

    if (!this->isEnabled() || !this->isVisible() || isEditing) {
        return;
    }

    m_isUpdatingTree = true;

    int scrollPosition = m_treeWidget->verticalScrollBar()->value();

    QString selectedInternalId;
    int selectedExtraId = -1;
    QTreeWidgetItem* current = m_treeWidget->currentItem();

    if (current) {
        // Fallback logic for collapsed parents
        QTreeWidgetItem* p = current->parent();
        while (p) {
            if (!p->isExpanded()) {
                current = p;
            }
            p = p->parent();
        }

        selectedInternalId = current->data(0, Qt::UserRole).toString();

        if (selectedInternalId == "save/inventory" || selectedInternalId == "save/weapon_levels") {
            selectedExtraId = current->data(0, Qt::UserRole + 1).toInt();
        }
    }

    QMap<QString, bool> expansionState;
    QTreeWidgetItemIterator it_exp(m_treeWidget);
    while (*it_exp) {
        if ((*it_exp)->isExpanded()) {
            QString path = (*it_exp)->text(0);
            QTreeWidgetItem* p = (*it_exp)->parent();
            while (p) {
                path = p->text(0) + "/" + path;
                p = p->parent();
            }
            expansionState[path] = true;
        }
        ++it_exp;
    }

    m_treeWidget->clear();
    populateTree();

    QTreeWidgetItemIterator it_restore(m_treeWidget);
    while (*it_restore) {
        QString path = (*it_restore)->text(0);
        QTreeWidgetItem* p = (*it_restore)->parent();
        while (p) {
            path = p->text(0) + "/" + path;
            p = p->parent();
        }
        if (expansionState.value(path, false)) {
            (*it_restore)->setExpanded(true);
        }
        ++it_restore;
    }

    onSearchChanged(m_searchBox->text());

    if (!selectedInternalId.isEmpty()) {
        QTreeWidgetItemIterator it(m_treeWidget);
        while (*it) {
            QTreeWidgetItem* item = *it;
            QString internalId = item->data(0, Qt::UserRole).toString();

            if (internalId == selectedInternalId) {
                if (internalId == "save/inventory" || internalId == "save/weapon_levels") {
                    if (item->data(0, Qt::UserRole + 1).toInt() == selectedExtraId) {
                        m_treeWidget->setCurrentItem(item);
                        item->setSelected(true);
                        break;
                    }
                }
                else {
                    m_treeWidget->setCurrentItem(item);
                    item->setSelected(true);
                    break;
                }
            }
            ++it;
        }
    }

    m_treeWidget->verticalScrollBar()->setValue(scrollPosition);
    m_isUpdatingTree = false;
}

void Inspector::populateTree()
{
    if (!GameData::instance().isGameActive()) {
        return;
    }

    auto* playerParamRoot = new QTreeWidgetItem(m_treeWidget, { "CPlayerParam" });
    playerParamRoot->setData(0, Qt::UserRole, "root/playerParam");
    populatePlayerParamNode(playerParamRoot);

    auto* saveDataRoot = new QTreeWidgetItem(m_treeWidget, { "PlayerSaveData" });
    saveDataRoot->setData(0, Qt::UserRole, "root/saveData");
    populateSaveDataNode(saveDataRoot);

    auto* weaponSpecRoot = new QTreeWidgetItem(m_treeWidget, { "Weapon Specs" });
    weaponSpecRoot->setData(0, Qt::UserRole, "root/weaponSpecs");
    populateWeaponSpecsNode(weaponSpecRoot);
}

void Inspector::populateSaveDataNode(QTreeWidgetItem* parent)
{
    PlayerSaveData* data = LunarTear::Get().Game().GetPlayerSaveData();
    if (!data) return;

    createReadOnlyItem(parent, "current_phase", QString::fromUtf8(data->current_phase));
    createEditableItem(parent, "player_name", QString::fromUtf8(data->player_name), "save/player_name");
    createEditableItem(parent, "current_hp", data->current_hp, "save/current_hp");
    createEditableItem(parent, "current_mp", data->current_mp, "save/current_mp");
    createEditableItem(parent, "current_level", data->current_level, "save/current_level");
    createEditableItem(parent, "current_xp", data->current_xp, "save/current_xp");
    createEditableItem(parent, "gold", data->gold, "save/gold");
    createReadOnlyItem(parent, "total_play_time", QString::number(data->total_play_time / 3600.0, 'f', 2) + " hours");

    auto weaponLevelsRoot = new QTreeWidgetItem(parent, { "Weapon Levels" });
    weaponLevelsRoot->setData(0, Qt::UserRole, "folder/weapon_levels");

    for (int i = 0; i < 64; ++i) {

		replicant::raw::RawWeaponBody* weaponSpec = GameData::instance().getWeaponSpecs()[i];
		if (!weaponSpec) continue;
		int nameId = weaponSpec->nameStringID;
        QString name = GetGameQString(nameId);

        if (name == "<NoText>" || name.isEmpty()) continue;

        int8_t level = data->weaponLevels[i];   

        QString valStr;
        if (level == -1) valStr = "-1 (Not Collected)";
        else valStr = QString::number(level);

        QTreeWidgetItem* item = createEditableItem(weaponLevelsRoot, name, valStr, "save/weapon_levels");
        item->setText(1, QString::number(level));
        item->setData(0, Qt::UserRole + 1, i);
    }

    auto inventoryRoot = new QTreeWidgetItem(parent, { "Inventory" });
    inventoryRoot->setData(0, Qt::UserRole, "folder/inventory");

    for (int i = 0; i < 768; ++i) {
        uint8_t count = static_cast<uint8_t>(data->Inventory[i]);
        QString name = GetGameQString(2000000 + 100 * i);
        if (name == "<NoText>") {
            continue;
        }
        QTreeWidgetItem* item = createEditableItem(inventoryRoot, name, count, "save/inventory");
        item->setData(0, Qt::UserRole + 1, i);
    }
}

void Inspector::populatePlayerParamNode(QTreeWidgetItem* parent)
{
    CPlayerParam* data = LunarTear::Get().Game().GetPlayerParam();
    if (!data) return;

    createEditableItem(parent, "maxHP", data->maxHP, "param/maxHP");
    createEditableItem(parent, "maxMP", data->maxMP, "param/maxMP");
    createEditableItem(parent, "attack_stat", data->attack_stat, "param/attack_stat");
    createEditableItem(parent, "magickAttack_stat", data->magickAttack_stat, "param/magickAttack_stat");
    createEditableItem(parent, "defense_stat", data->defense_stat, "param/defense_stat");
    createEditableItem(parent, "magickDefense_stat", data->magickDefense_stat, "param/magickDefense_stat");
}

void Inspector::populateWeaponSpecsNode(QTreeWidgetItem* parent)
{
    auto specs = GameData::instance().getWeaponSpecs();
    int index = 0;

    for (auto* body : specs) {
        if (!body) continue;

        QString weaponName = GetGameQString(body->nameStringID);
        if (weaponName.isEmpty() || weaponName == "<NoText>") {
            weaponName = QString("Weapon %1 (ID: %2)").arg(index).arg(body->weaponID);
        }

        auto* item = new QTreeWidgetItem(parent, { weaponName });
        QString baseId = "spec/" + QString::number(index) + "/";
        item->setData(0, Qt::UserRole, baseId + "root");

        // Helper to add fields
        auto addFields = [&](QTreeWidgetItem* node, const std::vector<FieldDef>& defs, size_t baseOffset, const QString& prefix) {
            for (const auto& f : defs) {
                QVariant val;
                void* ptr = (char*)body + baseOffset + f.offset;

                if (f.type == TYPE_UINT) val = *(uint32_t*)ptr;
                else if (f.type == TYPE_INT) val = *(int32_t*)ptr;
                else if (f.type == TYPE_FLOAT) val = *(float*)ptr;
                else if (f.type == TYPE_UINT8) val = *(uint8_t*)ptr;

                QString fullId = baseId + prefix + f.id;
                createEditableItem(node, f.name, val, fullId);
            }
            };

        auto* stringsNode = new QTreeWidgetItem(item, { "String IDs" });
        stringsNode->setData(0, Qt::UserRole, baseId + "folder/strings");

        auto* unkNode = new QTreeWidgetItem(item, { "Positioning / Unknowns" });
        unkNode->setData(0, Qt::UserRole, baseId + "folder/unks");

        for (const auto& f : kBodyFields) {
            QString sid = f.id;

            QTreeWidgetItem* target = item;

            // Grouping logic
            if (sid.contains("StringID")) target = stringsNode;
            // Catch anything that starts with u/f/b/i followed by a number, OR specific names
            else if (sid.startsWith("u") || sid.startsWith("f") || sid.startsWith("b") || sid.startsWith("i") ||
                sid == "heightBack" || sid == "handDisp")
            {
                // General props exception (keep weaponID, shopPrice, etc at root)
                if (sid != "weaponID" && sid != "shopPrice" && sid != "listOrder" && sid != "knockback") {
                    target = unkNode;
                }
            }

            QVariant val;
            void* ptr = (char*)body + f.offset;
            if (f.type == TYPE_UINT) val = *(uint32_t*)ptr;
            else if (f.type == TYPE_INT) val = *(int32_t*)ptr;
            else if (f.type == TYPE_FLOAT) val = *(float*)ptr;
            else if (f.type == TYPE_UINT8) val = *(uint8_t*)ptr;

            createEditableItem(target, f.name, val, baseId + f.id);
        }

        auto addSubStruct = [&](const QString& label, const QString& prefix, const std::vector<FieldDef>& fields, size_t structOffset) {
            auto* node = new QTreeWidgetItem(item, { label });
            node->setData(0, Qt::UserRole, baseId + "folder/" + prefix);
            addFields(node, fields, structOffset, prefix + "_");
            };

        addSubStruct("Level 1 Stats", "lvl1", kStatsFields, kSubStructOffsets.at("lvl1"));

        auto* lvl2 = new QTreeWidgetItem(item, { "Level 2" });
        lvl2->setData(0, Qt::UserRole, baseId + "folder/l2root");
        addFields(lvl2, kStatsFields, kSubStructOffsets.at("lvl2"), "lvl2_");
        auto* lvl2r = new QTreeWidgetItem(lvl2, { "Recipe -> Lvl 2" });
        lvl2r->setData(0, Qt::UserRole, baseId + "folder/l2rec");
        addFields(lvl2r, kRecipeFields, kSubStructOffsets.at("lvl2rcp"), "lvl2rcp_");

        auto* lvl3 = new QTreeWidgetItem(item, { "Level 3" });
        lvl3->setData(0, Qt::UserRole, baseId + "folder/l3root");
        addFields(lvl3, kStatsFields, kSubStructOffsets.at("lvl3"), "lvl3_");
        auto* lvl3r = new QTreeWidgetItem(lvl3, { "Recipe -> Lvl 3" });
        lvl3r->setData(0, Qt::UserRole, baseId + "folder/l3rec");
        addFields(lvl3r, kRecipeFields, kSubStructOffsets.at("lvl3rcp"), "lvl3rcp_");

        auto* lvl4 = new QTreeWidgetItem(item, { "Level 4" });
        lvl4->setData(0, Qt::UserRole, baseId + "folder/l4root");
        addFields(lvl4, kStatsFields, kSubStructOffsets.at("lvl4"), "lvl4_");
        auto* lvl4r = new QTreeWidgetItem(lvl4, { "Recipe -> Lvl 4" });
        lvl4r->setData(0, Qt::UserRole, baseId + "folder/l4rec");
        addFields(lvl4r, kRecipeFields, kSubStructOffsets.at("lvl4rcp"), "lvl4rcp_");

        index++;
    }
}


QTreeWidgetItem* Inspector::createEditableItem(QTreeWidgetItem* parent, const QString& name, const QVariant& value, const QString& internalId)
{
    auto* item = new QTreeWidgetItem(parent, { name, value.toString() });
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setData(0, Qt::UserRole, internalId);
    item->setForeground(0, QColor("#9da5b4"));
    return item;
}

QTreeWidgetItem* Inspector::createReadOnlyItem(QTreeWidgetItem* parent, const QString& name, const QVariant& value)
{
    auto* item = new QTreeWidgetItem(parent, { name, value.toString() });
    item->setForeground(1, QColor("#9da5b4"));
    return item;
}

void Inspector::onItemChanged(QTreeWidgetItem* item, int column)
{
    if (m_isUpdatingTree || column != 1) return;

    QString internalId = item->data(0, Qt::UserRole).toString();
    if (internalId.isEmpty()) return;

    bool ok;
    QVariant newValue = item->text(1);

    // --- Save Data / Param Handling ---
    PlayerSaveData* saveData = LunarTear::Get().Game().GetPlayerSaveData();
    CPlayerParam* playerParam = LunarTear::Get().Game().GetPlayerParam();

    if (internalId == "save/player_name" && saveData) {
        safeStringCopy(saveData->player_name, newValue.toString(), sizeof(saveData->player_name));
    }
    else if (internalId == "save/current_hp" && saveData) saveData->current_hp = newValue.toInt(&ok);
    else if (internalId == "save/current_mp" && saveData) saveData->current_mp = newValue.toFloat(&ok);
    else if (internalId == "save/current_level" && saveData) saveData->current_level = newValue.toInt(&ok);
    else if (internalId == "save/current_xp" && saveData) saveData->current_xp = newValue.toInt(&ok);
    else if (internalId == "save/gold" && saveData) saveData->gold = newValue.toInt(&ok);
    else if (internalId == "save/inventory" && saveData) {
        int itemId = item->data(0, Qt::UserRole + 1).toInt();
        int count = newValue.toInt(&ok);
        if (ok && count >= 0 && count <= 255) saveData->Inventory[itemId] = count;
    }
    else if (internalId == "save/weapon_levels" && saveData) {
        int weaponIdx = item->data(0, Qt::UserRole + 1).toInt();
        int level = newValue.toInt(&ok);
        if (ok && level >= -128 && level <= 127) {
            saveData->weaponLevels[weaponIdx] = static_cast<char>(level);
        }
    }
    else if (internalId == "param/maxHP" && playerParam) playerParam->maxHP = newValue.toInt(&ok);
    else if (internalId == "param/maxMP" && playerParam) playerParam->maxMP = newValue.toFloat(&ok);
    else if (internalId == "param/attack_stat" && playerParam) playerParam->attack_stat = newValue.toInt(&ok);
    else if (internalId == "param/magickAttack_stat" && playerParam) playerParam->magickAttack_stat = newValue.toInt(&ok);
    else if (internalId == "param/defense_stat" && playerParam) playerParam->defense_stat = newValue.toInt(&ok);
    else if (internalId == "param/magickDefense_stat" && playerParam) playerParam->magickDefense_stat = newValue.toInt(&ok);

    // --- Robust Weapon Specs Handling ---
    else if (internalId.startsWith("spec/")) {
        auto parts = internalId.split('/');
        if (parts.size() < 2) return;

        int index = parts[1].toInt();
        QString fieldId = parts.mid(2).join('/');

        auto specs = GameData::instance().getWeaponSpecs();
        if (index < 0 || index >= specs.size()) return;
        auto* body = specs[index];
        if (!body) return;

        const std::vector<FieldDef>* tableToSearch = &kBodyFields;
        size_t baseOffset = 0;

        for (auto const& [prefix, offset] : kSubStructOffsets) {
            if (fieldId.startsWith(prefix + "_")) {
                if (prefix.contains("rcp")) tableToSearch = &kRecipeFields;
                else tableToSearch = &kStatsFields;

                baseOffset = offset;
                fieldId = fieldId.mid(prefix.length() + 1);
                break;
            }
        }

        for (const auto& f : *tableToSearch) {
            if (f.id == fieldId) {
                void* ptr = (char*)body + baseOffset + f.offset;

                if (f.type == TYPE_UINT) *(uint32_t*)ptr = newValue.toUInt(&ok);
                else if (f.type == TYPE_INT) *(int32_t*)ptr = newValue.toInt(&ok);
                else if (f.type == TYPE_FLOAT) *(float*)ptr = newValue.toFloat(&ok);
                else if (f.type == TYPE_UINT8) *(uint8_t*)ptr = static_cast<uint8_t>(newValue.toUInt(&ok));
                break;
            }
        }
    }
}

void Inspector::updateWidgetState()
{
    bool isGameActive = GameData::instance().isGameActive();
    this->setEnabled(isGameActive);
    if (!isGameActive && !m_isUpdatingTree) {
        m_treeWidget->clear();
    }
}

void Inspector::onSearchChanged(const QString& text)
{
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        filterItem(m_treeWidget->topLevelItem(i), text);
    }
}

bool Inspector::filterItem(QTreeWidgetItem* item, const QString& filterText)
{
    bool textMatches = item->text(0).contains(filterText, Qt::CaseInsensitive);

    bool hasVisibleChild = false;
    for (int i = 0; i < item->childCount(); ++i) {
        if (filterItem(item->child(i), filterText)) {
            hasVisibleChild = true;
        }
    }

    bool shouldBeVisible = textMatches || hasVisibleChild;
    item->setHidden(!shouldBeVisible);

    if (hasVisibleChild && !filterText.isEmpty()) {
        item->setExpanded(true);
    }

    return shouldBeVisible;
}