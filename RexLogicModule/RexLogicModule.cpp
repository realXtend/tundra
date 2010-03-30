// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "RexLogicModule.h"
#include "ComponentManager.h"
#include "EventDataInterface.h"
#include "TextureInterface.h"
#include "SoundServiceInterface.h"
#include "InputServiceInterface.h"
#include "SceneManager.h"
#include "Avatar/AvatarControllable.h"
#include "CameraControllable.h"

#include "EventHandlers/NetworkEventHandler.h"
#include "EventHandlers/NetworkStateEventHandler.h"
#include "EventHandlers/InputEventHandler.h"
#include "EventHandlers/SceneEventHandler.h"
#include "EventHandlers/FrameworkEventHandler.h"
#include "EventHandlers/LoginHandler.h"
#include "EventHandlers/MainPanelHandler.h"

#include "EntityComponent/EC_FreeData.h"
#include "EntityComponent/EC_AttachedSound.h"
#include "EntityComponent/EC_OpenSimPrim.h"
#include "EntityComponent/EC_OpenSimPresence.h"
#include "EntityComponent/EC_OpenSimAvatar.h"
#include "EntityComponent/EC_NetworkPosition.h"
#include "EntityComponent/EC_Controllable.h"
#include "EntityComponent/EC_AvatarAppearance.h"

// External EC's
#include "EC_Highlight.h"
#include "EC_HoveringText.h"
#include "EC_Clone.h"

// Ogre -specific
#include "Renderer.h"
#include "RenderServiceInterface.h"
#include "OgreTextureResource.h"
#include "EC_OgreCamera.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMovableTextOverlay.h"
#include "EC_OgreAnimationController.h"
#include "EC_OgreMesh.h"
#include "EC_OgreMovableTextOverlay.h"
#include "EC_OgreCustomObject.h"

#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreViewport.h>
#include <OgreEntity.h>

#include "Avatar/Avatar.h"
#include "Environment/Primitive.h"
#include "WorldStream.h"

#include "Avatar/AvatarEditor.h"
#include "RexTypes.h"

#include "UiModule.h"

#include "EventManager.h"
#include "ConfigurationManager.h"
#include "ModuleManager.h"
#include "ConsoleCommand.h"
#include "ConsoleCommandServiceInterface.h"
#include "ServiceManager.h"

#include "MemoryLeakCheck.h"

namespace RexLogic
{

RexLogicModule::RexLogicModule() : ModuleInterfaceImpl(type_static_),
    send_input_state_(false),
    movement_damping_constant_(10.0f),
    camera_state_(CS_Follow),
    network_handler_(0),
    input_handler_(0),
    scene_handler_(0),
    network_state_handler_(0),
    framework_handler_(0),
    os_login_handler_(0),
    taiga_login_handler_(0),
    main_panel_handler_(0)
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
    DECLARE_MODULE_EC(EC_AttachedSound);
    DECLARE_MODULE_EC(EC_OpenSimPrim);
    DECLARE_MODULE_EC(EC_OpenSimPresence);
    DECLARE_MODULE_EC(EC_OpenSimAvatar);
    DECLARE_MODULE_EC(EC_NetworkPosition);
    DECLARE_MODULE_EC(EC_Controllable);
    DECLARE_MODULE_EC(EC_AvatarAppearance);
    DECLARE_MODULE_EC(EC_Highlight);
    DECLARE_MODULE_EC(EC_HoveringText);
    DECLARE_MODULE_EC(EC_Clone);
}

// virtual
void RexLogicModule::Initialize()
{
    PROFILE(RexLogicModule_Initialize);
    Foundation::EventManagerPtr eventmgr = framework_->GetEventManager();
    eventmgr->RegisterEventCategory("Action");

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
    main_panel_handler_ = new MainPanelHandler(framework_, this);

    movement_damping_constant_ = framework_->GetDefaultConfig().DeclareSetting(
        "RexLogicModule", "movement_damping_constant", 10.0f);

    dead_reckoning_time_ = framework_->GetDefaultConfig().DeclareSetting(
        "RexLogicModule", "dead_reckoning_time", 2.0f);

    camera_state_ = static_cast<CameraState>(framework_->GetDefaultConfig().DeclareSetting(
        "RexLogicModule", "default_camera_state", static_cast<int>(CS_Follow)));
}

