#include "MapView.h"
#include <QPainter>
#include <QCursor>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QResizeEvent> 
#include <QStyle>

MapView::MapView(QWidget* parent)
    : QWidget(parent) 
{
    setMinimumSize(400, 400);
    setStyleSheet("background-color: #1e1e1e;");
    setMouseTracking(true);
}

void MapView::setMapImage(const QPixmap& pixmap)
{
    m_mapPixmap = pixmap;
    resetAndCenterView();
}

void MapView::setPlayerPosition(const QPointF& pos)
{
    m_playerPosition = pos;
    update();
}

void MapView::setSelectedPoint(const QPointF& pos)
{
    m_selectedPoint = pos;
    update();
}

const QTransform& MapView::getViewTransform() const
{
    return m_transform;
}

// --- NEW METHOD ---
void MapView::resetAndCenterView()
{
    m_transform.reset();
    if (m_mapPixmap.isNull()) {
        update();
        return;
    }

    // Get widget and pixmap dimensions
    const QSize widgetSize = this->size();
    const QSize pixmapSize = m_mapPixmap.size();

    // Calculate the scale factor to fit the image inside the widget
    qreal scaleX = (qreal)widgetSize.width() / pixmapSize.width();
    qreal scaleY = (qreal)widgetSize.height() / pixmapSize.height();
    qreal scale = qMin(scaleX, scaleY); // Use the smaller scale to ensure it fits completely

    // Calculate the translation needed to center the scaled image
    qreal scaledWidth = pixmapSize.width() * scale;
    qreal scaledHeight = pixmapSize.height() * scale;
    qreal tx = (widgetSize.width() - scaledWidth) / 2.0;
    qreal ty = (widgetSize.height() - scaledHeight) / 2.0;

    // Apply the new transform
    m_transform.translate(tx, ty);
    m_transform.scale(scale, scale);

    update(); // Trigger a repaint with the new view
}

void MapView::enterCalibrationMode()
{
    m_isCalibrationMode = true;
    updateCursorShape();
    setProperty("inCalibration", true);
    style()->unpolish(this);
    style()->polish(this);
}

void MapView::leaveCalibrationMode()
{
    m_isCalibrationMode = false;
    updateCursorShape();
    setProperty("inCalibration", false);
    style()->unpolish(this);
    style()->polish(this);
}

// --- MODIFIED: When view is re-locked, it resets to the optimal view ---
void MapView::onViewLockToggled(bool locked)
{
    m_isViewLocked = locked;
    if (m_isViewLocked) {
        resetAndCenterView();
    }
    updateCursorShape();
}

QPointF MapView::mapWidgetToImage(const QPoint& widgetPos)
{
    QTransform invertedTransform = m_transform.inverted();
    return invertedTransform.map(QPointF(widgetPos));
}

void MapView::updateCursorShape()
{
    if (m_isCalibrationMode) {
        setCursor(Qt::CrossCursor);
    }
    else if (m_isPanning) {
        setCursor(Qt::ClosedHandCursor);
    }
    else if (!m_isViewLocked) {
        setCursor(Qt::OpenHandCursor);
    }
    else {
        setCursor(Qt::ArrowCursor);
    }
}

void MapView::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setTransform(m_transform);

    if (!m_mapPixmap.isNull()) {
        painter.drawPixmap(0, 0, m_mapPixmap);
    }

    const qreal penWidth = 1.5 / m_transform.m11();

    if (!m_selectedPoint.isNull()) {
        painter.setPen(QPen(QColor("#ffcb6b"), penWidth));
        painter.drawLine(m_selectedPoint - QPointF(8, 8), m_selectedPoint + QPointF(8, 8));
        painter.drawLine(m_selectedPoint - QPointF(8, -8), m_selectedPoint + QPointF(8, -8));
    }

    if (!m_playerPosition.isNull()) {
        painter.setBrush(QColor("#89ddff"));
        painter.setPen(QPen(QColor("#282c34"), penWidth));
        painter.drawEllipse(m_playerPosition, 6, 6);
    }
}

// --- NEW OVERRIDE IMPLEMENTATION ---
void MapView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event); // Pass the event to the base class
    if (m_isViewLocked) {
        // If the view is locked, we want it to intelligently resize
        resetAndCenterView();
    }
    // If the view is unlocked, we let the user's pan/zoom state persist,
    // even though it might look strange after a resize. When they re-lock, it will fix itself.
}

// --- UNCHANGED METHODS BELOW ---
void MapView::wheelEvent(QWheelEvent* event)
{
    if (m_isViewLocked) return;
    double scaleFactor = 1.15;
    if (event->angleDelta().y() < 0) {
        scaleFactor = 1.0 / scaleFactor;
    }
    QPointF widgetPos = event->position();
    QPointF imagePos = mapWidgetToImage(widgetPos.toPoint());
    m_transform.translate(imagePos.x(), imagePos.y());
    m_transform.scale(scaleFactor, scaleFactor);
    m_transform.translate(-imagePos.x(), -imagePos.y());
    update();
}

void MapView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_isCalibrationMode) {
            emit calibrationPointClicked(mapWidgetToImage(event->pos()));
        }
        else if (!m_isViewLocked) {
            m_isPanning = true;
            m_panLastMousePos = event->pos();
            updateCursorShape();
        }
    }
}

void MapView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isPanning) {
        QPoint delta = event->pos() - m_panLastMousePos;
        m_panLastMousePos = event->pos();
        double dx = delta.x() / m_transform.m11();
        double dy = delta.y() / m_transform.m22();
        m_transform.translate(dx, dy);
        update();
    }
}

void MapView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_isPanning) {
        m_isPanning = false;
        updateCursorShape();
    }
}