/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   RexLogicModule.cpp
 *  @brief  The main client module of Naali.
 *
 *          RexLogicModule is the main client module of Naali and controls
 *          most of the world logic e.g. the default world scene creation and deletion,
 *          avatars, prims, and camera.
 *
 *  @note   Avoid direct module dependency to RexLogicModule at all costs because
 *          it's likely to cause cyclic dependy which fails the whole application.
 *          Instead use the WorldLogicInterface or different entity-components which
 *          are not within RexLogicModule. 
 *
 *          Currenly e.g. PythonScriptModule is highly dependant of RexLogicModule,
 *          but work is made to overcome this. If some feature you need from RexLogicModule
 *          is missing, please try to find a delicate abstraction/mechanism/design for retrieving it,
 *          instead of just hacking it to RexLogicModule directly.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "RexLogicModule.h"
#include "SceneEvents.h"
#include "EventHandlers/NetworkEventHandler.h"
#include "EventHandlers/NetworkStateEventHandler.h"
#include "EventHandlers/InputEventHandler.h"
#include "EventHandlers/SceneEventHandler.h"
#include "EventHandlers/FrameworkEventHandler.h"
#include "EventHandlers/AvatarEventHandler.h"
#include "EventHandlers/LoginHandler.h"
#include "EventHandlers/MainPanelHandler.h"
#include "EntityComponent/EC_AttachedSound.h"

//#ifdef EC_FreeData_ENABLED
#include "EntityComponent/EC_FreeData.h"
//#endif
#ifdef EC_OpenSimAvatar_ENABLED
#include "EntityComponent/EC_OpenSimAvatar.h"
#endif
#ifdef EC_Controllable_ENABLED
#include "EntityComponent/EC_Controllable.h"
#endif

#ifdef EC_NetworkPosition_ENABLED
#include "EC_NetworkPosition.h"
#endif
#ifdef EC_HoveringWidget_ENABLED
#include "EC_HoveringWidget.h"
#endif

#include "AvatarModule.h"
#include "Avatar/AvatarHandler.h"
#include "Avatar/AvatarControllable.h"
#include "AvatarEditing/AvatarEditor.h"

#ifdef EC_AvatarAppearance_ENABLED
#include "EntityComponent/EC_AvatarAppearance.h"
#endif

#include "RexMovementInput.h"
#include "Environment/Primitive.h"
#include "Camera/CameraControllable.h"
#include "Communications/InWorldChat/Provider.h"
#include "SceneInteract.h"

#include "Camera/ObjectCameraController.h"
#include "Camera/CameraControl.h"
#include "NotificationWidget.h"

#include "EventManager.h"
#include "ConfigurationManager.h"
#include "ModuleManager.h"
#include "ConsoleCommand.h"
#include "ConsoleCommandServiceInterface.h"
#include "ServiceManager.h"
#include "ComponentManager.h"
#include "IEventData.h"
#include "TextureInterface.h"
#include "ISoundService.h"
#include "Input.h"
#include "SceneManager.h"
#include "WorldStream.h"
#include "Renderer.h"
#include "RenderServiceInterface.h"
#include "OgreTextureResource.h"
#include "EC_OgreCamera.h"
#include "EC_Placeable.h"
#include "EC_OgreMovableTextOverlay.h"
#include "EC_AnimationController.h"
#include "EC_Mesh.h"
#include "EC_OgreMovableTextOverlay.h"
#include "EC_OgreCustomObject.h"

// External EC's
#ifdef EC_Highlight_ENABLED
#include "EC_Highlight.h"
#endif

#ifdef EC_HoveringText_ENABLED
#include "EC_HoveringText.h"
#endif
#ifdef EC_Clone_ENABLED
#include "EC_Clone.h"
#endif
#ifdef EC_Light_ENABLED
#include "EC_Light.h"
#endif
#ifdef EC_OpenSimPresence_ENABLED
#include "EC_OpenSimPresence.h"
#endif
#ifdef EC_OpenSimPrim_ENABLED
#include "EC_OpenSimPrim.h"
#endif
#ifdef EC_Touchable_ENABLED
#include "EC_Touchable.h"
#endif
#ifdef EC_3DCanvas_ENABLED
#include "EC_3DCanvas.h"
#endif
#ifdef EC_3DCanvasSource_ENABLED
#include "EC_3DCanvasSource.h"
#endif
#ifdef EC_ChatBubble_ENABLED
#include "EC_ChatBubble.h"
#endif
#ifdef EC_Ruler_ENABLED
#include "EC_Ruler.h"
#endif
#ifdef EC_SoundRuler_ENABLED
#include "EC_SoundRuler.h"
#endif
#include <EC_Name.h>
#ifdef EC_ParticleSystem_ENABLED
#include "EC_ParticleSystem.h"
#endif
#ifdef EC_SoundListener_ENABLED
#include "EC_SoundListener.h"
#endif
#ifdef EC_Sound_ENABLED
#include "EC_Sound.h"
#endif
#ifdef EC_InputMapper_ENABLED
#include "EC_InputMapper.h"
#endif
#ifdef EC_Movable_ENABLED
#include "EC_Movable.h"
#endif

#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreViewport.h>
#include <OgreEntity.h>
#include <OgreBillboard.h>
#include <OgreBillboardSet.h>

#include <boost/make_shared.hpp>

#include "NaaliApplication.h"

#include "MemoryLeakCheck.h"