// virtual
void RexLogicModule::PostInitialize()
{
    // Input events.
    event_category_id_t eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Input");
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

    send_input_state_ = true;

    // Create login handlers, get login notifier from ether and pass
    // that into rexlogic login handlers for slots/signals setup
    os_login_handler_ = new OpenSimLoginHandler(framework_, this);
    taiga_login_handler_ = new TaigaLoginHandler(framework_, this);

    boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (ui_module.get())
    {
        QObject *notifier = ui_module->GetEtherLoginNotifier();
        if (notifier)
        {
            os_login_handler_->SetLoginNotifier(notifier);
            taiga_login_handler_->SetLoginNotifier(notifier);
        }
    }

    RegisterConsoleCommand(Console::CreateCommand("Login", 
        "Login to server. Usage: Login(user=Test User, passwd=test, server=localhost",
        Console::Bind(this, &RexLogicModule::ConsoleLogin)));

    RegisterConsoleCommand(Console::CreateCommand("Logout", 
        "Logout from server.",
        Console::Bind(this, &RexLogicModule::ConsoleLogout)));
        
    RegisterConsoleCommand(Console::CreateCommand("Fly",
        "Toggle flight mode.",
        Console::Bind(this, &RexLogicModule::ConsoleToggleFlyMode)));

    RegisterConsoleCommand(Console::CreateCommand("Highlight",
        "Adds/removes EC_Highlight for every prim and mesh. Usage: highlight(add|remove)."
        "If add is called and EC already exists for entity, EC's visibility is toggled.",
        Console::Bind(this, &RexLogicModule::ConsoleHighlightTest)));
}

void RexLogicModule::SubscribeToNetworkEvents(boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> currentProtocolModule)
{
    // NetworkState events
    LogicEventHandlerMap::iterator i;
    event_category_id_t eventcategoryid = framework_->GetEventManager()->QueryEventCategory("NetworkState");
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

    event_handlers_.clear();

    SAFE_DELETE(network_handler_);
    SAFE_DELETE(input_handler_);
    SAFE_DELETE(scene_handler_);
    SAFE_DELETE(network_state_handler_);
    SAFE_DELETE(framework_handler_);
    SAFE_DELETE(os_login_handler_);
    SAFE_DELETE(taiga_login_handler_);
    SAFE_DELETE(main_panel_handler_);
}

#ifdef _DEBUG
void RexLogicModule::DebugSanityCheckOgreCameraTransform()
{
    OgreRenderer::RendererPtr renderer = GetRendererPtr();
    if (!renderer.get())
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
void RexLogicModule::Update(f64 frametime)
{
    {
        PROFILE(RexLogicModule_Update);

        // interpolate & animate objects
        UpdateObjects(frametime);

        // update avatar stuff (download requests etc.)
        avatar_->Update(frametime);
        UpdateAvatarNameTags(avatar_->GetUserAvatar());

        // update sound listener position/orientation
        UpdateSoundListener();

        //! This is not needed anymore, as ether is the new login ui. If we want to send these world stream states to ui layer,
        //! 
        //ProtocolUtilities::Connection::State present_state = world_stream_->GetConnectionState();

        /// \todo Move this to OpenSimProtocolModule.
        if (!world_stream_->IsConnected() && world_stream_->GetConnectionState() == ProtocolUtilities::Connection::STATE_INIT_UDP)
        {
            world_stream_->CreateUdpConnection();
        }

        if (send_input_state_)
        {
            send_input_state_ = false;

            // can't send events during initalization, so workaround
            event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
            if (camera_state_ == CS_Follow)
                GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_THIRDPERSON, 0);
            else
                GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_FREECAMERA, 0);
        }

        if (world_stream_->IsConnected())
        {
            avatar_controllable_->AddTime(frametime);
            camera_controllable_->AddTime(frametime);

            // Update overlays last, after camera update
            UpdateAvatarOverlays();
        }
    }

    RESETPROFILER;
}

