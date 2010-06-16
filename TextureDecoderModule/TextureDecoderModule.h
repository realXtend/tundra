// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoderModule_h
#define incl_TextureDecoderModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "TextureDecoderModuleApi.h"

namespace Foundation
{
    class Framework;
}

namespace TextureDecoder
{
    class TextureService;
    typedef boost::shared_ptr<TextureService> TextureServicePtr;
    
    //! Texture decoder module
    class TEXTURE_DECODER_MODULE_API TextureDecoderModule : public Foundation::ModuleInterface
    {
    public:
        TextureDecoderModule();
        virtual ~TextureDecoderModule();

        virtual void Initialize();
        virtual void Uninitialize();
        virtual void Update(f64 frametime);
        virtual void PostInitialize();
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return type_name_static_; }

    private:
        //! Type name of the module.
        static std::string type_name_static_;

        //! Texture service
        TextureServicePtr texture_service_;

        //! Asset event category
        event_category_id_t asset_event_category_;

        //! Task event category
        event_category_id_t task_event_category_;
    };
}

#endif
