#include "Atlas.h"
#include "MapView.h"
#include "GameData.h" 
#include <LunarTear++.h>

#include <QListWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <QSplitter>
#include <QLineEdit>

namespace {
    std::string modId = "LTCon";
}

namespace {
    QString getPointUniqueId(const TeleportPoint& point) {
        return QString("%1@%2,%3,%4")
            .arg(point.name)
            .arg(point.pos.x())
            .arg(point.pos.y())
            .arg(point.pos.z());
    }
}

Atlas::Atlas(QWidget* parent)
    : QWidget(parent)
{
    try {
        m_modBasePath = QString::fromStdString(LunarTear::Get().GetModDirectory(modId));
    }
    catch (const LunarTearUninitializedError& e) {
        m_modBasePath = "";
    }
    m_atlasDataDir = m_modBasePath + "/resource/atlas";

    setupUi();

    applyStyling();
    setupConnections();
    populateMapList();

    m_playerUpdateTimer = new QTimer(this);
    connect(m_playerUpdateTimer, &QTimer::timeout, this, &Atlas::updatePlayerPosition);
    m_playerUpdateTimer->start(500);

}

void Atlas::setupUi()
{
    m_mapSearchBox = new QLineEdit();
    m_mapSearchBox->setPlaceholderText("Filter maps...");
    m_mapList = new QListWidget();

    m_pointsSearchBox = new QLineEdit();
    m_pointsSearchBox->setPlaceholderText("Filter points by name or comment...");
    m_pointsTable = new QTableWidget();
    m_mapView = new MapView();

    m_teleportButton = new QPushButton("Teleport");
    m_savePointButton = new QPushButton("Save Point");
    m_deletePointButton = new QPushButton("Delete Point");
    m_calibrateButton = new QPushButton("Calibrate Map");

    m_lockViewButton = new QPushButton("View Locked");
    m_lockViewButton->setCheckable(true);
    m_lockViewButton->setChecked(true);

    m_calibrationControlsWidget = new QWidget();
    auto calibLayout = new QHBoxLayout(m_calibrationControlsWidget);
    m_instructionLabel = new QLabel("Calibration instructions will appear here.");
    m_confirmCalibPointButton = new QPushButton("Confirm Game Position");
    m_cancelCalibButton = new QPushButton("Cancel Calibration");
    calibLayout->addWidget(m_instructionLabel, 1);
    calibLayout->addWidget(m_confirmCalibPointButton);
    calibLayout->addWidget(m_cancelCalibButton);
    m_calibrationControlsWidget->hide();

    m_pointsTable->setColumnCount(5);
    m_pointsTable->setHorizontalHeaderLabels({ "Name", "X", "Y", "Z", "Comment" });
    m_pointsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pointsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pointsTable->setAlternatingRowColors(true);
    m_pointsTable->setShowGrid(false);
    m_pointsTable->verticalHeader()->setVisible(false);
    QHeaderView* header = m_pointsTable->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Interactive);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::Stretch);

    auto mainLayout = new QHBoxLayout(this);
    auto mainSplitter = new QSplitter(Qt::Horizontal);

    auto leftWidget = new QWidget();
    auto leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->addWidget(m_mapSearchBox);
    leftLayout->addWidget(m_mapList);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    auto rightWidget = new QWidget();
    auto rightLayout = new QVBoxLayout(rightWidget);
    auto rightSplitter = new QSplitter(Qt::Vertical);

    auto topWidget = new QWidget();
    auto topLayout = new QVBoxLayout(topWidget);
    topLayout->addWidget(m_mapView, 1);
    topLayout->addWidget(m_calibrationControlsWidget);
    topLayout->setContentsMargins(0, 0, 0, 0);

    auto bottomWidget = new QWidget();
    auto bottomLayout = new QVBoxLayout(bottomWidget);
    auto buttonBarLayout = new QHBoxLayout();
    buttonBarLayout->addWidget(m_teleportButton);
    buttonBarLayout->addWidget(m_savePointButton);
    buttonBarLayout->addWidget(m_deletePointButton);
    buttonBarLayout->addStretch(1);
    buttonBarLayout->addWidget(m_calibrateButton);
    buttonBarLayout->addWidget(m_lockViewButton);
    bottomLayout->addLayout(buttonBarLayout);
    bottomLayout->addWidget(m_pointsSearchBox);
    bottomLayout->addWidget(m_pointsTable);
    bottomLayout->setContentsMargins(0, 0, 0, 0);

    rightSplitter->addWidget(topWidget);
    rightSplitter->addWidget(bottomWidget);
    rightSplitter->setStretchFactor(0, 2);
    rightSplitter->setStretchFactor(1, 1);
    rightSplitter->setHandleWidth(2);

    rightLayout->addWidget(rightSplitter);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    mainSplitter->addWidget(leftWidget);
    mainSplitter->addWidget(rightWidget);
    mainSplitter->setStretchFactor(1, 3);
    mainSplitter->setHandleWidth(2);
    mainLayout->addWidget(mainSplitter);
}

