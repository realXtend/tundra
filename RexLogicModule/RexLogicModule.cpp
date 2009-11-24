// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexLogicModule.h"
#include "ComponentManager.h"
#include "Poco/ClassLibrary.h"
#include "EventHandlers/NetworkEventHandler.h"
#include "EventHandlers/NetworkStateEventHandler.h"
#include "EventHandlers/InputEventHandler.h"
#include "EventHandlers/SceneEventHandler.h"
#include "EventHandlers/FrameworkEventHandler.h"
#include "EventDataInterface.h"
#include "TextureInterface.h"
#include "SceneManager.h"
#include "Login/LoginUI.h"
#include "Avatar/AvatarControllable.h"
#include "CameraControllable.h"

#include "EntityComponent/EC_FreeData.h"
#include "EntityComponent/EC_SpatialSound.h"
#include "EntityComponent/EC_OpenSimPrim.h"
#include "EntityComponent/EC_OpenSimPresence.h"
#include "EntityComponent/EC_OpenSimAvatar.h"
#include "EntityComponent/EC_Terrain.h"
#include "EntityComponent/EC_Water.h"
#include "EntityComponent/EC_NetworkPosition.h"
#include "EntityComponent/EC_Controllable.h"
#include "EntityComponent/EC_AvatarAppearance.h"
#include "InputEvents.h"
#include "Inventory/InventoryEvents.h"

// Ogre -specific
#include "Renderer.h"
#include "OgreTextureResource.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMovableTextOverlay.h"
#include "EC_OgreAnimationController.h"

#include <OgreManualObject.h>
#include <OgreSceneManager.h>

#include "Environment/Terrain.h"
#include "Environment/Water.h"
#include "Avatar/Avatar.h"
#include "Environment/Primitive.h"
#include "Environment/Sky.h"
#include "Environment/Environment.h"
#include "Environment/TerrainEditor.h"

#include "QtUtils.h"
#include "Avatar/AvatarEditor.h"
#include "RexTypes.h"

