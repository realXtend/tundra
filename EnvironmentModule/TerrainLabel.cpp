#include "StableHeaders.h"
#include "TerrainLabel.h"

#include <QMouseEvent>

namespace Environment
{
    TerrainLabel::TerrainLabel(QWidget *parent, Qt::WindowFlags f): QLabel(parent, f)
    {
        setMouseTracking(true);
    }

    TerrainLabel::TerrainLabel(const QString &text, QWidget *parent, Qt::WindowFlags f): QLabel(text, parent, f)
    {
        setMouseTracking(true);
    }

    TerrainLabel::~TerrainLabel()
    {

    }

    void TerrainLabel::mousePressEvent(QMouseEvent *event)
    {
        emit SendMouseEvent(event);
    }

    void TerrainLabel::mouseMoveEvent(QMouseEvent *event)
    {
        emit SendMouseEvent(event);
        emit SendMouseEventWithCanvasName(event, objectName());
    }

    void TerrainLabel::mouseReleaseEvent(QMouseEvent *event)
    {
        emit SendMouseEvent(event);
    }
}