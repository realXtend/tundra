// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_TextureInterface_h
#define incl_Interfaces_TextureInterface_h

#include "ResourceInterface.h"
#include "CoreModuleApi.h"

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
        virtual uint GetWidth() const = 0;

        //! returns height
        virtual uint GetHeight() const = 0;

        //! returns number of components
        virtual uint GetComponents() const = 0;

        //! returns quality level, -1 if not known
        virtual int GetLevel() const = 0;

        //! returns data
        /*! each pixel of each component should be encoded as a u8, so data size should be width * height * components
         */
        virtual u8* GetData() = 0;
    };
}

#endif