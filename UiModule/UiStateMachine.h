// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiStateMachine_h
#define incl_UiModule_UiStateMachine_h

#include "UiModuleApi.h"
#include "UiDefines.h"

#include <QStateMachine>
#include <QParallelAnimationGroup>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QMap>

namespace CoreUi
{
    class UI_MODULE_API UiStateMachine : public QStateMachine
    {

    Q_OBJECT

    public:
        UiStateMachine(QGraphicsView *view, QObject *parent = 0);

    public slots:
        void SwitchToInworldScene();
        void SwitchToEtherScene();
        void RegisterScene(QString name, QGraphicsScene *scene);
        void SwitchToScene(QString name);

        void SetConnectionState(UiDefines::ConnectionState new_connection_state);

    private slots:
        void SetTransitions();
        void ViewKeyEvent(QKeyEvent *key_event);

        void AnimationsStart();
        void AnimationsFinished();

        void CheckAnimationTargets(QParallelAnimationGroup *animations);

    private:
        QStateMachine *state_machine_;
        QState *state_ether_;
        QState *state_inworld_;
        QState *state_connecting_;
        QState *state_animating_change_;

        QGraphicsView *view_;
        QGraphicsScene *current_scene_;

        QMap<QString, QGraphicsScene*> scene_map_;
        QMap<QGraphicsScene*, QParallelAnimationGroup*> animations_map_;

        UiDefines::ConnectionState connection_state_;

    signals:
        void EtherTogglePressed();
        void SceneOutAnimationFinised();

    };
}

#endif // ETHERSTATEMACHINE_H
