// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "UiStateMachine.h"

#include <QPropertyAnimation>
#include <QGraphicsWidget>
#include <QDebug>

#include "MemoryLeakCheck.h"

namespace UiServices
{
    UiStateMachine::UiStateMachine(QGraphicsView *view, QObject *parent)
        : QStateMachine(parent),
          view_(view),
          current_scene_(view->scene()),
          connection_state_(Disconnected)
    {
        state_ether_ = new QState(this);
        state_inworld_ = new QState(this);
        state_connecting_ = new QState(this);
        state_animating_change_ = new QState(this);

        SetTransitions();
        setInitialState(state_inworld_);
        start();
    }

    // Private

    void UiStateMachine::SetTransitions()
    {
        state_ether_->addTransition(this, SIGNAL( EtherTogglePressed()), state_inworld_);
        state_inworld_->addTransition(this, SIGNAL( EtherTogglePressed()), state_ether_);

        connect(state_ether_, SIGNAL( exited() ), SLOT( AnimationsStart() ));
        connect(state_inworld_, SIGNAL( exited() ), SLOT( AnimationsStart() ));
        connect(view_, SIGNAL( ViewKeyPressed(QKeyEvent *) ), SLOT( ViewKeyEvent(QKeyEvent *) ));
    }

    void UiStateMachine::ViewKeyEvent(QKeyEvent *key_event)
    {
        switch (key_event->key())
        {
            case Qt::Key_Escape:
                if (connection_state_ == Connected)
                    emit EtherTogglePressed();
                break;
        }
    }

    void UiStateMachine::SetConnectionState(ConnectionState new_connection_state)
    {
        connection_state_ = new_connection_state;

        switch (connection_state_)
        {
            case Disconnected:
                SwitchToEtherScene();
                break;
            case Connected:
                SwitchToInworldScene();
                break;
            case Failed:
                connection_state_ = Disconnected;
                break;
            default:
                return;
        }
    }

    void UiStateMachine::SwitchToInworldScene()
    {
        if (current_scene_ == scene_map_["Ether"])
            AnimationsStart();
    }

    void UiStateMachine::SwitchToEtherScene()
    {
        if (current_scene_ == scene_map_["Inworld"])
            AnimationsStart();
    }

    void UiStateMachine::AnimationsStart()
    {
        // Create group if doesent exist yet
        QParallelAnimationGroup *animations_group;
        if (!animations_map_.contains(current_scene_))
        {
            animations_group = new QParallelAnimationGroup(this);
            animations_map_[current_scene_] = animations_group;
            connect(animations_group, SIGNAL( finished() ), SLOT( AnimationsFinished() ));
        }
        else
            animations_group = animations_map_[current_scene_];

        if (animations_group->state() == QAbstractAnimation::Running)
            return;

        animations_group->clear();
        foreach (QGraphicsItem *item, current_scene_->items())
        {
            QGraphicsWidget *widget = dynamic_cast<QGraphicsWidget*>(item);
            if (!widget)
                continue;
            if (!widget->isVisible())
                continue;

            QPropertyAnimation *anim = new QPropertyAnimation(widget, "opacity", animations_group);
            anim->setDuration(300);
            anim->setStartValue(widget->opacity());
            anim->setEndValue(0);
            animations_group->addAnimation(anim);
        }

        CheckAnimationTargets(animations_group);
        animations_group->setDirection(QAbstractAnimation::Forward);
        animations_group->start();
    }

    void UiStateMachine::AnimationsFinished()
    {
        if (animations_map_[current_scene_]->direction() != QAbstractAnimation::Forward)
            return;

        QString scene_name;
        if (current_scene_ == scene_map_["Ether"])
            scene_name = "Inworld";
        else if (current_scene_ == scene_map_["Inworld"])
            scene_name = "Ether";
        SwitchToScene(scene_name);
    }

    // Public

    void UiStateMachine::RegisterScene(QString name, QGraphicsScene *scene)
    {
        if (!scene_map_.contains(name))
            scene_map_[name] = scene;

        // Scene spesific connections
        if (name == "Ether")
            connect(scene, SIGNAL( EtherSceneReadyForSwitch() ), SLOT( SwitchToInworldScene() ));
    }
    
    void UiStateMachine::SwitchToScene(QString name)
    {
        if (!scene_map_.contains(name))
            return;

        disconnect(current_scene_, SIGNAL( changed(const QList<QRectF> &) ), view_, SLOT( SceneChange() ));
        
        current_scene_ = scene_map_[name];
        current_scene_->setSceneRect(view_->viewport()->rect());
        if (view_->scene() != current_scene_)
            view_->setScene(current_scene_);
                
        connect(current_scene_, SIGNAL( changed(const QList<QRectF> &) ), view_, SLOT( SceneChange() ));

        if (animations_map_.contains(current_scene_))
        {
            QParallelAnimationGroup *animations = animations_map_[current_scene_];
            CheckAnimationTargets(animations);
            animations->setDirection(QAbstractAnimation::Backward);
            animations->start();
        }
    }

    void UiStateMachine::CheckAnimationTargets(QParallelAnimationGroup *animations)
    {
        for (int i=0; i<animations->animationCount(); i++)
        {
            QPropertyAnimation *anim = dynamic_cast<QPropertyAnimation *>(animations->animationAt(i));
            if (!anim)
                continue;
            if (!anim->targetObject())
                animations->removeAnimation(anim);
        }
    }
}
