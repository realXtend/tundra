// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoder_Texture_h
#define incl_TextureDecoder_Texture_h

#include "TextureInterface.h"

namespace TextureDecoder
{
    class Texture : public Foundation::TextureInterface
    {
    public:
        Texture(const std::string& id);
        Texture(const std::string& id, Core::uint width, Core::uint height, Core::uint components);
        virtual ~Texture();

        virtual Core::uint GetWidth() { return width_; }
        virtual Core::uint GetHeight() { return height_; }
        virtual Core::uint GetComponents() { return components_; }
        virtual Core::u8* GetData() { return &data_[0]; }
        virtual const std::string& GetTypeName();

        void SetSize(Core::uint width, Core::uint height, Core::uint components);

    private:
        Core::uint width_;
        Core::uint height_;
        Core::uint components_;
        std::vector<Core::u8> data_;
    };
}

#endif