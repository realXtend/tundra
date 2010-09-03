// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiStateMachine_h
#define incl_UiModule_UiStateMachine_h

#include "UiModuleApi.h"
#include "UiTypes.h"

#include <QStateMachine>
#include <QParallelAnimationGroup>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QMap>

namespace Foundation { class KeyBindings; }

namespace CoreUi
{
    typedef QMap<QString, QGraphicsProxyWidget*> UniversalWidgetMap;

    class UI_MODULE_API UiStateMachine : public QStateMachine
    {

    Q_OBJECT

    public:
        UiStateMachine(QGraphicsView *view, QObject *parent = 0);

    public slots:
        void SwitchToInworldScene();
        void SwitchToEtherScene();
        void SwitchToBuildScene();
        void ToggleEther();

        /** Registers new scene.
         *  The instance which creates new scene is also responsible for its deletion.
         *  @param name Name of the scene.
         *  @param scene Graphics scene.
         *  @sa UnregisterScene.
         */
        void RegisterScene(const QString &name, QGraphicsScene *scene);

        /** Unregisters graphics scene.
         *  @param name Name of the scene.
         *  @return True if the scene was found and deleted succesfully, false otherwise.
         *  @note Does not delete the scene, only removes it from the scene map.
         */
        bool UnregisterScene(const QString &name);

        /** Switches the active scene.
         *  @param name Name of the scene.
         *  @return True if the scene existed and was activate ok, false otherwise.
         */
        bool SwitchToScene(const QString &name);

        /** Returns scene with the requested name for introspection.
         *  @param name Name of the scene.
         *  @return Graphic scene with the requested name, or null if not found.
         */
        QGraphicsScene *GetScene(const QString &name) const;

        /// Returns name of the current scene.
        QString GetCurrentSceneName() const { return current_scene_name_;}

        void SetConnectionState(UiServices::ConnectionState new_connection_state);
        void SetServiceGetter(QObject *service_getter);

        void RegisterUniversalWidget(const QString &name, QGraphicsProxyWidget *widget);

        UniversalWidgetMap GetUniversalWidgets() { return universal_widgets_; }

    private slots:
        void DelayedSceneChange();
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

        UiServices::ConnectionState connection_state_;

        QList<QKeySequence> ether_toggle_seq_list_;

        QString current_scene_name_;
        QString next_scene_name_;

        UniversalWidgetMap universal_widgets_;

    signals:
        void EtherTogglePressed();
        void SceneOutAnimationFinised();
        void SceneChanged(const QString &oldName, const QString &newName);
        void SceneChangeComplete();

    };
}

#endif // ETHERSTATEMACHINE_H