void Atlas::applyStyling()
{
    const QString bgColor = "#282c34";
    const QString textColor = "#abb2bf";
    const QString borderColor = "#2c313a";
    const QString highlightColor = "#61afef";
    const QString secondaryBgColor = "#21252b";
    const QString disabledColor = "#5c6370";
    const QString handleColor = "#5c6370";

    QString styleSheet = QString(R"(

        QWidget { background-color: %1; color: %2; font-family: 'Consolas', 'Courier New', monospace; font-size: 14px; }
        QListWidget, QTableWidget, QLineEdit { background-color: %5; border: 1px solid %3; border-radius: 4px; padding: 4px; }
        QListWidget::item:selected, QTableWidget::item:selected { background-color: %4; color: %5; }
        QPushButton { background-color: #3a3f4b; border: 1px solid %3; border-radius: 4px; padding: 3px 10px; font-size: 12px; }
        QPushButton:hover { background-color: #4b5162; }
        QPushButton:pressed { background-color: #404552; }
        QPushButton:disabled { background-color: %3; color: %6; }
        QPushButton:checkable:checked { background-color: %4; color: %5; border-color: %4; }
        QHeaderView::section { background-color: %1; border: 1px solid %3; padding: 4px; }
        QTableWidget { gridline-color: %3; alternate-background-color: #2c313a; }
        QSplitter::handle { background-color: %3; }
        QSplitter::handle:hover { background-color: %4; }
        MapView { border: 1px solid %3; }
        MapView[inCalibration="true"] { border: 2px solid #e53935; }
        QLabel { padding: 4px; }
        QScrollBar:vertical { border: none; background: %5; width: 10px; margin: 0px; }
        QScrollBar::handle:vertical { background: %7; min-height: 25px; border-radius: 0px; }
        QScrollBar::handle:vertical:hover { background: %4; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }
        QScrollBar:horizontal { border: none; background: %5; height: 10px; margin: 0px; }
        QScrollBar::handle:horizontal { background: %7; min-width: 25px; border-radius: 0px; }
        QScrollBar::handle:horizontal:hover { background: %4; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; }
    )").arg(bgColor, textColor, borderColor, highlightColor, secondaryBgColor, disabledColor, handleColor);

    this->setStyleSheet(styleSheet);
}

void Atlas::setupConnections()
{
    connect(m_mapList, &QListWidget::currentItemChanged, this, &Atlas::onMapSelected);
    connect(m_pointsTable, &QTableWidget::currentItemChanged, this, &Atlas::onPointSelected);
    connect(m_mapSearchBox, &QLineEdit::textChanged, this, &Atlas::onMapSearchChanged);
    connect(m_pointsSearchBox, &QLineEdit::textChanged, this, &Atlas::onPointSearchChanged);
    connect(m_teleportButton, &QPushButton::clicked, this, &Atlas::onTeleportClicked);
    connect(m_savePointButton, &QPushButton::clicked, this, &Atlas::onSavePointClicked);
    connect(m_deletePointButton, &QPushButton::clicked, this, &Atlas::onDeletePointClicked);
    connect(m_calibrateButton, &QPushButton::clicked, this, &Atlas::onCalibrateClicked);
    connect(m_lockViewButton, &QPushButton::toggled, this, &Atlas::onLockViewToggled);
    connect(m_lockViewButton, &QPushButton::toggled, m_mapView, &MapView::onViewLockToggled);
    connect(m_mapView, &MapView::calibrationPointClicked, this, &Atlas::onCalibrationPixelClicked);
    connect(m_confirmCalibPointButton, &QPushButton::clicked, this, &Atlas::onConfirmCalibPointClicked);
    connect(m_cancelCalibButton, &QPushButton::clicked, this, &Atlas::onCancelCalibClicked);
    connect(m_pointsTable, &QTableWidget::itemDoubleClicked, this, &Atlas::onPointCommentDoubleClicked);
}

void Atlas::onLockViewToggled(bool checked)
{
    if (checked) {
        m_lockViewButton->setText("View Locked");
    }
    else {
        m_lockViewButton->setText("View Unlocked");
    }
}

void Atlas::populateMapList()
{
    m_mapList->clear();
    QDir dir(m_atlasDataDir);
    QFileInfoList fileList = dir.entryInfoList({ "*.json" }, QDir::Files);
    for (const QFileInfo& fileInfo : fileList) {
        m_mapList->addItem(fileInfo.baseName());
    }
    qInfo() << "Atlas: Found" << fileList.count() << "map data files.";
    updateMapListHighlight();
}

void Atlas::loadMapData(const QString& mapId)
{
    m_currentMapId = mapId;
    m_currentPoints.clear();
    m_currentCalibration = MapCalibration();

    QString filePath = QString("%1/%2.json").arg(m_atlasDataDir, mapId);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Atlas: Could not open" << filePath;
        populatePointsTable();
        return;
    }

    QJsonObject mapData = QJsonDocument::fromJson(file.readAll()).object();
    QString relativeImagePath = mapData["imageFile"].toString("resource/maps/default.png");
    QString absoluteImagePath = m_modBasePath + "/" + relativeImagePath;
    QPixmap mapPixmap(absoluteImagePath);

    if (mapPixmap.isNull()) {
        qWarning() << "Atlas: Failed to load map image:" << absoluteImagePath;
    }
    m_mapView->setMapImage(mapPixmap);

    QJsonObject calibObj = mapData["calibration"].toObject();
    m_currentCalibration.isCalibrated = calibObj["isCalibrated"].toBool(false);
    m_currentCalibration.scaleX = calibObj["scaleX"].toDouble(1.0);
    m_currentCalibration.scaleY = calibObj["scaleY"].toDouble(1.0);
    m_currentCalibration.offsetX = calibObj["offsetX"].toDouble(0.0);
    m_currentCalibration.offsetY = calibObj["offsetY"].toDouble(0.0);

    m_lockViewButton->setChecked(true);

    QJsonObject pointNotes = mapData["pointNotes"].toObject();
    auto parsePoint = [](const QJsonObject& pointObj) {
        TeleportPoint point;
        point.name = pointObj["name"].toString();
        const auto posObj = pointObj["pos"].toObject();
        point.pos = QVector3D(posObj["x"].toDouble(), posObj["y"].toDouble(), posObj["z"].toDouble());
        return point;
        };
    QJsonArray userPoints = mapData["userPoints"].toArray();
    for (const QJsonValue& val : userPoints) {
        TeleportPoint point = parsePoint(val.toObject());
        point.isUserDefined = true;
        point.comment = pointNotes.value(getPointUniqueId(point)).toString();
        m_currentPoints.append(point);
    }
    QJsonArray gamePoints = mapData["gamePoints"].toArray();
    for (const QJsonValue& val : gamePoints) {
        TeleportPoint point = parsePoint(val.toObject());
        point.isUserDefined = false;
        point.comment = pointNotes.value(getPointUniqueId(point)).toString();
        m_currentPoints.append(point);
    }

    populatePointsTable();
    m_mapView->setSelectedPoint(QPointF());
}

void Atlas::saveCurrentMapData()
{
    if (m_currentMapId.isEmpty()) return;
    QJsonObject rootObject;
    rootObject["mapId"] = m_currentMapId;
    rootObject["imageFile"] = QString("resource/maps/%1.png").arg(m_currentMapId);
    QJsonObject calibObj;
    calibObj["isCalibrated"] = m_currentCalibration.isCalibrated;
    calibObj["scaleX"] = m_currentCalibration.scaleX;
    calibObj["scaleY"] = m_currentCalibration.scaleY;
    calibObj["offsetX"] = m_currentCalibration.offsetX;
    calibObj["offsetY"] = m_currentCalibration.offsetY;
    rootObject["calibration"] = calibObj;

    QJsonArray gamePointsArray;
    QJsonArray userPointsArray;
    QJsonObject pointNotesObject;
    for (const TeleportPoint& point : m_currentPoints) {
        QJsonObject pointObj;
        pointObj["name"] = point.name;
        QJsonObject posObj;
        posObj["x"] = point.pos.x();
        posObj["y"] = point.pos.y();
        posObj["z"] = point.pos.z();
        pointObj["pos"] = posObj;
        if (!point.comment.isEmpty()) {
            pointNotesObject[getPointUniqueId(point)] = point.comment;
        }
        if (point.isUserDefined) {
            userPointsArray.append(pointObj);
        }
        else {
            gamePointsArray.append(pointObj);
        }
    }
    rootObject["gamePoints"] = gamePointsArray;
    rootObject["userPoints"] = userPointsArray;
    rootObject["pointNotes"] = pointNotesObject;

    QString filePath = QString("%1/%2.json").arg(m_atlasDataDir, m_currentMapId);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Atlas: FAILED to save map data to" << filePath;
        return;
    }
    file.write(QJsonDocument(rootObject).toJson(QJsonDocument::Indented));
}

void Atlas::populatePointsTable() {
    int previousRow = m_pointsTable->currentRow();
    m_pointsTable->blockSignals(true);
    m_pointsTable->setRowCount(0);
    m_pointsTable->setSortingEnabled(false);

    for (int i = 0; i < m_currentPoints.size(); ++i) {
        const TeleportPoint& point = m_currentPoints[i];
        int row = m_pointsTable->rowCount();
        m_pointsTable->insertRow(row);

        QTableWidgetItem* nameItem = new QTableWidgetItem(point.name);
        nameItem->setData(Qt::UserRole, i);
        if (!point.isUserDefined) nameItem->setForeground(QColor("#9da5b4"));

        m_pointsTable->setItem(row, 0, nameItem);
        m_pointsTable->setItem(row, 1, new QTableWidgetItem(QString::number(point.pos.x())));
        m_pointsTable->setItem(row, 2, new QTableWidgetItem(QString::number(point.pos.y())));
        m_pointsTable->setItem(row, 3, new QTableWidgetItem(QString::number(point.pos.z())));
        m_pointsTable->setItem(row, 4, new QTableWidgetItem(point.comment));
    }

    m_pointsTable->setSortingEnabled(true);
    m_pointsTable->blockSignals(false);

    if (previousRow >= 0 && previousRow < m_pointsTable->rowCount()) {
        m_pointsTable->setCurrentCell(previousRow, 0);
    }
    else {
        m_mapView->setSelectedPoint(QPointF());
        onPointSelected(nullptr, nullptr);
    }
}

void Atlas::onMapSearchChanged(const QString& text) {
    QString searchText = text;
    searchText.replace(' ', '_').replace('-', '_');
    for (int i = 0; i < m_mapList->count(); ++i) {
        QListWidgetItem* item = m_mapList->item(i);
        QString itemText = item->text();
        itemText.replace(' ', '_').replace('-', '_');
        item->setHidden(!itemText.contains(searchText, Qt::CaseInsensitive));
    }
}

void Atlas::onPointSearchChanged(const QString& text) {
    for (int i = 0; i < m_pointsTable->rowCount(); ++i) {
        QString name = m_pointsTable->item(i, 0)->text();
        QString comment = m_pointsTable->item(i, 4)->text();
        bool match = name.contains(text, Qt::CaseInsensitive) || comment.contains(text, Qt::CaseInsensitive);
        m_pointsTable->setRowHidden(i, !match);
    }
}

void Atlas::onMapSelected() {
    if (m_calibrationState != 0) {
        onCancelCalibClicked();
    }
    QListWidgetItem* currentItem = m_mapList->currentItem();
    if (!currentItem) {
        m_currentMapId.clear();
        m_currentPoints.clear();
        populatePointsTable();
        m_mapView->setMapImage(QPixmap());
        return;
    }
    loadMapData(currentItem->text());
    onPointSearchChanged(m_pointsSearchBox->text());
}

void Atlas::onPointSelected(QTableWidgetItem* current, QTableWidgetItem* previous) {
    if (!current) {
        m_deletePointButton->setEnabled(false);
        m_teleportButton->setEnabled(false);
        m_mapView->setSelectedPoint(QPointF());
        return;
    }

    int row = current->row();
    QTableWidgetItem* nameItem = m_pointsTable->item(row, 0);
    if (!nameItem) {
        m_mapView->setSelectedPoint(QPointF());
        return;
    }

    int originalIndex = nameItem->data(Qt::UserRole).toInt();
    if (originalIndex < 0 || originalIndex >= m_currentPoints.size()) {
        m_mapView->setSelectedPoint(QPointF());
        return;
    }

    const TeleportPoint& selectedPoint = m_currentPoints[originalIndex];

    m_deletePointButton->setEnabled(selectedPoint.isUserDefined);
    m_teleportButton->setEnabled(true);

    QPointF pixelPos = gameToPixel(selectedPoint.pos);
    m_mapView->setSelectedPoint(pixelPos);
}

void Atlas::onTeleportClicked()
{
    if (!GameData::instance().isGameActive()) {
        QMessageBox::warning(this, "Teleport", "Cannot teleport, game is not in a playable state.");
        return;
    }
    auto selectedItems = m_pointsTable->selectedItems();
    if (selectedItems.isEmpty()) return;

    QTableWidgetItem* nameItem = m_pointsTable->item(selectedItems.first()->row(), 0);
    if (!nameItem) return;

    int originalIndex = nameItem->data(Qt::UserRole).toInt();
    const TeleportPoint& point = m_currentPoints[originalIndex];
    const QString targetMapId = m_currentMapId;
    const QString playerCurrentMapId = GameData::instance().getCurrentPhase();

    if (targetMapId != playerCurrentMapId) {
        int choice = QMessageBox::question(this, "Confirm Map Change",
            QString("You are about to teleport from '%1' to '%2'.\nThis will trigger a loading screen.\n\nContinue?").arg(playerCurrentMapId, targetMapId),
            QMessageBox::Yes | QMessageBox::No);

        if (choice == QMessageBox::No) {
            return;
        }
    }

    qInfo() << "Initiating teleport to" << targetMapId << "at" << point.pos;
    float currentRot = GameData::instance().getPlayerRotationY();

    GameData::instance().teleportToPoint(targetMapId, point.pos, currentRot);
}

void Atlas::updatePlayerPosition()
{

    QString currentPhase = GameData::instance().isGameActive() ? GameData::instance().getCurrentPhase() : "";

    if (currentPhase != m_currentPlayerPhase) {

        m_currentPlayerPhase = currentPhase;

        updateMapListHighlight();
    }

    if (m_currentMapId.isEmpty() || !m_currentCalibration.isCalibrated) {
        m_mapView->setPlayerPosition(QPointF());
        return;
    }

    if (m_currentPlayerPhase != m_currentMapId) {
        m_mapView->setPlayerPosition(QPointF());
        return;
    }

    QVector3D playerPos = GameData::instance().getPlayerPosition();
    QPointF playerPixelPos = gameToPixel(playerPos);
    m_mapView->setPlayerPosition(playerPixelPos);
}


void Atlas::updateMapListHighlight()
{
    for (int i = 0; i < m_mapList->count(); ++i) {
        QListWidgetItem* item = m_mapList->item(i);
        QFont font = item->font();
        bool isCurrent = (!m_currentPlayerPhase.isEmpty() && item->text() == m_currentPlayerPhase);

        font.setBold(isCurrent);
        item->setFont(font);

        if (isCurrent) {
            item->setForeground(QColor("#61afef"));
        }
        else {
            item->setForeground(QColor("#abb2bf"));
        }
    }
}

QPointF Atlas::gameToPixel(const QVector3D& gamePos) const {
    if (!m_currentCalibration.isCalibrated) return QPointF();
    double pixelX = (gamePos.x() - m_currentCalibration.offsetX) / m_currentCalibration.scaleX;
    double pixelY = (gamePos.z() - m_currentCalibration.offsetY) / m_currentCalibration.scaleY;
    return QPointF(pixelX, pixelY);
}

void Atlas::onSavePointClicked() {
    if (!GameData::instance().isGameActive()) {
        QMessageBox::warning(this, "Save Point", "Cannot save point, game is not in a playable state.");
        return;
    }
    if (m_currentMapId.isEmpty() || GameData::instance().getCurrentPhase() != m_currentMapId) {
        QMessageBox::warning(this, "Save Point", "Player is not on the currently selected map.");
        return;
    }
    QVector3D playerPos = GameData::instance().getPlayerPosition();
    bool ok;
    QString name = QInputDialog::getText(this, "Save New Point", "Enter a name for this location:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;
    TeleportPoint newPoint;
    newPoint.name = name;
    newPoint.pos = playerPos;
    newPoint.isUserDefined = true;
    m_currentPoints.append(newPoint);
    saveCurrentMapData();
    populatePointsTable();
}

void Atlas::onDeletePointClicked() {
    auto selectedItems = m_pointsTable->selectedItems();
    if (selectedItems.isEmpty()) return;

    QTableWidgetItem* nameItem = m_pointsTable->item(selectedItems.first()->row(), 0);
    if (!nameItem) return;

    int indexToRemove = nameItem->data(Qt::UserRole).toInt();
    const TeleportPoint& pointToDelete = m_currentPoints[indexToRemove];

    if (!pointToDelete.isUserDefined) {
        QMessageBox::information(this, "Delete Point", "Cannot delete a built-in game point.");
        return;
    }

    int choice = QMessageBox::question(this, "Confirm Delete",
        QString("Are you sure you want to delete the point '%1'?").arg(pointToDelete.name),
        QMessageBox::Yes | QMessageBox::No);

    if (choice == QMessageBox::Yes) {
        m_currentPoints.removeAt(indexToRemove);
        saveCurrentMapData();
        loadMapData(m_currentMapId);
    }
}

void Atlas::onCalibrateClicked() {
    if (m_currentMapId.isEmpty()) {
        QMessageBox::warning(this, "Calibrate", "Please select a map first.");
        return;
    }
    if (!GameData::instance().isGameActive()) {
        QMessageBox::warning(this, "Calibrate", "Calibration requires the game to be in a playable state.");
        return;
    }
    m_calibrationState = 1;
    updateCalibrationUI();
    m_mapView->enterCalibrationMode();
}

void Atlas::onCancelCalibClicked() {
    m_calibrationState = 0;
    updateCalibrationUI();
    m_mapView->leaveCalibrationMode();
}

void Atlas::updateCalibrationUI() {
    if (m_calibrationState == 0) {
        m_calibrationControlsWidget->hide();
        m_calibrateButton->setEnabled(true);
    }
    else {
        m_calibrationControlsWidget->show();
        m_calibrateButton->setEnabled(false);
        m_confirmCalibPointButton->setEnabled(false);
        if (m_calibrationState == 1) {
            m_instructionLabel->setText("<b>Step 1/2:</b> Go to a landmark in-game, then left-click the same spot on the map above.");
        }
        else if (m_calibrationState == 2) {
            m_instructionLabel->setText("<b>Step 2/2:</b> Go to a second, distant landmark, then left-click it on the map.");
        }
    }
}

void Atlas::onCalibrationPixelClicked(QPointF imagePos) {
    if (m_calibrationState == 1) {
        m_calibPixel1 = imagePos;
        m_instructionLabel->setText("<b>Step 1/2:</b> Map position captured. Now go to that position in-game and click 'Confirm Game Position'.");
        m_confirmCalibPointButton->setEnabled(true);
    }
    else if (m_calibrationState == 2) {
        m_calibPixel2 = imagePos;
        m_instructionLabel->setText("<b>Step 2/2:</b> Map position captured. Now go to that position in-game and click 'Confirm Game Position'.");
        m_confirmCalibPointButton->setEnabled(true);
    }
}

void Atlas::onConfirmCalibPointClicked() {
    if (!GameData::instance().isGameActive()) {
        QMessageBox::warning(this, "Calibrate", "Game is no longer in a playable state.");
        onCancelCalibClicked();
        return;
    }
    QVector3D gamePos = GameData::instance().getPlayerPosition();
    if (m_calibrationState == 1) {
        m_calibGame1 = gamePos;
        m_calibrationState = 2;
        updateCalibrationUI();
    }
    else if (m_calibrationState == 2) {
        m_calibGame2 = gamePos;
        QPointF deltaPixel = m_calibPixel2 - m_calibPixel1;
        QVector3D deltaGame = m_calibGame2 - m_calibGame1;
        if (qAbs(deltaPixel.x()) < 5 || qAbs(deltaPixel.y()) < 5) {
            QMessageBox::critical(this, "Calibration Error", "The two points are too close on the map. Please choose more distant landmarks.");
            onCancelCalibClicked();
            return;
        }
        m_currentCalibration.scaleX = deltaGame.x() / deltaPixel.x();
        m_currentCalibration.scaleY = deltaGame.z() / deltaPixel.y();
        m_currentCalibration.offsetX = m_calibGame1.x() - (m_calibPixel1.x() * m_currentCalibration.scaleX);
        m_currentCalibration.offsetY = m_calibGame1.z() - (m_calibPixel1.y() * m_currentCalibration.scaleY);
        m_currentCalibration.isCalibrated = true;
        saveCurrentMapData();
        QMessageBox::information(this, "Success", "Map calibrated successfully!");
        onCancelCalibClicked();
    }
}

void Atlas::onPointCommentDoubleClicked(QTableWidgetItem* item) {
    if (item->column() != 4) {
        return;
    }

    int row = item->row();
    QTableWidgetItem* nameItem = m_pointsTable->item(row, 0);
    if (!nameItem) return;

    int originalIndex = nameItem->data(Qt::UserRole).toInt();
    if (originalIndex < 0 || originalIndex >= m_currentPoints.size()) return;

    TeleportPoint& pointToEdit = m_currentPoints[originalIndex];

    bool ok;
    QString newComment = QInputDialog::getText(this, "Edit Comment",
        QString("Enter a note for '%1':").arg(pointToEdit.name),
        QLineEdit::Normal,
        pointToEdit.comment,
        &ok);

    if (ok && newComment != pointToEdit.comment) {
        pointToEdit.comment = newComment;
        item->setText(newComment);
        saveCurrentMapData();
    }
}