// virtual
bool RexLogicModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
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

bool RexLogicModule::HandleResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data)
{
    // Pass the event to the avatar manager
    avatar_->HandleResourceEvent(event_id, data);
    // Pass the event to the primitive manager
    primitive_->HandleResourceEvent(event_id, data);

    return false;
}

bool RexLogicModule::HandleInventoryEvent(event_id_t event_id, Foundation::EventDataInterface* data)
{
    // Pass the event to the avatar manager
    return avatar_->HandleInventoryEvent(event_id, data);
}

bool RexLogicModule::HandleAssetEvent(event_id_t event_id, Foundation::EventDataInterface* data)
{
    // Pass the event to the avatar manager
    return avatar_->HandleAssetEvent(event_id, data);
}

void RexLogicModule::LogoutAndDeleteWorld()
{
    AboutToDeleteWorld();

    world_stream_->RequestLogout();
    world_stream_->ForceServerDisconnect(); // Because the current server doesn't send a logoutreplypacket.

    if (avatar_)
        avatar_->HandleLogout();
    if (primitive_)
        primitive_->HandleLogout();

    if (framework_->HasScene("World"))
        DeleteScene("World");

    pending_parents_.clear();
}

//XXX \todo add dll exports or fix by some other way (e.g. qobjects)
//wrappers for calling stuff elsewhere in logic module from outside (python api module)
void RexLogicModule::SetAvatarYaw(Real newyaw)
{
    avatar_controllable_->SetYaw(newyaw);
}

void RexLogicModule::SetAvatarRotation(Quaternion newrot)
{
    avatar_controllable_->SetRotation(newrot);
}

void RexLogicModule::SetCameraYawPitch(Real newyaw, Real newpitch)
{
    camera_controllable_->SetYawPitch(newyaw, newpitch);
}

entity_id_t RexLogicModule::GetUserAvatarId()
{
    RexLogic::AvatarPtr avatarPtr = GetAvatarHandler();
    Scene::EntityPtr entity = avatarPtr->GetUserAvatar();
    return entity->GetId();
}

Vector3df RexLogicModule::GetCameraUp()
{
    if (camera_entity_.expired())
        return Vector3df();

    Foundation::ComponentPtr placeable = camera_entity_.lock()->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
    OgreRenderer::EC_OgrePlaceable* placeable_ptr = dynamic_cast<OgreRenderer::EC_OgrePlaceable*>(placeable.get());
    if (!placeable_ptr)
        return Vector3df();

    //! \todo check if Ogre or OpenSim axis convention should actually be used
    return placeable_ptr->GetOrientation() * Vector3df(0.0f,1.0f,0.0f);
}

Vector3df RexLogicModule::GetCameraRight()
{
    if (camera_entity_.expired())
        return Vector3df();

    Foundation::ComponentPtr placeable = camera_entity_.lock()->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
    OgreRenderer::EC_OgrePlaceable* placeable_ptr = dynamic_cast<OgreRenderer::EC_OgrePlaceable*>(placeable.get());
    if (!placeable_ptr)
        return Vector3df();

    //! \todo check if Ogre or OpenSim axis convention should actually be used
    return placeable_ptr->GetOrientation() * Vector3df(1.0f,0.0f,0.0f);
}