namespace RexLogic
{

RexLogicModule::RexLogicModule() : ModuleInterfaceImpl(type_static_),
    send_input_state_(false),
    login_failed_showed_(false),
    movement_damping_constant_(10.0f),
    camera_state_(CS_Follow)
{
}

RexLogicModule::~RexLogicModule()
{
}

// virtual
void RexLogicModule::Load()
{
    PROFILE(RexLogicModule_Load);

    DECLARE_MODULE_EC(EC_FreeData);
    DECLARE_MODULE_EC(EC_SpatialSound);
    DECLARE_MODULE_EC(EC_OpenSimPrim);
    DECLARE_MODULE_EC(EC_OpenSimPresence);
    DECLARE_MODULE_EC(EC_OpenSimAvatar);
    DECLARE_MODULE_EC(EC_Terrain);
    DECLARE_MODULE_EC(EC_Water);
    DECLARE_MODULE_EC(EC_NetworkPosition);
    DECLARE_MODULE_EC(EC_Controllable);
    DECLARE_MODULE_EC(EC_AvatarAppearance);
    
    AutoRegisterConsoleCommand(Console::CreateCommand("Login", 
        "Login to server. Usage: Login(user=Test User, passwd=test, server=localhost",
        Console::Bind(this, &RexLogicModule::ConsoleLogin)));

    AutoRegisterConsoleCommand(Console::CreateCommand("Logout", 
        "Logout from server.",
        Console::Bind(this, &RexLogicModule::ConsoleLogout)));
        
    AutoRegisterConsoleCommand(Console::CreateCommand("Fly",
        "Toggle flight mode.",
        Console::Bind(this, &RexLogicModule::ConsoleToggleFlyMode)));

    LogInfo("Module " + Name() + " loaded.");
}

// virtual
void RexLogicModule::Unload()
{
    LogInfo("Module " + Name() + " unloaded.");
}

// virtual
void RexLogicModule::Initialize()
{
    PROFILE(RexLogicModule_Initialize);

    framework_->GetEventManager()->RegisterEventCategory("Action");

    /// \todo fixme, register WorldLogic to the framework as realxtend worldlogicinterface!
    // WorldLogic::registerSystem(framework);
    // world_logic_ = new WorldLogic(framework);
    avatar_ = AvatarPtr(new Avatar(this));
    avatar_editor_ = AvatarEditorPtr(new AvatarEditor(this));
    primitive_ = PrimitivePtr(new Primitive(this));
    world_stream_ = WorldStreamConnectionPtr(new ProtocolUtilities::WorldStream(framework_));
    network_handler_ = new NetworkEventHandler(framework_, this);
    network_state_handler_ = new NetworkStateEventHandler(framework_, this);
    input_handler_ = new InputEventHandler(framework_, this);
    scene_handler_ = new SceneEventHandler(framework_, this);
    framework_handler_ = new FrameworkEventHandler(world_stream_.get(), framework_, this);
    avatar_controllable_ = AvatarControllablePtr(new AvatarControllable(this));
    camera_controllable_ = CameraControllablePtr(new CameraControllable(framework_));
    terrain_editor_ = TerrainEditorPtr(new TerrainEditor(this));

    movement_damping_constant_ = framework_->GetDefaultConfig().DeclareSetting(
        "RexLogicModule", "movement_damping_constant", 10.0f);

    dead_reckoning_time_ = framework_->GetDefaultConfig().DeclareSetting(
        "RexLogicModule", "dead_reckoning_time", 2.0f);

    camera_state_ = static_cast<CameraState>(framework_->GetDefaultConfig().DeclareSetting(
        "RexLogicModule", "default_camera_state", static_cast<int>(CS_Follow)));

    LogInfo("Module " + Name() + " initialized.");
}

// virtual
void RexLogicModule::PostInitialize()
{
    // Input events.
    Core::event_category_id_t eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Input");
    if (eventcategoryid != 0)
    {
        event_handlers_[eventcategoryid].push_back(boost::bind(
            &AvatarControllable::HandleInputEvent, avatar_controllable_.get(), _1, _2));
        event_handlers_[eventcategoryid].push_back(boost::bind(
            &CameraControllable::HandleInputEvent, camera_controllable_.get(), _1, _2));
        event_handlers_[eventcategoryid].push_back(boost::bind(
            &InputEventHandler::HandleInputEvent, input_handler_, _1, _2));
    } else
        LogError("Unable to find event category for Input");

	// Action events.
	eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Action");
	if (eventcategoryid != 0)
	{
		event_handlers_[eventcategoryid].push_back(boost::bind(
			&AvatarControllable::HandleActionEvent, avatar_controllable_.get(), _1, _2));
		event_handlers_[eventcategoryid].push_back(
			boost::bind(&CameraControllable::HandleActionEvent, camera_controllable_.get(), _1, _2));
	} else
		LogError("Unable to find event category for Action");

	// Scene events.
	eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Scene");
	if (eventcategoryid != 0)
	{
		event_handlers_[eventcategoryid].push_back(boost::bind(
			&SceneEventHandler::HandleSceneEvent, scene_handler_, _1, _2));
		event_handlers_[eventcategoryid].push_back(boost::bind(
			&AvatarControllable::HandleSceneEvent, avatar_controllable_.get(), _1, _2));
		event_handlers_[eventcategoryid].push_back(boost::bind(
			&CameraControllable::HandleSceneEvent, camera_controllable_.get(), _1, _2));
	} else
		LogError("Unable to find event category for Scene");

	// Resource events
	eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Resource");
	if (eventcategoryid != 0)
		event_handlers_[eventcategoryid].push_back(
			boost::bind(&RexLogicModule::HandleResourceEvent, this, _1, _2));
	else
		LogError("Unable to find event category for Resource");

    // Inventory events
	eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Inventory");
	if (eventcategoryid != 0)
		event_handlers_[eventcategoryid].push_back(
			boost::bind(&RexLogicModule::HandleInventoryEvent, this, _1, _2));
	else
		LogError("Unable to find event category for Inventory");

    // Asset events
	eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Asset");
	if (eventcategoryid != 0)
		event_handlers_[eventcategoryid].push_back(
			boost::bind(&RexLogicModule::HandleAssetEvent, this, _1, _2));
	else
		LogError("Unable to find event category for Asset");
    
	// Framework events
	eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Framework");
	if (eventcategoryid != 0)
		event_handlers_[eventcategoryid].push_back(boost::bind(
			&FrameworkEventHandler::HandleFrameworkEvent, framework_handler_, _1, _2));
	else
		LogError("Unable to find event category for Framework");

	boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>
		(Foundation::Service::ST_Renderer).lock();
	if (renderer)
	{
		Ogre::Camera *cam = renderer->GetCurrentCamera();
		cam->setPosition(-10, -10, -10);
		cam->lookAt(0,0,0);
	}

	send_input_state_ = true;

	// Create the login window.
	loginUI_ = new Login(framework_, this);
}

void RexLogicModule::SubscribeToNetworkEvents(boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> currentProtocolModule)
{
    // NetworkState events
	LogicEventHandlerMap::iterator i;
    Core::event_category_id_t eventcategoryid = framework_->GetEventManager()->QueryEventCategory("NetworkState");
    if (eventcategoryid != 0)
	{
		i = event_handlers_.find(eventcategoryid);
		if (i == event_handlers_.end())
		{
			event_handlers_[eventcategoryid].push_back(boost::bind(
				&NetworkStateEventHandler::HandleNetworkStateEvent, network_state_handler_, _1, _2));
			LogInfo("System " + Name() + " subscribed to network events [NetworkState] and added to LogicEventHandlerMap");
		}
		else
		{
			LogInfo("System " + Name() + " had already added [NetworkState] event to LogicEventHandlerMap");
		}
	}
    else
        LogError("Unable to find event category for NetworkState");

    // NetworkIn events
    eventcategoryid = framework_->GetEventManager()->QueryEventCategory("NetworkIn");
    if (eventcategoryid != 0)
	{
		i = event_handlers_.find(eventcategoryid);
		if (i == event_handlers_.end())
		{
			event_handlers_[eventcategoryid].push_back(boost::bind(
				&NetworkEventHandler::HandleOpenSimNetworkEvent, network_handler_, _1, _2));
			LogInfo("System " + Name() + " subscribed to network events [NetworkIn]");
		}
		else
		{
			LogInfo("System " + Name() + " had already added [NetworkIn] event to LogicEventHandlerMap");
		}
	}
    else
        LogError("Unable to find event category for NetworkIn");
}

void RexLogicModule::SendModifyLandMessage(Core::f32 x, Core::f32 y, Core::u8 brush, Core::u8 action, Core::Real seconds, Core::Real height)
{
    GetServerConnection()->SendModifyLandPacket(x, y, brush, action, seconds, height);
}

void RexLogicModule::DeleteScene(const std::string &name)
{
    if (!framework_->HasScene(name))
    {
        LogWarning("Tried to delete scene, but it didn't exist!");
        return;
    }

    if (activeScene_ && activeScene_->Name() == name)
        activeScene_.reset(); ///\todo Check in SceneManager that scene names surely are unique. -jj.

    framework_->RemoveScene(name);
    assert(!framework_->HasScene(name));
}

// virtual
void RexLogicModule::Uninitialize()
{
    if (world_stream_->IsConnected())
        LogoutAndDeleteWorld();

    world_stream_.reset();
    avatar_.reset();
    avatar_editor_.reset();
    primitive_.reset();
    avatar_controllable_.reset();
    camera_controllable_.reset();
    environment_.reset();
    terrain_editor_.reset();

    event_handlers_.clear();

    SAFE_DELETE (network_handler_);
    SAFE_DELETE (input_handler_);
    SAFE_DELETE (scene_handler_);
    SAFE_DELETE (network_state_handler_);
    SAFE_DELETE (framework_handler_);
	SAFE_DELETE (loginUI_);

    LogInfo("Module " + Name() + " uninitialized.");
}

#ifdef _DEBUG
void RexLogicModule::DebugSanityCheckOgreCameraTransform()
{
    boost::shared_ptr<OgreRenderer::Renderer> renderer = GetFramework()->GetServiceManager()->GetService
        <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    if (!renderer)
        return;

    Ogre::Camera *camera = renderer->GetCurrentCamera();
    Ogre::Vector3 up = camera->getUp();
    Ogre::Vector3 fwd = camera->getDirection();
    Ogre::Vector3 right = camera->getRight();
    float l1 = up.length();
    float l2 = fwd.length();
    float l3 = right.length();
    float p1 = up.dotProduct(fwd);
    float p2 = fwd.dotProduct(right);
    float p3 = right.dotProduct(up);
    std::stringstream ss;
    if (abs(l1 - 1.f) > 1e-3f || abs(l2 - 1.f) > 1e-3f || abs(l3 - 1.f) > 1e-3f ||
        abs(p1) > 1e-3f || abs(p2) > 1e-3f || abs(p3) > 1e-3f)
    {
        ss << "Warning! Camera TM base not orthonormal! Pos. magnitudes: " << l1 << ", " << l2 << ", " <<
            l3 << ", Dot product magnitudes: " << p1 << ", " << p2 << ", " << p3;
        LogDebug(ss.str());
    }
}
#endif

// virtual
void RexLogicModule::Update(Core::f64 frametime)
{
    {
        PROFILE(RexLogicModule_Update);

        // interpolate & animate objects
        UpdateObjects(frametime);

        // update avatar stuff (download requests etc.)
        avatar_->Update(frametime);

        // Poll the connection state and update the info to the UI.
        ProtocolUtilities::Connection::State present_state = world_stream_->GetConnectionState();
        if (present_state == ProtocolUtilities::Connection::STATE_LOGIN_FAILED)
        {
            if (!login_failed_showed_)
            {
                login_failed_showed_ = true;
                GetLoginUI()->ShowMessageToUser(QString(world_stream_->GetConnectionErrorMessage().c_str()), 10);
            }
        }
        else if ( present_state != ProtocolUtilities::Connection::STATE_CONNECTED && 
                  present_state != ProtocolUtilities::Connection::STATE_DISCONNECTED &&
                  present_state != ProtocolUtilities::Connection::STATE_ENUM_COUNT)
        {
            login_failed_showed_ = false;
            GetLoginUI()->UpdateLoginProgressUI(QString(""), 0, present_state);
        }

        /// \todo Move this to OpenSimProtocolModule.
        if (!world_stream_->IsConnected() && world_stream_->GetConnectionState() == ProtocolUtilities::Connection::STATE_INIT_UDP)
        {
            world_stream_->CreateUdpConnection();
        }

        if (send_input_state_)
        {
            send_input_state_ = false;

            // can't send events during initalization, so workaround
            Core::event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
            if (camera_state_ == CS_Follow)
                GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_THIRDPERSON, 0);
            else
                GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_FREECAMERA, 0);
        }

