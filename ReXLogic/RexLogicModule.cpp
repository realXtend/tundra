// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexLogicModule.h"
#include "ComponentManager.h"
#include "Poco/ClassLibrary.h"
#include "NetworkEventHandler.h"
#include "NetworkStateEventHandler.h"
#include "InputEventHandler.h"
#include "SceneEventHandler.h"
#include "FrameworkEventHandler.h"
#include "EventDataInterface.h"
#include "TextureInterface.h"
#include "SceneManager.h"
#include "RexLoginWindow.h"
#include "AvatarControllable.h"
#include "CameraControllable.h"

#include "EC_FreeData.h"
#include "EC_SpatialSound.h"
#include "EC_OpenSimPrim.h"
#include "EC_OpenSimPresence.h"
#include "EC_OpenSimAvatar.h"
#include "EC_Terrain.h"
#include "EC_Water.h"
#include "EC_NetworkPosition.h"
#include "EC_Controllable.h"
#include "EC_AvatarAppearance.h"
#include "InputEvents.h"

// Ogre -specific
#include "Renderer.h"
#include "OgreTextureResource.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMovableTextOverlay.h"
#include "EC_OgreAnimationController.h"

#include <OgreManualObject.h>
#include <OgreSceneManager.h>

#include "Terrain.h"
#include "Water.h"
#include "Avatar.h"
#include "Primitive.h"
#include "Sky.h"
#include "Environment.h"

#include "QtUtils.h"
#include "AssetUploader.h"
#include "InventoryModel.h"
#include "InventoryWindow.h"
#include "RexTypes.h"

namespace RexLogic
{
    RexLogicModule::RexLogicModule() : ModuleInterfaceImpl(type_static_),
        send_input_state_(false),
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

        AutoRegisterConsoleCommand(Console::CreateCommand("Upload",
            "Upload an asset. Usage: Upload(AssetType, Name, Description)",
            Console::Bind(this, &RexLogicModule::UploadAsset)));

        AutoRegisterConsoleCommand(Console::CreateCommand("MultiUpload",
            "Upload multiple assets.",
            Console::Bind(this, &RexLogicModule::UploadMultipleAssets)));

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
        primitive_ = PrimitivePtr(new Primitive(this));
        rexserver_connection_ = RexServerConnectionPtr(new RexServerConnection(framework_));
        network_handler_ = new NetworkEventHandler(framework_, this);
        network_state_handler_ = new NetworkStateEventHandler(framework_, this);
        input_handler_ = new InputEventHandler(framework_, this);
        scene_handler_ = new SceneEventHandler(framework_, this);
        framework_handler_ = new FrameworkEventHandler(rexserver_connection_.get());
        avatar_controllable_ = AvatarControllablePtr(new AvatarControllable(this));
        camera_controllable_ = CameraControllablePtr(new CameraControllable(framework_));
        asset_uploader_ = AssetUploaderPtr(new AssetUploader());

        movement_damping_constant_ = framework_->GetDefaultConfig().DeclareSetting("RexLogicModule", "movement_damping_constant", 10.0f);

        dead_reckoning_time_ = framework_->GetDefaultConfig().DeclareSetting("RexLogicModule", "dead_reckoning_time", 2.0f);

