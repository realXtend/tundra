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

        //!Slot used to manage QActions
        void ShowWidget();

        /*!Create camera view and qaction
         *  \param  name The name of the camera to insert in map.
         */
        void CreateCameraView(QString name);

        /*!Create camera handler
         *  \param  name The name of the handler to insert in map
         *  \param  type Camera handler type. CameraHandler::Perpective used as default
         */
        void CreateCameraHandler(QString name, CameraHandler::CameraType type);

        /*!Connect camera view signals to camera handlers slots
         * \param view_name The ame of the camera view
         * \param handler_name The name of the camera handler
         */
        void ConnectViewToHandler(QString view_name, QString handler_name);

		void SceneAdded(const QString &name);

    private:
        Q_DISABLE_COPY(CameraModule);

		Scene::ScenePtr scene_;

        // Module name
        static const std::string module_name_;

		//! Scene category event to handle
        event_category_id_t scene_event_category_;
        //! Network category event to handle
        event_category_id_t network_state_event_category_;

        //! Map to control camera handlers
        QMap<QString,CameraHandler*> controller_camera_handlers_;
        //! Map to control camera views
        QMap<QString,CameraView*> controller_camera_views_;
        //! Map to control qactions to show or hide widgets
        QMap<QAction*,CameraView*> controller_qaction_widgets_;
        //! Map to control conection between cameraview and camera handler
        QMap<CameraView*,CameraHandler*> controller_view_handlers_;


                
        //! timer to control cameras views render 
        QTimer *viewport_poller_;
    };
}

#endif // incl_CameraModule_h