        if (world_stream_->IsConnected())
        {
            avatar_controllable_->AddTime(frametime);
            camera_controllable_->AddTime(frametime);

            // Update avatar name overlay positions.
            GetAvatarHandler()->UpdateAvatarNameOverlayPositions();

            // Update environment-spesific visual effects.
            GetEnvironmentHandler()->UpdateVisualEffects(frametime);
        }
    }

    RESETPROFILER;
}

// virtual
bool RexLogicModule::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data)
{
    PROFILE(RexLogicModule_HandleEvent);
    LogicEventHandlerMap::iterator i = event_handlers_.find(category_id);
    if (i != event_handlers_.end())
    {
        for (size_t j=0 ; j<i->second.size() ; j++)
        {
            if ((i->second[j])(event_id, data))
                return true;
        }
    }
    return false;
}

bool RexLogicModule::HandleResourceEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
{
    // Pass the event to the avatar manager
    avatar_->HandleResourceEvent(event_id, data);
    // Pass the event to the primitive manager
    primitive_->HandleResourceEvent(event_id, data);

    if (event_id == Resource::Events::RESOURCE_READY)
    {
        Resource::Events::ResourceReady *res = dynamic_cast<Resource::Events::ResourceReady*>(data);
        assert(res);
        if (!res)
            return false;
        
        OgreRenderer::OgreTextureResource *tex = dynamic_cast<OgreRenderer::OgreTextureResource *>(res->resource_.get());
        if (tex)
        {
            // Pass the texture asset to the terrain manager - the texture might be in the terrain.
            terrain_->OnTextureReadyEvent(res);
            // Pass the texture asset to the sky manager - the texture might be in the sky.
            sky_->OnTextureReadyEvent(res);
        }
    }

    return false;
}

