// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_BuildSceneManager_h
#define incl_WorldBuildingModule_BuildSceneManager_h

#include "ForwardDefines.h"
#include "WorldBuildingServiceInterface.h"

#include "WorldObjectView.h"
#include "BuildingWidget.h"

#include "PythonHandler.h"
#include "CameraHandler.h"

#include "ui_ObjectInfoWidget.h"
#include "ui_ObjectManipulationsWidget.h"

#include <QtInputKeyEvent.h>
#include <QObject>

class QtAbstractPropertyBrowser;
class QtProperty;

namespace Ui
{
    class BuildinWidget;
}

namespace CoreUi
{
    class UiStateMachine;
}

namespace WorldBuilding
{
    class BuildScene;
    class AnchorLayout;
    class PropertyEditorHandler;
    class WorldObjectView;

    namespace Helpers
    {
        class UiHelper;
    }

    typedef CoreUi::UiStateMachine StateMachine;

    class BuildSceneManager : public Foundation::WorldBuildingServiceInterface
    {
        
    Q_OBJECT
    
    public:
        BuildSceneManager(QObject *parent, Foundation::Framework *framework);
        virtual ~BuildSceneManager();

        bool inworld_state;

    public slots:
        //! Handle our key context input
        void KeyPressed(KeyEvent *key);
        void KeyReleased(KeyEvent *key);

        // Public functions called by WorldBuildingModule
        void ToggleBuildScene();
        void ShowBuildScene();
        void HideBuildScene();

        void CreateCamera();
        void ResetCamera();
        void ResetEditing();

        void ObjectSelected(Scene::Entity *entity);
        void ObjectDeselected();
        
        void RotateObject(qreal x, qreal y);
        void Zoom(qreal delta);

        // WorldBuildingServiceInterface
        virtual QObject *GetPythonHandler() const;

    private slots:
        void InitialseScene();
        void SceneChangedNotification(const QString &old_name, const QString &new_name);
        void ObjectSelected(bool selected);

        void ModeToggleMove();
        void ModeToggleScale();
        void ModeToggleRotate();
        void NewObjectClicked();
        void DuplicateObjectClicked();
        void DeleteObjectClicked();

        void ManipModeChanged(PythonParams::ManipulationMode mode);

        StateMachine *GetStateMachine() const;

    private:
        Foundation::Framework *framework_;
        QString scene_name_;

        BuildScene *scene_;
        AnchorLayout *layout_;

        WorldObjectView* world_object_view_;
        ObjectViewData object_view_data_;

        Ui::BuildingWidget *object_info_widget_;
        Ui::BuildingWidget *object_manipulations_widget_;

        Ui_ObjectInfoWidget object_info_ui;
        Ui_ObjectManipulationsWidget object_manip_ui;

        Helpers::UiHelper *ui_helper_;
        PythonHandler *python_handler_;
        
        PropertyEditorHandler *property_editor_handler_;
        bool prim_selected_;
        Scene::Entity* selected_entity_;

        View::CameraHandler *camera_handler_;
        View::CameraID selected_camera_id_;        
    };
}

#endif