#pragma once
#include <QWidget>
#include <QPixmap>
#include <QPointF>
#include <QTransform>

class MapView : public QWidget
{
    Q_OBJECT

public:
    explicit MapView(QWidget* parent = nullptr);

    void setMapImage(const QPixmap& pixmap);
    void setPlayerPosition(const QPointF& pos); // Position in SOURCE IMAGE pixel coordinates
    void setSelectedPoint(const QPointF& pos); // Position in SOURCE IMAGE pixel coordinates

    const QTransform& getViewTransform() const;

public slots:
    void enterCalibrationMode();
    void leaveCalibrationMode();
    void onViewLockToggled(bool locked);
    void resetAndCenterView(); 

signals:
    void calibrationPointClicked(QPointF imagePos);

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QPointF mapWidgetToImage(const QPoint& widgetPos);
    void updateCursorShape();

    QPixmap m_mapPixmap;
    QPointF m_playerPosition;   // Stored in image coordinates
    QPointF m_selectedPoint;    // Stored in image coordinates

    QTransform m_transform;
    bool m_isPanning = false;
    QPoint m_panLastMousePos;
    bool m_isViewLocked = true;

    bool m_isCalibrationMode = false;
};