bool RexLogicModule::HandleInventoryEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
{
    // Pass the event to the avatar manager
    return avatar_->HandleInventoryEvent(event_id, data);
}

bool RexLogicModule::HandleAssetEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
{
    // Pass the event to the avatar manager
    return avatar_->HandleAssetEvent(event_id, data);
}

void RexLogicModule::LogoutAndDeleteWorld()
{
    world_stream_->RequestLogout();
    world_stream_->ForceServerDisconnect(); // Because the current server doesn't send a logoutreplypacket.

    if (avatar_)
        avatar_->HandleLogout();
    
    if (framework_->HasScene("World"))
        DeleteScene("World");
}

void RexLogicModule::ShowAvatarEditor()
{
    if (avatar_editor_)
        avatar_editor_->Toggle();
}

//wrappers for calling stuff elsewhere in logic module from outside (python api module)
void RexLogicModule::SetAvatarYaw(Core::Real newyaw)
{
    avatar_controllable_->SetYaw(newyaw);
}

void RexLogicModule::SetAvatarRotation(Core::Quaternion newrot)
{
    std::cout << "RexLogicModule::SetAvatarRotation" << std::endl;
    avatar_controllable_->SetRotation(newrot);
}

void RexLogicModule::SetCameraYawPitch(Core::Real newyaw, Core::Real newpitch)
{
    camera_controllable_->SetYawPitch(newyaw, newpitch);
}