namespace RexLogic
{

std::string RexLogicModule::type_name_static_ = "RexLogic";

RexLogicModule::RexLogicModule() :
    IModule(type_name_static_),
    movement_damping_constant_(10.0f),
    camera_state_(CS_Follow),
    network_handler_(0),
    input_handler_(0),
    scene_handler_(0),
    network_state_handler_(0),
    framework_handler_(0),
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

//#ifdef EC_FreeData_ENABLED
    DECLARE_MODULE_EC(EC_FreeData);
//#endif

    DECLARE_MODULE_EC(EC_AttachedSound);

    // External EC's
#ifdef EC_Highlight_ENABLED
    DECLARE_MODULE_EC(EC_Highlight);
#endif
#ifdef EC_HoveringText_ENABLED
    DECLARE_MODULE_EC(EC_HoveringText);
#endif
#ifdef EC_Clone_ENABLED
    DECLARE_MODULE_EC(EC_Clone);
#endif
#ifdef EC_Light_ENABLED
    DECLARE_MODULE_EC(EC_Light);
#endif
#ifdef EC_OpenSimPresence_ENABLED
    DECLARE_MODULE_EC(EC_OpenSimPresence);
#endif
#ifdef EC_OpenSimPrim_ENABLED
    DECLARE_MODULE_EC(EC_OpenSimPrim);
#endif
#ifdef EC_Touchable_ENABLED
    DECLARE_MODULE_EC(EC_Touchable);
#endif
#ifdef EC_3DCanvas_ENABLED
    DECLARE_MODULE_EC(EC_3DCanvas);
#endif
#ifdef EC_3DCanvasSource_ENABLED
    DECLARE_MODULE_EC(EC_3DCanvasSource);
#endif
#ifdef EC_Ruler_ENABLED
    DECLARE_MODULE_EC(EC_Ruler);
#endif
#ifdef EC_SoundRuler_ENABLED
    DECLARE_MODULE_EC(EC_SoundRuler);
#endif
//#ifdef EC_Name_ENABLED
    DECLARE_MODULE_EC(EC_Name);
//#endif
#ifdef EC_Movable_ENABLED
    DECLARE_MODULE_EC(EC_ParticleSystem);
#endif
#ifdef EC_SoundListener_ENABLED
    DECLARE_MODULE_EC(EC_SoundListener);
#endif
#ifdef EC_Sound_ENABLED
    DECLARE_MODULE_EC(EC_Sound);
#endif
#ifdef EC_InputMapper_ENABLED    
    DECLARE_MODULE_EC(EC_InputMapper);
#endif
#ifdef EC_Movable_ENABLED
    DECLARE_MODULE_EC(EC_Movable);
#endif
}

// virtual
void RexLogicModule::Initialize()
{
    PROFILE(RexLogicModule_Initialize);
    framework_->GetEventManager()->RegisterEventCategory("Action");

    primitive_ = PrimitivePtr(new Primitive(this));
    world_stream_ = WorldStreamPtr(new ProtocolUtilities::WorldStream(framework_));
    network_handler_ = new NetworkEventHandler(this);
    network_state_handler_ = new NetworkStateEventHandler(this);
    input_handler_ = new InputEventHandler(this);
    scene_handler_ = new SceneEventHandler(this);
    framework_handler_ = new FrameworkEventHandler(world_stream_.get(), this);
    avatar_event_handler_ = new AvatarEventHandler(this);
    camera_controllable_ = CameraControllablePtr(new CameraControllable(this));
    main_panel_handler_ = new MainPanelHandler(this);
    in_world_chat_provider_ = InWorldChatProviderPtr(new InWorldChat::Provider(framework_));
    obj_camera_controller_ = ObjectCameraControllerPtr(new ObjectCameraController(this, camera_controllable_.get()));
    camera_control_widget_ = CameraControlPtr(new CameraControl(this));
    notification_widget_ = NotificationWidgetPtr(new NotificationWidget(this));
    QObject::connect(obj_camera_controller_.get(), SIGNAL(FocusOnObject()), notification_widget_.get(), SLOT(FocusOnObject()));
    
    SceneInteract *sceneInteract = new SceneInteract(framework_);
    QObject::connect(sceneInteract, SIGNAL(EntityClicked(Scene::Entity*)), obj_camera_controller_.get(), SLOT(EntityClicked(Scene::Entity*)));
    framework_->RegisterDynamicObject("sceneinteract", sceneInteract);

    movement_damping_constant_ = framework_->GetDefaultConfig().DeclareSetting(
        "RexLogicModule", "movement_damping_constant", 10.0f);

    dead_reckoning_time_ = framework_->GetDefaultConfig().DeclareSetting(
        "RexLogicModule", "dead_reckoning_time", 2.0f);

    camera_state_ = static_cast<CameraState>(framework_->GetDefaultConfig().DeclareSetting(
        "RexLogicModule", "default_camera_state", static_cast<int>(CS_Follow)));

    // Register ourselves as world logic service.
    boost::shared_ptr<RexLogicModule> rexlogic = framework_->GetModuleManager()->GetModule<RexLogicModule>().lock();
    boost::weak_ptr<WorldLogicInterface> service = boost::dynamic_pointer_cast<WorldLogicInterface>(rexlogic);
    framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_WorldLogic, service);

    // Register login service.
    login_service_ = boost::shared_ptr<LoginHandler>(new LoginHandler(this));
    framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Login, login_service_);

    // For getting ether shots upon exit, desconstuctor LogoutAndDeleteWorld() call is too late
    connect(framework_->GetNaaliApplication(), SIGNAL(aboutToQuit()), this, SIGNAL(AboutToDeleteWorld()));
}

