// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EtherStateMachine_h
#define incl_UiModule_EtherStateMachine_h

#include <QStateMachine>
#include <QParallelAnimationGroup>
#include <QGraphicsScene>
#include <QPair>
#include <QVector>
#include <QList>

#include "View/EllipseMenu.h"

namespace Ether
{
    namespace Logic
    {
        class EtherStateMachine : public QStateMachine
        {

        Q_OBJECT

        public:
            EtherStateMachine(QObject *parent, QGraphicsScene *scene, QPair<View::EllipseMenu*, View::EllipseMenu*> menus);

            bool scene_visible;

        private slots:
            void ToggleSceneVisibility();
            void AnimationFinished();

        private:
            QStateMachine *state_machine_;
            QState *state_visible_;
            QState *state_hidden_;
            QParallelAnimationGroup *visibility_animations_;

            QGraphicsScene *scene_;
            QPair<View::EllipseMenu*,View::EllipseMenu*> menus_;

        signals:
            void EtherSceneOutAnimationFinised();

        };
    }
}

#endif // ETHERSTATEMACHINE_H