Real RexLogicModule::GetCameraFOV()
{
    if (camera_entity_.expired())
        return 0.0f;

    Foundation::ComponentPtr camera = camera_entity_.lock()->GetComponent(OgreRenderer::EC_OgreCamera::NameStatic());
    OgreRenderer::EC_OgreCamera* camera_ptr = dynamic_cast<OgreRenderer::EC_OgreCamera*>(camera.get());
    if (!camera_ptr)
        return 0.0f;

    return camera_ptr->GetVerticalFov(); 
}

Real RexLogicModule::GetCameraViewportWidth()
{
    OgreRenderer::RendererPtr renderer = GetRendererPtr();
    if (!renderer.get())
        return 0;

    Ogre::Viewport *viewport = renderer->GetViewport();
    return viewport->getActualWidth();
}

Real RexLogicModule::GetCameraViewportHeight()
{
    OgreRenderer::RendererPtr renderer = GetRendererPtr();
    if (!renderer.get())
        return 0;

    Ogre::Viewport *viewport = renderer->GetViewport();
    return viewport->getActualHeight();
}

Vector3df RexLogicModule::GetCameraPosition()
{
    if (camera_entity_.expired())
        return Vector3df();

    Foundation::ComponentPtr placeable = camera_entity_.lock()->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
    OgreRenderer::EC_OgrePlaceable* placeable_ptr = dynamic_cast<OgreRenderer::EC_OgrePlaceable*>(placeable.get());
    if (!placeable_ptr)
        return Vector3df();

    return placeable_ptr->GetPosition();
}

Quaternion RexLogicModule::GetCameraOrientation()
{
    if (camera_entity_.expired())
        return Quaternion::IDENTITY;

    Foundation::ComponentPtr placeable = camera_entity_.lock()->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
    OgreRenderer::EC_OgrePlaceable* placeable_ptr = dynamic_cast<OgreRenderer::EC_OgrePlaceable*>(placeable.get());
    if (!placeable_ptr)
        return Quaternion::IDENTITY;

    return placeable_ptr->GetOrientation();
}

void RexLogicModule::SendRexPrimData(entity_id_t entityid)
{
    GetPrimitiveHandler()->SendRexPrimData(entityid);
}

OgreRenderer::RendererPtr RexLogicModule::GetRendererPtr()
{
    return framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
}

Console::CommandResult RexLogicModule::ConsoleLogin(const StringVector &params)
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

Console::CommandResult RexLogicModule::ConsoleLogout(const StringVector &params)
{
    if (world_stream_->IsConnected())
    {
        LogoutAndDeleteWorld();
        return Console::ResultSuccess();
    } 
    else
    {
        return Console::ResultFailure("Not connected to server.");
    }
}

void RexLogicModule::StartLoginOpensim(QString qfirstAndLast, QString qpassword, QString qserverAddressWithPort)
{
    if (!qserverAddressWithPort.startsWith("http://"))
        qserverAddressWithPort = "http://" + qserverAddressWithPort;

    QMap<QString, QString> map;
    map["AuthType"] = "OpenSim";
    map["Username"] = qfirstAndLast;
    map["Password"] = qpassword;
    map["WorldAddress"] = qserverAddressWithPort;

    os_login_handler_->ProcessOpenSimLogin(map);
}

Console::CommandResult RexLogicModule::ConsoleToggleFlyMode(const StringVector &params)
{
    event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
    GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::TOGGLE_FLYMODE, 0);
    return Console::ResultSuccess();
}

