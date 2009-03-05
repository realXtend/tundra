// For conditions of distribution and use, see copyright notice in license.txt

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


RexLogic::RexLogic() : ModuleInterface_Impl(Foundation::Module::Type_WorldLogic), framework_(NULL)
{
}

RexLogic::~RexLogic()
{
    assert(framework_ == NULL);
}

// virtual
void RexLogic::Load()
{
    DECLARE_MODULE_EC(EC_Collision);
    DECLARE_MODULE_EC(EC_ObjFreeData);
    DECLARE_MODULE_EC(EC_ObjGeneralProps);    
    DECLARE_MODULE_EC(EC_SelectPriority);
    DECLARE_MODULE_EC(EC_ServerScript);
    DECLARE_MODULE_EC(EC_SpatialSound);

    LOG("System " + Name() + " loaded.");
}

// virtual
void RexLogic::Unload()
{
    LOG("System " + Name() + " unloaded.");
}

// virtual
void RexLogic::Initialize(Foundation::Framework *framework)
{
    assert(framework_ == NULL);
    framework_ = framework;

    // fixme, register WorldLogic to the framework as realxtend worldlogicinterface!
    // WorldLogic::registerSystem(framework);
    world_logic_ = new WorldLogic(framework);

    // Register components for network messages, should be done somewhere else...?
    world_logic_->GetNetworkHandler()->RegisterForNetworkMessages(EC_Collision::Name(),EC_Collision::GetNetworkMessages());
    world_logic_->GetNetworkHandler()->RegisterForNetworkMessages(EC_ObjFreeData::Name(),EC_ObjFreeData::GetNetworkMessages());
    world_logic_->GetNetworkHandler()->RegisterForNetworkMessages(EC_ObjGeneralProps::Name(),EC_ObjGeneralProps::GetNetworkMessages());    
    world_logic_->GetNetworkHandler()->RegisterForNetworkMessages(EC_SelectPriority::Name(),EC_SelectPriority::GetNetworkMessages());
    world_logic_->GetNetworkHandler()->RegisterForNetworkMessages(EC_ServerScript::Name(),EC_ServerScript::GetNetworkMessages());
    world_logic_->GetNetworkHandler()->RegisterForNetworkMessages(EC_SpatialSound::Name(),EC_SpatialSound::GetNetworkMessages()); 

    LOG("System " + Name() + " initialized.");
}

// virtual 
void RexLogic::Uninitialize(Foundation::Framework *framework)
{
    assert(framework_);
    SAFE_DELETE (world_logic_);

    framework_ = NULL;


    LOG("System " + Name() + " uninitialized.");
}

// virtual
void RexLogic::Update()
{    
    // fixme, simulate a network message arriving to networkeventhandler
    std::string tempnetworkmessage = "fixme this_is_networkmessage";
    world_logic_->GetNetworkHandler()->HandleNetworkMessage(tempnetworkmessage);
}


POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(RexLogic)
POCO_END_MANIFEST