        camera_state_ = static_cast<CameraState>(framework_->GetDefaultConfig().DeclareSetting("RexLogicModule", "default_camera_state", static_cast<int>(CS_Follow)));

        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual
    void RexLogicModule::PostInitialize()
    {
        // Get the event category id's.
        // NetworkState events.
        Core::event_category_id_t eventcategoryid = framework_->GetEventManager()->QueryEventCategory("NetworkState");
        if (eventcategoryid != 0)
            event_handlers_[eventcategoryid].push_back(boost::bind(&NetworkStateEventHandler::HandleNetworkStateEvent, network_state_handler_, _1, _2));
        else
            LogError("Unable to find event category for NetworkState");

        // OpenSimNetworkIn events.
        eventcategoryid = framework_->GetEventManager()->QueryEventCategory("OpenSimNetworkIn");
        if (eventcategoryid != 0)
            event_handlers_[eventcategoryid].push_back(boost::bind(&NetworkEventHandler::HandleOpenSimNetworkEvent, network_handler_, _1, _2));
        else
            LogError("Unable to find event category for OpenSimNetworkIn");

        // Input events.
        eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Input");
        if (eventcategoryid != 0)
        {
            event_handlers_[eventcategoryid].push_back(boost::bind(&AvatarControllable::HandleInputEvent, avatar_controllable_.get(), _1, _2));
            event_handlers_[eventcategoryid].push_back(boost::bind(&CameraControllable::HandleInputEvent, camera_controllable_.get(), _1, _2));
            event_handlers_[eventcategoryid].push_back(boost::bind(&InputEventHandler::HandleInputEvent, input_handler_, _1, _2));
        } else
            LogError("Unable to find event category for Input");

        // Action events.
        eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Action");
        if (eventcategoryid != 0)
        {
            event_handlers_[eventcategoryid].push_back(boost::bind(&AvatarControllable::HandleActionEvent, avatar_controllable_.get(), _1, _2));
            event_handlers_[eventcategoryid].push_back(boost::bind(&CameraControllable::HandleActionEvent, camera_controllable_.get(), _1, _2));
        } else
            LogError("Unable to find event category for Action");

        // Scene events.
        eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Scene");
        if (eventcategoryid != 0)
        {
            event_handlers_[eventcategoryid].push_back(boost::bind(&SceneEventHandler::HandleSceneEvent, scene_handler_, _1, _2));
            event_handlers_[eventcategoryid].push_back(boost::bind(&AvatarControllable::HandleSceneEvent, avatar_controllable_.get(), _1, _2));
            event_handlers_[eventcategoryid].push_back(boost::bind(&CameraControllable::HandleSceneEvent, camera_controllable_.get(), _1, _2));
        } else
            LogError("Unable to find event category for Scene");

        // Resource events
        eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Resource");
        if (eventcategoryid != 0)
            event_handlers_[eventcategoryid].push_back(boost::bind(&RexLogicModule::HandleResourceEvent, this, _1, _2));
        else
            LogError("Unable to find event category for Resource");

        // Framework events
        eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Framework");
        if (eventcategoryid != 0)
            event_handlers_[eventcategoryid].push_back(boost::bind(&FrameworkEventHandler::HandleFrameworkEvent, framework_handler_, _1, _2));
        else
            LogError("Unable to find event category for Framework");

        boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (renderer)
        {
            Ogre::Camera *cam = renderer->GetCurrentCamera();
            cam->setPosition(-10, -10, -10);
            cam->lookAt(0,0,0);
        }

        send_input_state_ = true;
        
        // Create the login window.
        loginWindow_ = new RexLoginWindow(framework_, this);
        connectionState_ = OpenSimProtocol::Connection::STATE_DISCONNECTED;

        ///\note This won't be staying here.
        inventoryWindow_ = new InventoryWindow(framework_, this);
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
        // Hackish fix for crash at exit because EC_OgreEnvironment remains
        DeleteScene("World");
        
        if (rexserver_connection_->IsConnected())
            LogoutAndDeleteWorld();
        
        rexserver_connection_.reset();
        avatar_.reset();
        primitive_.reset();
        avatar_controllable_.reset();
        camera_controllable_.reset();
        environment_.reset();
        asset_uploader_.reset();

        event_handlers_.clear();

        SAFE_DELETE (network_handler_);
        SAFE_DELETE (input_handler_);
        SAFE_DELETE (scene_handler_);
        SAFE_DELETE (network_state_handler_);
        SAFE_DELETE (framework_handler_);

        SAFE_DELETE(loginWindow_);
        ///\note This won't be staying here.
        SAFE_DELETE(inventoryWindow_);

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
            /// \todo Move this to the Login UI class.
            OpenSimProtocol::Connection::State cur_state = rexserver_connection_->GetConnectionState();
            if (cur_state != connectionState_)
            {
                loginWindow_->UpdateConnectionStateToUI(cur_state);
                connectionState_ = cur_state;
            }

            /// \todo Move this to OpenSimProtocolModule.
            if (!rexserver_connection_->IsConnected() &&
                rexserver_connection_->GetConnectionState() == OpenSimProtocol::Connection::STATE_INIT_UDP)
            {
                rexserver_connection_->CreateUDPConnection();
            }

            if (send_input_state_)
            {
                send_input_state_ = false;

                // can't send events during initalization, so workaround
                Core::event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
                if (camera_state_ == CS_Follow)
                    GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_THIRDPERSON, NULL);
                else
                    GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_FREECAMERA, NULL);
            }

            if (rexserver_connection_->IsConnected())
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
        // Pass the event to the avatar maanger
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

    void RexLogicModule::LogoutAndDeleteWorld()
    {
        rexserver_connection_->RequestLogout();
        rexserver_connection_->ForceServerDisconnect(); // Because the current server doesn't send a logoutreplypacket.

        if (framework_->HasScene("World"))
            DeleteScene("World");
    }

    void RexLogicModule::ShowInventory()
    {
        inventoryWindow_->Show();
    }

    void RexLogicModule::HideInventory()
    {
        inventoryWindow_->Hide();
    }

    //XXX temporary workarounds for a linking prob in pymodule, would like to call these directly (if this is the right idea for av / view control to begin with)
    void RexLogicModule::SetAvatarYaw(Core::Real newyaw)
    {
        avatar_controllable_->SetYaw(newyaw);
    }

    //XXX another temporary workarounds for a linking prob in pymodule
    void RexLogicModule::SetAvatarRotation(Core::Quaternion newrot)
    {
        std::cout << "RexLogicModule::SetAvatarRotation" << std::endl;
        avatar_controllable_->SetRotation(newrot);
    }

    void RexLogicModule::SetCameraYawPitch(Core::Real newyaw, Core::Real newpitch)
    {
        camera_controllable_->SetYawPitch(newyaw, newpitch);
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

        bool success = rexserver_connection_->ConnectToServer(name, passwd, server);

        // overwrite the password so it won't stay in-memory
        passwd.replace(0, passwd.size(), passwd.size(), ' ');

        if (success)
            return Console::ResultSuccess();
        else
            return Console::ResultFailure("Failed to connect to server.");
    }

    Console::CommandResult RexLogicModule::ConsoleLogout(const Core::StringVector &params)
    {
        if (rexserver_connection_->IsConnected())
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
        GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::TOGGLE_FLYMODE, NULL);
        return Console::ResultSuccess();
    }

    Console::CommandResult RexLogicModule::UploadAsset(const Core::StringVector &params)
    {
        using namespace RexTypes;
        using namespace OpenSimProtocol;

        std::string name = "(No Name)";
        std::string description = "(No Description)";
        
        if (params.size() < 1)
            return Console::ResultFailure("Invalid syntax. Usage: \"upload [asset_type] [name] [description]."
                "Name and description are optional. Supported asset types:\n"
                "Texture\nMesh\nSkeleton\nMaterialScript\nParticleScript\nFlashAnimation");

        asset_type_t asset_type = GetAssetTypeFromTypeName(params[0]);

        if (asset_type == -1)
            return Console::ResultFailure("Invalid asset type. Supported parameters:\n"
                "Texture\nMesh\nSkeleton\nMaterialScript\nParticleScript\nFlashAnimation");

        if (params.size() > 1)
            name = params[1];

        if (params.size() > 2)
            description = params[2];

        std::string filter = GetOpenFileNameFilter(asset_type);
        std::string filename = Foundation::QtUtils::GetOpenFileName(filter, "Open", Foundation::QtUtils::GetCurrentPath());
        if (filename == "")
            return Console::ResultFailure("No file chosen.");

        if (!asset_uploader_->HasUploadCapability())
        {
            std::string upload_url = rexserver_connection_->GetCapability("NewFileAgentInventory");
            if (upload_url == "")
                return Console::ResultFailure("Could not set upload capability for asset uploader.");

            asset_uploader_->SetUploadCapability(upload_url);
        }

        InventoryPtr inventory = GetInventory();

        // Get the category name for this asset type.
        std::string cat_name = GetCategoryNameForAssetType(asset_type);
        RexUUID folder_id;

        // Check out if this inventory category exists.
        InventoryFolder *folder = inventory->GetFirstChildFolderByName(cat_name.c_str());
        if (!folder)
        {
            // I doesn't. Create new inventory folder.
            InventoryFolder *parent = inventory->GetMyInventoryFolder();
            folder_id.Random();
            InventoryFolder *newFolder = inventory->GetOrCreateNewFolder(folder_id, *inventory->GetRoot());
            newFolder->SetName(cat_name);

            // Notify the server about the new inventory folder.
            rexserver_connection_->SendCreateInventoryFolderPacket(parent->GetID(), folder_id, asset_type, cat_name);
        }
        else
            folder_id = folder->GetID();

        // Upload.
        Core::Thread thread(boost::bind(&AssetUploader::UploadFile, asset_uploader_,
            asset_type, filename, name, description, folder_id));

        return Console::ResultSuccess();
    }

    Console::CommandResult RexLogicModule::UploadMultipleAssets(const Core::StringVector &params)
    {
        CreateRexInventoryFolders();

        Core::StringList filenames = Foundation::QtUtils::GetOpenRexFileNames(Foundation::QtUtils::GetCurrentPath());
        if (filenames.empty())
            return Console::ResultFailure("No files chosen.");

        if (!asset_uploader_->HasUploadCapability())
        {
            std::string upload_url = rexserver_connection_->GetCapability("NewFileAgentInventory");
            if (upload_url == "")
                return Console::ResultFailure("Could not set upload capability for asset uploader.");

            asset_uploader_->SetUploadCapability(upload_url);
        }

        // Multiupload.
        Core::Thread thread(boost::bind(&AssetUploader::UploadFiles, asset_uploader_, filenames, GetInventory().get()));

        return Console::ResultSuccess();
    }

    void RexLogicModule::CreateRexInventoryFolders()
    {
        if (!GetInventory().get())
        {
            LogError("Inventory doens't exist yet! Can't create folder to it.");
            return;
        }

        using namespace RexTypes;
        using namespace OpenSimProtocol;

        const char *asset_types[] = { "Texture", "Mesh", "Skeleton", "MaterialScript", "ParticleScript", "FlashAnimation" };
        asset_type_t asset_type;
        for(int i = 0; i < NUMELEMS(asset_types); ++i)
        {
            asset_type = GetAssetTypeFromTypeName(asset_types[i]);
            std::string cat_name = GetCategoryNameForAssetType(asset_type);
            RexUUID folder_id;

            // Check out if this inventory category exists.
            InventoryPtr inventory = GetInventory();
            InventoryFolder *folder = inventory->GetFirstChildFolderByName(cat_name.c_str());
            if (!folder)
            {
                // I doesn't. Create new inventory folder.
                InventoryFolder *parent = inventory->GetMyInventoryFolder();
                folder_id.Random();
                InventoryFolder *newFolder = inventory->GetOrCreateNewFolder(folder_id, *parent);
                newFolder->SetName(cat_name);
                
                // Notify the server about the new inventory folder.
                rexserver_connection_->SendCreateInventoryFolderPacket(parent->GetID(), folder_id, asset_type, cat_name);
            }
            else
                folder_id = folder->GetID();
        }
    }

    void RexLogicModule::SwitchCameraState()
    {
        if (camera_state_ == CS_Follow)
        {
            camera_state_ = CS_Free;

            Core::event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
            GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_FREECAMERA, NULL);
        }
        else
        {
            camera_state_ = CS_Follow;

            Core::event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
            GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_THIRDPERSON, NULL);
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
        //sky_->CreateDefaultSky();
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

    WaterPtr RexLogicModule::GetWaterHandler()
    {
        return water_;
    }

    AvatarPtr RexLogicModule::GetAvatarHandler()
    {
        return avatar_;
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
        return rexserver_connection_->GetInfo().inventory;
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

        // Also create a default terrain to the Scene. This is done here dynamically instead of fixed in RexLogic,
        // since we might have 0-N terrains later on, depending on where we actually connect to. Now of course
        // we just create one default terrain.
        CreateTerrain();
        
        // Also create a default sky to the scene.
        CreateSky();
        
        // Create a water handler.
        CreateWater();
        
        CreateEnvironment();

        return GetCurrentActiveScene();
    }

    Scene::EntityPtr RexLogicModule::GetEntityWithComponent(Core::entity_id_t entityid, const std::string &requiredcomponent)
    {
        if (!activeScene_)
            return Scene::EntityPtr();

        Scene::EntityPtr entity = activeScene_->GetEntity(entityid);
        if(entity && entity->GetComponent(requiredcomponent))
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

