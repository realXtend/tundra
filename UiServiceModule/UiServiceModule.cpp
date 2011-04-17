// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UiServiceModule.h"
#include "UiService.h"
#include "UiAPI.h"
#include "UiGraphicsView.h"

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
    if (GetFramework()->IsHeadless())
        return;

    // Register UI service.
    assert(GetFramework()->Ui()->GraphicsView());
    service_ = boost::shared_ptr<UiService>(new UiService(framework_, GetFramework()->Ui()->GraphicsView()));
    framework_->GetServiceManager()->RegisterService(Service::ST_Gui, service_);
    framework_->RegisterDynamicObject("uiservice", service_.get());
}

void UiServiceModule::PostInitialize()
{
}

void UiServiceModule::Uninitialize()
{
    if (service_)
        framework_->GetServiceManager()->UnregisterService(service_);
    service_.reset();
}

void UiServiceModule::Update(f64 frametime)
{
    RESETPROFILER;
}

void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Foundation::Framework *fw)
{
    IModule *module = new UiServiceModule();
    fw->GetModuleManager()->DeclareStaticModule(module);
}
}
