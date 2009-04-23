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
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Texture, texture_service_.get());

        LogInfo("Module " + Name() + " initialized.");
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
        framework_->GetServiceManager()->UnregisterService(texture_service_.get());
        texture_service_.reset();
        LogInfo("Module " + Name() + " uninitialized.");
    }
}

using namespace TextureDecoder;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(TextureDecoderModule)
POCO_END_MANIFEST

