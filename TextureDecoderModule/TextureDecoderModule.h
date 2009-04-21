// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoderModule_h
#define incl_TextureDecoderModule_h

#include "ModuleInterface.h"
#include "TextureDecoderModuleApi.h"

namespace Foundation
{
    class Framework;
}

namespace TextureDecoder
{
    class Decoder;
    typedef boost::shared_ptr<Decoder> DecoderPtr;
    
    //! texture decoder module
    class TEXTURE_DECODER_MODULE_API TextureDecoderModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        TextureDecoderModule();
        virtual ~TextureDecoderModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void Uninitialize();
        virtual void Update(Core::f64 frametime);

        virtual bool HandleEvent(
            Core::event_category_id_t category_id,
            Core::event_id_t event_id, 
            Foundation::EventDataInterface* data);
        
        //! returns framework
        Foundation::Framework *GetFramework() { return framework_; }

        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_TextureDecoder;

    private:
        //! texture decoder worker
        DecoderPtr decoder_;
    };
}

#endif
