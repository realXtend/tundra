// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "PhononPlayerModule.h"
#include "Service.h"

#include "MemoryLeakCheck.h" 

namespace PhononPlayer
{
    std::string PhononPlayerModule::type_name_static_ = "PhononPlayer";

    PhononPlayerModule::PhononPlayerModule()
        : ModuleInterface(type_name_static_)
    {
    }

    PhononPlayerModule::~PhononPlayerModule()
    {
    }

    void PhononPlayerModule::Load()
    {
    }

    void PhononPlayerModule::Unload()
    {
    }

    void PhononPlayerModule::Initialize() 
    {
        player_service_ = MediaPlayer::ServicePtr(new Service());

        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Player, player_service_);
    }

    void PhononPlayerModule::PostInitialize()
    {
    }

    void PhononPlayerModule::Uninitialize()
    {
        if (player_service_)
            framework_->GetServiceManager()->UnregisterService(player_service_);
    }

    void PhononPlayerModule::Update(f64 frametime)
    {
    }

    bool PhononPlayerModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        return false;
    }

} // PhononPlayer

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace PhononPlayer;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
POCO_EXPORT_CLASS(PhononPlayerModule)
POCO_END_MANIFEST