// virtual
void RexLogicModule::PostInitialize()
{
    Foundation::EventManagerPtr eventMgr = framework_->GetEventManager();
    eventMgr->RegisterEventSubscriber(this, 104);

    // Input events.
    event_category_id_t eventcategoryid = eventMgr->QueryEventCategory("Input");
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &CameraControllable::HandleInputEvent, camera_controllable_.get(), _1, _2));
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &InputEventHandler::HandleInputEvent, input_handler_, _1, _2));
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &ObjectCameraController::HandleInputEvent, obj_camera_controller_, _1, _2));
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &NotificationWidget::HandleInputEvent, notification_widget_, _1, _2));
    

    // Create the input handler that reacts to avatar-related input events and moves the avatar and default camera accordingly.
    avatarInput = boost::make_shared<RexMovementInput>(framework_);

    // Action events.
    eventcategoryid = eventMgr->QueryEventCategory("Action");
    event_handlers_[eventcategoryid].push_back(
        boost::bind(&CameraControllable::HandleActionEvent, camera_controllable_.get(), _1, _2));

    // Scene events.
    eventcategoryid = eventMgr->QueryEventCategory("Scene");
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &SceneEventHandler::HandleSceneEvent, scene_handler_, _1, _2));
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &CameraControllable::HandleSceneEvent, camera_controllable_.get(), _1, _2));
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &InWorldChat::Provider::HandleSceneEvent, in_world_chat_provider_.get(), _1, _2));
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &ObjectCameraController::HandleSceneEvent, obj_camera_controller_.get(), _1, _2));

    // Resource events
    eventcategoryid = eventMgr->QueryEventCategory("Resource");
    event_handlers_[eventcategoryid].push_back(
        boost::bind(&RexLogicModule::HandleResourceEvent, this, _1, _2));

    // Framework events
    eventcategoryid = eventMgr->QueryEventCategory("Framework");
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &FrameworkEventHandler::HandleFrameworkEvent, framework_handler_, _1, _2));
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &ObjectCameraController::HandleFrameworkEvent, obj_camera_controller_.get(), _1, _2));

    // Avatar events
    eventcategoryid = eventMgr->QueryEventCategory("Avatar");
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &AvatarEventHandler::HandleAvatarEvent, avatar_event_handler_, _1, _2));
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &ObjectCameraController::HandleAvatarEvent, obj_camera_controller_.get(), _1, _2));

    // NetworkState events
    eventcategoryid = eventMgr->QueryEventCategory("NetworkState");
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &InWorldChat::Provider::HandleNetworkStateEvent, in_world_chat_provider_.get(), _1, _2));
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &NetworkStateEventHandler::HandleNetworkStateEvent, network_state_handler_, _1, _2));
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &ObjectCameraController::HandleNetworkStateEvent, obj_camera_controller_.get(), _1, _2));

    // NetworkIn events
    eventcategoryid = eventMgr->QueryEventCategory("NetworkIn");
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &NetworkEventHandler::HandleOpenSimNetworkEvent, network_handler_, _1, _2));
    
    RegisterConsoleCommand(Console::CreateCommand("Login", 
        "Login to server. Usage: Login(user=Test User, passwd=test, server=localhost",
        Console::Bind(this, &RexLogicModule::ConsoleLogin)));

    RegisterConsoleCommand(Console::CreateCommand("Logout", 
        "Logout from server.",
        Console::Bind(this, &RexLogicModule::ConsoleLogout)));
        
    RegisterConsoleCommand(Console::CreateCommand("Fly",
        "Toggle flight mode.",
        Console::Bind(this, &RexLogicModule::ConsoleToggleFlyMode)));

#ifdef EC_Highlight_ENABLED
    RegisterConsoleCommand(Console::CreateCommand("Highlight",
        "Adds/removes EC_Highlight for every prim and mesh. Usage: highlight(add|remove)."
        "If add is called and EC already exists for entity, EC's visibility is toggled.",
        Console::Bind(this, &RexLogicModule::ConsoleHighlightTest)));
#endif

    obj_camera_controller_->PostInitialize();
}

Scene::ScenePtr RexLogicModule::CreateNewActiveScene(const QString &name)
{
    if (framework_->HasScene(name))
    {
        LogWarning("Tried to create new active scene, but it already existed!");
        Scene::ScenePtr scene = framework_->GetScene(name);
        framework_->SetDefaultWorldScene(scene);
        return scene;
    }

    Scene::ScenePtr scene = framework_->CreateScene(name);
    framework_->SetDefaultWorldScene(scene);

    // Connect ComponentAdded&Removed signals.
    connect(scene.get(), SIGNAL(ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type)),
            SLOT(NewComponentAdded(Scene::Entity*, IComponent*)));
    connect(scene.get(), SIGNAL(ComponentRemoved(Scene::Entity*, IComponent*, AttributeChange::Type)),
            SLOT(ComponentRemoved(Scene::Entity*, IComponent*)));

    // Listen to component changes to serialize them via RexFreeData
    primitive_->RegisterToComponentChangeSignals(scene);

    CreateOpenSimViewerCamera(scene);

    return scene;
}

