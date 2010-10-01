// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UiServiceModule.h"
#include "UiService.h"
#include "NaaliUi.h"
#include "NaaliGraphicsView.h"

#include "MemoryLeakCheck.h"

std::string UiServiceModule::type_name_static_ = "UiService";

UiServiceModule::UiServiceModule() : IModule(type_name_static_)
{
}

UiServiceModule::~UiServiceModule()
{
}

void UiServiceModule::PreInitialize()
{
}

void UiServiceModule::Initialize()
{
    // Register UI service.
    assert(GetFramework()->Ui()->GraphicsView());
    service_ = boost::shared_ptr<UiService>(new UiService(GetFramework()->Ui()->GraphicsView()));
    framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Gui, service_);
}

void UiServiceModule::PostInitialize()
{
}

void UiServiceModule::Uninitialize()
{
    framework_->GetServiceManager()->UnregisterService(service_);
    service_.reset();
}

void UiServiceModule::Update(f64 frametime)
{
    RESETPROFILER;
}

// virtual
bool UiServiceModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
{
    return false;
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(UiServiceModule)
POCO_END_MANIFEST
