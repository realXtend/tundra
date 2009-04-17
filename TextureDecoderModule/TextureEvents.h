// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoder_TextureEvents_h
#define incl_TextureDecoder_TextureEvents_h

#include "EventDataInterface.h"
#include "TextureInterface.h"

namespace TextureDecoder
{
    //! texture events
    namespace Event
    {
        //! texture ready event id
        static const Core::event_id_t TEXTURE_READY = 1;
        
        //! texture ready event data
        class TextureReady : public Foundation::EventDataInterface
        {
        public:
            TextureReady(const std::string& id, int level, Foundation::TexturePtr texture) :
                id_(id),
                level_(level),
                texture_(texture)
            {
            }
            
            virtual ~TextureReady()
            {
            }
        
            std::string id_;
            int level_;
            Foundation::TexturePtr texture_;
        };
    }
}
    
#endif