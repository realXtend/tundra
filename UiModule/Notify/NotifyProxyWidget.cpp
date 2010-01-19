// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Notify/NotifyProxyWidget.h"

#include <QGraphicsScene>
#include <QWidget>

namespace CoreUi
{
    NotifyProxyWidget::NotifyProxyWidget(QWidget *widget)
        : QGraphicsProxyWidget(0, Qt::Widget)
    {
        setWidget(widget);
        hide();
    }

    NotifyProxyWidget::~NotifyProxyWidget()
    {

    }

    QVariant NotifyProxyWidget::itemChange(GraphicsItemChange change, const QVariant &value)
    { 
        if (change == QGraphicsItem::ItemSceneChange)
        {
            QGraphicsScene *scene_pointer = qvariant_cast<QGraphicsScene *>(value);
            if (scene_pointer)
            {
                connect(scene_pointer, SIGNAL( sceneRectChanged(const QRectF &) ), this, SLOT( SceneRectChanged(const QRectF &) ));
                SceneRectChanged(scene_pointer->sceneRect());
            }
        }
        return QGraphicsProxyWidget::itemChange(change, value);
    }

    void NotifyProxyWidget::CheckPosition()
    {
        if (scene())
            SceneRectChanged(scene()->sceneRect());
    }

    void NotifyProxyWidget::SceneRectChanged(const QRectF &new_scene_rect)
    {
        if (!isVisible())
            return;

        QPointF lock_position;
        lock_position.setX(new_scene_rect.right()-size().width());
        lock_position.setY(25);
        setPos(lock_position);
    }
}