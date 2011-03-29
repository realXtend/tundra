//$ HEADER_MOD_FILE $
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   
 *  @brief  
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EtherModule.h"
#include "EtherLogic.h"

#include "UiServiceInterface.h"
#include "Input.h"
#include "LoginServiceInterface.h"
#include "UiServiceInterface.h"
#include "UiProxyWidget.h"
#include "EventManager.h"

#include "WorldLogicInterface.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "Entity.h"
#include "Renderer.h"
#include "ConfigurationManager.h"

#include "TundraLogicModule.h"
#include "Client.h"
#include "SceneManager.h"

#include <Ogre.h>

//#ifdef ENABLE_TAIGA_LOGIN
#include "../ProtocolUtilities/NetworkEvents.h"
//#endif

#include "../TundraLogicModule/TundraEvents.h"

#include "MemoryLeakCheck.h"

namespace Ether
{

#include "LoggingFunctions.h"
	DEFINE_POCO_LOGGING_FUNCTIONS("Ether")

	std::string EtherModule::type_name_static_ = "Ether";

	EtherModule::EtherModule() :
	IModule(type_name_static_),
		ether_logic_(0),
		framework_category_(0),
		network_category_(0),
		connected_(false),
		ui_scene_("")
	{
	}

	EtherModule::~EtherModule()
	{
	}

	void EtherModule::PreInitialize()
	{
	}

	void EtherModule::Initialize()
	{
	}