Core::entity_id_t RexLogicModule::GetUserAvatarId()
{
    RexLogic::AvatarPtr avatarPtr = GetAvatarHandler();
    Scene::EntityPtr entity = avatarPtr->GetUserAvatar();
    return entity->GetId();
}

Core::Vector3df RexLogicModule::GetCameraUp(){
    boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    Ogre::Camera *camera = renderer->GetCurrentCamera();
    Ogre::Vector3 up = camera->getUp();
    return Core::Vector3df(up.x, up.y, up.z);
}

Core::Vector3df RexLogicModule::GetCameraRight(){
    boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    Ogre::Camera *camera = renderer->GetCurrentCamera();
    Ogre::Vector3 right = camera->getRight();
    return Core::Vector3df(right.x, right.y, right.z);
}

void RexLogicModule::SendRexPrimData(Core::entity_id_t entityid)
{
    GetPrimitiveHandler()->SendRexPrimData(entityid);
}


Console::CommandResult RexLogicModule::ConsoleLogin(const Core::StringVector &params)
{
    std::string name = "Test User";
    std::string passwd = "test";
    std::string server = "localhost";

    if (params.size() > 0)
        name = params[0];
    if (params.size() > 1)
    {
        passwd = params[1];
        const std::string &param_pass = params[1];

        // overwrite the password so it won't stay in-memory
        const_cast<std::string&>(param_pass).replace(0, param_pass.size(), param_pass.size(), ' ');
    }
    if (params.size() > 2)
        server = params[2];

	//! REMOVE
    //bool success = world_stream_->ConnectToServer(name, passwd, server);

    // overwrite the password so it won't stay in-memory
    //passwd.replace(0, passwd.size(), passwd.size(), ' ');

    //if (success)
    //    return Console::ResultSuccess();
    //else
    //    return Console::ResultFailure("Failed to connect to server.");
	return Console::ResultFailure("Cannot login from console no more");
}

Console::CommandResult RexLogicModule::ConsoleLogout(const Core::StringVector &params)
{
    if (world_stream_->IsConnected())
    {
        LogoutAndDeleteWorld();
        return Console::ResultSuccess();
    } else
    {
        return Console::ResultFailure("Not connected to server.");
    }
}

Console::CommandResult RexLogicModule::ConsoleToggleFlyMode(const Core::StringVector &params)
{
    Core::event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
    GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::TOGGLE_FLYMODE, 0);
    return Console::ResultSuccess();
}

