#include "StableHeaders.h"
#include "RexLogic.h"
#include "Entity.h"
#include "EntityManager.h"
#include "ComponentManager.h"
#include "Poco/ClassLibrary.h"

#include "EC_Collision.h"
#include "EC_ObjFreeData.h"
#include "EC_ObjGeneralProps.h"
#include "EC_SelectPriority.h"
#include "EC_ServerScript.h"
#include "EC_SpatialSound.h"
#include "WorldLogic.h"


RexLogic::RexLogic() : ModuleInterface_Impl(Foundation::Module::Type_WorldLogic), mFramework(NULL)
{
}

RexLogic::~RexLogic()
{
    assert(mFramework == NULL);
}

// virtual
void RexLogic::load()
{
    DECLARE_MODULE_EC(EC_Collision);
    DECLARE_MODULE_EC(EC_ObjFreeData);
    DECLARE_MODULE_EC(EC_ObjGeneralProps);    
    DECLARE_MODULE_EC(EC_SelectPriority);
    DECLARE_MODULE_EC(EC_ServerScript);
    DECLARE_MODULE_EC(EC_SpatialSound);

    LOG("System " + name() + " loaded.");
}

// virtual
void RexLogic::unload()
{
    LOG("System " + name() + " unloaded.");
}

// virtual
void RexLogic::initialize(Foundation::Framework *framework)
{
    assert(mFramework == NULL);
    mFramework = framework;

    // fixme, register WorldLogic to the framework as realxtend worldlogicinterface!
    // WorldLogic::registerSystem(framework);
    mWorldLogic = new WorldLogic(framework);

    // Register components for network messages, should be done somewhere else...?
    mWorldLogic->getNetworkHandler()->registerForNetworkMessages(EC_Collision::name(),EC_Collision::getNetworkMessages());
    mWorldLogic->getNetworkHandler()->registerForNetworkMessages(EC_ObjFreeData::name(),EC_ObjFreeData::getNetworkMessages());
    mWorldLogic->getNetworkHandler()->registerForNetworkMessages(EC_ObjGeneralProps::name(),EC_ObjGeneralProps::getNetworkMessages());    
    mWorldLogic->getNetworkHandler()->registerForNetworkMessages(EC_SelectPriority::name(),EC_SelectPriority::getNetworkMessages());
    mWorldLogic->getNetworkHandler()->registerForNetworkMessages(EC_ServerScript::name(),EC_ServerScript::getNetworkMessages());
    mWorldLogic->getNetworkHandler()->registerForNetworkMessages(EC_SpatialSound::name(),EC_SpatialSound::getNetworkMessages()); 

    LOG("System " + name() + " initialized.");
}

// virtual 
void RexLogic::uninitialize(Foundation::Framework *framework)
{
    assert(mFramework);
    mFramework = NULL;

    LOG("System " + name() + " uninitialized.");
}

// virtual
void RexLogic::update()
{    
    // fixme, simulate a network message arriving to networkeventhandler
    std::string tempnetworkmessage = "fixme this_is_networkmessage";
    mWorldLogic->getNetworkHandler()->handleNetworkMessage(tempnetworkmessage);
}


POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(RexLogic)
POCO_END_MANIFEST


