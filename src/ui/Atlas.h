#pragma once

#include <QWidget>
#include <QList>
#include <QTableWidgetItem>
#include <QVector3D>
#include <QPointF>
#include <QTimer>

class QListWidget;
class QTableWidget;
class QPushButton;
class MapView;
class QLabel;
class QLineEdit;

struct TeleportPoint {
    QString name;
    QVector3D pos;
    bool isUserDefined;
    QString comment;
};

struct MapCalibration {
    bool isCalibrated = false;
    double scaleX = 1.0;
    double scaleY = 1.0;
    double offsetX = 0.0;
    double offsetY = 0.0;
};

class Atlas : public QWidget
{
    Q_OBJECT

public:
    explicit Atlas(QWidget* parent = nullptr);

private slots:
    void onMapSelected();
    void onPointSelected(QTableWidgetItem* current, QTableWidgetItem* previous);
    void onTeleportClicked();
    void onSavePointClicked();
    void onDeletePointClicked();
    void onLockViewToggled(bool checked);
    void onPointCommentDoubleClicked(QTableWidgetItem* item);
    void onMapSearchChanged(const QString& text);
    void onPointSearchChanged(const QString& text);
    void onCalibrateClicked();
    void onConfirmCalibPointClicked();
    void onCancelCalibClicked();
    void onCalibrationPixelClicked(QPointF imagePos);
    void updatePlayerPosition();

private:
    void setupUi();
    void setupConnections();
    void applyStyling();
    void populateMapList();
    void loadMapData(const QString& mapId);
    void saveCurrentMapData();
    void populatePointsTable();
    void updateCalibrationUI();
    void updateMapListHighlight();
    QPointF gameToPixel(const QVector3D& gamePos);

    QListWidget* m_mapList;
    QTableWidget* m_pointsTable;
    MapView* m_mapView;
    QLineEdit* m_mapSearchBox;
    QLineEdit* m_pointsSearchBox;
    QPushButton* m_teleportButton;
    QPushButton* m_savePointButton;
    QPushButton* m_deletePointButton;
    QPushButton* m_calibrateButton;
    QPushButton* m_lockViewButton;
    QWidget* m_calibrationControlsWidget;
    QLabel* m_instructionLabel;
    QPushButton* m_confirmCalibPointButton;
    QPushButton* m_cancelCalibButton;
    QString m_atlasDataDir;
    QString m_modBasePath;
    QString m_currentMapId;
    QString m_currentPlayerPhase; 
    QList<TeleportPoint> m_currentPoints;
    MapCalibration m_currentCalibration;
    QTimer* m_playerUpdateTimer;
    int m_calibrationState = 0;
    QPointF m_calibPixel1, m_calibPixel2;
    QVector3D m_calibGame1, m_calibGame2;
};