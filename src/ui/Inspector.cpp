#include "Inspector.h"
#include "GameData.h"
#include <LunarTear++.h>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSpinBox>
#include <QTreeWidgetItemIterator>
#include <QScrollBar>
#include <QApplication>
#include <QLineEdit>

void safeStringCopy(char* dest, const QString& src, size_t destSize) {
    QByteArray utf8 = src.toUtf8();
    strncpy(dest, utf8.constData(), destSize - 1);
    dest[destSize - 1] = '\0';
}


Inspector::Inspector(QWidget* parent)
    : QWidget(parent)
{
    m_itemNames[0] = "Medicinal Herb";
    m_itemNames[1] = "Health Salve";
    m_itemNames[2] = "Recovery Potion";
    m_itemNames[3] = "Health Salve (Large)";
    m_itemNames[4] = "Recovery Potion (Small)";
    m_itemNames[5] = "Recovery Potion (Medium)";
    m_itemNames[6] = "Recovery Potion (Large)";
    m_itemNames[7] = "Recovery Potion (Max)";
    m_itemNames[11] = "Health Elixir (Small)";
    m_itemNames[12] = "Health Elixir (Medium)";
    m_itemNames[13] = "Health Elixir (Large)";
    m_itemNames[16] = "Lifesaver";
    m_itemNames[21] = "Strength Drop";
    m_itemNames[22] = "Strength Capsule";
    m_itemNames[23] = "Magic Drop";
    m_itemNames[24] = "Magic Capsule";
    m_itemNames[25] = "Defense Drop";
    m_itemNames[26] = "Defense Capsule";
    m_itemNames[27] = "Spirit Drop";
    m_itemNames[28] = "Spirit Capsule";
    m_itemNames[31] = "Antidotal Weed";
    m_itemNames[32] = "Echo Grass";
    m_itemNames[33] = "Smelling Salts";
    m_itemNames[41] = "Speed Fertilizer";
    m_itemNames[42] = "Flowering Fertilizer";
    m_itemNames[43] = "Bounty Fertilizer";
    m_itemNames[46] = "Pumpkin Seed";
    m_itemNames[47] = "Watermelon Seed";
    m_itemNames[48] = "Melon Seed";
    m_itemNames[49] = "Gourd Seed";
    m_itemNames[50] = "Tomato Seed";
    m_itemNames[51] = "Eggplant Seed";
    m_itemNames[52] = "Bell Pepper Seed";
    m_itemNames[53] = "Bean Seed";
    m_itemNames[54] = "Wheat Seedling";
    m_itemNames[55] = "Rice Plant Seedling";
    m_itemNames[56] = "Dahlia Bulb";
    m_itemNames[57] = "Tulip Bulb";
    m_itemNames[58] = "Freesia Bulb";
    m_itemNames[59] = "Red Moonflower Seed";
    m_itemNames[60] = "Gold Moonflower Seed";
    m_itemNames[61] = "Peach Moonflower Seed";
    m_itemNames[62] = "Pink Moonflower Seed";
    m_itemNames[63] = "Blue Moonflower Seed";
    m_itemNames[64] = "Indigo Moonflower Seed";
    m_itemNames[65] = "White Moonflower Seed";
    m_itemNames[71] = "Pumpkin";
    m_itemNames[72] = "Watermelon";
    m_itemNames[73] = "Melon";
    m_itemNames[74] = "Gourd";
    m_itemNames[75] = "Tomato";
    m_itemNames[76] = "Eggplant";
    m_itemNames[77] = "Bell Pepper";
    m_itemNames[78] = "Beans";
    m_itemNames[79] = "Wheat";
    m_itemNames[80] = "Rice";
    m_itemNames[81] = "Dahlia";
    m_itemNames[82] = "Tulip";
    m_itemNames[83] = "Freesia";
    m_itemNames[84] = "Red Moonflower";
    m_itemNames[85] = "Gold Moonflower";
    m_itemNames[86] = "Peach Moonflower";
    m_itemNames[87] = "Pink Moonflower";
    m_itemNames[88] = "Blue Moonflower";
    m_itemNames[89] = "Indigo Moonflower";
    m_itemNames[90] = "White Moonflower";
    m_itemNames[101] = "Lugworm";
    m_itemNames[102] = "Earthworm";
    m_itemNames[103] = "Lure";
    m_itemNames[111] = "Sardine";
    m_itemNames[112] = "Carp";
    m_itemNames[113] = "Blowfish";
    m_itemNames[114] = "Bream";
    m_itemNames[115] = "Shark";
    m_itemNames[116] = "Blue Marlin";
    m_itemNames[117] = "Dunkleosteus";
    m_itemNames[118] = "Rainbow Trout";
    m_itemNames[119] = "Black Bass";
    m_itemNames[120] = "Giant Catfish";
    m_itemNames[121] = "Royal Fish";
    m_itemNames[122] = "Hyneria";
    m_itemNames[123] = "Sandfish";
    m_itemNames[124] = "Rhizodont";
    m_itemNames[125] = "Shaman Fish";
    m_itemNames[131] = "Aquatic Plant";
    m_itemNames[132] = "Deadwood";
    m_itemNames[133] = "Rusty Bucket";
    m_itemNames[134] = "Empty Can";
    m_itemNames[138] = "Gold Ore";
    m_itemNames[139] = "Silver Ore";
    m_itemNames[140] = "Copper Ore";
    m_itemNames[141] = "Iron Ore";
    m_itemNames[142] = "Crystal";
    m_itemNames[143] = "Pyrite";
    m_itemNames[144] = "Moldavite";
    m_itemNames[145] = "Meteorite";
    m_itemNames[146] = "Amber";
    m_itemNames[147] = "Fluorite";
    m_itemNames[148] = "Clay";
    m_itemNames[153] = "Berries";
    m_itemNames[154] = "Royal Fern";
    m_itemNames[155] = "Tree Branch";
    m_itemNames[156] = "Log";
    m_itemNames[157] = "Natural Rubber";
    m_itemNames[158] = "Ivy";
    m_itemNames[159] = "Lichen";
    m_itemNames[160] = "Mushroom";
    m_itemNames[161] = "Sap";
    m_itemNames[167] = "Mutton";
    m_itemNames[168] = "Boar Meat";
    m_itemNames[169] = "Wool";
    m_itemNames[170] = "Boar Hide";
    m_itemNames[171] = "Wolf Hide";
    m_itemNames[172] = "Wolf Fang";
    m_itemNames[173] = "Giant Spider Silk";
    m_itemNames[174] = "Bat Fang";
    m_itemNames[175] = "Bat Wing";
    m_itemNames[176] = "Goat Meat";
    m_itemNames[177] = "Goat Hide";
    m_itemNames[178] = "Venison";
    m_itemNames[179] = "Rainbow Spider Silk";
    m_itemNames[180] = "Boar Liver";
    m_itemNames[181] = "Scorpion Claw";
    m_itemNames[182] = "Scorpion Tail";
    m_itemNames[183] = "Dented Metal Board";
    m_itemNames[184] = "Stripped Bolt";
    m_itemNames[185] = "Broken Lens";
    m_itemNames[186] = "Severed Cable";
    m_itemNames[187] = "Broken Arm";
    m_itemNames[188] = "Broken Antenna";
    m_itemNames[189] = "Broken Motor";
    m_itemNames[190] = "Broken Battery";
    m_itemNames[191] = "Mysterious Switch";
    m_itemNames[192] = "Large Gear";
    m_itemNames[193] = "Titanium Alloy";
    m_itemNames[194] = "Memory Alloy";
    m_itemNames[195] = "Rusted Clump";
    m_itemNames[196] = "Machine Oil";
    m_itemNames[201] = "Forlorn Necklace";
    m_itemNames[202] = "Twisted Ring";
    m_itemNames[203] = "Broken Earring";
    m_itemNames[204] = "Pretty Choker";
    m_itemNames[205] = "Metal Piercing";
    m_itemNames[206] = "Subdued Bracelet";
    m_itemNames[207] = "Technical Guide";
    m_itemNames[208] = "Grubby Book";
    m_itemNames[209] = "Thick Dictionary";
    m_itemNames[210] = "Closed Book";
    m_itemNames[211] = "Used Coloring Book";
    m_itemNames[212] = "Old Schoolbook";
    m_itemNames[213] = "Dirty Bag";
    m_itemNames[214] = "Flashy Hat";
    m_itemNames[215] = "Leather Gloves";
    m_itemNames[216] = "Silk Handkerchief";
    m_itemNames[217] = "Leather Boots";
    m_itemNames[218] = "Complex Machine";
    m_itemNames[219] = "Elaborate Machine";
    m_itemNames[220] = "Simple Machine";
    m_itemNames[221] = "Stopped Clock";
    m_itemNames[222] = "Broken Wristwatch";
    m_itemNames[223] = "Rusty Kitchen Knife";
    m_itemNames[224] = "Broken Saw";
    m_itemNames[225] = "Dented Metal Bat";
    m_itemNames[227] = "Shell";
    m_itemNames[228] = "Gastropod";
    m_itemNames[229] = "Bivalve";
    m_itemNames[230] = "Seaweed";
    m_itemNames[231] = "Empty Bottle";
    m_itemNames[232] = "Driftwood";
    m_itemNames[233] = "Pearl";
    m_itemNames[234] = "Black Pearl";
    m_itemNames[235] = "Crab";
    m_itemNames[236] = "Starfish";
    m_itemNames[242] = "Sea Turtle Egg";
    m_itemNames[243] = "Broken Pottery";
    m_itemNames[244] = "Desert Rose";
    m_itemNames[245] = "Giant Egg";
    m_itemNames[246] = "Damascus Steel";
    m_itemNames[247] = "Eagle Egg";
    m_itemNames[248] = "Chicken Egg";
    m_itemNames[250] = "Mouse Tail";
    m_itemNames[251] = "Lizard Tail";
    m_itemNames[255] = "Deer Antler";
    m_itemNames[255] = "Light Key";
    m_itemNames[255] = "Darkness Key";
    m_itemNames[255] = "Pressed Freesia";
    m_itemNames[255] = "Potted Freesia";
    m_itemNames[255] = "Freesia (Delivery)";
    m_itemNames[255] = "Pile of Junk";
    m_itemNames[255] = "Old Gold Coin";
    m_itemNames[255] = "Marked Map";
    m_itemNames[255] = "AA Keycard";
    m_itemNames[255] = "KA Keycard";
    m_itemNames[255] = "SA Keycard";
    m_itemNames[255] = "TA Keycard";
    m_itemNames[255] = "NA Keycard";
    m_itemNames[255] = "HA Keycard";
    m_itemNames[255] = "MA Keycard";
    m_itemNames[255] = "YA Keycard";
    m_itemNames[255] = "RA Keycard";
    m_itemNames[255] = "WA Keycard";
    m_itemNames[256] = "Moon Key";
    m_itemNames[257] = "Star Key";
    m_itemNames[258] = "Fine Flour";
    m_itemNames[259] = "Coarse Flour";
    m_itemNames[260] = "Perfume Bottle";
    m_itemNames[261] = "Postman's Parcel";
    m_itemNames[262] = "Lover's Letter";
    m_itemNames[263] = "Water Filter";
    m_itemNames[264] = "Royal Compass";
    m_itemNames[265] = "Vapor Moss";
    m_itemNames[266] = "Valley Spider Silk";
    m_itemNames[267] = "Animal Guidebook";
    m_itemNames[268] = "Ore Guidebook";
    m_itemNames[269] = "Plant Guidebook";
    m_itemNames[270] = "Red Book";
    m_itemNames[271] = "Blue Book";
    m_itemNames[272] = "Old Lady's Elixir";
    m_itemNames[273] = "Old Lady's Elixir+";
    m_itemNames[274] = "Parcel for The Aerie";
    m_itemNames[275] = "Parcel for Seafront";
    m_itemNames[276] = "Cookbook";
    m_itemNames[277] = "Parcel for Facade";
    m_itemNames[278] = "Max's Herbs";
    m_itemNames[279] = "Drifting Cargo";
    m_itemNames[280] = "Drifting Cargo 2";
    m_itemNames[281] = "Drifting Cargo 3";
    m_itemNames[282] = "Drifting Cargo 4";
    m_itemNames[283] = "Old Package";
    m_itemNames[284] = "Mermaid Tear";
    m_itemNames[285] = "Mandrake Leaf";
    m_itemNames[286] = "Energizer";
    m_itemNames[287] = "Toad Oil";
    m_itemNames[288] = "Sleep-B-Gone";
    m_itemNames[289] = "Antidote";
    m_itemNames[290] = "Gold Bracelet";
    m_itemNames[291] = "Elite Kitchen Knife";
    m_itemNames[292] = "Elevator Parts";
    m_itemNames[293] = "Dirty Treasure Map";
    m_itemNames[294] = "Restored Treasure Map";
    m_itemNames[295] = "Jade Hair Ornament";
    m_itemNames[296] = "Employee List";
    m_itemNames[297] = "Small Safe";
    m_itemNames[298] = "Safe Key";
    m_itemNames[299] = "Great Tree Root";
    m_itemNames[300] = "Eye of Power";
    m_itemNames[301] = "Ribbon";
    m_itemNames[302] = "Yonah's Ribbon";
    m_itemNames[303] = "Bronze Key";
    m_itemNames[304] = "Brass Key";
    m_itemNames[305] = "Boar Tusk";
    m_itemNames[306] = "Cultivator's Handbook";
    m_itemNames[307] = "Red Bag";
    m_itemNames[308] = "Lantern";
    m_itemNames[309] = "Empty Lantern";
    m_itemNames[310] = "Hold Key";
    m_itemNames[311] = "Passageway Key";
    m_itemNames[312] = "Goat Key";
    m_itemNames[313] = "Lizard Key";
    m_itemNames[314] = "Unlocking Procedure Memo";
    m_itemNames[315] = "Red Jewel?";
    m_itemNames[316] = "Red Flowers";
    m_itemNames[317] = "Apples";
    m_itemNames[318] = "[2031800]NoText";
    m_itemNames[319] = "[2031900]NoText";
    m_itemNames[320] = "\"Look at the Sky\"";
    m_itemNames[321] = "\"Don't Try So Hard\"";
    m_itemNames[322] = "\"My Birthday!\"";
    m_itemNames[323] = "Love Letter 2/12/3340";
    m_itemNames[324] = "Love Letter 3/28/3340";
    m_itemNames[325] = "Love Letter 5/1/3340";
    m_itemNames[326] = "Letter from the Mayor";
    m_itemNames[327] = "The Postman's Request";
    m_itemNames[328] = "The Postman's Thanks";
    m_itemNames[329] = "Invitation from a Stranger";
    m_itemNames[330] = "Grand Re-Opening Notice";
    m_itemNames[331] = "Wedding Invitation";
    m_itemNames[332] = "Letter from the King";
    m_itemNames[333] = "Underground Research Record 1";
    m_itemNames[334] = "Underground Research Record 2";
    m_itemNames[335] = "Underground Research Record 3";
    m_itemNames[336] = "Underground Research Record 4";
    m_itemNames[337] = "Letter to the Chief";
    m_itemNames[338] = "Letter to Two Brothers Weaponry";
    m_itemNames[339] = "Letter to Popola";
    m_itemNames[340] = "Letter to a Faraway Lover";
    m_itemNames[341] = "Letter from Emil";
    m_itemNames[342] = "Weapon Upgrade Notice";
    m_itemNames[343] = "Letter from the Chief of The Aerie";
    m_itemNames[352] = "Project Gestalt Report 0923";
    m_itemNames[353] = "Project Gestalt Report 9182";
    m_itemNames[354] = "Project Gestalt Report 10432";
    m_itemNames[355] = "Project Gestalt Report 11242";
    m_itemNames[512] = "World Map";
    m_itemNames[515] = "<NIER>'s Village Map";
    m_itemNames[516] = "Lost Shrine Area Map";
    m_itemNames[517] = "Lost Shrine Map";
    m_itemNames[518] = "The Aerie Map";
    m_itemNames[519] = "Seafront Map";
    m_itemNames[520] = "Desert Map";
    m_itemNames[521] = "Facade Map";
    m_itemNames[522] = "Barren Temple Map";
    m_itemNames[523] = "Junk Heap Area Map";
    m_itemNames[524] = "Junk Heap Map";
    m_itemNames[525] = "Manor Map";
    m_itemNames[526] = "Forest of Myth Map";
    m_itemNames[527] = "Underground Facility Map";
    m_itemNames[529] = "Shadowlord's Castle Map";
    m_itemNames[531] = "Northern Plains Map";
    m_itemNames[532] = "Southern Plains Map";
    m_itemNames[533] = "Eastern Road Map";
    m_itemNames[534] = "Beneath the Forest of Myth Map";
    m_itemNames[535] = "Tokyo Map";


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
    // Check if an editor widget inside the tree has focus.
    QWidget* focusedWidget = QApplication::focusWidget();
    bool isEditing = (focusedWidget != nullptr) && (focusedWidget->parentWidget() == m_treeWidget->viewport());

    if (!this->isEnabled() || !this->isVisible() || isEditing) {
        return;
    }

    m_isUpdatingTree = true;

    int scrollPosition = m_treeWidget->verticalScrollBar()->value();

    QString selectedInternalId;
    int selectedItemId = -1;
    QTreeWidgetItem* current = m_treeWidget->currentItem();
    if (current) {
        selectedInternalId = current->data(0, Qt::UserRole).toString();
        if (selectedInternalId == "save/inventory") {
            selectedItemId = current->data(0, Qt::UserRole + 1).toInt();
        }
    }

    QMap<QString, bool> expansionState;
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* topItem = m_treeWidget->topLevelItem(i);
        expansionState[topItem->text(0)] = topItem->isExpanded();
        for (int j = 0; j < topItem->childCount(); ++j) {
            QTreeWidgetItem* childItem = topItem->child(j);
            expansionState[topItem->text(0) + "/" + childItem->text(0)] = childItem->isExpanded();
        }
    }

    m_treeWidget->clear();
    populateTree();

    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* topItem = m_treeWidget->topLevelItem(i);
        if (expansionState.value(topItem->text(0), false)) {
            topItem->setExpanded(true);
            for (int j = 0; j < topItem->childCount(); ++j) {
                QTreeWidgetItem* childItem = topItem->child(j);
                if (expansionState.value(topItem->text(0) + "/" + childItem->text(0), false)) {
                    childItem->setExpanded(true);
                }
            }
        }
    }

    onSearchChanged(m_searchBox->text()); 

    if (!selectedInternalId.isEmpty()) {
        QTreeWidgetItemIterator it(m_treeWidget);
        while (*it) {
            QTreeWidgetItem* item = *it;
            QString internalId = item->data(0, Qt::UserRole).toString();
            if (internalId == selectedInternalId) {
                if (internalId == "save/inventory") {
                    if (item->data(0, Qt::UserRole + 1).toInt() == selectedItemId) {
                        m_treeWidget->setCurrentItem(item);
                        break;
                    }
                }
                else {
                    m_treeWidget->setCurrentItem(item);
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
    populatePlayerParamNode(playerParamRoot);

    auto* saveDataRoot = new QTreeWidgetItem(m_treeWidget, { "PlayerSaveData" });
    populateSaveDataNode(saveDataRoot);
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

    auto inventoryRoot = new QTreeWidgetItem(parent, { "Inventory" });
    for (int i = 0; i < 768; ++i) {
        uint8_t count = static_cast<uint8_t>(data->Inventory[i]);
        QString name = m_itemNames.count(i) ? m_itemNames[i] : QString("Item ID %1").arg(i);
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

    PlayerSaveData* saveData = LunarTear::Get().Game().GetPlayerSaveData();
    CPlayerParam* playerParam = LunarTear::Get().Game().GetPlayerParam();

    if (internalId == "save/player_name" && saveData) {
        safeStringCopy(saveData->player_name, newValue.toString(), sizeof(saveData->player_name));
    }
    else if (internalId == "save/current_hp" && saveData) {
        saveData->current_hp = newValue.toInt(&ok);
    }
    else if (internalId == "save/current_mp" && saveData) {
        saveData->current_mp = newValue.toFloat(&ok);
    }
    else if (internalId == "save/current_level" && saveData) {
        saveData->current_level = newValue.toInt(&ok);
    }
    else if (internalId == "save/current_xp" && saveData) {
        saveData->current_xp = newValue.toInt(&ok);
    }
    else if (internalId == "save/gold" && saveData) {
        saveData->gold = newValue.toInt(&ok);
    }
    else if (internalId == "save/inventory" && saveData) {
        int itemId = item->data(0, Qt::UserRole + 1).toInt();
        int count = newValue.toInt(&ok);
        if (ok && count >= 0 && count <= 255) {
            saveData->Inventory[itemId] = count;
        }
    }
    else if (internalId == "param/maxHP" && playerParam) {
        playerParam->maxHP = newValue.toInt(&ok);
    }
    else if (internalId == "param/maxMP" && playerParam) {
        playerParam->maxMP = newValue.toFloat(&ok);
    }
    else if (internalId == "param/attack_stat" && playerParam) {
        playerParam->attack_stat = newValue.toInt(&ok);
    }
    else if (internalId == "param/magickAttack_stat" && playerParam) {
        playerParam->magickAttack_stat = newValue.toInt(&ok);
    }
    else if (internalId == "param/defense_stat" && playerParam) {
        playerParam->defense_stat = newValue.toInt(&ok);
    }
    else if (internalId == "param/magickDefense_stat" && playerParam) {
        playerParam->magickDefense_stat = newValue.toInt(&ok);
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

    // Only auto-expand if there's a filter active and a child is visible
    if (hasVisibleChild && !filterText.isEmpty()) {
        item->setExpanded(true);
    }

    return shouldBeVisible;
}