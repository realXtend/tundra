// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ConsoleCommandServiceInterface.h"
#include "Decoder.h"
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
        
        AutoRegisterConsoleCommand(Console::CreateCommand(
            "DecodeTexture", "Fetch & decode texture. Usage: DecodeTexture(uuid)", 
            Console::Bind(this, &TextureDecoderModule::ConsoleDecodeTexture)));
    }

    // virtual
    void TextureDecoderModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void TextureDecoderModule::Initialize()
    {
        decoder_ = DecoderPtr(new Decoder(framework_));
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Texture, decoder_.get());

        LogInfo("Module " + Name() + " initialized.");
    }
    
    // virtual
    void TextureDecoderModule::Update(Core::f64 frametime)
    {
        if (decoder_)
            decoder_->Update(frametime);
    }

    // virtual 
    void TextureDecoderModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(decoder_.get());
        decoder_.reset();
        LogInfo("Module " + Name() + " uninitialized.");
    }
    
 
    bool TextureDecoderModule::HandleEvent(
        Core::event_category_id_t category_id,
        Core::event_id_t event_id, 
        Foundation::EventDataInterface* data)
    {
        return false;
    }
    
    Console::CommandResult TextureDecoderModule::ConsoleDecodeTexture(const Core::StringVector &params)
    {
        if (params.size() < 1)
            return Console::ResultFailure("Usage: DecodeTexture(uuid)");

        if (decoder_)
            decoder_->QueueTextureRequest(params[0]);

        return Console::ResultSuccess();
    }
}

using namespace TextureDecoder;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(TextureDecoderModule)
POCO_END_MANIFEST

