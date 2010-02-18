// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EtherScene.h"

#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QList>
#include <QBrush>

#include <QDebug>

namespace Ether
{
    namespace View
    {
        EtherScene::EtherScene(QObject *parent, const QRectF &scene_rect)
            : QGraphicsScene(scene_rect, parent),
              supress_key_events_(false)
        {
            QPixmap bg_image("./data/ui/images/ether/tile.png");
            QBrush bg_brush(bg_image);            
            setBackgroundBrush(bg_brush);
        }

        void EtherScene::keyPressEvent(QKeyEvent *ke)
        {
            QGraphicsScene::keyPressEvent(ke);
            if (ke->isAutoRepeat() || supress_key_events_)
                return;

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

                case Qt::Key_Return:
                case Qt::Key_Enter:
                    emit EnterPressed();
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
