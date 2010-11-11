//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UiExternalModule.h"
#include "InputServiceInterface.h"

#include "EventManager.h"
#include "ServiceManager.h"
#include "ConfigurationManager.h"
#include "Framework.h"
#include "WorldStream.h"
#include "NetworkEvents.h"
#include "InputEvents.h"
#include "SceneManager.h"
#include "MainWindow.h"
#include "IEventData.h"
#include "UiServiceInterface.h"
#include "LoginServiceInterface.h"

#include <QObject>
#include <QApplication>
#include <QFontDatabase>
#include <QDir>
#include <QByteArray>
//#include "qcstring.h"


namespace UiExternalServices
{
    std::string UiExternalModule::type_name_static_ = "UIExternal";

    UiExternalModule::UiExternalModule() :
        IModule(type_name_static_),
		qWin_(0),
		menu_manager_(0),
		panel_manager_(0),
		toolbar_manager_(0)
    {
    }

    UiExternalModule::~UiExternalModule()
    {
    }

    void UiExternalModule::Load()
    {
    }

    void UiExternalModule::Unload()
    {
    }

    void UiExternalModule::Initialize()
    {
        qWin_ = dynamic_cast<QMainWindow*>(GetFramework()->GetMainWindow()->parentWidget());
        if (qWin_)
        {
			qWin_->setObjectName("Naali MainWindow");

		   //Create MenuManager and PanelManager and ToolBarManager
		   menu_manager_ = new ExternalMenuManager(qWin_->menuBar(), this);
		   panel_manager_ = new ExternalPanelManager(qWin_);
		   toolbar_manager_ = new ExternalToolBarManager(qWin_);
		   
			// Register UI service
     	    ui_external_scene_service_ = UiExternalServicePtr(new UiExternalService(this));
            framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_ExternalUi, ui_external_scene_service_);
			LogDebug("Ui External service READY");
			
			//Get Event Category 
			 scene_event_category_ = framework_->GetEventManager()->QueryEventCategory("Scene");

			//Configure Static Stuff of the main window
			createStaticContent();
        }
        else
		{
			LogWarning("Could not acquire QMainWindow!");
		}
    }

    void UiExternalModule::PostInitialize()
    {
		/*conf = qWin_->saveState();
		if (conf.isNull() || conf.isEmpty())
			LogWarning("Could not save settings to restore!!!"); */

		//TODO: REMOVE THE FOLLOWING ACTIONS AND CREATE WHEN THE FUNCTIONALITY IS DONE..
		//Create->Scene
		QAction *action = new QAction("Scene:TODO", qWin_); 
		menu_manager_->AddExternalMenuAction(action, "Scene:TODO", "Create");
		//Create->Region
		QAction *action2 = new QAction("Region:TODO", qWin_);
		menu_manager_->AddExternalMenuAction(action2, "Region:TODO", "Create");
		//Panels->Area
		QAction *action3 = new QAction("Area:TODO", qWin_);
		menu_manager_->AddExternalMenuAction(action3, "Area:TODO", "Panels");
		//Panels->Assets
		QAction *action4 = new QAction("Assets:TODO", qWin_);
		menu_manager_->AddExternalMenuAction(action4, "Assets:TODO", "Panels");

		//test settings!
		QAction *action5 = new QAction("Restore_test", qWin_);
		menu_manager_->AddExternalMenuAction(action5, "Restore_test", "Test");
		if (!connect(action5, SIGNAL(triggered()), SLOT(Test())))
			LogWarning("Could not connect with Framework to Exit!!!!"); 

		QAction *action6 = new QAction("Save_test", qWin_);
		menu_manager_->AddExternalMenuAction(action6, "Save_test", "Test");
		if (!connect(action6, SIGNAL(triggered()), SLOT(SaveState())))
			LogWarning("Could not connect with Framework to Exit!!!!"); 


		//TRY TO SAVE STATE OF THE QMAINWINDOW AND RESTORE IT..
		QByteArray state = QString(framework_->GetDefaultConfig().GetSetting<std::string>("ExternalMainWindow", "config").c_str()).toAscii();//.toByteArray();
		qWin_->restoreState(state);


		//qWin_->restoreState(QByteArray(framework_->GetDefaultConfig().GetSetting<std::string>("ExternalMainWindow", "config").data()));
	
		ui_external_scene_service_->SetEnableEditMode(true);
    }

    void UiExternalModule::Uninitialize()
    {
		//TRY TO SAVE STATE OF THE QMAINWINDOW AND RESTORE IT..
		QByteArray test1 = QByteArray("loquesea");
		QString test2 = QString(conf.data());
		
		framework_->GetDefaultConfig().SetSetting("ExternalMainWindow", "test1", QString(test1).toStdString());
		framework_->GetDefaultConfig().SetSetting("ExternalMainWindow", "test2", QString(conf).toStdString());

		framework_->GetDefaultConfig().SetSetting("ExternalMainWindow", "test", std::string("./data/assetcache"));
		framework_->GetDefaultConfig().SetSetting("ExternalMainWindow", "config", QString(QByteArray(qWin_->saveState())).toStdString());
		
        framework_->GetServiceManager()->UnregisterService(ui_external_scene_service_);
        ui_external_scene_service_.reset();
    }

    void UiExternalModule::Update(f64 frametime)
    {
    }

    bool UiExternalModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
    {
   //     if (category_id == scene_event_category_)
			//ui_external_scene_service_.get()->HandleEvent(category_id, event_id, data);
		return false;
    }

	void UiExternalModule::createStaticContent()
	{
		//Create Static Toolbar
		staticToolBar_ = new StaticToolBar("Control Bar",qWin_,framework_);
		staticToolBar_->setObjectName("Control Bar");
		toolbar_manager_->AddExternalToolbar(staticToolBar_, "Control Bar");

		UiServiceInterface *ui_service = framework_->GetService<UiServiceInterface>();
		if (ui_service){
			connect(ui_service, SIGNAL(SceneChanged(const QString&, const QString&)), ui_external_scene_service_.get(), SLOT(SceneChanged(const QString&, const QString&)));
			connect(ui_service, SIGNAL(SceneChanged(const QString&, const QString&)), panel_manager_, SLOT(SceneChanged(const QString&, const QString&)));
			connect(ui_service, SIGNAL(SceneChanged(const QString&, const QString&)), toolbar_manager_, SLOT(SceneChanged(const QString&, const QString&)));
			connect(ui_service, SIGNAL(SceneChanged(const QString&, const QString&)), menu_manager_, SLOT(SceneChanged(const QString&, const QString&)));
		}
		
		//File -> Enter World
		QAction *action = new QAction("Ether", qWin_);
		menu_manager_->AddExternalMenuAction(action, "Ether", "File");
		QString *enter = new QString("Ether");
		if (!connect(action, SIGNAL(triggered()), SLOT(SwitchToEtherScene())))
			LogWarning("Could not connect with Ether scene to go back!!!!");    

		//File -> Exit
		QAction *action2 = new QAction("Exit", qWin_);
		menu_manager_->AddExternalMenuAction(action2, "Exit", "File");
		if (!connect(action2, SIGNAL(triggered()), SLOT(ExitApp())))
			LogWarning("Could not connect with Framework to Exit!!!!");  
        
	}

	void UiExternalModule::SwitchToEtherScene	(){
		UiServicePtr ui = framework_->GetService<UiServiceInterface>(Foundation::Service::ST_Gui).lock();
		if (ui)
			ui.get()->SwitchToScene("Ether");	
	}

	void UiExternalModule::ExitApp(){
		framework_->Exit();	
	}

	void UiExternalModule::Test(){
		qWin_->restoreState(conf);
	}

	void UiExternalModule::SaveState(){
		conf = qWin_->saveState();
		//if (conf.isNull() || conf.isEmpty())
		//	LogWarning("Could not save settings to restore!!!"); 
	}
}

/************** Poco Module Loading System **************/

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace UiExternalServices;

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(UiExternalModule)
POCO_END_MANIFEST