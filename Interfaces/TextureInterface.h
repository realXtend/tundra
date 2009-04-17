// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ResourceInterface_h
#define incl_Interfaces_TextureInterface_h

#include "ResourceInterface.h"

namespace Foundation
{
    class TextureInterface;    
    typedef boost::shared_ptr<TextureInterface> TexturePtr;

    //! a generic texture image with raw data
    class MODULE_API TextureInterface : public ResourceInterface
    {
    public:
        //! default constructor
        TextureInterface() : ResourceInterface() {}        

        //! constructor
        /*! \param id identifier of texture resource
         */
        TextureInterface(const std::string& id) : ResourceInterface(id) {}

        //! returns width
        virtual Core::uint GetWidth() = 0;

        //! returns height
        virtual Core::uint GetHeight() = 0;

        //! returns number of components
        virtual Core::uint GetComponents() = 0;

        //! returns data
        /*! each pixel of each component should be encoded as a u8, so data size should be width * height * components
         */
        virtual Core::u8* GetData() = 0;
    };
}

#endif