void RexLogicModule::CreateOpenSimViewerCamera(Scene::ScenePtr scene)
{
    Scene::EntityPtr entity = scene->CreateEntity(scene->GetNextFreeId());
    if (!entity)
    {
        LogWarning("Could not create an entity to the scene!");
        return;
    }

    // Create camera entity into the scene
    Foundation::ComponentManagerPtr compMgr = framework_->GetComponentManager();
    ComponentPtr placeable = compMgr->CreateComponent(EC_Placeable::TypeNameStatic());
    ComponentPtr camera = compMgr->CreateComponent(EC_OgreCamera::TypeNameStatic());
    assert(placeable && camera);
    if (!placeable || !camera)
    {
        LogWarning("Could not create EC_Placeable of EC_OgreCamera!");
        return;
    }

    entity->AddComponent(placeable);
    entity->AddComponent(camera);

#ifdef EC_SoundListener_ENABLED
    ComponentPtr sound_listener = compMgr->CreateComponent(EC_SoundListener::TypeNameStatic());
    assert(sound_listener);
    if (!sound_listener)
    {
        LogWarning("Could not create EC_SoundListener!");
        return;
    }
    entity->AddComponent(sound_listener);
#endif

    scene->EmitEntityCreated(entity);
    
    EC_OgreCamera *camera_ptr = checked_static_cast<EC_OgreCamera*>(camera.get());
    camera_ptr->SetPlaceable(placeable);
    camera_ptr->SetActive();
    camera_entity_ = entity;
    // Set camera controllable to use this camera entity.
    //Note: it's a weak pointer so will not keep the camera alive needlessly
    camera_controllable_->SetCameraEntity(entity);
}

void RexLogicModule::DeleteScene(const QString &name)
{
    if (!framework_->HasScene(name))
    {
        LogWarning("Tried to delete scene, but it didn't exist!");
        return;
    }

    framework_->RemoveScene(name);
    assert(!framework_->HasScene(name));
}

// virtual
void RexLogicModule::Uninitialize()
{
    if (world_stream_->IsConnected())
        LogoutAndDeleteWorld();

    world_stream_.reset();
    primitive_.reset();
    camera_controllable_.reset();

    event_handlers_.clear();

    SAFE_DELETE(network_handler_);
    SAFE_DELETE(input_handler_);
    SAFE_DELETE(scene_handler_);
    SAFE_DELETE(network_state_handler_);
    SAFE_DELETE(framework_handler_);
    SAFE_DELETE(main_panel_handler_);
    SAFE_DELETE(avatar_event_handler_);

    // Unregister world logic service.
    boost::shared_ptr<RexLogicModule> rexlogic = framework_->GetModuleManager()->GetModule<RexLogicModule>().lock();
    boost::weak_ptr<WorldLogicInterface> service = boost::dynamic_pointer_cast<WorldLogicInterface>(rexlogic);
    framework_->GetServiceManager()->UnregisterService(service);

    // Unregister login service.
    framework_->GetServiceManager()->UnregisterService(login_service_);
    login_service_.reset();
}

#ifdef _DEBUG
void RexLogicModule::DebugSanityCheckOgreCameraTransform()
{
    OgreRenderer::RendererPtr renderer = GetOgreRendererPtr();
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

        /// \todo Move this to OpenSimProtocolModule.
        if (!world_stream_->IsConnected() && world_stream_->GetConnectionState() == ProtocolUtilities::Connection::STATE_INIT_UDP)
            world_stream_->CreateUdpConnection();

        // interpolate & animate objects
        UpdateObjects(frametime);

        // update primitive stuff (EC network sync etc.)
        primitive_->Update(frametime);

        // update sound listener position/orientation
        UpdateSoundListener();

        // workaround for not being able to send events during initialization
        static bool send_input_state = true;
        if (send_input_state)
        {
            send_input_state = false;
            event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
            if (camera_state_ == CS_Follow)
                GetFramework()->GetEventManager()->SendEvent(event_category, InputEvents::INPUTSTATE_THIRDPERSON, 0);
            else
                GetFramework()->GetEventManager()->SendEvent(event_category, InputEvents::INPUTSTATE_FREECAMERA, 0);
        }

        if (world_stream_->IsConnected())
        {
            camera_controllable_->AddTime(frametime);
            // Update overlays last, after camera update
            UpdateAvatarNameTags(GetAvatarHandler()->GetUserAvatar());
        }
    }

    RESETPROFILER;
}

// virtual
bool RexLogicModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
{
    // RexLogicModule does not directly handle any of its own events. Instead, there is a list of delegate objects
    // which handle events of each category. Pass the received event to the proper handler of the category of the event.
    PROFILE(RexLogicModule_HandleEvent);
    LogicEventHandlerMap::iterator i = event_handlers_.find(category_id);
    if (i != event_handlers_.end())
        for(size_t j=0 ; j<i->second.size() ; j++)
            if ((i->second[j])(event_id, data))
                return true;
    return false;
}

Scene::EntityPtr RexLogicModule::GetUserAvatarEntity() const
{
    if (!GetServerConnection()->IsConnected())
        return Scene::EntityPtr();

    return GetAvatarEntity(GetServerConnection()->GetInfo().agentID);
}

Scene::EntityPtr RexLogicModule::GetCameraEntity() const
{
    return camera_entity_.lock();
}

