/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   RexLogicModule.cpp
 *  @brief  The main client module of Naali.
 *
 *          @todo Longer description here.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "RexLogicModule.h"
#include "EventHandlers/NetworkEventHandler.h"
#include "EventHandlers/NetworkStateEventHandler.h"
#include "EventHandlers/InputEventHandler.h"
#include "EventHandlers/SceneEventHandler.h"
#include "EventHandlers/FrameworkEventHandler.h"
#include "EventHandlers/LoginHandler.h"
#include "EventHandlers/MainPanelHandler.h"
#include "EntityComponent/EC_FreeData.h"
#include "EntityComponent/EC_AttachedSound.h"
#include "EntityComponent/EC_OpenSimAvatar.h"
#include "EntityComponent/EC_NetworkPosition.h"
#include "EntityComponent/EC_Controllable.h"
#include "EntityComponent/EC_AvatarAppearance.h"
#include "EntityComponent/EC_HoveringWidget.h"
#include "Avatar/Avatar.h"
#include "Avatar/AvatarEditor.h"
#include "Avatar/AvatarControllable.h"
#include "Environment/Primitive.h"
#include "CameraControllable.h"
#include "Communications/InWorldChat/Provider.h"

#include "EventManager.h"
#include "ConfigurationManager.h"
#include "ModuleManager.h"
#include "ConsoleCommand.h"
#include "ConsoleCommandServiceInterface.h"
#include "ServiceManager.h"
#include "ComponentManager.h"
#include "EventDataInterface.h"
#include "TextureInterface.h"
#include "SoundServiceInterface.h"
#include "InputServiceInterface.h"
#include "SceneManager.h"
#include "WorldStream.h"
#include "UiModule.h"
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

// External EC's
#include "EC_Highlight.h"
#include "EC_HoveringText.h"
#include "EC_Clone.h"
#include "EC_Light.h"
#include "EC_OpenSimPresence.h"
#include "EC_OpenSimPrim.h"
#include "EC_Touchable.h"
#include "EC_3DCanvas.h"

#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreViewport.h>
#include <OgreEntity.h>
#include <OgreBillboard.h>
#include <OgreBillboardSet.h>

#include "MemoryLeakCheck.h"

namespace RexLogic
{

RexLogicModule::RexLogicModule() :
    ModuleInterfaceImpl(type_static_),
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
    DECLARE_MODULE_EC(EC_OpenSimAvatar);
    DECLARE_MODULE_EC(EC_NetworkPosition);
    DECLARE_MODULE_EC(EC_Controllable);
    DECLARE_MODULE_EC(EC_AvatarAppearance);
    DECLARE_MODULE_EC(EC_HoveringWidget);
    // External EC's
    DECLARE_MODULE_EC(EC_Highlight);
    DECLARE_MODULE_EC(EC_HoveringText);
    DECLARE_MODULE_EC(EC_Clone);
    DECLARE_MODULE_EC(EC_Light);
    DECLARE_MODULE_EC(EC_OpenSimPresence);
    DECLARE_MODULE_EC(EC_OpenSimPrim);
    DECLARE_MODULE_EC(EC_Touchable);
    DECLARE_MODULE_EC(EC_3DCanvas);
}

// virtual
void RexLogicModule::Initialize()
{
    PROFILE(RexLogicModule_Initialize);
    framework_->GetEventManager()->RegisterEventCategory("Action");

    avatar_ = AvatarPtr(new Avatar(this));
    avatar_editor_ = AvatarEditorPtr(new AvatarEditor(this));
    primitive_ = PrimitivePtr(new Primitive(this));
    world_stream_ = WorldStreamPtr(new ProtocolUtilities::WorldStream(framework_));
    network_handler_ = new NetworkEventHandler(this);
    network_state_handler_ = new NetworkStateEventHandler(this);
    input_handler_ = new InputEventHandler(this);
    scene_handler_ = new SceneEventHandler(this);
    framework_handler_ = new FrameworkEventHandler(world_stream_.get(), this);
    avatar_controllable_ = AvatarControllablePtr(new AvatarControllable(this));
    camera_controllable_ = CameraControllablePtr(new CameraControllable(framework_));
    main_panel_handler_ = new MainPanelHandler(this);
    in_world_chat_provider_ = InWorldChatProviderPtr(new InWorldChat::Provider(framework_));

    movement_damping_constant_ = framework_->GetDefaultConfig().DeclareSetting(
        "RexLogicModule", "movement_damping_constant", 10.0f);

    dead_reckoning_time_ = framework_->GetDefaultConfig().DeclareSetting(
        "RexLogicModule", "dead_reckoning_time", 2.0f);

    camera_state_ = static_cast<CameraState>(framework_->GetDefaultConfig().DeclareSetting(
        "RexLogicModule", "default_camera_state", static_cast<int>(CS_Follow)));

    // Register ourselves as world logic service.
    boost::shared_ptr<RexLogicModule> rexlogic = framework_->GetModuleManager()->GetModule<RexLogicModule>(Foundation::Module::MT_WorldLogic).lock();
    boost::weak_ptr<WorldLogicInterface> service = boost::dynamic_pointer_cast<WorldLogicInterface>(rexlogic);
    framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_WorldLogic, service);
}

