// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexLogicModule.h"
#include "ComponentManager.h"
#include "Poco/ClassLibrary.h"
#include "NetworkEventHandler.h"
#include "NetworkStateEventHandler.h"
#include "InputEventHandler.h"
#include "SceneEventHandler.h"
#include "EventDataInterface.h"
#include "TextureInterface.h"
#include "CameraController.h"
#include "SceneManager.h"
#include "RexLoginWindow.h"

#include "EC_Viewable.h"
#include "EC_FreeData.h"
#include "EC_SpatialSound.h"
#include "EC_OpenSimPrim.h"
#include "EC_OpenSimAvatar.h"
#include "EC_Terrain.h"
#include "EC_Water.h"
#include "EC_OgreSky.h"
#include "EC_NetworkPosition.h"
#include "InputEvents.h"

// Ogre -specific
#include "../OgreRenderingModule/Renderer.h"
#include "../OgreRenderingModule/OgreTextureResource.h"

#include <OgreManualObject.h>
#include <OgreSceneManager.h>

#include "Terrain.h"
#include "Water.h"
#include "Avatar.h"
#include "Primitive.h"
#include "Sky.h"

namespace RexLogic
{
    RexLogicModule::RexLogicModule() : ModuleInterfaceImpl(type_static_), current_controller_(Controller_Avatar), send_input_state_(false)
    {
    }

    RexLogicModule::~RexLogicModule()
    {
    }