void RexLogicModule::SwitchCameraState()
{
    if (camera_state_ == CS_Follow)
    {
        camera_state_ = CS_Free;

        Core::event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
        GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_FREECAMERA, 0);
    }
    else
    {
        camera_state_ = CS_Follow;

        Core::event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
        GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_THIRDPERSON, 0);
    }
}

void RexLogicModule::CreateTerrain()
{
    terrain_ = TerrainPtr(new Terrain(this));

    Scene::EntityPtr entity = activeScene_->CreateEntity(activeScene_->GetNextFreeId());
    entity->AddEntityComponent(GetFramework()->GetComponentManager()->CreateComponent("EC_Terrain"));

    terrain_->FindCurrentlyActiveTerrain();
}

void RexLogicModule::CreateWater()
{
    water_ = WaterPtr(new Water(this));
    water_->CreateWaterGeometry();
}

void RexLogicModule::CreateSky()
{
    sky_ = SkyPtr(new Sky(this));
    Scene::EntityPtr entity = activeScene_->CreateEntity(activeScene_->GetNextFreeId());
    entity->AddEntityComponent(GetFramework()->GetComponentManager()->CreateComponent("EC_OgreSky"));

    sky_->FindCurrentlyActiveSky();

    if (!GetEnvironmentHandler()->UseCaelum())
        sky_->CreateDefaultSky();
}

void RexLogicModule::CreateEnvironment()
{
    environment_ = EnvironmentPtr(new Environment(this));
    environment_->CreateEnvironment();
}

TerrainPtr RexLogicModule::GetTerrainHandler()
{
    return terrain_;
}

TerrainEditorPtr RexLogicModule::GetTerrainEditor()
{
    return terrain_editor_;
}

WaterPtr RexLogicModule::GetWaterHandler()
{
    return water_;
}

AvatarPtr RexLogicModule::GetAvatarHandler()
{
    return avatar_;
}

AvatarEditorPtr RexLogicModule::GetAvatarEditor()
{
    return avatar_editor_;
}

PrimitivePtr RexLogicModule::GetPrimitiveHandler()
{
    return primitive_;
}

SkyPtr RexLogicModule::GetSkyHandler()
{
    return sky_;
}

EnvironmentPtr RexLogicModule::GetEnvironmentHandler()
{
    return environment_;
}

InventoryPtr RexLogicModule::GetInventory() const
{
    return world_stream_->GetInfo().inventory;
}

void RexLogicModule::SetCurrentActiveScene(Scene::ScenePtr scene)
{
    activeScene_ = scene;
}

Scene::ScenePtr RexLogicModule::GetCurrentActiveScene()
{
    return activeScene_;
}

Scene::ScenePtr RexLogicModule::CreateNewActiveScene(const std::string &name)
{
    if (framework_->HasScene(name))
    {
        LogWarning("Tried to create new active scene, but it already existed!");
        Scene::ScenePtr newActiveScene = framework_->GetScene(name);
        SetCurrentActiveScene(newActiveScene);
        return newActiveScene;
    }

    activeScene_ = framework_->CreateScene(name);
    framework_->SetDefaultWorldScene(activeScene_);

    // Also create a default terrain to the Scene. This is done here dynamically instead of fixed in RexLogic,
    // since we might have 0-N terrains later on, depending on where we actually connect to. Now of course
    // we just create one default terrain.
    CreateTerrain();

    CreateEnvironment();

    // Also create a default sky to the scene.
    CreateSky();

    // Create a water handler.
    CreateWater();

    return GetCurrentActiveScene();
}

Scene::EntityPtr RexLogicModule::GetEntityWithComponent(Core::entity_id_t entityid, const std::string &requiredcomponent)
{
    if (!activeScene_)
        return Scene::EntityPtr();

    Scene::EntityPtr entity = activeScene_->GetEntity(entityid);
    if (entity && entity->GetComponent(requiredcomponent))
        return entity;
    else
        return Scene::EntityPtr();
}

Scene::EntityPtr RexLogicModule::GetPrimEntity(const RexUUID &entityuuid)
{
    IDMap::iterator iter = UUIDs_.find(entityuuid);
    if (iter == UUIDs_.end())
        return Scene::EntityPtr();
    else
        return GetPrimEntity(iter->second);
} 

