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
#include "EC_Placeable.h"
#include "NaaliApplication.h"

#include <QColor>


namespace Camera
{

	const std::string CameraModule::module_name_ = std::string("CameraModule");

	CameraModule::CameraModule() :
	    QObject(),
        IModule(module_name_),
		scene_event_category_(0),
        network_state_event_category_(0),
        scene_(0),
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
        //Get Scene and Network event Category 
        scene_event_category_ = framework_->GetEventManager()->QueryEventCategory("Scene");
        connect(framework_, SIGNAL(DefaultWorldSceneChanged(Scene::SceneManager*)), this, SLOT(DefaultWorldSceneChanged(Scene::SceneManager*)));
        connect(framework_->GetNaaliApplication(), SIGNAL(ExitRequested()), this, SLOT(ExitRequested()));


        //Generate widgets
        ReadConfig();
        
	}
	void CameraModule::PostInitialize()
	{
        //Check if UiExternalIsAvailable 
        UiServiceInterface *ui = GetFramework()->GetService<UiServiceInterface>();
        if (ui){            
			//Create Action, insert into menu Camera
			QAction *action = new QAction("New Camera View",this);
			if (ui->AddExternalMenuAction(action, "New Camera View", tr("View"))){
				connect(action, SIGNAL(triggered()), SLOT(CreateNewCamera()));
			}
        }
        connect(viewport_poller_, SIGNAL(timeout()), SLOT(UpdateObjectViewport()));

        //connect qdoc visibility to camera views to manage close event
        QMapIterator<CameraWidget*,CameraHandler*> i(controller_view_handlers_);
        while (i.hasNext()) {
            i.next();
            QDockWidget *doc = dynamic_cast<QDockWidget*>(i.key()->parent());           
            if (doc)
				doc->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
                //connect(doc,SIGNAL(visibilityChanged(bool)), i.key(), SLOT(ParentVisibilityChanged(bool)));          
        }
	}    

	void CameraModule::Uninitialize()
	{
        //Save widget status to .ini file
        SaveConfig();
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
                        QMapIterator<CameraWidget*,CameraHandler*> i(controller_view_handlers_);
                        while (i.hasNext()) {
                            i.next();
                            if (i.key()->isVisible())                                                            
                                i.value()->FocusToEntity(entity_data->entity);                            
                        }
                        handled = true;
                        viewport_poller_->start(200);
                    }
                    break;
                }            
                case Scene::Events::EVENT_ENTITY_NONE_CLICKED:
                {
                    viewport_poller_->stop();
                    handled = true;
                    break;
                }

            }
        }
        return handled;
	}

    void CameraModule::ExitRequested()
    {
        QMapIterator<CameraWidget*,CameraHandler*> i(controller_view_handlers_);
        while (i.hasNext()) {
            i.next();
            QDockWidget *doc = dynamic_cast<QDockWidget*>(i.key()->parent());           
            if (doc)
                disconnect(doc,SIGNAL(visibilityChanged(bool)), i.key(), SLOT(ParentVisibilityChanged(bool)));          
        }

    }

    void CameraModule::DefaultWorldSceneChanged(Scene::SceneManager *scene)
    {
        scene_ = scene;
       QMapIterator<CameraWidget*,CameraHandler*> i(controller_view_handlers_);
        while (i.hasNext()) {
            i.next();
            i.value()->CreateCamera(scene);
        }
    }

    void CameraModule::UpdateObjectViewport()
    {
        QMapIterator<CameraWidget*,CameraHandler*> i(controller_view_handlers_);
        while (i.hasNext()) {
            i.next();
            if (i.key()->isVisible())
                i.key()->GetRenderer()->setPixmap(i.value()->RenderCamera(i.key()->widgetRenderer->size()));
        }
    }

    void CameraModule::CreateNewCameraFromConfig(int camera_type, int projection_type, bool wireframe)
    {
        //Create camera view
        //Create camera handler
        ConnectViewToHandler(CreateCameraWidget(), CreateCameraHandler(),camera_type, projection_type, wireframe);

    }

    void CameraModule::CreateNewCamera()
    {
        //Create camera view
        //Create camera handler
        CameraWidget *new_camera = CreateCameraWidget();
        CameraHandler *handler = CreateCameraHandler();
        ConnectViewToHandler(new_camera, handler);

        //conect qdoc visibility signal
        QDockWidget *doc = dynamic_cast<QDockWidget*>(new_camera->parent());           
        if (doc)
			doc->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
            //connect(doc,SIGNAL(visibilityChanged(bool)), new_camera, SLOT(ParentVisibilityChanged(bool)));          
    }
    
    CameraWidget* CameraModule::CreateCameraWidget()
    {
        CameraWidget* camera_view = new CameraWidget();
        //Check if UiExternalIsAvailable 
        UiServiceInterface *ui = GetFramework()->GetService<UiServiceInterface>();
        if (ui)
        {
            ui->AddWidgetToScene(camera_view,true,true);            
            ui->ShowWidget(camera_view);
        }


        return camera_view;
    }

    CameraHandler* CameraModule::CreateCameraHandler()
    {        
        CameraHandler *camera_handler = new CameraHandler(GetFramework());        
        if (scene_ != 0)
            camera_handler->CreateCamera(scene_);
        return camera_handler;
    }

    void CameraModule::ConnectViewToHandler(CameraWidget *camera_view, CameraHandler *camera_handler, int camera_type, int projection_type, bool wireframe)
    {
        //init camera view
        camera_view->comboBoxCameras->addItem("Perspective");
        camera_view->comboBoxCameras->addItem("Front");
        camera_view->comboBoxCameras->addItem("Back");
        camera_view->comboBoxCameras->addItem("Left");
        camera_view->comboBoxCameras->addItem("Rigth");
        camera_view->comboBoxCameras->addItem("Top");
        camera_view->comboBoxCameras->addItem("Bottom");

        camera_view->comboBoxProjection->addItem("Orthographic");
        camera_view->comboBoxProjection->addItem("Perspective");

        camera_view->comboBoxCameras->setCurrentIndex(camera_type);        
        camera_view->comboBoxProjection->setCurrentIndex(projection_type);
        camera_view->checkBoxWireframe->setChecked(wireframe);

        //init camera handler
        camera_handler->SetCameraType(camera_type);
        camera_handler->SetCameraProjection(projection_type);
        camera_handler->SetCameraWireframe(wireframe);

        //connect signals
         //new camera signal
        connect(camera_view->buttonNewCamera, SIGNAL(released()),this,  SLOT(CreateNewCamera())); 
		camera_view->buttonNewCamera->

        //connect(camera_view, SIGNAL(closeSignal(CameraWidget*)),this,  SLOT(DeleteCameraWidget(CameraWidget*))); 
		connect(camera_view->buttonDeleteCamera, SIGNAL(released()),this,  SLOT(DeleteCameraWidget())); 
        
        //movement signals
        connect(camera_view->GetRenderer(), SIGNAL(Move(qreal, qreal)),camera_handler,  SLOT(Move(qreal, qreal))); 
        connect(camera_view->GetRenderer(), SIGNAL(Zoom(qreal,Qt::KeyboardModifiers)),camera_handler,  SLOT(Zoom(qreal,Qt::KeyboardModifiers))); 
       
        //camera type signal
        connect(camera_view->comboBoxCameras, SIGNAL(currentIndexChanged(int)),camera_handler,  SLOT(SetCameraType(int))); 

        //projection type signal
        connect(camera_view->comboBoxProjection, SIGNAL(currentIndexChanged(int)),camera_handler,  SLOT(SetCameraProjection(int))); 
        //wireframe signal
        connect(camera_view->checkBoxWireframe, SIGNAL(stateChanged(int)),camera_view,  SIGNAL(checkBoxStateChanged(int))); 
        connect(camera_view, SIGNAL(checkBoxStateChanged(int)),this,  SLOT(SetCameraWireframe(int))); 

        //save in map
        controller_view_handlers_.insert(camera_view, camera_handler);       

    }

    void CameraModule::DeleteCameraWidget()
    {
		CameraWidget *camera_view = dynamic_cast<CameraWidget *>(sender()->parent()->parent());

		if (!camera_view)
            return;

        UiServiceInterface *ui = GetFramework()->GetService<UiServiceInterface>();
        if (ui)        
            ui->RemoveWidgetFromScene(camera_view);

        QMap<CameraWidget*,CameraHandler*>::const_iterator i = controller_view_handlers_.find(camera_view);
        while (i != controller_view_handlers_.end() && i.key() == camera_view) {
            CameraWidget *camera = i.key();
            CameraHandler *handler = i.value();            
            controller_view_handlers_.remove(camera);
            delete camera;
            delete handler;       
        } 
    }
    void CameraModule::ReadConfig()
    {
        // Init config file if file/segments doesnt exist
        QSettings camera_config(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/CameraWidgets");

        QStringList cameras = camera_config.childGroups();

        for (int i = 0; i< cameras.length(); i++)
        {
            camera_config.beginGroup(cameras[i]);
            QVariant camera_type = camera_config.value("CameraType");
            QVariant projection_type = camera_config.value("ProjectionType");
            QVariant wireframe = camera_config.value("Wireframe");
            CreateNewCameraFromConfig(camera_type.toInt(), projection_type.toInt(), wireframe.toBool());
            camera_config.endGroup();
        }
        
    }

    void CameraModule::SaveConfig()
    {
        // Init config file if file/segments doesnt exist
        QSettings camera_config(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/CameraWidgets");
        //delete previous file
        camera_config.clear();
        
        QMapIterator<CameraWidget*,CameraHandler*> i(controller_view_handlers_);
        while (i.hasNext()) {
            i.next();
            camera_config.beginGroup(i.key()->windowTitle());
            camera_config.setValue("CameraType", i.value()->GetCameraType());
            camera_config.setValue("ProjectionType", i.value()->GetProjectionType());
            camera_config.setValue("Wireframe", i.value()->IsWireframeEnabled());
            camera_config.endGroup();
        }
    }

    void CameraModule::SetCameraWireframe(int state)
    {
        CameraWidget* camera_view = qobject_cast<CameraWidget*>(sender());
        if (!camera_view)
            return;

        QMap<CameraWidget*,CameraHandler*>::const_iterator i = controller_view_handlers_.find(camera_view);
        while (i != controller_view_handlers_.end() && i.key() == camera_view) {            
            if (state == 0)
                i.value()->SetCameraWireframe(false);
            else
                i.value()->SetCameraWireframe(true);
            i++;
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

