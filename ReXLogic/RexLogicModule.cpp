// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexLogicModule.h"
#include "ComponentManager.h"
#include "Poco/ClassLibrary.h"
#include "NetworkEventHandler.h"
#include "EventDataInterface.h"

#include "EC_ObjIdentity.h"
#include "EC_Collision.h"
#include "EC_ObjFreeData.h"
#include "EC_ObjGeneralProps.h"
#include "EC_SelectPriority.h"
#include "EC_ServerScript.h"
#include "EC_SpatialSound.h"

namespace RexLogic
{
    RexLogicModule::RexLogicModule() : ModuleInterface_Impl(type_static_), framework_(NULL)
    {
    }

    RexLogicModule::~RexLogicModule()
    {
        assert(framework_ == NULL);
    }

    // virtual
    void RexLogicModule::Load()
    {
        DECLARE_MODULE_EC(EC_ObjIdentity);
        DECLARE_MODULE_EC(EC_Collision);
        DECLARE_MODULE_EC(EC_ObjFreeData);
        DECLARE_MODULE_EC(EC_ObjGeneralProps);    
        DECLARE_MODULE_EC(EC_SelectPriority);
        DECLARE_MODULE_EC(EC_ServerScript);
        DECLARE_MODULE_EC(EC_SpatialSound);

        LogInfo("Module " + Name() + " loaded.");
    }

    // virtual
    void RexLogicModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void RexLogicModule::Initialize(Foundation::Framework *framework)
    {
        assert(framework_ == NULL);
        framework_ = framework;

        // fixme, register WorldLogic to the framework as realxtend worldlogicinterface!
        // WorldLogic::registerSystem(framework);
        // world_logic_ = new WorldLogic(framework);
        network_handler_ = new NetworkEventHandler(framework);

        // framework_->GetEventManager()->RegisterEventCategory("OpenSimNetwork"); // TODO tucofixme, this registering should be done in network module, it's here for testing purposes only.

        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual
    void RexLogicModule::PostInitialize(Foundation::Framework *framework)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = 
                framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        if (sceneManager->HasScene("World") == false)
            sceneManager->CreateScene("World");

        Core::event_category_id_t eventcategoryid = framework_->GetEventManager()->QueryEventCategory("OpenSimNetwork"); // TODO tucofixme, right event category
        if(eventcategoryid != 0)
            event_handlers_[eventcategoryid] = boost::bind(&NetworkEventHandler::HandleOpenSimNetworkEvent,network_handler_,_1,_2);
        else
            LogInfo("Unable to find event category for OpenSimNetwork");

    }

    // virtual 
    void RexLogicModule::Uninitialize(Foundation::Framework *framework)
    {
        assert(framework_);
        SAFE_DELETE (network_handler_);

        framework_ = NULL;
        LogInfo("Module " + Name() + " uninitialized.");
    }

    // virtual
    void RexLogicModule::Update()
    {
        // tucofixme, test event system
        // Foundation::EventDataInterface params;

        // framework_->GetEventManager()->SendEvent(framework_->GetEventManager()->QueryEventCategory("OpenSimNetwork"),0,NULL);
        // framework_->GetEventManager()->SendEvent(framework_->GetEventManager()->QueryEventCategory("OpenSimNetwork"),1,&params);
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


