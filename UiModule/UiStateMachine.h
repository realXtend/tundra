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

namespace Foundation { class KeyBindings; }

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
        void SwitchToBuildScene();
        void RegisterScene(QString name, QGraphicsScene *scene);
        void SwitchToScene(QString name);
        void ToggleEther();

        void SetConnectionState(UiDefines::ConnectionState new_connection_state);
        void SetServiceGetter(QObject *service_getter);

    private slots:
        void SetTransitions();
        void ViewKeyEvent(QKeyEvent *key_event);
        void UpdateKeyBindings(Foundation::KeyBindings *bindings);

        void StateSwitch();
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

        QList<QKeySequence> ether_toggle_seq_list_;

        QString next_scene_name_;

    signals:
        void EtherTogglePressed();
        void SceneOutAnimationFinised();

    };
}

#endif // ETHERSTATEMACHINE_H
