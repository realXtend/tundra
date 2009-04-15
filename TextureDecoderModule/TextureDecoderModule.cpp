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
            "DecodeTexture", "Fetch & decode texture. Usage: DecodeTexture(uuid, [reduction])", 
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
        LogInfo("Module " + Name() + " initialized.");
    }
    
    // virtual
    void TextureDecoderModule::PostInitialize()
    {
        // Decode worker depends on an asset service, at this point it should exist
        decoder_ = DecoderPtr(new Decoder(framework_));
    }

    // virtual
    void TextureDecoderModule::Update(Core::f64 frametime)
    {
    }

    // virtual 
    void TextureDecoderModule::Uninitialize()
    {
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
        {
            return Console::ResultFailure("Usage: DecodeTexture(uuid, [reduction])");
        }

        if (decoder_)
        {
            Core::uint reduction = 0;
            if (params.size() > 1)
            {
                try
                {
                    reduction = Core::ParseString<int>(params[1]);
                } catch (std::exception) {}
            }
            
            decoder_->DecodeTexture(params[0], reduction);
        }

        return Console::ResultSuccess();
    }
}

using namespace TextureDecoder;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(TextureDecoderModule)
POCO_END_MANIFEST