	void EtherModule::PostInitialize()
	{
		input_ = framework_->GetInput()->RegisterInputContext("EtherInput", 101);
		input_->SetTakeKeyboardEventsOverQt(true);
		connect(input_.get(), SIGNAL(KeyPressed(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));

		UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
		if (ui)
		{
			// Start ether logic and register to scene service
			ether_logic_ = new Ether::Logic::EtherLogic(GetFramework());
			ui->RegisterScene("Ether", ether_logic_->GetScene());
			ether_logic_->Start();
			// Switch ether scene active on startup
			ui->SwitchToScene("Ether");
			// Connect the switch signal to needed places
			connect(ui, SIGNAL(SceneChanged(const QString&, const QString&)), 
				ether_logic_->GetQObjSceneController(), SLOT(UiServiceSceneChanged(const QString&, const QString&)));
			connect(ui, SIGNAL(SceneChanged(const QString&, const QString&)), 
				this, SLOT(SceneChanged(const QString&, const QString&)));
			LogDebug("Ether Logic STARTED");


			//window_ = new LoginWidget(framework_);
			//connect(window_, SIGNAL(ExitClicked()), SLOT(Exit()));

			//ui->AddWidgetToScene(window_, Qt::Widget);
			//ui->ShowWidget(window_);

			//Foundation::LoginServiceInterface *login = framework_->GetService<Foundation::LoginServiceInterface>();
			//if (login)
			//{
			//    connect(window_, SIGNAL(Connect(const QMap<QString, QString> &)),
			//        login, SLOT(ProcessLoginData(const QMap<QString, QString> &)));

			//    // Connect also to Tundra login
			//    connect(window_, SIGNAL(Connect(const QMap<QString, QString> &)),
			//        this, SLOT(ProcessTundraLogin(const QMap<QString, QString> &)));
			//    
			//    connect(login, SIGNAL(LoginStarted()), window_, SLOT(StartProgressBar()));

			//    connect(login, SIGNAL(LoginFailed(const QString &)), window_, SLOT(StopProgressBar()));
			//    connect(login, SIGNAL(LoginFailed(const QString &)), window_, SLOT(SetStatus(const QString &)));

			//    connect(login, SIGNAL(LoginSuccessful()), window_, SLOT(Connected()));
			//}
		}

		framework_category_ = framework_->GetEventManager()->QueryEventCategory("Framework");
		tundra_category_ = framework_->GetEventManager()->QueryEventCategory("Tundra");

		Foundation::WorldLogicInterface *worldLogic = framework_->GetService<Foundation::WorldLogicInterface>();
		if (worldLogic)
			connect(worldLogic, SIGNAL(AboutToDeleteWorld()), SLOT(TakeEtherScreenshots()));
		else
			LogWarning("Could not get world logic service.");

		boost::shared_ptr<TundraLogic::Client> client=framework_->GetModule<TundraLogic::TundraLogicModule>()->GetClient();
		if(client)
			connect(client.get(), SIGNAL(DisconnectEther()), SLOT(TakeEtherScreenshotsForTundra()));
	}

	void EtherModule::Uninitialize()
	{
		SAFE_DELETE(ether_logic_);
		input_.reset();
	}

	void EtherModule::Update(f64 frametime)
	{
		RESETPROFILER;
	}

	// virtual
	bool EtherModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
	{
		UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();

		if (category_id == framework_category_ && event_id == Foundation::NETWORKING_REGISTERED)
		{
			network_category_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
		}
		//#ifdef ENABLE_TAIGA_LOGIN
		else if(category_id == network_category_)
		{
			using namespace ProtocolUtilities::Events;
			switch(event_id)
			{
			case EVENT_SERVER_CONNECTED:
				connected_ = true;
				ether_logic_->GetSceneController()->ShowStatusInformation("Connected, loading world content...", 60000);
				break;
			case EVENT_USER_KICKED_OUT:
			case EVENT_SERVER_DISCONNECTED:
				connected_ = false;
				ether_logic_->SetConnectionState(Disconnected);
				if (ui)
					ui->SwitchToScene("Ether");
				break;
			case EVENT_CONNECTION_FAILED:
				connected_ = false;
				ether_logic_->SetConnectionState(Failed);
				if (ui)
					ui->SwitchToScene("Ether");
				break;
			}
		}
		//#endif
		else if (category_id == tundra_category_)
		{
			switch(event_id)
			{
			case TundraLogic::Events::EVENT_TUNDRA_CONNECTED:
				connected_ = true;
				if (ui && ether_logic_)
				{
					ether_logic_->SetConnectionState(Connected);
					ui->SwitchToMainScene();
					//ui->SwitchToScene("Inworld");
				}
				break;
			case TundraLogic::Events::EVENT_TUNDRA_DISCONNECTED:
				connected_ = false;
				if (ui && ether_logic_)
				{
					ether_logic_->SetConnectionState(Disconnected);
				    ui->SwitchToScene("Ether");
				}
				break;
			}
		}

		return false;
	}

	void EtherModule::HandleKeyEvent(KeyEvent *key)
	{
		// We only act on key presses that are not repeats.
		if (key->eventType != KeyEvent::KeyPressed || key->keyPressCount > 1)
			return;

		if (framework_->IsEditionless())
			return;

		const QKeySequence &toggleMenu = framework_->GetInput()->KeyBinding("Ether.ToggleEther", Qt::Key_Escape);
		if (key->keyCode == toggleMenu)
		{
			UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
			if (ui)
				if (connected_ && ui_scene_ == "Ether")
					//ui->SwitchToScene("Inworld");
					ui->SwitchToMainScene();
				else
					ui->SwitchToScene("Ether");
		}
	}



	void EtherModule::Exit()
	{
		framework_->Exit();
	}

	void EtherModule::TakeEtherScreenshots()
	{
		Foundation::WorldLogicInterface *worldLogic = framework_->GetService<Foundation::WorldLogicInterface>();
		if (!worldLogic)
			return;

		Scene::EntityPtr avatar_entity = worldLogic->GetUserAvatarEntity();
		if (!avatar_entity)
			return;

		EC_Placeable *ec_placeable = avatar_entity->GetComponent<EC_Placeable>().get();
		EC_Mesh *ec_mesh = avatar_entity->GetComponent<EC_Mesh>().get();

		if (!ec_placeable || !ec_mesh || !avatar_entity->HasComponent("EC_AvatarAppearance"))
			return;
		if (!ec_mesh->GetEntity())
			return;

		// Head bone pos setup
		Vector3df avatar_position = ec_placeable->GetPosition();
		Quaternion avatar_orientation = ec_placeable->GetOrientation();
		Ogre::SkeletonInstance* skel = ec_mesh->GetEntity()->getSkeleton();
		float adjustheight = ec_mesh->GetAdjustPosition().z;
		Vector3df avatar_head_position;

		QString view_bone_name = worldLogic->GetAvatarAppearanceProperty("headbone");
		if (!view_bone_name.isEmpty() && skel && skel->hasBone(view_bone_name.toStdString()))
		{
			adjustheight += 0.15f;
			Ogre::Bone* bone = skel->getBone(view_bone_name.toStdString());
			Ogre::Vector3 headpos = bone->_getDerivedPosition();
			Vector3df ourheadpos(-headpos.z + 0.5f, -headpos.x, headpos.y + adjustheight);
			avatar_head_position = avatar_position + (avatar_orientation * ourheadpos);
		}
		else
		{
			// Fallback: will get screwed up shot but not finding the headbone should not happen, ever
			avatar_head_position = ec_placeable->GetPosition();
		}

		//Get paths where to store the screenshots and pass to renderer for screenshots.
		QPair<QString, QString> paths = ether_logic_->GetLastLoginScreenshotData(framework_->GetConfigManager()->GetPath());
		boost::shared_ptr<Foundation::RenderServiceInterface> render_service = 
		    framework_->GetServiceManager()->GetService<Foundation::RenderServiceInterface>(Service::ST_Renderer).lock();

		if (render_service && !paths.first.isEmpty() && !paths.second.isEmpty())
		{
		    QPixmap render_result;
		    render_result = render_service->RenderImage();
		    render_result.save(paths.first);
		    render_result = render_service->RenderAvatar(avatar_head_position, avatar_orientation);
		    render_result.save(paths.second);
		}
	}

	void EtherModule::TakeEtherScreenshotsForTundra()
	{
		
		boost::shared_ptr<TundraLogic::Client> client=framework_->GetModule<TundraLogic::TundraLogicModule>()->GetClient();
		QString name= "Avatar" + QString::number(client->GetConnectionID());
		Scene::EntityPtr avatar_entity = framework_->GetDefaultWorldScene()->GetEntityByName(name);

		if (!avatar_entity)
			return;

		EC_Placeable *ec_placeable = avatar_entity->GetComponent<EC_Placeable>().get();
		EC_Mesh *ec_mesh = avatar_entity->GetComponent<EC_Mesh>().get();

		if (!ec_placeable || !ec_mesh || !avatar_entity->HasComponent("EC_AvatarAppearance"))
			return;
		if (!ec_mesh->GetEntity())
			return;

		// Head bone pos setup
		Vector3df avatar_position = ec_placeable->GetPosition();
		Quaternion avatar_orientation = ec_placeable->GetOrientation();
		Ogre::SkeletonInstance* skel = ec_mesh->GetEntity()->getSkeleton();
		float adjustheight = ec_mesh->GetAdjustPosition().z;
		Vector3df avatar_head_position;
		if(skel->hasBone("Bip01_Head")){
			Ogre::Bone* bone = skel->getBone("Bip01_Head");
			adjustheight += 0.15f;
			Ogre::Vector3 headpos = bone->_getDerivedPosition();
			Vector3df ourheadpos(-headpos.z + 0.5f, -headpos.x, headpos.y + adjustheight);
			avatar_head_position = avatar_position + (avatar_orientation * ourheadpos);
		}else{
			adjustheight += 0.8f;
			Ogre::Vector3 headpos = skel->getRootBone()->_getDerivedPosition();
			Vector3df ourheadpos(-headpos.z + 0.5f, -headpos.x, headpos.y + adjustheight);
			avatar_head_position = avatar_position + (avatar_orientation * ourheadpos);
		}

		//Get paths where to store the screenshots and pass to renderer for screenshots.
		QPair<QString, QString> paths = ether_logic_->GetLastLoginScreenshotData(framework_->GetConfigManager()->GetPath());
		boost::shared_ptr<Foundation::RenderServiceInterface> render_service = 
		    framework_->GetServiceManager()->GetService<Foundation::RenderServiceInterface>(Service::ST_Renderer).lock();

		if (render_service && !paths.first.isEmpty() && !paths.second.isEmpty())
		{
		    QPixmap render_result;
		    render_result = render_service->RenderImage();
		    render_result.save(paths.first);
		    render_result = render_service->RenderAvatar(avatar_head_position, avatar_orientation);
		    render_result.save(paths.second);
		}
	}

	void EtherModule::SceneChanged(const QString &old_name, const QString &new_name)
	{
		ui_scene_ = new_name;
	}

}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace Ether;

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(EtherModule)
POCO_END_MANIFEST
