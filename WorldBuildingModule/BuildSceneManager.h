//$ HEADER_MOD_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_BuildSceneManager_h
#define incl_WorldBuildingModule_BuildSceneManager_h

#include "ForwardDefines.h"
#include "WorldBuildingServiceInterface.h"

#include "WorldObjectView.h"
#include "BuildingWidget.h"
#include "BuildToolbar.h"

#include "PythonHandler.h"
#include "CameraHandler.h"

#include "ui_ObjectInfoWidget.h"
#include "ui_ObjectManipulationsWidget.h"

#include "KeyEvent.h"

#include <QObject>
#include <QTimer>
#include <QPair>
//$ BEGIN_MOD $
#include <QToolBar>
//$ END_MOD $
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
    typedef QPair<QWidget*,QGraphicsProxyWidget*> TransferPair;

    class BuildSceneManager : public Foundation::WorldBuildingServiceInterface
    {
        
    Q_OBJECT
    
    public:
        BuildSceneManager(QObject *parent, Foundation::Framework *framework);
        virtual ~BuildSceneManager();

        bool inworld_state;

    public slots:
        //! Cleans the passed python widgets from our layout
        void CleanPyWidgets();
        bool IsBuildingActive();

        //! Handle our key context input
        void KeyPressed(KeyEvent *key);
        void KeyReleased(KeyEvent *key);

        //! Public functions called by WorldBuildingModule
        void ToggleBuildScene();
        void ShowBuildScene();
        void HideBuildScene();

        void CreateCamera();
        void ResetCamera();
        void ResetEditing();

        void ObjectSelected(Scene::Entity *entity);
        void ObjectDeselected();
        void UpdateObjectViewport();
        
        void RotateObject(qreal x, qreal y);
        void Zoom(qreal delta);

        // WorldBuildingServiceInterface
        virtual QObject *GetPythonHandler() const;
//$ BEGIN_MOD $
		/*! Gets the current entity clicked and update the property widget with this
		 *	\param entity clicked entity
		 */
		void GetSelectedEntity(Scene::Entity *entity);
//$ END_MOD $
	private slots:
        void InitScene();
        void SceneChangedNotification(const QString &old_name, const QString &new_name);
        void ObjectSelected(bool selected);

        void ModeToggleMove();
        void ModeToggleScale();
        void ModeToggleRotate();
        void NewObjectClicked();
        void DuplicateObjectClicked();
        void DeleteObjectClicked();
//$ BEGIN_MOD $
		/*! To toggle move mode when the widget is not in Build
		 */
		void ExternalToggleMove();

		/*! To toggle scale mode when the widget is not in Build
		 */
		void ExternalToggleScale();

		/*! To toggle rotate mode when the widget is not in Build
		 */
		void ExternalToggleRotate();
//$ END_MOD $
        void ManipModeChanged(PythonParams::ManipulationMode mode);

        void HandleWidgetTransfer(const QString &name, QGraphicsProxyWidget *widget);
        void HandleTransfersBack();
        void HandlePythonWidget(const QString &type, QWidget *widget);

        void ToggleLights();
//$ BEGIN_MOD $
		/*! Creates the content of edit toolbar, the Asset widget and the Properties widget
		 */
		void CreateEditContent();

		/*! Action for toggle the mode move
		 */
		void ActionToolBarMove();

		/*! Action for toggle the scale move
		 */
        void ActionToolBarScale();

		/*! Action for toggle the rotate move
		 */
        void ActionToolBarRotate();

		/*! Action for create a cube
		 */
		void ActionToolBarCreate();

		/*! Action for show the properties widget
		 */
		void ActionToolBarProperties();

		/*! Action for show the asset widget
		 */
		void ActionToolBarAsset();

		/*! Checks if the edit mode is active and if it is true, enable the edit 
		 *	bar and the editing, if not, disable all.
		 */
		void ActiveEditMode(bool active);

		/*! If build scene is nor active, active it and creates an object
		 */
		void ChangeAndCreateObject();

		/*! Transfer the widget into build or Asset widget
		 */
		void TransferAssetWidget(bool inside);
		
		/*! Change the mode os manipulation object when uses the Asset Widget
		 */
		void ManipExternalModeChanged(PythonParams::ManipulationMode mode);

		void ShowBuildPanels();
//$ END_MOD $

    private:
        Foundation::Framework *framework_;
        QString scene_name_;

        BuildScene *scene_;
        AnchorLayout *layout_;

        WorldObjectView* world_object_view_;
        Ui::BuildingWidget *object_info_widget_;
        Ui::BuildingWidget *object_manipulations_widget_;
        Ui::BuildToolbar *toolbar_;

        Ui_ObjectInfoWidget object_info_ui;
        Ui_ObjectManipulationsWidget object_manip_ui;

        Helpers::UiHelper *ui_helper_;
        PythonHandler *python_handler_;
        
        PropertyEditorHandler *property_editor_handler_;
        Scene::Entity* selected_entity_;

        View::CameraHandler *camera_handler_;
        View::CameraID selected_camera_id_;

        QList<QWidget*> toggle_visibility_widgets_;
        QList<QWidget*> python_deleted_widgets_;

		QTimer *viewport_poller_;
        bool override_server_time_;
        bool prim_selected_;

        QMap<QString, TransferPair > tranfer_widgets_;
//$ BEGIN_MOD $
		//Actions to disable when worldbuildscene is loaded. Are the menu entries to show widgets outside..
		QAction *action;
		QAction *action2;

		//! Pointer to edit toolbar.
		QToolBar* editToolbar_;
		//! Properties asset widgets
		QWidget* propertyWidget_;
		QWidget* assignWidget_;
		//! Elements of assignWidget
		Ui_ObjectManipulationsWidget asset_ui_;
		//! The elements os Asset widget is inside or outside of build 
		bool inside_;

		QList<QWidget*> panel_widgets_;
//$ END_MOD $
    };
}

#endif