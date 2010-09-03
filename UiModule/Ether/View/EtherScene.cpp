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
              supress_key_events_(false),
              connected_(false)
        {
#ifdef DYNAMIC_LOGIN_SCENE
            bg_image_disconnected_ = QPixmap("./data/ui/images/ether/main_background_disconnected.png");
            bg_image_connected_ = QPixmap("./data/ui/images/ether/main_background_connected.png");
            QBrush bg_brush(bg_image_disconnected_);            
            setBackgroundBrush(bg_brush);

            connect(this, SIGNAL( sceneRectChanged(const QRectF &) ),
                    SLOT( RectChanged(const QRectF &) ));
#endif
        }

        EtherScene::~EtherScene()
        {
        }

        void EtherScene::keyPressEvent(QKeyEvent *ke)
        {
            QGraphicsScene::keyPressEvent(ke);
            if (ke->isAccepted())
                return;

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

        void EtherScene::SetConnectionStatus(bool connected) 
        {
            connected_ = connected;
#ifdef DYNAMIC_LOGIN_SCENE
            RectChanged(sceneRect());
#endif
        }

        void EtherScene::RectChanged(const QRectF &new_rect)
        {
            QPixmap bg;
            if (connected_)
                bg = bg_image_connected_.scaled(new_rect.size().toSize());
            else
                bg = bg_image_disconnected_.scaled(new_rect.size().toSize());
            setBackgroundBrush(QBrush(bg));
        }

        void EtherScene::SupressKeyEvents(bool enabled) 
        { 
            supress_key_events_ = enabled; 
        }

        void EtherScene::EmitSwitchSignal()
        {
            emit EtherSceneReadyForSwitch();
        }

        void EtherScene::HandleWidgetTransfer(const QString &name, QGraphicsProxyWidget *widget)
        {
            if (!widget)
                return;
            if (!isActive())
                return;
            if (widget->scene() == this)
                return;
            if (name.toLower() != "console") // Ether only accepts the console
                return;

            addItem(widget);
            widget->hide();
        }
    }
}
