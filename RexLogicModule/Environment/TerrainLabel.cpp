#include "StableHeaders.h"
#include "TerrainLabel.h"

#include <QMouseEvent>

namespace RexLogic
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

    void TerrainLabel::mousePressEvent(QMouseEvent *ev)
    {
        emit SendMouseEvent(ev);
        QLabel::mousePressEvent(ev);
    }

    void TerrainLabel::mouseMoveEvent(QMouseEvent *ev)
    {
        emit SendMouseEvent(ev);
        QLabel::mouseMoveEvent(ev);
    }

    void TerrainLabel::mouseReleaseEvent(QMouseEvent *ev)
    {
        emit SendMouseEvent(ev);
        QLabel::mouseReleaseEvent(ev);
    }
}