Scene::EntityPtr RexLogicModule::GetEntityWithComponent(uint entity_id, const QString &component) const
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return Scene::EntityPtr();

    Scene::EntityPtr entity = scene->GetEntity(entity_id);
    if (entity && entity->GetComponent(component))
        return entity;
    else
        return Scene::EntityPtr();
}

const QString &RexLogicModule::GetAvatarAppearanceProperty(const QString &name) const
{
    static const QString prop = GetUserAvatarEntity()->GetComponent<EC_AvatarAppearance>()->GetProperty(name.toStdString()).c_str();
    return prop;
}

float RexLogicModule::GetCameraControllablePitch() const
{
    if (camera_controllable_)
        return camera_controllable_->GetPitch();
    else
        return 0.0;
}
void RexLogicModule::SwitchCameraState()
{
    if (camera_state_ == CS_Follow)
    {
        camera_state_ = CS_Free;

        event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
        GetFramework()->GetEventManager()->SendEvent(event_category, InputEvents::INPUTSTATE_FREECAMERA, 0);
    }
    else
    {
        camera_state_ = CS_Follow;

        event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
        GetFramework()->GetEventManager()->SendEvent(event_category, InputEvents::INPUTSTATE_THIRDPERSON, 0);
    }
}

void RexLogicModule::CameraTripod()
{
    if (camera_state_ == CS_Follow)
    {
        camera_state_ = CS_Tripod;
        event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
        GetFramework()->GetEventManager()->SendEvent(event_category, InputEvents::INPUTSTATE_CAMERATRIPOD, 0);
    }
    else
    {
        camera_state_ = CS_Follow;
        event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
        GetFramework()->GetEventManager()->SendEvent(event_category, InputEvents::INPUTSTATE_THIRDPERSON, 0);
    }
}

Avatar::AvatarHandlerPtr RexLogicModule::GetAvatarHandler() const
{
    boost::shared_ptr<Avatar::AvatarModule> avatar_module = framework_->GetModuleManager()->GetModule<Avatar::AvatarModule>().lock();
    assert(avatar_module);
    if (avatar_module)
        return avatar_module->GetAvatarHandler();
    else
        return Avatar::AvatarHandlerPtr();
}

Avatar::AvatarEditorPtr RexLogicModule::GetAvatarEditor() const
{
    boost::shared_ptr<Avatar::AvatarModule> avatar_module = framework_->GetModuleManager()->GetModule<Avatar::AvatarModule>().lock();
    assert(avatar_module);
    if (avatar_module)
        return avatar_module->GetAvatarEditor();
    else
        return Avatar::AvatarEditorPtr();
}

Avatar::AvatarControllablePtr RexLogicModule::GetAvatarControllable() const
{
    boost::shared_ptr<Avatar::AvatarModule> avatar_module = framework_->GetModuleManager()->GetModule<Avatar::AvatarModule>().lock();
    assert(avatar_module);
    if (avatar_module)
        return avatar_module->GetAvatarControllable();
    else
        return Avatar::AvatarControllablePtr();
}

PrimitivePtr RexLogicModule::GetPrimitiveHandler() const
{
    return primitive_;
}

//XXX \todo add dll exports or fix by some other way (e.g. qobjects)
//wrappers for calling stuff elsewhere in logic module from outside (python api module)
void RexLogicModule::SetAvatarYaw(float newyaw)
{
    GetAvatarControllable()->SetYaw(newyaw);
}

void RexLogicModule::SetAvatarRotation(const Quaternion &newrot)
{
    GetAvatarControllable()->SetRotation(newrot);
}

void RexLogicModule::SetCameraYawPitch(float newyaw, float newpitch)
{
    camera_controllable_->SetYawPitch(newyaw, newpitch);
}

void RexLogicModule::LogoutAndDeleteWorld()
{
    emit AboutToDeleteWorld();

    world_stream_->RequestLogout();
    world_stream_->ForceServerDisconnect(); // Because the current server doesn't send a logoutreplypacket.

    if (GetAvatarHandler())
        GetAvatarHandler()->HandleLogout();
    if (primitive_)
        primitive_->HandleLogout();

    if (framework_->HasScene("World"))
        DeleteScene("World");

    pending_parents_.clear();
    UUIDs_.clear();
}

void RexLogicModule::SendRexPrimData(uint entityid)
{
    GetPrimitiveHandler()->SendRexPrimData((entity_id_t)entityid);
}

Scene::EntityPtr RexLogicModule::GetPrimEntity(const RexUUID &entityuuid) const
{
    IDMap::const_iterator iter = UUIDs_.find(entityuuid);
    if (iter == UUIDs_.end())
        return Scene::EntityPtr();
    else
        return GetPrimEntity(iter->second);
}

Scene::EntityPtr RexLogicModule::GetAvatarEntity(const RexUUID &entityuuid) const
{
    IDMap::const_iterator iter = UUIDs_.find(entityuuid);
    if (iter == UUIDs_.end())
        return Scene::EntityPtr();
    else
        return GetAvatarEntity(iter->second);
}

void RexLogicModule::RegisterFullId(const RexUUID &fullid, entity_id_t entityid)
{
    IDMap::iterator iter = UUIDs_.find(fullid);
    if (iter == UUIDs_.end())
    {
        UUIDs_[fullid] = entityid;
        avatar_event_handler_->SendRegisterEvent(fullid, entityid);
    }
}

