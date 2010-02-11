// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EtherStateMachine.h"
#include "View/InfoCard.h"

#include <QDebug>

namespace Ether
{
    namespace Logic
    {
        EtherStateMachine::EtherStateMachine(QObject *parent, QGraphicsScene *scene, QPair<View::EllipseMenu*, View::EllipseMenu*> menus)
            : QStateMachine(parent),
              scene_(scene),
              scene_visible(false),
              menus_(menus)
        {
            state_visible_ = new QState(this);
            state_hidden_ = new QState(this);
            setInitialState(state_hidden_);

            state_visible_->addTransition(scene_, SIGNAL( EtherTogglePressed()), state_hidden_);
            state_hidden_->addTransition(scene_, SIGNAL( EtherTogglePressed()), state_visible_);

            visibility_animations_ = new QParallelAnimationGroup(this);

            connect(state_hidden_, SIGNAL( exited () ), SLOT( ToggleSceneVisibility() ));
            connect(state_visible_, SIGNAL( exited () ), SLOT( ToggleSceneVisibility() ));
            connect(visibility_animations_, SIGNAL( finished() ), SLOT( AnimationFinished() ));
            
            start();
        }

        void EtherStateMachine::ToggleSceneVisibility()
        {
            if (scene_visible)
            {
                visibility_animations_->clear();
                visibility_animations_->setDirection(QAbstractAnimation::Forward);

                foreach (QGraphicsItem *item, scene_->items())
                {
                    QPropertyAnimation *anim = new QPropertyAnimation(dynamic_cast<QGraphicsWidget*>(item), "opacity", visibility_animations_);
                    anim->setEasingCurve(QEasingCurve::Linear);
                    anim->setDuration(300);
                    anim->setStartValue(item->opacity());
                    anim->setEndValue(0);
                    visibility_animations_->addAnimation(anim);
                }
            }
            else
                visibility_animations_->setDirection(QAbstractAnimation::Backward);

            visibility_animations_->start();
            scene_visible = !scene_visible;
        }

        void EtherStateMachine::AnimationFinished()
        {
            if (visibility_animations_->direction() == QAbstractAnimation::Forward)
                emit EtherSceneOutAnimationFinised();
        }
    }
}