// virtual
void RexLogicModule::PostInitialize()
{

    // Input events.
    event_category_id_t eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Input");

    event_handlers_[eventcategoryid].push_back(boost::bind(
        &AvatarControllable::HandleInputEvent, avatar_controllable_.get(), _1, _2));
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &CameraControllable::HandleInputEvent, camera_controllable_.get(), _1, _2));
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &InputEventHandler::HandleInputEvent, input_handler_, _1, _2));

    // Action events.
    eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Action");
    
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &AvatarControllable::HandleActionEvent, avatar_controllable_.get(), _1, _2));
    event_handlers_[eventcategoryid].push_back(
        boost::bind(&CameraControllable::HandleActionEvent, camera_controllable_.get(), _1, _2));

    // Scene events.
    eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Scene");

    event_handlers_[eventcategoryid].push_back(boost::bind(
        &SceneEventHandler::HandleSceneEvent, scene_handler_, _1, _2));
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &AvatarControllable::HandleSceneEvent, avatar_controllable_.get(), _1, _2));
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &CameraControllable::HandleSceneEvent, camera_controllable_.get(), _1, _2));

    // Resource events
    eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Resource");
    event_handlers_[eventcategoryid].push_back(
        boost::bind(&RexLogicModule::HandleResourceEvent, this, _1, _2));

    // Inventory events
    eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Inventory");
    event_handlers_[eventcategoryid].push_back(
        boost::bind(&RexLogicModule::HandleInventoryEvent, this, _1, _2));

    // Asset events
    eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Asset");
    event_handlers_[eventcategoryid].push_back(
        boost::bind(&RexLogicModule::HandleAssetEvent, this, _1, _2));
    
    // Framework events
    eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Framework");
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &FrameworkEventHandler::HandleFrameworkEvent, framework_handler_, _1, _2));

    // NetworkState events
    eventcategoryid = framework_->GetEventManager()->QueryEventCategory("NetworkState");
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &InWorldChat::Provider::HandleNetworkStateEvent, in_world_chat_provider_.get(), _1, _2));
    event_handlers_[eventcategoryid].push_back(boost::bind(
       &NetworkStateEventHandler::HandleNetworkStateEvent, network_state_handler_, _1, _2));
    LogInfo("System " + Name() + " subscribed to network events [NetworkState] and added to LogicEventHandlerMap");

    // NetworkIn events
    eventcategoryid = framework_->GetEventManager()->QueryEventCategory("NetworkIn");
    event_handlers_[eventcategoryid].push_back(boost::bind(
        &NetworkEventHandler::HandleOpenSimNetworkEvent, network_handler_, _1, _2));
    LogInfo("System " + Name() + " subscribed to network events [NetworkIn]");

    send_input_state_ = true;

    // Create login handlers, get login notifier from ether and pass
    // that into rexlogic login handlers for slots/signals setup
    os_login_handler_ = new OpenSimLoginHandler(framework_, this);
    taiga_login_handler_ = new TaigaLoginHandler(framework_, this);

    UiModulePtr ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
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

    // Listen to component changes to serialize them via RexFreeData
    primitive_->RegisterToComponentChangeSignals(activeScene_);

    // Create camera entity into the scene
    Foundation::Framework* fw = GetFramework();
    Foundation::ComponentPtr placeable = fw->GetComponentManager()->CreateComponent(OgreRenderer::EC_OgrePlaceable::TypeNameStatic());
    Foundation::ComponentPtr camera = fw->GetComponentManager()->CreateComponent(OgreRenderer::EC_OgreCamera::TypeNameStatic());
    if ((placeable) && (camera))
    {
        Scene::EntityPtr entity = activeScene_->CreateEntity(activeScene_->GetNextFreeId());
        entity->AddComponent(placeable);
        entity->AddComponent(camera);
        
        OgreRenderer::EC_OgreCamera* camera_ptr = checked_static_cast<OgreRenderer::EC_OgreCamera*>(camera.get());
        camera_ptr->SetPlaceable(placeable);
        camera_ptr->SetActive();
        camera_entity_ = entity;
        // Set camera controllable to use this camera entity.
        //Note: it's a weak pointer so will not keep the camera alive needlessly
        camera_controllable_->SetCameraEntity(entity);
    }

    event_category_id_t scene_event_category = framework_->GetEventManager()->QueryEventCategory("Scene");

    return GetCurrentActiveScene();
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

    boost::shared_ptr<RexLogicModule> rexlogic = framework_->GetModuleManager()->GetModule<RexLogicModule>(Foundation::Module::MT_WorldLogic).lock();
    boost::weak_ptr<WorldLogicInterface> service = boost::dynamic_pointer_cast<WorldLogicInterface>(rexlogic);
    framework_->GetServiceManager()->UnregisterService(service);
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

        // interpolate & animate objects
        UpdateObjects(frametime);

        // update avatar stuff (download requests etc.)
        avatar_->Update(frametime);

        // update primitive stuff (EC network sync etc.)
        primitive_->Update(frametime);

        // update sound listener position/orientation
        UpdateSoundListener();

        /// \todo Move this to OpenSimProtocolModule.
        if (!world_stream_->IsConnected() && world_stream_->GetConnectionState() == ProtocolUtilities::Connection::STATE_INIT_UDP)
            world_stream_->CreateUdpConnection();

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
            //UpdateAvatarOverlays();
            UpdateAvatarNameTags(avatar_->GetUserAvatar());
            input_handler_->Update(frametime);

            UpdateAvatarOverlays();

            UpdateAvatarNameTags(avatar_->GetUserAvatar());
            
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

