// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EtherScene_h
#define incl_UiModule_EtherScene_h

#include <QGraphicsScene>
#include <QStateMachine>
#include <QState>
#include <QMap>

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

#include "UiModuleFwd.h"
//#include "InfoCard.h"

namespace Ether
{
    namespace View
    {

        class EtherScene : public QGraphicsScene
        {

        Q_OBJECT

        public:
            EtherScene(QObject *parent, const QRectF &scene_rect);

            void SupressKeyEvents(bool enabled) { supress_key_events_ = enabled; }
            void EmitSwitchSignal();

        protected:
            void keyPressEvent(QKeyEvent *ke);
            void mousePressEvent(QGraphicsSceneMouseEvent *mouse_event);

        private slots:
            void RectChanged(const QRectF &new_rect);

        private:
            QPixmap bg_image_;
            bool supress_key_events_;

        signals:
            void UpPressed();
            void DownPressed();
            void LeftPressed();
            void RightPressed();
            void EnterPressed();

            void ItemClicked(View::InfoCard *clicked_item);

            void EtherSceneReadyForSwitch();
        };
    }
}

#endif // ETHERSCENE_H