Console::CommandResult RexLogicModule::ConsoleHighlightTest(const StringVector &params)
{
    if (!activeScene_)
        return Console::ResultFailure("No active scene found.");

    if (params.size() != 1 || (params[0] != "add" && params[0] != "remove"))
        return Console::ResultFailure("Invalid syntax. Usage: highlight(add|remove).");

    for(Scene::SceneManager::iterator iter = activeScene_->begin(); iter != activeScene_->end(); ++iter)
    {
        Scene::Entity &entity = **iter;
        OgreRenderer::EC_OgreMesh *ec_mesh = entity.GetComponent<OgreRenderer::EC_OgreMesh>().get();
        OgreRenderer::EC_OgreCustomObject *ec_custom = entity.GetComponent<OgreRenderer::EC_OgreCustomObject>().get();
        if (ec_mesh || ec_custom)
        {
            if (params[0] == "add")
            {
                boost::shared_ptr<EC_Highlight> highlight = entity.GetComponent<EC_Highlight>();
                if (!highlight)
                {
                    // If we didn't have the higihlight component yet, create one now.
                    entity.AddComponent(framework_->GetComponentManager()->CreateComponent("EC_Highlight"));
                    highlight = entity.GetComponent<EC_Highlight>();
                    assert(highlight.get());
                }

                if (highlight->IsVisible())
                    highlight->Hide();
                else
                    highlight->Show();
            }
            else if (params[0] == "remove")
            {
                boost::shared_ptr<EC_Highlight> highlight = entity.GetComponent<EC_Highlight>();
                if (highlight)
                    entity.RemoveComponent(highlight);
            }
        }
    }

    return Console::ResultSuccess();
}

void RexLogicModule::SwitchCameraState()
{
    if (camera_state_ == CS_Follow)
    {
        camera_state_ = CS_Free;

        event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
        GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_FREECAMERA, 0);
    }
    else
    {
        camera_state_ = CS_Follow;

        event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
        GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_THIRDPERSON, 0);
    }
}

AvatarPtr RexLogicModule::GetAvatarHandler() const
{
    return avatar_;
}

AvatarEditorPtr RexLogicModule::GetAvatarEditor() const
{
    return avatar_editor_;
}

PrimitivePtr RexLogicModule::GetPrimitiveHandler() const
{
    return primitive_;
}

InventoryPtr RexLogicModule::GetInventory() const
{
    return world_stream_->GetInfo().inventory;
}

void RexLogicModule::SetCurrentActiveScene(Scene::ScenePtr scene)
{
    activeScene_ = scene;
}

Scene::ScenePtr RexLogicModule::GetCurrentActiveScene() const
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

    // Create camera entity into the scene
    {
        Foundation::Framework* fw = GetFramework();
        
        Foundation::ComponentPtr placeable = fw->GetComponentManager()->CreateComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
        Foundation::ComponentPtr camera = fw->GetComponentManager()->CreateComponent(OgreRenderer::EC_OgreCamera::NameStatic());

        if ((placeable) && (camera))
        {    
            Scene::EntityPtr entity = activeScene_->CreateEntity(activeScene_->GetNextFreeId());
            
            entity->AddComponent(placeable);
            entity->AddComponent(camera);
            
            OgreRenderer::EC_OgreCamera* camera_ptr = checked_static_cast<OgreRenderer::EC_OgreCamera*>(camera.get());
            camera_ptr->SetPlaceable(placeable);
            camera_ptr->SetActive();
            camera_entity_ = entity;
            // Set camera controllable to use this camera entity. Note: it's a weak pointer so will not keep the camera alive needlessly
            camera_controllable_->SetCameraEntity(entity);
        }
    }    

    event_category_id_t scene_event_category = framework_->GetEventManager()->QueryEventCategory("Scene");
    if (scene_event_category == 0)
        LogError("Failed to query \"Scene\" event category");

    // Also create a default terrain to the Scene. This is done here dynamically instead of fixed in RexLogic,
    // since we might have 0-N terrains later on, depending on where we actually connect to. Now of course
    // we just create one default terrain.
    //CreateTerrain();

    // Also create a default sky to the scene.
    //CreateSky();

    // Create a water handler.
    //CreateWater();

    return GetCurrentActiveScene();
}

Scene::EntityPtr RexLogicModule::GetEntity(entity_id_t entityid)
{
    if (!activeScene_)
        return Scene::EntityPtr();

    return activeScene_->GetEntity(entityid);
}

