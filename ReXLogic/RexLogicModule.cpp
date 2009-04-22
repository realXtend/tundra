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
#include "CameraController.h"
#include "SceneManager.h"

#include "EC_Viewable.h"
#include "EC_FreeData.h"
#include "EC_SpatialSound.h"
#include "EC_OpenSimPrim.h"
#include "EC_OpenSimAvatar.h"
#include "EC_Terrain.h"

#include "../InputModuleOIS/InputModuleOIS.h"

// Ogre -specific
#include "../OgreRenderingModule/Renderer.h"

#include <OgreManualObject.h>
#include <OgreSceneManager.h>

#include "Terrain.h"

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
            LogError("Unable to find event category for OpenSimNetworkIn");

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
        
        OgreRenderer::Renderer *renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
        Ogre::Camera *cam = renderer->GetCurrentCamera();
        cam->setPosition(-10, -10, -10);
        cam->lookAt(0,0,0);

        send_input_state_ = true;
    }

    void RexLogicModule::DeleteScene(const std::string &name)
    {
        Scene::SceneManager *scene_manager = dynamic_cast<Scene::SceneManager *>
            (framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager));
        
        if (!scene_manager)
            return;
        
        if (!scene_manager->HasScene(name))
        {
            LogWarning("Tried to delete scene, but it didn't exist!");
            return;
        }

        if (activeScene_ && activeScene_->Name() == name)
            activeScene_.reset(); ///\todo Check in SceneManager that scene names surely are unique. -jj.

        scene_manager->DeleteScene(name);
        assert(!scene_manager->HasScene(name));
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
        avatar_controller_.reset();
        camera_controller_.reset();

        SAFE_DELETE (network_handler_);
        SAFE_DELETE (input_handler_);
		SAFE_DELETE (scene_handler_);
		SAFE_DELETE (network_state_handler_);
        
		LogInfo("Module " + Name() + " uninitialized.");
    }

    // virtual
    void RexLogicModule::Update(Core::f64 frametime)
    {
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
            return (i->second)(event_id,data);
        else
            return false;
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

        // Create a single entity with a EC_Terrain component to the scene.
        Foundation::SceneManagerServiceInterface *sceneManager = 
            framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);

        Foundation::EntityPtr entity = activeScene_->CreateEntity(activeScene_->GetNextFreeId());
        entity->AddEntityComponent(GetFramework()->GetComponentManager()->CreateComponent("EC_Terrain"));

        terrain_->FindCurrentlyActiveTerrain();
    }

    TerrainPtr RexLogicModule::GetTerrainHandler()
    {
        return terrain_;
    }

    void RexLogicModule::SetCurrentActiveScene(Foundation::ScenePtr scene)
    {
        activeScene_ = scene;
    }

    Foundation::ScenePtr RexLogicModule::GetCurrentActiveScene()
    {
        return activeScene_;
    }

    Foundation::ScenePtr RexLogicModule::CreateNewActiveScene(const std::string &name)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = 
            framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);

        if (sceneManager->HasScene(name))
        {
            LogWarning("Tried to create new active scene, but it already existed!");
            Foundation::ScenePtr newActiveScene = sceneManager->GetScene(name);
            SetCurrentActiveScene(newActiveScene);
            return newActiveScene;
        }

        activeScene_ = sceneManager->CreateScene(name);

        // Also create a default terrain to the Scene. This is done here dynamically instead of fixed in RexLogic,
        // since we might have 0-N terrains later on, depending on where we actually connect to. Now of course
        // we just create one default terrain.
        CreateTerrain();

        return GetCurrentActiveScene();
    }

}

using namespace RexLogic;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(RexLogicModule)
POCO_END_MANIFEST


