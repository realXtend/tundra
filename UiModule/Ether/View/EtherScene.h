// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EtherScene_h
#define incl_UiModule_EtherScene_h

#include <QGraphicsScene>
#include <QStateMachine>
#include <QState>
#include <QMap>

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

#include "InfoCard.h"

namespace Ether
{
    namespace View
    {
        class EtherScene : public QGraphicsScene
        {

        Q_OBJECT

        public:
            EtherScene(QObject *parent, const QRectF &scene_rect);

        protected:
            void keyPressEvent(QKeyEvent *ke);
            void mousePressEvent(QGraphicsSceneMouseEvent *mouse_event);

        signals:
            void UpPressed();
            void DownPressed();
            void LeftPressed();
            void RightPressed();
            void ItemClicked(View::InfoCard *clicked_item);
            void EtherTogglePressed();
        };
    }
}

#endif // ETHERSCENE_H