void RexLogicModule::UnregisterFullId(const RexUUID &fullid)
{
    IDMap::iterator iter = UUIDs_.find(fullid);
    if (iter != UUIDs_.end())
    {
        UUIDs_.erase(iter);
        avatar_event_handler_->SendUnregisterEvent(fullid);
    }
}

void RexLogicModule::HandleObjectParent(entity_id_t entityid)
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;
        
    Scene::EntityPtr entity = scene->GetEntity(entityid);
    if (!entity)
        return;

    boost::shared_ptr<EC_Placeable> child_placeable = entity->GetComponent<EC_Placeable>();
    if (!child_placeable)
        return;

    // If object is a prim, parent id is in the prim component, and in presence component for an avatar
    boost::shared_ptr<EC_OpenSimPrim> prim = entity->GetComponent<EC_OpenSimPrim>();
    boost::shared_ptr<EC_OpenSimPresence> presence = entity->GetComponent<EC_OpenSimPresence>();

    entity_id_t parentid = 0;
    if (prim)
        parentid = prim->ParentId;
    if (presence)
        parentid = presence->parentId;

    if (parentid == 0)
    {
        // No parent, attach to scene root
        child_placeable->SetParent(ComponentPtr());
        return;
    }

    Scene::EntityPtr parent_entity = scene->GetEntity(parentid);
    if (!parent_entity)
    {
        // If can't get the parent entity yet, add to pending parent list
        pending_parents_[parentid].insert(entityid);
        return;
    }

    ComponentPtr parent_placeable = parent_entity->GetComponent(EC_Placeable::TypeNameStatic());
    child_placeable->SetParent(parent_placeable);
}

void RexLogicModule::HandleMissingParent(entity_id_t entityid)
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;

    // Make sure we actually can get this now
    Scene::EntityPtr parent_entity = scene->GetEntity(entityid);
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

void RexLogicModule::StartLoginOpensim(const QString &firstAndLast, const QString &password, const QString &serverAddressWithPort)
{
    QMap<QString, QString> map;
    map["AvatarType"] = "OpenSim";
    map["Username"] = firstAndLast;
    map["Password"] = password;
    map["WorldAddress"] = serverAddressWithPort;

    login_service_->ProcessLoginData(map);
}

void RexLogicModule::SetAllTextOverlaysVisible(bool visible)
{
#ifdef EC_HoveringText_ENABLED

    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;

    Scene::EntityList entities = scene->GetEntitiesWithComponent(EC_HoveringText::TypeNameStatic());
    foreach(Scene::EntityPtr entity, entities)
    {
        boost::shared_ptr<EC_HoveringText> overlay = entity->GetComponent<EC_HoveringText>();
        assert(overlay.get());
        if (visible)
            overlay->Show();
        else
            overlay->Hide();
    }
#endif
}

void RexLogicModule::UpdateObjects(f64 frametime)
{
    PROFILE(RexLogicModule_UpdateObjects);
    
    using namespace OgreRenderer;

    //! \todo probably should not be directly in RexLogicModule
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;

    // Damping interpolation factor, dependent on frame time
    float factor = pow(2.0, -frametime * movement_damping_constant_);
    clamp(factor, 0.0f, 1.0f);
    float rev_factor = 1.0 - factor;

    found_avatars_.clear();

    for(Scene::SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        Scene::Entity &entity = *iter->second;

        boost::shared_ptr<EC_Placeable> ogrepos = entity.GetComponent<EC_Placeable>();
        boost::shared_ptr<EC_NetworkPosition> netpos = entity.GetComponent<EC_NetworkPosition>();
        if (ogrepos && netpos)
        {
            if (netpos->time_since_update_ <= dead_reckoning_time_)
            {
                netpos->time_since_update_ += frametime; 

                // Interpolate motion
                // acceleration disabled until figured out what goes wrong. possibly mostly irrelevant with OpenSim server
                // netpos->velocity_ += netpos->accel_ * frametime;
                netpos->position_ += netpos->velocity_ * frametime;

                // Interpolate rotation
                if (netpos->rotvel_.getLengthSQ() > 0.001)
                {
                    Quaternion rot_quat1;
                    Quaternion rot_quat2;
                    Quaternion rot_quat3;

                    rot_quat1.fromAngleAxis(netpos->rotvel_.x * 0.5 * frametime, Vector3df(1,0,0));
                    rot_quat2.fromAngleAxis(netpos->rotvel_.y * 0.5 * frametime, Vector3df(0,1,0));
                    rot_quat3.fromAngleAxis(netpos->rotvel_.z * 0.5 * frametime, Vector3df(0,0,1));

                    netpos->orientation_ *= rot_quat1;
                    netpos->orientation_ *= rot_quat2;
                    netpos->orientation_ *= rot_quat3;
                }

                // Dampened (smooth) movement
                if (netpos->damped_position_ != netpos->position_)
                    netpos->damped_position_ = netpos->position_ * rev_factor + netpos->damped_position_ * factor;

                if (netpos->damped_orientation_ != netpos->orientation_)
                    netpos->damped_orientation_.slerp(netpos->orientation_, netpos->damped_orientation_, factor);

                ogrepos->SetPosition(netpos->damped_position_);
                ogrepos->SetOrientation(netpos->damped_orientation_);
            }
        }

        // If is an avatar, handle update for avatar animations
        if (entity.GetComponent(EC_OpenSimAvatar::TypeNameStatic()))
        {
            found_avatars_.push_back(iter->second);
            GetAvatarHandler()->UpdateAvatarAnimations(entity.GetId(), frametime);
        }

        // General animation controller update
        boost::shared_ptr<EC_AnimationController> animctrl = entity.GetComponent<EC_AnimationController>();
        if (animctrl)
            animctrl->Update(frametime);

        // Attached sound update
        boost::shared_ptr<EC_Placeable> placeable = entity.GetComponent<EC_Placeable>();
        boost::shared_ptr<EC_AttachedSound> sound = entity.GetComponent<EC_AttachedSound>();
        if (placeable && sound)
        {
            sound->Update(frametime);
            sound->SetPosition(placeable->GetPosition());
        }
    }
}