    // virtual
    void RexLogicModule::Load()
    {
        DECLARE_MODULE_EC(EC_Viewable);
        DECLARE_MODULE_EC(EC_FreeData);
        DECLARE_MODULE_EC(EC_SpatialSound);
        DECLARE_MODULE_EC(EC_OpenSimPrim);
        DECLARE_MODULE_EC(EC_OpenSimAvatar);
        DECLARE_MODULE_EC(EC_Terrain);
        DECLARE_MODULE_EC(EC_Water);
        DECLARE_MODULE_EC(EC_NetworkPosition);
        
        AutoRegisterConsoleCommand(Console::CreateCommand("Login", 
            "Login to server. Usage: Login(user=Test User, passwd=test, server=localhost",
            Console::Bind(this, &RexLogicModule::ConsoleLogin)));

        AutoRegisterConsoleCommand(Console::CreateCommand("Logout", 
            "Logout from server.",
            Console::Bind(this, &RexLogicModule::ConsoleLogout)));

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
        /// \todo fixme, register WorldLogic to the framework as realxtend worldlogicinterface!
        // WorldLogic::registerSystem(framework);
        // world_logic_ = new WorldLogic(framework);
        avatar_ = AvatarPtr(new Avatar(this));
        primitive_ = PrimitivePtr(new Primitive(this));
        avatar_controller_ = AvatarControllerPtr(new AvatarController(framework_, this));
        camera_controller_ = CameraControllerPtr(new CameraController(this));
        rexserver_connection_ = RexServerConnectionPtr(new RexServerConnection(framework_));
        network_handler_ = new NetworkEventHandler(framework_, this);
        network_state_handler_ = new NetworkStateEventHandler(framework_, this);
        input_handler_ = new InputEventHandler(framework_, this);
        scene_handler_ = new SceneEventHandler(framework_, this);

        current_controller_ = Controller_Avatar;
        input_handler_->SetState(avatar_controller_);
        
        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual
    void RexLogicModule::PostInitialize()
    {
        // Get the event category id's.
        // NetworkState events.
        Core::event_category_id_t eventcategoryid = framework_->GetEventManager()->QueryEventCategory("NetworkState");
        if (eventcategoryid != 0)
            event_handlers_[eventcategoryid] = boost::bind(&NetworkStateEventHandler::HandleNetworkStateEvent, network_state_handler_, _1, _2);
        else
            LogError("Unable to find event category for NetworkState");

        // OpenSimNetworkIn events.
        eventcategoryid = framework_->GetEventManager()->QueryEventCategory("OpenSimNetworkIn");
        if (eventcategoryid != 0)
            event_handlers_[eventcategoryid] = boost::bind(&NetworkEventHandler::HandleOpenSimNetworkEvent, network_handler_, _1, _2);
        else
            LogError("Unable to find event category for OpenSimNetworkIn");
    
        // Input events.
        eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Input");
        if (eventcategoryid != 0)
            event_handlers_[eventcategoryid] = boost::bind(&InputEventHandler::HandleInputEvent, input_handler_, _1, _2);
        else
            LogError("Unable to find event category for Input");           
        
        // Scene events.
        eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Scene");
        if (eventcategoryid != 0)
            event_handlers_[eventcategoryid] = boost::bind(&SceneEventHandler::HandleSceneEvent, scene_handler_, _1, _2);
        else
            LogError("Unable to find event category for Scene");

        // Resource events
        eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Resource");
        if (eventcategoryid != 0)
            event_handlers_[eventcategoryid] = boost::bind(&RexLogicModule::HandleResourceEvent, this, _1, _2);
        else
            LogError("Unable to find event category for Resource");        
                            
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
        if (rexserver_connection_->IsConnected())
        {
            //! \todo tucofixme, at the moment don't wait for LogoutReply packet, just close connection.
            rexserver_connection_->RequestLogout();
            rexserver_connection_->ForceServerDisconnect(); 
        }
        
        rexserver_connection_.reset();
        avatar_.reset();
        primitive_.reset();
        avatar_controller_.reset();
        camera_controller_.reset();

        SAFE_DELETE (network_handler_);
        SAFE_DELETE (input_handler_);
		SAFE_DELETE (scene_handler_);
		SAFE_DELETE (network_state_handler_);
		
		SAFE_DELETE(loginWindow_);
        
		LogInfo("Module " + Name() + " uninitialized.");
    }
    
    // virtual
    void RexLogicModule::Update(Core::f64 frametime)
    {
        // Poll the connection state and update the info to the UI.
        OpenSimProtocol::Connection::State cur_state = rexserver_connection_->GetConnectionState();
        if (cur_state != connectionState_)
        {
            loginWindow_->UpdateConnectionStateToUI(cur_state);
            connectionState_ = cur_state;
        }
        
        if (!rexserver_connection_->IsConnected() &&
            rexserver_connection_->GetConnectionState() == OpenSimProtocol::Connection::STATE_INIT_UDP)
        {
            rexserver_connection_->CreateUDPConnection();
        }
        
        input_handler_->Update(frametime);

        if (send_input_state_)
        {
            send_input_state_ = false;

            // can't send events during initalization, so workaround
            Core::event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
            if (current_controller_ == Controller_Avatar)
                GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_THIRDPERSON, NULL);
            else
                GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_FREECAMERA, NULL);
        }
    }

    // virtual
    bool RexLogicModule::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        LogicEventHandlerMap::iterator i = event_handlers_.find(category_id);
        if (i != event_handlers_.end())
            return (i->second)(event_id, data);
        else
            return false;
    }
    
    bool RexLogicModule::HandleResourceEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        // Pass the event to the mesh manager.
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
//                sky_->OnTextureReadyEvent(res);
            }
        }
        
        return false;
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
            const_cast<std::string&>(param_pass).erase();
        } if (params.size() > 2)
            server = params[2];

        bool success = rexserver_connection_->ConnectToServer(name,
		        passwd, server);

        // overwrite the password so it won't stay in-memory
        //!\ todo does this overwrite the string properly? Probably not.
        passwd.erase();
        

        if (success)
            return Console::ResultSuccess();
        else
            return Console::ResultFailure("Failed to connect to server.");
    }

    Console::CommandResult RexLogicModule::ConsoleLogout(const Core::StringVector &params)
    {
        if (rexserver_connection_->IsConnected())
        {
            rexserver_connection_->RequestLogout();
            return Console::ResultSuccess();
        } else
        {
            return Console::ResultFailure("Not connected to server.");
        }
    }
        

    void RexLogicModule::SwitchController()
    {
        if (current_controller_ == Controller_Avatar)
        {
            current_controller_ = Controller_Camera;
            input_handler_->SetState(camera_controller_);

            Core::event_category_id_t event_category = GetFramework()->GetEventManager()->QueryEventCategory("Input");
            GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_FREECAMERA, NULL);
        } else
        {
            current_controller_ = Controller_Avatar;
            input_handler_->SetState(avatar_controller_);

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
        sky_->CreateDefaultSky(true);
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
}

using namespace RexLogic;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(RexLogicModule)
POCO_END_MANIFEST