Scene::EntityPtr RexLogicModule::GetEntityWithComponent(uint entity_id, const std::string &component) const
{
    if (!activeScene_)
        return Scene::EntityPtr();

    Scene::EntityPtr entity = activeScene_->GetEntity(entity_id);
    if (entity && entity->GetComponent(component))
        return entity;
    else
        return Scene::EntityPtr();
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

void RexLogicModule::SetCurrentActiveScene(Scene::ScenePtr scene)
{
    activeScene_ = scene;
}

Scene::ScenePtr RexLogicModule::GetCurrentActiveScene() const
{
    return activeScene_;
}

//XXX \todo add dll exports or fix by some other way (e.g. qobjects)
//wrappers for calling stuff elsewhere in logic module from outside (python api module)
void RexLogicModule::SetAvatarYaw(Real newyaw)
{
    avatar_controllable_->SetYaw(newyaw);
}

void RexLogicModule::SetAvatarRotation(const Quaternion &newrot)
{
    avatar_controllable_->SetRotation(newrot);
}

void RexLogicModule::SetCameraYawPitch(Real newyaw, Real newpitch)
{
    camera_controllable_->SetYawPitch(newyaw, newpitch);
}

entity_id_t RexLogicModule::GetUserAvatarId() const
{
    return GetAvatarHandler()->GetUserAvatar()->GetId();
}

Vector3df RexLogicModule::GetCameraUp() const
{
    if (camera_entity_.expired())
        return Vector3df();

    OgreRenderer::EC_OgrePlaceable *placeable = camera_entity_.lock()->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
    if (placeable)
        //! \todo check if Ogre or OpenSim axis convention should actually be used
        return placeable->GetOrientation() * Vector3df(0.0f,1.0f,0.0f);
    else
        return Vector3df();
}

Vector3df RexLogicModule::GetCameraRight() const
{
    if (camera_entity_.expired())
        return Vector3df();

    OgreRenderer::EC_OgrePlaceable *placeable = camera_entity_.lock()->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
    if (placeable)
        //! \todo check if Ogre or OpenSim axis convention should actually be used
        return placeable->GetOrientation() * Vector3df(1.0f,0.0f,0.0f);
    else
        return Vector3df();
}

void RexLogicModule::EntityHovered(Scene::Entity* entity)
{
    EC_HoveringWidget* widget = entity->GetComponent<EC_HoveringWidget>().get();
    if(widget)
        widget->HoveredOver();
}

Vector3df RexLogicModule::GetCameraPosition() const
{
    if (camera_entity_.expired())
        return Vector3df();

    OgreRenderer::EC_OgrePlaceable *placeable = camera_entity_.lock()->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
    if (placeable)
        return placeable->GetPosition();
    else
        return Vector3df();
}

Quaternion RexLogicModule::GetCameraOrientation() const
{
    if (camera_entity_.expired())
        return Quaternion::IDENTITY;

    OgreRenderer::EC_OgrePlaceable *placeable = camera_entity_.lock()->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
    if (placeable)
        return placeable->GetOrientation();
    else
        return Quaternion::IDENTITY;
}

Real RexLogicModule::GetCameraViewportWidth() const
{
    OgreRenderer::RendererPtr renderer = GetOgreRendererPtr();
    if (renderer.get())
        return renderer->GetViewport()->getActualWidth();
    else
        return 0;
}

Real RexLogicModule::GetCameraViewportHeight() const
{
    OgreRenderer::RendererPtr renderer = GetOgreRendererPtr();
    if (renderer.get())
        return renderer->GetViewport()->getActualHeight();
    else
        return 0;
}

Real RexLogicModule::GetCameraFOV() const
{
    if (camera_entity_.expired())
        return 0.0f;

    OgreRenderer::EC_OgreCamera* camera = camera_entity_.lock()->GetComponent<OgreRenderer::EC_OgreCamera>().get();
    if (camera)
        return camera->GetVerticalFov();
    else
        return 0.0f;
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
    activeScene_.reset();
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
    UUIDs_[fullid] = entityid;
}

void RexLogicModule::UnregisterFullId(const RexUUID &fullid)
{
    IDMap::iterator iter = UUIDs_.find(fullid);
    if (iter != UUIDs_.end())
        UUIDs_.erase(iter);
}

void RexLogicModule::HandleObjectParent(entity_id_t entityid)
{
    if (!activeScene_)
        return;

    Scene::EntityPtr entity = activeScene_->GetEntity(entityid);
    if (!entity)
        return;

    boost::shared_ptr<OgreRenderer::EC_OgrePlaceable> child_placeable = entity->GetComponent<OgreRenderer::EC_OgrePlaceable>();
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
        child_placeable->SetParent(Foundation::ComponentPtr());
        return;
    }

    Scene::EntityPtr parent_entity = activeScene_->GetEntity(parentid);
    if (!parent_entity)
    {
        // If can't get the parent entity yet, add to pending parent list
        pending_parents_[parentid].insert(entityid);
        return;
    }

    Foundation::ComponentPtr parent_placeable = parent_entity->GetComponent(OgreRenderer::EC_OgrePlaceable::TypeNameStatic());
    child_placeable->SetParent(parent_placeable);
}