void RexLogicModule::UpdateSoundListener()
{
#ifdef EC_SoundListener_ENABLED
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;
    
    // In freelook, use camera position. Otherwise use avatar position
    if (camera_controllable_->GetState() == CameraControllable::FreeLook)
    {
        EC_SoundListener *listener = GetCameraEntity()->GetComponent<EC_SoundListener>().get();
        if (listener && !listener->active.Get())
            listener->active.Set(true, AttributeChange::Default);
    }
    else if (GetUserAvatarEntity())
    {
        EC_SoundListener *listener = GetUserAvatarEntity()->GetComponent<EC_SoundListener>().get();
        if (listener && !listener->active.Get())
            listener->active.Set(true, AttributeChange::Default);
    }
#endif
}

bool RexLogicModule::HandleResourceEvent(event_id_t event_id, IEventData* data)
{
    // Pass the event to the primitive manager
    primitive_->HandleResourceEvent(event_id, data);
    return false;
}

void RexLogicModule::UpdateAvatarNameTags(Scene::EntityPtr users_avatar)
{
    Scene::ScenePtr current_scene = framework_->GetDefaultWorldScene();
    if (!current_scene.get() || !users_avatar.get())
        return;

    Scene::EntityList all_avatars = current_scene->GetEntitiesWithComponent("EC_OpenSimPresence");

    // Get users position
    boost::shared_ptr<EC_Placeable> placeable = users_avatar->GetComponent<EC_Placeable>();
    if (!placeable)
        return;

    foreach (Scene::EntityPtr avatar, all_avatars)
    {
        // Update avatar name tag/hovering widget
        placeable = avatar->GetComponent<EC_Placeable>();
        if (!placeable)
            continue;

#ifdef EC_HoveringWidget_ENABLED
        boost::shared_ptr<EC_HoveringWidget> widget = avatar->GetComponent<EC_HoveringWidget>();
        if (!widget)
            continue;

        // We need to update the positions so that the distance is right, otherwise were always one frame behind.
        GetCameraEntity()->GetComponent<EC_Placeable>()->GetSceneNode()->_update(false, true);
        placeable->GetSceneNode()->_update(false, true);

        Vector3df camera_position = GetCameraEntity()->GetComponent<EC_Placeable>().get()->GetPosition();
        f32 distance = camera_position.getDistanceFrom(placeable->GetPosition());
        widget->SetCameraDistance(distance);
#endif

#ifdef EC_ChatBubble_ENABLED
        // Update chat bubble
        boost::shared_ptr<EC_ChatBubble> chat_bubble = avatar->GetComponent<EC_ChatBubble>();
        if (!chat_bubble)
            continue;
        if (chat_bubble->IsVisible())
        {
            chat_bubble->SetScale(distance/10);
        }

#ifdef EC_HoveringWidget_ENABLED
        if (chat_bubble->IsVisible())
            widget->Hide();
        else
            widget->Show();
#endif

#endif
    }
}

InWorldChatProviderPtr RexLogicModule::GetInWorldChatProvider() const
{
    return in_world_chat_provider_;
}

