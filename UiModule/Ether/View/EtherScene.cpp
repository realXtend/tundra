// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EtherScene.h"

#include "Ether/View/InfoCard.h"

#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QList>
#include <QBrush>

#include <QDebug>

#include "MemoryLeakCheck.h"

namespace Ether
{
    namespace View
    {
        EtherScene::EtherScene(QObject *parent, const QRectF &scene_rect)
            : QGraphicsScene(scene_rect, parent),
              supress_key_events_(false)
        {
            bg_image_ = QPixmap("./data/ui/images/ether/main_background.png");
            QBrush bg_brush(bg_image_);            
            setBackgroundBrush(bg_brush);

            connect(this, SIGNAL( sceneRectChanged(const QRectF &) ),
                    SLOT( RectChanged(const QRectF &) ));
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

        void EtherScene::RectChanged(const QRectF &new_rect)
        {
            QPixmap bg = bg_image_.scaled(new_rect.size().toSize());
            setBackgroundBrush(QBrush(bg));
        }

        void EtherScene::EmitSwitchSignal()
        {
            emit EtherSceneReadyForSwitch();
        }
    }
}
