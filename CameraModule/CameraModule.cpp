//$ HEADER_NEW_FILE $
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   CameraModule.cpp
 *  @brief  Simple OpenSim Empty module. receives the chat message and plays it 
 *			using the Festival Empty wuth the configuration established in the current session.
 */

#include "StableHeaders.h"

#include "CameraModule.h"
#include "UiServiceInterface.h"
#include "EventManager.h"
#include "SceneEvents.h"
#include "NetworkEvents.h"
#include "EC_OpenSimPrim.h"
#include "EC_Placeable.h"

#include <QColor>


namespace Camera
{

	const std::string CameraModule::module_name_ = std::string("CameraModule");

	CameraModule::CameraModule() :
	    QObject(),
        IModule(module_name_),
		scene_event_category_(0),
        network_state_event_category_(0),
        viewport_poller_(new QTimer(this))
	{


	}

	CameraModule::~CameraModule()
	{
	}

	void CameraModule::Load()
	{
	}

	void CameraModule::UnLoad()
	{
	}

	void CameraModule::Initialize()
	{
		connect(GetFramework(), SIGNAL(SceneAdded(const QString&)), this, SLOT(SceneAdded(const QString&)));
        //Get Scene and Network event Category 
        scene_event_category_ = framework_->GetEventManager()->QueryEventCategory("Scene");
        network_state_event_category_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
        
	}
	void CameraModule::PostInitialize()
	{
		//Check if UiExternalIsAvailable 
    //    UiServiceInterface *ui = GetFramework()->GetService<UiServiceInterface>();
    //    if (ui){
	//		CameraView* cam_view = new CameraView("Perspective");        
	//		controller_camera_views_.insert("Perspective",cam_view);
     //       QAction *cam_action = new QAction("Camera", this);
    //        ui->AddExternalMenuAction(cam_action,"Camera",tr("View"));
	//		connect(cam_action, SIGNAL(triggered()), SLOT(ShowWidget()));
    //     }

        //create camera view add to scene and create qaction
        CreateCameraView("Perpective");
        CreateCameraView("Front");
        CreateCameraView("Back");
        CreateCameraView("Left");
        CreateCameraView("Rigth");
        CreateCameraView("Top");
        CreateCameraView("Bottom");

        // Create camera handlers
        CreateCameraHandler("Perpective", CameraHandler::Perspective);
        CreateCameraHandler("Front", CameraHandler::Front);
        CreateCameraHandler("Back", CameraHandler::Back);
        CreateCameraHandler("Left", CameraHandler::Left);
        CreateCameraHandler("Rigth", CameraHandler::Rigth);
        CreateCameraHandler("Top", CameraHandler::Top);
        CreateCameraHandler("Bottom", CameraHandler::Bottom);

        //connect view signals to handler slots
        ConnectViewToHandler("Perpective","Perpective");
        ConnectViewToHandler("Front","Front");
        ConnectViewToHandler("Back","Back");
        ConnectViewToHandler("Left","Left");
        ConnectViewToHandler("Rigth","Rigth");
        ConnectViewToHandler("Top","Top");
        ConnectViewToHandler("Bottom","Bottom");

        connect(viewport_poller_, SIGNAL(timeout()), SLOT(UpdateObjectViewport()));
	}

	void CameraModule::SceneAdded(const QString &name)
	{
		scene_ = GetFramework()->GetScene(name);
		QMapIterator<CameraView*,CameraHandler*> i(controller_view_handlers_);
        while (i.hasNext()) {
             i.next();
             if (i.key())
                   i.value()->CreateCamera(scene_);
        }     
	}


	void CameraModule::Uninitialize()
	{
	}

	void CameraModule::Update(f64 frametime)
	{

	}

	bool CameraModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
	{
        bool handled = false;

        if (category_id == scene_event_category_)
        { 
            switch(event_id)
            {
                case Scene::Events::EVENT_ENTITY_CLICKED:
                {               
                    Scene::Events::EntityClickedData *entity_data = checked_static_cast<Scene::Events::EntityClickedData*>(data);
                    if (entity_data)
                    {
                        QMapIterator<CameraView*,CameraHandler*> i(controller_view_handlers_);
                        while (i.hasNext()) {
                            i.next();
                            if (i.key()->isVisible())                                                            
                                i.value()->FocusToEntity(entity_data->entity);                            
                        }
                        viewport_poller_->start(200);
                    }
                    break;
                }            
                case Scene::Events::EVENT_ENTITY_NONE_CLICKED:
                {
                    viewport_poller_->stop();
                    break;
                }

            }
        }
        return handled;
	}

    void CameraModule::UpdateObjectViewport()
    {
        QMapIterator<CameraView*,CameraHandler*> i(controller_view_handlers_);
        while (i.hasNext()) {
            i.next();
            if (i.key()->isVisible())
                i.key()->setPixmap(i.value()->RenderCamera(i.key()->size()));
        }
    }

    void CameraModule::CreateCameraView(QString name)
    {
        CameraView* camera_view = new CameraView(name);        
        controller_camera_views_.insert(name,camera_view);

        //Check if UiExternalIsAvailable 
        UiServiceInterface *ui = GetFramework()->GetService<UiServiceInterface>();
        if (ui){
            ui->AddWidgetToScene(camera_view,true,true);                                 
			//Create Action, insert into menu Camera
			QAction *action = new QAction(name,this);
            controller_qaction_widgets_.insert(action,camera_view);
			if (ui->AddExternalMenuAction(action, name, tr("Cameras"))){
				connect(action, SIGNAL(triggered()), SLOT(ShowWidget()));
			}
        }
    }

    void CameraModule::CreateCameraHandler(QString name, CameraHandler::CameraType type)
    {        
        controller_camera_handlers_.insert(name,new CameraHandler(GetFramework(),type));
    }

    void CameraModule::ConnectViewToHandler(QString view_name, QString handler_name)
    {
        //check view
        QMap<QString, CameraView*>::const_iterator v = controller_camera_views_.find(view_name);
        while (v != controller_camera_views_.end() && v.key() == view_name) {
            //check handler 
            QMap<QString, CameraHandler*>::const_iterator h = controller_camera_handlers_.find(handler_name);
            while (h != controller_camera_handlers_.end() && h.key() == handler_name) {
                //insert in qmap
                controller_view_handlers_.insert(v.value(),h.value());
                //connect signals v.value() = CameraView, h.value() = CameraHandler
                connect(v.value(), SIGNAL(Move(qreal, qreal)),h.value(),  SLOT(Move(qreal, qreal))); 
                connect(v.value(), SIGNAL(Zoom(qreal)),h.value(),  SLOT(Zoom(qreal))); 
                h++;
            }
            v++;            
        }         
    }

    void CameraModule::ShowWidget()
    {
        QAction* action = qobject_cast<QAction*>(sender());

        QMap<QAction*,CameraView*>::const_iterator i = controller_qaction_widgets_.find(action);
        while (i != controller_qaction_widgets_.end() && i.key() == action) {
            UiServiceInterface *ui = GetFramework()->GetService<UiServiceInterface>();
            if(!i.value()->isVisible())
              ui->ShowWidget(i.value());
            else
              ui->HideWidget(i.value());
           ++i;
        }         
    }


}


extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}


using namespace Camera;

POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(CameraModule)
POCO_END_MANIFEST

