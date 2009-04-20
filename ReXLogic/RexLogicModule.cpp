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

#include "EC_Viewable.h"
#include "EC_FreeData.h"
#include "EC_SpatialSound.h"
#include "EC_OpenSimPrim.h"
#include "EC_OpenSimAvatar.h"

#include "../InputModuleOIS/InputModuleOIS.h"
#include "../OgreRenderingModule/Renderer.h"

#include <OgreManualObject.h>
#include <OgreSceneManager.h>

namespace RexLogic
{
    RexLogicModule::RexLogicModule() : ModuleInterfaceImpl(type_static_)
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
        rexserver_connection_ = RexServerConnectionPtr(new RexServerConnection(framework_)); 
        network_handler_ = new NetworkEventHandler(framework_, this);
        network_state_handler_ = new NetworkStateEventHandler(framework_, this);
        input_handler_ = new InputEventHandler(framework_, this);
        input_handler_->SetState(avatar_controller_);
        scene_handler_ = new SceneEventHandler(framework_, this);

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
    }

    // virtual 
    void RexLogicModule::Uninitialize()
    {
        if (rexserver_connection_->IsConnected())
        {
            //! \todo tucofixme, at the moment don't wait for LogoutReply packet, just close connection.
            rexserver_connection_->RequestLogout();
            rexserver_connection_->CloseServerConnection(); 
        }
        
        rexserver_connection_.reset();
        avatar_controller_.reset();

        SAFE_DELETE (network_handler_);
        SAFE_DELETE (input_handler_);
		SAFE_DELETE (scene_handler_);
		SAFE_DELETE (network_state_handler_);
        
		LogInfo("Module " + Name() + " uninitialized.");
    }

    // virtual
    void RexLogicModule::Update(Core::f64 frametime)
    {
        GhostCameraFreelook(frametime);
    }

    void RexLogicModule::GhostCameraFreelook(Core::f64 frametime)
    {
        const float keybSensitivity = 20.f;
        float dt = (float)frametime * keybSensitivity;
        OgreRenderer::Renderer *renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
        Ogre::Camera *cam = renderer->GetCurrentCamera();
        Ogre::Vector3 pos = cam->getPosition();

        Ogre::Vector3 front = cam->getDirection();
        Ogre::Vector3 up = cam->getUp();
        Ogre::Vector3 right = cam->getRight();

        Input::InputModuleOIS *input = dynamic_cast<Input::InputModuleOIS*>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_Input).lock().get());

        if (input)
        {
            if (input->IsKeyDown(OIS::KC_W))
                pos += front * dt;
            if (input->IsKeyDown(OIS::KC_S))
                pos -= front * dt;
            if (input->IsKeyDown(OIS::KC_A))
                pos -= right * dt;
            if (input->IsKeyDown(OIS::KC_D))
                pos += right * dt;

            if (input->IsKeyDown(OIS::KC_Q))
                pos += up * dt;
            if (input->IsKeyDown(OIS::KC_Z))
                pos -= up * dt;

            cam->setAutoTracking(false);
            cam->setPosition(pos);

            const float mouseSensitivity = 1.f;
            const float lookAmount = (float)frametime * mouseSensitivity;
            if (input->IsKeyDown(OIS::KC_J))
                cam->yaw(Ogre::Radian(lookAmount));
            if (input->IsKeyDown(OIS::KC_L))
                cam->yaw(Ogre::Radian(-lookAmount));
            if (input->IsKeyDown(OIS::KC_K))
                cam->pitch(Ogre::Radian(-lookAmount));
            if (input->IsKeyDown(OIS::KC_I))
                cam->pitch(Ogre::Radian(lookAmount));
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
}

using namespace RexLogic;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(RexLogicModule)
POCO_END_MANIFEST


