// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_TextureServiceInterface_h
#define incl_Interfaces_TextureServiceInterface_h

#include "ServiceInterface.h"

namespace Foundation
{    
    //! Texture decoding service.
    /*!
        \ingroup Services_group
        
        Through this interface, texture assets can be requested to be downloaded and decoded.
        The decoded textures are returned through the RESOURCE_READY event as raw texture resources 
        (Foundation::TextureInterface).
        Implemented by the \ref TextureDecoderModule.
    */
    class TextureServiceInterface : public ServiceInterface
    {
    public:
        TextureServiceInterface() {}
        virtual ~TextureServiceInterface() {}

        //! Requests a texture to be received and decoded
        /*! When texture data becomes available, an event will be sent for each quality level decoded
            \param asset_id texture ID, UUID for legacy UDP assets
            \return request tag, will be sent back along with RESOURCE_READY event
         */
        virtual request_tag_t RequestTexture(const std::string& asset_id) = 0;

        //! Removes a texture from the disk cache with the texture id
        //! @param texture_is as std::string
        virtual void DeleteFromCache(const std::string &texture_id) = 0;
    };
}

#endif