bool RexLogicModule::CheckInfoIconIntersection(int x, int y, Foundation::RaycastResult *result)
{
#ifdef EC_HoveringWidget_ENABLED
    bool ret_val = false;
    QList<EC_HoveringWidget*> visible_widgets;

    float scr_x = x/(float)GetOgreRendererPtr()->GetWindowWidth();
    float scr_y = y/(float)GetOgreRendererPtr()->GetWindowHeight();

    //expand to range -1 -> 1
    scr_x = (scr_x*2)-1;
    scr_y = (scr_y*2)-1;

    //divert y because after view/projection transforms, y increses upwards
    scr_y = -scr_y;

    EC_OgreCamera * camera = 0;

    Scene::ScenePtr current_scene = framework_->GetDefaultWorldScene();
    if (!current_scene.get())
        return ret_val;

    Scene::SceneManager::iterator iter = current_scene->begin();
    Scene::SceneManager::iterator end = current_scene->end();
    while (iter != end)
    {
        Scene::EntityPtr entity = iter->second;
        EC_HoveringWidget* widget = entity->GetComponent<EC_HoveringWidget>().get();
        EC_OgreCamera* c = entity->GetComponent<EC_OgreCamera>().get();
        if (c)
            camera = c;

        if (widget && widget->IsVisible())
            visible_widgets.append(widget);

        ++iter;
    }

    if (!camera)
        return false;

    Ogre::Vector3 nearest_world_pos(Ogre::Vector3::ZERO);
    QRectF nearest_rect;
    Ogre::Vector3 cam_pos = camera->GetCamera()->getDerivedPosition();
    EC_HoveringWidget* nearest_widget = 0;
    for(int i=0; i< visible_widgets.size();i++)
    {
        Ogre::Matrix4 worldmat;
        Ogre::Vector3 world_pos;
        EC_HoveringWidget* widget = visible_widgets.at(i);
        if(!widget)
            continue;
        Ogre::BillboardSet* bbset = widget->GetButtonsBillboardSet();
        Ogre::Billboard* board = widget->GetButtonsBillboard();
        if(!bbset || !board)
            continue;
        QSizeF scr_size = widget->GetButtonsBillboardScreenSpaceSize();
        Ogre::Vector3 pos = board->getPosition();

        bbset->getWorldTransforms(&worldmat);
        pos = worldmat*pos;
        world_pos = pos;
        pos = camera->GetCamera()->getViewMatrix()*pos;
        pos = camera->GetCamera()->getProjectionMatrix()*pos;

        QRectF rect(pos.x - scr_size.width()*0.5, pos.y - scr_size.height()*0.5, scr_size.width(), scr_size.height());
        if (rect.contains(scr_x, scr_y))
        {
            if (nearest_widget!=0)
                if ((world_pos-cam_pos).length() > (nearest_world_pos-cam_pos).length())
                    continue;

            nearest_world_pos = world_pos;
            nearest_rect = rect;
            nearest_widget = widget;
        }
    }

    //check if entity is between the board and mouse
    if (result->entity_)
    {
        Ogre::Vector3 ent_pos(result->pos_.x,result->pos_.y,result->pos_.z);
        //if true, the entity is closer to camera
        if (Ogre::Vector3(ent_pos-cam_pos).length()<Ogre::Vector3(nearest_world_pos-cam_pos).length())
        {
            EC_HoveringWidget* widget = result->entity_->GetComponent<EC_HoveringWidget>().get();
            if (widget)
                widget->EntityClicked();
            return ret_val;
        }
    }

    if (nearest_widget)
    {
        scr_x -=nearest_rect.left();
        scr_y -=nearest_rect.top();

        scr_x /= nearest_rect.width();
        scr_y /= nearest_rect.height();

        nearest_widget->WidgetClicked(scr_x, 1-scr_y);
        ret_val = true;
    }

    return ret_val;
#else
    return false;
#endif
}

OgreRenderer::RendererPtr RexLogicModule::GetOgreRendererPtr() const
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

    StartLoginOpensim(name.c_str(), passwd.c_str(), server.c_str());

    return Console::ResultSuccess();
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

Console::CommandResult RexLogicModule::ConsoleToggleFlyMode(const StringVector &params)
{
    event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
    GetFramework()->GetEventManager()->SendEvent(event_category, InputEvents::TOGGLE_FLYMODE, 0);
    return Console::ResultSuccess();
}

Console::CommandResult RexLogicModule::ConsoleHighlightTest(const StringVector &params)
{
#ifdef EC_Highlight_ENABLED
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return Console::ResultFailure("No active scene found.");

    if (params.size() != 1 || (params[0] != "add" && params[0] != "remove"))
        return Console::ResultFailure("Invalid syntax. Usage: highlight(add|remove).");

    for(Scene::SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        Scene::Entity &entity = *iter->second;
        EC_Mesh *ec_mesh = entity.GetComponent<EC_Mesh>().get();
        EC_OgreCustomObject *ec_custom = entity.GetComponent<EC_OgreCustomObject>().get();
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
#endif
    return Console::ResultSuccess();
}

void RexLogicModule::EmitIncomingEstateOwnerMessageEvent(QVariantList params)
{
    emit OnIncomingEstateOwnerMessage(params);
}

void RexLogicModule::NewComponentAdded(Scene::Entity *entity, IComponent *component)
{
#ifdef EC_SoundListener_ENABLED ///\todo Should find a way to remove this handling of EC_SoundListener here. -jj.
    if (component->TypeName() == EC_SoundListener::TypeNameStatic())
    {
        LogDebug("Added new sound listener to the listener list.");
//        EC_SoundListener *listener = entity->GetComponent<EC_SoundListener>().get();
//        connect(listener, SIGNAL(OnAttributeChanged(IAttribute *, AttributeChange::Type)), 
//            SLOT(ActiveListenerChanged());
        soundListeners_ << entity;
    }
#endif

#ifdef EC_Movable_ENABLED ///\todo When the Connection API is complete, remove this altogether. The EC_Movable can access the connection via that. -jj.
    else if (component->TypeName() == EC_Movable::TypeNameStatic())
    {
        entity->GetComponent<EC_Movable>()->SetWorldStreamPtr(GetServerConnection());
    }
#endif
}

void RexLogicModule::ComponentRemoved(Scene::Entity *entity, IComponent *component)
{
#ifdef EC_SoundListener_ENABLED ///\todo Should find a way to remove this handling of EC_SoundListener here. -jj.
    if (component->TypeName() == EC_SoundListener::TypeNameStatic())
    {
        LogDebug("Removed sound listener from the listener list.");
        soundListeners_.removeOne(entity);
    }
#endif
}

void RexLogicModule::FindActiveListener()
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;

    // Iterate throught possible listeners and find the active one.
/*
    foreach(Scene::Entity *listener, soundListeners_)
        if (listener->GetComponent<EC_SoundListener>()->IsActive())
        {
            activeSoundListener_ = scene->GetEntity(listener->GetId());
            break;
        }
*/
}

} // namespace RexLogic

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace RexLogic;

POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(RexLogicModule)
POCO_END_MANIFEST

