//$ HEADER_NEW_FILE $
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   CameraModule.h
 *  @brief  Camera module create different camera views and connect mouse and zoom signals to camera handlers
 *			
 */

#ifndef incl_CameraModule_h
#define incl_CameraModule_h

#include "CameraModuleApi.h"
#include "ModuleLoggingFunctions.h"
#include "Core.h"

#include "CameraHandler.h"
#include "CameraView.h"


#include <QObject>
#include <QList>
#include <QMap>

namespace Camera
{
    class CameraHanlder;
    class CameraWidget;

    class CAMERA_MODULE_API CameraModule :  public QObject, public IModule
    {
        Q_OBJECT

    public:
        //! Constructor.
        CameraModule();
        //! Destructor 
        virtual ~CameraModule();
         /*************** ModuleInterfaceImpl ***************/
        void Load();		
		void UnLoad();		
        void Initialize();		
        void PostInitialize();		
        void Uninitialize();        		
        void Update(f64 frametime);
		//! To manage scene and network events
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

        //! Returns name of this module.
        static const std::string &NameStatic() { return module_name_; } 

		//! Logging
        MODULE_LOGGING_FUNCTIONS

    public slots:
        //!Slots used to update widget viewport
        void UpdateObjectViewport();

        /*!Create a camera widget and a camaera handler and connect them
         * \param title The Name of the camera widget
         * \param restored Widget restored
         * \param camera_type Type of camera
         * \param projection_type Type of projection
         * \param wireframe Wireframe enable/disable
         */
        void CreateNewCamera(QString title = 0, bool restored = false, int camera_type = 0, int projection_type = 1, bool wireframe = 0);

        /*!Create camera view 
         * \param title The name of the camera widget
         * \param restored Widget restored
         */
        CameraWidget* CreateCameraWidget(QString title = 0, bool restored=false);

        /*!Create camera handler
         */
        CameraHandler* CreateCameraHandler();

        /*!Connect camera view signals to camera handlers slots
         * \param view_name The ame of the camera view
         * \param handler_name The name of the camera handler
         * \param camera_type Type of camera
         * \param projection_type Type of projection
         * \param wireframe Wireframe enable/disable
         */
        void ConnectViewToHandler(CameraWidget *camera_view, CameraHandler *camera_handler, int camera_type = 0, int projection_type = 1, bool wireframe = 0);

        /*!Create the camera view when the world scene is changed
         * \param view_name The ame of the camera view
         * \param handler_name The name of the camera handler
		 */
        void DefaultWorldSceneChanged(Scene::SceneManager *scene);

        /*!Delete camera widget
         * \param widget camera widget
		 */
		void DeleteCameraWidget(QWidget *widget);

        /*!Delete the camera widget when it is hidden
		 */
        void OnCameraWidgetHidden();
        /*!Create a new camera widget when new button is clicked
		 */
        void OnNewButtonClicked();
        /*!Delete the camera widget when delete button is clicked
		 */
        void OnDeleteButtonClicked();
        /*!Change the state of wireframe
         * \param state State of wireframe
		 */
        void OnWireframeCheckBoxChanged(int state);  
        /*!Zoom in the camera
         * \param checked Zoom in is checked
		 */
        void OnNearPlusButtonClicked(bool checked);
        /*!Zoom out the camera
         * \param checked Zoom out is checked
		 */
        void OnNearMinusButtonClicked(bool checked);
        void OnFarPlusButtonClicked(bool checked);
        void OnFarMinusButtonClicked(bool checked);

        /*!Create a new camera (dynamic widget)
         * \param name Name of dynamic widhet
         * \param module Name of module
         * \param properties Properties of dynamic widget
		 */
		void OnCreateNewCamera(const QString &name,const QString &module,const QVariantList properties);

    private:
        Q_DISABLE_COPY(CameraModule);      
        /*!Save the camera widgets in initial configuration file
		 */
        void SaveConfig();
        /*!Read the configuration of camera widgets of initial configuration file
		 */
        void ReadConfig();
        /*!Generate a valid widget title
		 */
        void GenerateValidWidgetTitle(QString &title);

		Scene::SceneManager *scene_;

        // Module name
        static const std::string module_name_;

		//! Scene category event to handle
        event_category_id_t scene_event_category_;
        //! Network category event to handle
        event_category_id_t network_state_event_category_;

        //! Map to control conection between CameraWidget and camera handler
        QMap<CameraWidget*,CameraHandler*> controller_view_handlers_;

        //! timer to control cameras views render 
        QTimer *viewport_poller_;

        //! set of created camera titles to generate unique camera title
        QSet<QString> camera_view_titles_;

        //! percent to add or to substract to near or far clip 
        int click_distance_percent_;

        //! Widgets list to delete in the next update. Append widgets when delete button is clicked and when camera widgets is hidden
        QList<CameraWidget*> dirty_widgets_;
    };
}

#endif // incl_CameraModule_h