Scene::EntityPtr RexLogicModule::GetAvatarEntity(const RexUUID &entityuuid)
{
    IDMap::iterator iter = UUIDs_.find(entityuuid);
    if (iter == UUIDs_.end())
        return Scene::EntityPtr();
    else
        return GetAvatarEntity(iter->second);
}

void RexLogicModule::RegisterFullId(const RexUUID &fullid, Core::entity_id_t entityid)
{
    UUIDs_[fullid] = entityid;
}

void RexLogicModule::UnregisterFullId(const RexUUID &fullid)
{
    IDMap::iterator iter = UUIDs_.find(fullid);
    if (iter != UUIDs_.end())
        UUIDs_.erase(iter);
}

void RexLogicModule::UpdateObjects(Core::f64 frametime)
{
    //! \todo probably should not be directly in RexLogicModule
    if (!activeScene_)
        return;

    // Damping interpolation factor, dependent on frame time
    Core::Real factor = pow(2.0, -frametime * movement_damping_constant_);
    if (factor < 0.0) factor = 0.0;
    if (factor > 1.0) factor = 1.0;
    Core::Real rev_factor = 1.0 - factor;

    for(Scene::SceneManager::iterator iter = activeScene_->begin();
        iter != activeScene_->end(); ++iter)
    {
        Scene::Entity &entity = **iter;
        Foundation::ComponentPtr ogrepos_ptr = entity.GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
        Foundation::ComponentPtr netpos_ptr = entity.GetComponent(EC_NetworkPosition::NameStatic());
        if (ogrepos_ptr && netpos_ptr)
        {
            OgreRenderer::EC_OgrePlaceable &ogrepos = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(ogrepos_ptr.get()); 
            EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(netpos_ptr.get()); 

            if (netpos.time_since_update_ <= dead_reckoning_time_)
            {
                netpos.time_since_update_ += frametime; 

                // Interpolate motion
                // acceleration disabled until figured out what goes wrong. possibly mostly irrelevant with OpenSim server
                // netpos.velocity_ += netpos.accel_ * frametime;
                netpos.position_ += netpos.velocity_ * frametime;

                // Interpolate rotation
                if (netpos.rotvel_.getLengthSQ() > 0.001)
                {
                    Core::Quaternion rot_quat1;
                    Core::Quaternion rot_quat2;
                    Core::Quaternion rot_quat3;

                    rot_quat1.fromAngleAxis(netpos.rotvel_.x * 0.5 * frametime, Core::Vector3df(1,0,0));
                    rot_quat2.fromAngleAxis(netpos.rotvel_.y * 0.5 * frametime, Core::Vector3df(0,1,0));
                    rot_quat3.fromAngleAxis(netpos.rotvel_.z * 0.5 * frametime, Core::Vector3df(0,0,1));

                    netpos.rotation_ *= rot_quat1;
                    netpos.rotation_ *= rot_quat2;
                    netpos.rotation_ *= rot_quat3;
                }

                // Dampened (smooth) movement
                if (netpos.damped_position_ != netpos.position_)
                    netpos.damped_position_ = netpos.position_ * rev_factor + netpos.damped_position_ * factor;

                if (netpos.damped_rotation_ != netpos.rotation_)
                    netpos.damped_rotation_.slerp(netpos.rotation_, netpos.damped_rotation_, factor);

                ogrepos.SetPosition(netpos.damped_position_);
                ogrepos.SetOrientation(netpos.damped_rotation_);
            }
        }

        Foundation::ComponentPtr animctrl_ptr = entity.GetComponent(OgreRenderer::EC_OgreAnimationController::NameStatic());
        if (animctrl_ptr)
        {
            // If is an avatar, handle update for avatar animations
            if (entity.GetComponent(EC_OpenSimAvatar::NameStatic()))
                avatar_->UpdateAvatarAnimations(entity.GetId(), frametime);

            // General animation controller update
            OgreRenderer::EC_OgreAnimationController &animctrl = *checked_static_cast<OgreRenderer::EC_OgreAnimationController*>(animctrl_ptr.get());
            animctrl.Update(frametime);
        }
    }
}

} // namespace RexLogic

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace RexLogic;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(RexLogicModule)
POCO_END_MANIFEST

