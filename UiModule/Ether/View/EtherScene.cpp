// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EtherScene.h"

#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QList>

#include <QDebug>

namespace Ether
{
    namespace View
    {
        EtherScene::EtherScene(QObject *parent, const QRectF &scene_rect)
            : QGraphicsScene(scene_rect, parent)
        {
        }

        void EtherScene::keyPressEvent(QKeyEvent *ke)
        {
            QGraphicsScene::keyPressEvent(ke);
            if (ke->isAutoRepeat())
                return;

            // Send key presses to showed scenes logic handler
            switch (ke->key())
            {
                case Qt::Key_W:
                case Qt::Key_Up:
                    emit UpPressed();
                    break;

                case Qt::Key_S:
                case Qt::Key_Down:
                    emit DownPressed();
                    break;

                case Qt::Key_A:
                case Qt::Key_Left:
                    emit LeftPressed();
                    break;

                case Qt::Key_D:
                case Qt::Key_Right:
                    emit RightPressed();
                    break;

                case Qt::Key_Escape:
                    emit EtherTogglePressed();
                    break;

                default:
                    break;
            }
        }

        void EtherScene::mousePressEvent(QGraphicsSceneMouseEvent *mouse_event)
        {
            if (mouse_event->button() == Qt::LeftButton)
            {
                QPointF click_pos = mouse_event->buttonDownScenePos(Qt::LeftButton);
                View::InfoCard *clicked_item = dynamic_cast<View::InfoCard *>(itemAt(click_pos));
                if (clicked_item)
                    emit ItemClicked(clicked_item);
            }
            QGraphicsScene::mousePressEvent(mouse_event);
        }
    }
}
