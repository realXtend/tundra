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
    std::string TextureDecoderModule::type_name_static_ = "TextureDecoder";

    TextureDecoderModule::TextureDecoderModule() : IModule(type_name_static_)
    {
    }

    TextureDecoderModule::~TextureDecoderModule()
    {
    }

    // virtual
    void TextureDecoderModule::Initialize()
    {
        texture_service_ = TextureServicePtr(new TextureService(framework_));
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Texture, texture_service_);
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
    }
    
    bool TextureDecoderModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
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

POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(TextureDecoderModule)
POCO_END_MANIFEST

