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

        void CreateNewCamera(QString title = 0, bool restored = false, int camera_type = 0, int projection_type = 1, bool wireframe = 0);

        /*!Create camera view 
         */
        CameraWidget* CreateCameraWidget(QString title = 0, bool restored=false);

        /*!Create camera handler         
         */
        CameraHandler* CreateCameraHandler();

        /*!Connect camera view signals to camera handlers slots
         * \param view_name The ame of the camera view
         * \param handler_name The name of the camera handler
         */
        void ConnectViewToHandler(CameraWidget *camera_view, CameraHandler *camera_handler, int camera_type = 0, int projection_type = 1, bool wireframe = 0);

        void DefaultWorldSceneChanged(Scene::SceneManager *scene);

        void DeleteCameraWidget();

        void SetCameraWireframe(int state);        

    private:
        Q_DISABLE_COPY(CameraModule);      

        void SaveConfig();

        void ReadConfig();

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

        QSet<QString> camera_view_titles_;
    };
}

#endif // incl_CameraModule_h