Scene::EntityPtr RexLogicModule::GetEntityWithComponent(entity_id_t entityid, const std::string &requiredcomponent)
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

void RexLogicModule::RegisterFullId(const RexUUID &fullid, entity_id_t entityid)
{
    UUIDs_[fullid] = entityid;
}

void RexLogicModule::UnregisterFullId(const RexUUID &fullid)
{
    IDMap::iterator iter = UUIDs_.find(fullid);
    if (iter != UUIDs_.end())
        UUIDs_.erase(iter);
}

void RexLogicModule::UpdateObjects(f64 frametime)
{
    //! \todo probably should not be directly in RexLogicModule
    if (!activeScene_)
        return;

    // Damping interpolation factor, dependent on frame time
    Real factor = pow(2.0, -frametime * movement_damping_constant_);
    if (factor < 0.0) factor = 0.0;
    if (factor > 1.0) factor = 1.0;
    Real rev_factor = 1.0 - factor;

    found_avatars_.clear();
    
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
                    Quaternion rot_quat1;
                    Quaternion rot_quat2;
                    Quaternion rot_quat3;

                    rot_quat1.fromAngleAxis(netpos.rotvel_.x * 0.5 * frametime, Vector3df(1,0,0));
                    rot_quat2.fromAngleAxis(netpos.rotvel_.y * 0.5 * frametime, Vector3df(0,1,0));
                    rot_quat3.fromAngleAxis(netpos.rotvel_.z * 0.5 * frametime, Vector3df(0,0,1));

                    netpos.orientation_ *= rot_quat1;
                    netpos.orientation_ *= rot_quat2;
                    netpos.orientation_ *= rot_quat3;
                }

                // Dampened (smooth) movement
                if (netpos.damped_position_ != netpos.position_)
                    netpos.damped_position_ = netpos.position_ * rev_factor + netpos.damped_position_ * factor;

                if (netpos.damped_orientation_ != netpos.orientation_)
                    netpos.damped_orientation_.slerp(netpos.orientation_, netpos.damped_orientation_, factor);

                ogrepos.SetPosition(netpos.damped_position_);
                ogrepos.SetOrientation(netpos.damped_orientation_);
            }
        }

        // If is an avatar, handle update for avatar animations
        if (entity.GetComponent(EC_OpenSimAvatar::NameStatic()))
        {
            found_avatars_.push_back(*iter);
            avatar_->UpdateAvatarAnimations(entity.GetId(), frametime);
        }
           
        Foundation::ComponentPtr animctrl_ptr = entity.GetComponent(OgreRenderer::EC_OgreAnimationController::NameStatic());
        if (animctrl_ptr)
        {
            // General animation controller update
            OgreRenderer::EC_OgreAnimationController &animctrl = *checked_static_cast<OgreRenderer::EC_OgreAnimationController*>(animctrl_ptr.get());
            animctrl.Update(frametime);
        }
        
        // Attached sound update
        Foundation::ComponentPtr sound_ptr = entity.GetComponent(EC_AttachedSound::NameStatic());
        if (ogrepos_ptr && sound_ptr)
        {
            OgreRenderer::EC_OgrePlaceable &ogrepos = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(ogrepos_ptr.get());
            EC_AttachedSound &sound = *checked_static_cast<EC_AttachedSound*>(sound_ptr.get());
            sound.Update(frametime);
            sound.SetPosition(ogrepos.GetPosition());
        }
    }
}

void RexLogicModule::UpdateAvatarOverlays()
{
    // Ali: testing EC_HoveringText instead of EC_OgreMovableTextOverlay
    return;

    for (uint i = 0; i < found_avatars_.size(); ++i)
    {
        Scene::Entity* entity = found_avatars_[i].lock().get();
        if (entity)
        {
            OgreRenderer::EC_OgreMovableTextOverlay* overlay = entity->GetComponent<OgreRenderer::EC_OgreMovableTextOverlay>().get();
            if (overlay)
                overlay->Update();
        }
    }
}

