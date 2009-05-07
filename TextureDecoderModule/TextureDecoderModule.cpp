// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ConsoleCommandServiceInterface.h"
#include "TextureService.h"
#include "TextureDecoderModule.h"

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
        LogInfo("Module " + Name() + " loaded.");
    }

    // virtual
    void TextureDecoderModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void TextureDecoderModule::Initialize()
    {
        texture_service_ = TextureServicePtr(new TextureService(framework_));
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Texture, texture_service_);

        LogInfo("Module " + Name() + " initialized.");
    }
    
     // virtual
    void TextureDecoderModule::PostInitialize()
    {   
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        asset_event_category_ = event_manager->QueryEventCategory("Asset");
    } 
    
    // virtual
    void TextureDecoderModule::Update(Core::f64 frametime)
    {
        if (texture_service_)
            texture_service_->Update(frametime);
    }

    // virtual 
    void TextureDecoderModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(texture_service_);
        texture_service_.reset();
        LogInfo("Module " + Name() + " uninitialized.");
    }
    
    bool TextureDecoderModule::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (category_id == asset_event_category_)
        {
            if (texture_service_)
                return texture_service_->HandleAssetEvent(event_id, data);
            else return false;
        }
        
        return false;
    }
}

using namespace TextureDecoder;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(TextureDecoderModule)
POCO_END_MANIFEST