void RexLogicModule::HandleMissingParent(entity_id_t entityid)
{
    if (!activeScene_)
        return;

    // Make sure we actually can get this now
    Scene::EntityPtr parent_entity = activeScene_->GetEntity(entityid);
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

void RexLogicModule::StartLoginOpensim(QString &qfirstAndLast, QString &qpassword, QString &qserverAddressWithPort)
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

void RexLogicModule::SetAllTextOverlaysVisible(bool visible)
{
    if (!activeScene_)
        return;

    Scene::EntityList entities = activeScene_->GetEntitiesWithComponent(EC_HoveringText::TypeNameStatic());
    foreach(Scene::EntityPtr entity, entities)
    {
        boost::shared_ptr<EC_HoveringText> overlay = entity->GetComponent<EC_HoveringText>();
        assert(overlay.get());
        if (visible)
            overlay->Show();
        else
            overlay->Hide();
    }
}


void RexLogicModule::UpdateObjects(f64 frametime)
{
    using namespace OgreRenderer;

    //! \todo probably should not be directly in RexLogicModule
    if (!activeScene_)
        return;

    // Damping interpolation factor, dependent on frame time
    Real factor = pow(2.0, -frametime * movement_damping_constant_);
    clamp(factor, 0.0f, 1.0f);
    Real rev_factor = 1.0 - factor;

    found_avatars_.clear();

    for(Scene::SceneManager::iterator iter = activeScene_->begin(); iter != activeScene_->end(); ++iter)
    {
        Scene::Entity &entity = **iter;

        boost::shared_ptr<EC_OgrePlaceable> ogrepos = entity.GetComponent<EC_OgrePlaceable>();
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
            found_avatars_.push_back(*iter);
            avatar_->UpdateAvatarAnimations(entity.GetId(), frametime);
        }

        // General animation controller update
        boost::shared_ptr<EC_OgreAnimationController> animctrl = entity.GetComponent<EC_OgreAnimationController>();
        if (animctrl)
            animctrl->Update(frametime);

        // Attached sound update
        boost::shared_ptr<EC_OgrePlaceable> placeable = entity.GetComponent<EC_OgrePlaceable>();
        boost::shared_ptr<EC_AttachedSound> sound = entity.GetComponent<EC_AttachedSound>();
        if (placeable && sound)
        {
            sound->Update(frametime);
            sound->SetPosition(placeable->GetPosition());
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
    {
        listener_pos = GetCameraPosition();
    }
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

void RexLogicModule::AboutToDeleteWorld()
{
    // Lets take some screenshots before deleting the scene
    UiModulePtr ui_module =
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
            GetOgreRendererPtr()->CaptureWorldAndAvatarToFile(
                avatar_head_position, avatar_orientation, paths.first.toStdString(), paths.second.toStdString());
        }
    }
}

void RexLogicModule::UpdateAvatarNameTags(Scene::EntityPtr users_avatar)
{
    Scene::ScenePtr current_scene = framework_->GetDefaultWorldScene();
    if (!current_scene.get() || !users_avatar.get())
        return;

    Scene::EntityList all_avatars = current_scene->GetEntitiesWithComponent("EC_OpenSimPresence");

    // Get users position
    boost::shared_ptr<EC_HoveringWidget> widget;
    boost::shared_ptr<OgreRenderer::EC_OgrePlaceable> placeable = users_avatar->GetComponent<OgreRenderer::EC_OgrePlaceable>();
    if (!placeable)
        return;

    Vector3Df camera_position = this->GetCameraPosition();
    foreach (Scene::EntityPtr avatar, all_avatars)
    {
        placeable = avatar->GetComponent<OgreRenderer::EC_OgrePlaceable>();
        widget = avatar->GetComponent<EC_HoveringWidget>();
        if (!placeable || !widget)
            continue;

        f32 distance = camera_position.getDistanceFrom(placeable->GetPosition());
        widget->SetCameraDistance(distance);
    }
}

void RexLogicModule::EntityClicked(Scene::Entity* entity)
{
    /*boost::shared_ptr<EC_HoveringText> name_tag = entity->GetComponent<EC_HoveringText>();
    if (name_tag.get())
        name_tag->Clicked();*/

    boost::shared_ptr<EC_HoveringWidget> info_icon = entity->GetComponent<EC_HoveringWidget>();
    if(info_icon.get())
        info_icon->EntityClicked();
}

InWorldChatProviderPtr RexLogicModule::GetInWorldChatProvider() const
{
    return in_world_chat_provider_;
}

bool RexLogicModule::CheckInfoIconIntersection(int x, int y, Foundation::RaycastResult *result)
{
    bool ret_val = false;
    QList<EC_HoveringWidget*> visible_widgets;

    Real scr_x = x/(Real)GetOgreRendererPtr()->GetWindowWidth();
    Real scr_y = y/(Real)GetOgreRendererPtr()->GetWindowHeight();

    //expand to range -1 -> 1
    scr_x = (scr_x*2)-1;
    scr_y = (scr_y*2)-1;

    //divert y because after view/projection transforms, y increses upwards
    scr_y = -scr_y;

    OgreRenderer::EC_OgreCamera * camera;

    Scene::ScenePtr current_scene = framework_->GetDefaultWorldScene();
    if (!current_scene.get())
        return ret_val;

    Scene::SceneManager::iterator iter = current_scene->begin();
    Scene::SceneManager::iterator end = current_scene->end();
    while (iter != end)
    {
        Scene::EntityPtr entity = (*iter);
        EC_HoveringWidget* widget = entity->GetComponent<EC_HoveringWidget>().get();
        OgreRenderer::EC_OgreCamera* c  = entity->GetComponent<OgreRenderer::EC_OgreCamera>().get();
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

}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace RexLogic;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(RexLogicModule)
POCO_END_MANIFEST