void RexLogicModule::UpdateSoundListener()
{
    boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = 
        framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
    if (!soundsystem)
        return;

    // In freelook, use camera position. Otherwise use avatar position
    Vector3df listener_pos;
    if (camera_controllable_->GetState() == CameraControllable::FreeLook)
        listener_pos = GetCameraPosition();
    else
    {
        Scene::EntityPtr entity = avatar_->GetUserAvatar();
        if (!entity)
            return;
        OgreRenderer::EC_OgrePlaceable* placeable = entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
        if (!placeable)
            return;
        listener_pos = placeable->GetPosition();
    }

    soundsystem->SetListener(listener_pos, GetCameraOrientation());
}

void RexLogicModule::HandleObjectParent(entity_id_t entityid)
{
    Scene::EntityPtr entity = GetEntity(entityid);
    if (!entity)
        return;

    OgreRenderer::EC_OgrePlaceable *child_placeable = entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
    if (!child_placeable)
        return;

    // If object is a prim, parent id is in the prim component, and in presence component for an avatar
    EC_OpenSimPrim* prim = entity->GetComponent<EC_OpenSimPrim>().get();
    EC_OpenSimPresence* presence = entity->GetComponent<EC_OpenSimPresence>().get();

    entity_id_t parentid = 0;
    if (prim)
        parentid = prim->ParentId;
    if (presence)    
        parentid = presence->ParentId;
        
    if (parentid == 0)
    {
        // No parent, attach to scene root
        child_placeable->SetParent(Foundation::ComponentPtr());
        return;
    }
    
    Scene::EntityPtr parent_entity = GetEntity(parentid);
    if (!parent_entity)
    {
        // If can't get the parent entity yet, add to pending parent list
        pending_parents_[parentid].insert(entityid);
        return;
    }   
    
    Foundation::ComponentPtr parent_placeable = parent_entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
    child_placeable->SetParent(parent_placeable);
}

void RexLogicModule::HandleMissingParent(entity_id_t entityid)
{
    // Make sure we actually can get this now
    Scene::EntityPtr parent_entity = GetEntity(entityid);
    if (!parent_entity)
        return;
    
    // See if any accumulated objects missing the parent
    ObjectParentMap::iterator i = pending_parents_.find(entityid);
    if (i == pending_parents_.end())
        return;
   
    std::set<entity_id_t>::const_iterator j = i->second.begin();
    while (j != i->second.end())
    {
        HandleObjectParent(*j);
        ++j;
    }

    pending_parents_.erase(i);
}

void RexLogicModule::SetAllTextOverlaysVisible(bool visible)
{
    QList<EC_HoveringText *> overlays;
    Scene::ScenePtr word_scene = framework_->GetDefaultWorldScene();
    if (word_scene.get())
    {
        for (Scene::SceneManager::iterator iter = word_scene->begin(); iter != word_scene->end(); ++iter)
        {
            Scene::Entity &entity = **iter;
            EC_HoveringText *overlay = entity.GetComponent<EC_HoveringText>().get();
            if (overlay)
                overlays.append(overlay);
        }
    }

    // Set visibility for all found text overlays
    foreach(EC_HoveringText* overlay, overlays)
    {
        if (visible)
            overlay->Show();
        else
            overlay->Hide();
    }
}

