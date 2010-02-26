// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ConsoleCommandServiceInterface.h"
#include "TextureService.h"
#include "TextureDecoderModule.h"
#include "Framework.h"
#include "EventManager.h"
#include "ServiceManager.h"

namespace TextureDecoder
{
    TextureDecoderModule::TextureDecoderModule() : ModuleInterfaceImpl(type_static_)
    {
    }

    TextureDecoderModule::~TextureDecoderModule()
    {
    }

    // virtual
    void TextureDecoderModule::Load()
    {
        LogInfo(Name() + " loaded.");
    }

    // virtual
    void TextureDecoderModule::Unload()
    {
        LogInfo(Name() + " unloaded.");
    }

    // virtual
    void TextureDecoderModule::Initialize()
    {
        texture_service_ = TextureServicePtr(new TextureService(framework_));
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Texture, texture_service_);

        LogInfo(Name() + " initialized.");
    }
    
     // virtual
    void TextureDecoderModule::PostInitialize()
    {   
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        asset_event_category_ = event_manager->QueryEventCategory("Asset");
        task_event_category_ = event_manager->QueryEventCategory("Task");
    }
    
    // virtual
    void TextureDecoderModule::Update(f64 frametime)
    {
        {
            PROFILE(TextureDecoderModule_Update);

            if (texture_service_)
                texture_service_->Update(frametime);
        }
        RESETPROFILER;
    }

    // virtual 
    void TextureDecoderModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(texture_service_);
        texture_service_.reset();
        LogInfo(Name() + " uninitialized.");
    }
    
    bool TextureDecoderModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        PROFILE(TextureDecoderModule_HandleEvent);
        if (category_id == asset_event_category_)
        {
            if (texture_service_)
                return texture_service_->HandleAssetEvent(event_id, data);
            else return false;
        }
        if (category_id == task_event_category_)
        {
            if (texture_service_)
                return texture_service_->HandleTaskEvent(event_id, data);
            else return false;
        }
        return false;
    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace TextureDecoder;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(TextureDecoderModule)
POCO_END_MANIFEST

