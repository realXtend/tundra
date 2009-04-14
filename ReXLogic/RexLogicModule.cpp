// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexLogicModule.h"
#include "ComponentManager.h"
#include "Poco/ClassLibrary.h"
#include "NetworkEventHandler.h"
#include "InputEventHandler.h"
#include "SceneEventHandler.h"
#include "EventDataInterface.h"

#include "EC_Viewable.h"
#include "EC_FreeData.h"
#include "EC_SpatialSound.h"
#include "EC_OpenSimPrim.h"

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
        rexserver_connection_ = RexServerConnectionPtr(new RexServerConnection(framework_)); 
        network_handler_ = new NetworkEventHandler(framework_, this);
        input_handler_ = new InputEventHandler(framework_, this);  
        scene_handler_ = new SceneEventHandler(framework_, this); 
        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual
    void RexLogicModule::PostInitialize()
    {
        Foundation::SceneManagerServiceInterface *sceneManager = 
                framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        if (!sceneManager->HasScene("World"))
            sceneManager->CreateScene("World");

        Core::event_category_id_t eventcategoryid = framework_->GetEventManager()->QueryEventCategory("OpenSimNetworkIn");
        if (eventcategoryid != 0)
            event_handlers_[eventcategoryid] = boost::bind(&NetworkEventHandler::HandleOpenSimNetworkEvent, network_handler_, _1, _2);
        else
            LogError("Unable to find event category for OpenSimNetworkIn");
            
        eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Input");
        if (eventcategoryid != 0)
            event_handlers_[eventcategoryid] = boost::bind(&InputEventHandler::HandleInputEvent, input_handler_, _1, _2);
        else
            LogError("Unable to find event category for Input");           

        eventcategoryid = framework_->GetEventManager()->QueryEventCategory("Scene");
        if (eventcategoryid != 0)
            event_handlers_[eventcategoryid] = boost::bind(&SceneEventHandler::HandleSceneEvent, scene_handler_, _1, _2);
        else
            LogError("Unable to find event category for Scene");
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
        SAFE_DELETE (network_handler_);
        SAFE_DELETE (input_handler_);
		SAFE_DELETE (scene_handler_);
        
		LogInfo("Module " + Name() + " uninitialized.");
    }

    // virtual
    void RexLogicModule::Update(Core::f64 frametime)
    {

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


