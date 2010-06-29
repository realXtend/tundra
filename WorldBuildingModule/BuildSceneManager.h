// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_BuildSceneManager_h
#define incl_WorldBuildingModule_BuildSceneManager_h

#include "Foundation.h"
#include "WorldBuildingServiceInterface.h"
#include "Entity.h"

#include "UiStateMachine.h"
#include "BuildingWidget.h"
#include "UiHelper.h"
#include "PythonHandler.h"

#include "ui_ObjectInfoWidget.h"
#include "ui_ObjectManipulationsWidget.h"

#include <QObject>

class QtAbstractPropertyBrowser;
class QtProperty;

namespace WorldBuilding
{
    class BuildScene;
    class AnchorLayout;
    class PropertyEditorHandler;

    typedef CoreUi::UiStateMachine StateMachine;

    class BuildSceneManager : public Foundation::WorldBuildingServiceInterface
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

        // WorldBuildingServiceInterface
        virtual QObject *GetPythonHandler() const;
        
        // Add below to service interface later
        void ObjectSelected(Scene::Entity *entity);
        
    private slots:
        void InitialseScene();
        void ObjectSelected(bool selected);
        StateMachine *GetStateMachine();

    private:
        Foundation::Framework *framework_;
        QString scene_name_;

        BuildScene *scene_;
        AnchorLayout *layout_;

        Ui::BuildingWidget *object_info_widget_;
        Ui::BuildingWidget *object_manipulations_widget_;

        Ui_ObjectInfoWidget object_info_ui;
        Ui_ObjectManipulationsWidget object_manip_ui;

        Helpers::UiHelper ui_helper_;
        PythonHandler *python_handler_;

        PropertyEditorHandler *property_editor_handler_;

        bool prim_selected_;
    };
}

#endif