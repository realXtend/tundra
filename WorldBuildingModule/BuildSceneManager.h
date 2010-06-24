// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_BuildSceneManager_h
#define incl_WorldBuildingModule_BuildSceneManager_h

#include "Foundation.h"
#include "UiStateMachine.h"
#include "BuildingWidget.h"
#include "PythonHandler.h"

#include <QObject>

namespace WorldBuilding
{
    class BuildScene;
    class AnchorLayout;

    typedef CoreUi::UiStateMachine StateMachine;

    class BuildSceneManager : public QObject
    {
        
    Q_OBJECT
    
    public:
        BuildSceneManager(QObject *parent, Foundation::Framework *framework);
        virtual ~BuildSceneManager();

        bool inworld_state;

    public slots:
        void ToggleBuildScene();
        void ShowBuildScene();
        void HideBuildScene();

        QObject *GetPythonHandler() { return python_handler_; }

    private slots:
        void InitialseScene();
        StateMachine *GetStateMachine();

    private:
        Foundation::Framework *framework_;
        QString scene_name_;

        BuildScene *scene_;
        AnchorLayout *layout_;

        Ui::BuildingWidget *object_info_widget_;
        Ui::BuildingWidget *object_manipulations_widget_;

        PythonHandler *python_handler_;
        
    };
}

#endif