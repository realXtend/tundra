// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoder_TextureResource_h
#define incl_TextureDecoder_TextureResource_h

#include "TextureInterface.h"

namespace TextureDecoder
{
    class TextureResource : public Foundation::TextureInterface
    {
    public:
        TextureResource(const std::string& id);
        TextureResource(const std::string& id, Core::uint width, Core::uint height, Core::uint components);
        virtual ~TextureResource();

        virtual Core::uint GetWidth() const { return width_; }
        virtual Core::uint GetHeight() const { return height_; }
        virtual Core::uint GetComponents() const { return components_; }
        virtual int GetLevel() const { return level_; }
        virtual Core::u8* GetData() { return &data_[0]; }
        virtual const std::string& GetType() const;

        void SetSize(Core::uint width, Core::uint height, Core::uint components);
        void SetLevel(int level) { level_ = level; }

    private:
        Core::uint width_;
        Core::uint height_;
        Core::uint components_;
        int level_;
        std::vector<Core::u8> data_;
    };
}

#endif