void RexLogicModule::AboutToDeleteWorld()
{
    // Lets take some screenshots before deleting the scene
    boost::shared_ptr<UiServices::UiModule> ui_module =
        framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();

    if (!avatar_ && !ui_module)
        return;

    Scene::EntityPtr avatar_entity = avatar_->GetUserAvatar();
    if (!avatar_entity)
        return;

    OgreRenderer::EC_OgrePlaceable *ec_placeable = avatar_entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
    OgreRenderer::EC_OgreMesh *ec_mesh = avatar_entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();
    EC_AvatarAppearance *ec_appearance = avatar_entity->GetComponent<EC_AvatarAppearance>().get();
    if (!ec_placeable || !ec_mesh || !ec_appearance)
        return;

    if (ec_placeable && ec_mesh && ec_appearance)
    {
        // Head bone pos setup
        Vector3Df avatar_position = ec_placeable->GetPosition();
        Quaternion avatar_orientation = ec_placeable->GetOrientation();
        Ogre::SkeletonInstance* skel = ec_mesh->GetEntity()->getSkeleton();
        std::string view_bone_name;
        Real adjustheight = ec_mesh->GetAdjustPosition().z;
        Vector3df avatar_head_position;

        if (ec_appearance->HasProperty("headbone"))
        {
            view_bone_name = ec_appearance->GetProperty("headbone");
            adjustheight += 0.15;
            if (!view_bone_name.empty())
            {
                if (skel && skel->hasBone(view_bone_name))
                {
                    Ogre::Bone* bone = skel->getBone(view_bone_name);
                    Ogre::Vector3 headpos = bone->_getDerivedPosition();
                    Vector3df ourheadpos(-headpos.z + 0.5f, -headpos.x, headpos.y + adjustheight);
                    avatar_head_position = avatar_position + (avatar_orientation * ourheadpos);
                }
            }
        }
        else
        {
            // Fallback: will get screwed up shot but not finding the headbone should not happen, ever
            avatar_head_position = ec_placeable->GetPosition();
        }

        // Get paths where to store the screenshots
        QPair<QString, QString> paths = ui_module->GetScreenshotPaths();

        // Pass all variables to renderer for screenshot
        if (!paths.first.isEmpty() && !paths.second.isEmpty())
        {
            SetAllTextOverlaysVisible(false);
            GetRendererPtr()->CaptureWorldAndAvatarToFile(
                avatar_head_position, avatar_orientation, paths.first.toStdString(), paths.second.toStdString());
        }
    }
}

void RexLogicModule::UpdateAvatarNameTags(Scene::EntityPtr users_avatar)
{
    QList<Scene::EntityPtr> all_avatars;

    Scene::ScenePtr current_scene = framework_->GetDefaultWorldScene();
    if (!current_scene.get() || !users_avatar.get())
        return;

    // Iterate all avatars to a list
    Scene::SceneManager::iterator iter = current_scene->begin();
    Scene::SceneManager::iterator end = current_scene->end();
    while (iter != end)
    {
        Scene::EntityPtr entity = (*iter);
        ++iter;
        if (!entity.get() || entity.get() == users_avatar.get())
            continue;
        if (entity->GetComponent<EC_OpenSimPresence>().get())
            all_avatars.append(entity);
    }

    // Get users position
    boost::shared_ptr<EC_HoveringText> name_tag;
    boost::shared_ptr<OgreRenderer::EC_OgrePlaceable> placable = users_avatar->GetComponent<OgreRenderer::EC_OgrePlaceable>();
    if (!placable.get())
        return;

    Vector3Df users_position = placable->GetPosition();
    foreach (Scene::EntityPtr avatar, all_avatars)
    {
        placable = avatar->GetComponent<OgreRenderer::EC_OgrePlaceable>();
        name_tag = avatar->GetComponent<EC_HoveringText>();
        if (!placable.get() || !name_tag.get())
            continue;

        // Check distance, update name tag visibility
        f32 distance = users_position.getDistanceFrom(placable->GetPosition());
        if (distance > 13.0)
        {
            if (name_tag->IsVisible())
                name_tag->AnimatedHide();
        }
        else if (!name_tag->IsVisible())
            name_tag->AnimatedShow();
    }
}

void RexLogicModule::EntityClicked(Scene::Entity* entity)
{
    boost::shared_ptr<EC_HoveringText> name_tag = entity->GetComponent<EC_HoveringText>();
    if (name_tag.get())
        name_tag->Clicked();
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

