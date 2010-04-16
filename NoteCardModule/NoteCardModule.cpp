// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NoteCardModule.h"
#include "NoteCardManager.h"
#include "EventManager.h"
#include "SceneEvents.h"
#include "NetworkEvents.h"
#include "SceneManager.h"
#include "EC_NoteCard.h"

namespace NoteCard
{
    std::string NoteCardModule::name_static_ = "NoteCardModule";
    
    NoteCardModule::NoteCardModule() :
        ModuleInterfaceImpl(name_static_),
        manager_(0),
        scene_event_category_(0),
        framework_event_category_(0),
        input_event_category_(0),
        network_state_event_category_(0)
    {
    }

    NoteCardModule::~NoteCardModule()
    {
    }
    
    void NoteCardModule::Load()
    {
        DECLARE_MODULE_EC(EC_NoteCard);
    }

    void NoteCardModule::Initialize()
    {
        manager_ = new NoteCardManager(GetFramework());
        event_manager_ = framework_->GetEventManager();
    }

    void NoteCardModule::PostInitialize()
    {
        scene_event_category_ = event_manager_->QueryEventCategory("Scene");
        framework_event_category_ = event_manager_->QueryEventCategory("Framework");
        input_event_category_ = event_manager_->QueryEventCategory("Input");
    }

    void NoteCardModule::SubscribeToNetworkEvents()
    {
        network_state_event_category_ = event_manager_->QueryEventCategory("NetworkState");
    }

    void NoteCardModule::Uninitialize()
    {
        if (manager_)
        {
            manager_->deleteLater();
            manager_ = 0;
        }
    }

    void NoteCardModule::Update(f64 frametime)
    {
        RESETPROFILER;
    }

    bool NoteCardModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (category_id == framework_event_category_ && event_id == Foundation::NETWORKING_REGISTERED)
        {
            SubscribeToNetworkEvents();
        }

        return false;
    }

}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace NoteCard;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(NoteCardModule)
POCO_END_MANIFEST 

