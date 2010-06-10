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
        TextureResource(const std::string& id, uint width, uint height, uint components);
        virtual ~TextureResource();

        virtual bool IsValid() const;
        virtual uint GetWidth() const { return width_; }
        virtual uint GetHeight() const { return height_; }
        virtual uint GetComponents() const { return components_; }
        virtual int GetLevel() const { return level_; }
        virtual u8* GetData() { return &data_[0]; }
        virtual uint GetDataSize() { return data_size_; }
        virtual int GetFormat() { return format_; }
        virtual const std::string& GetType() const;
        static const std::string& GetTypeStatic();

        void SetSize(uint width, uint height, uint components);
        void SetDataSize(uint data_size) { data_size_ = data_size; }
        void SetWidth(uint width) { width_ = width; }
        void SetHeight(uint height) { height_ = height; }
        void SetLevel(int level) { level_ = level; }
        void SetFormat(int format) { format_ = format; }

    private:
        uint width_;
        uint height_;
        uint components_;
        uint data_size_;
        int format_;
        int level_;
        std::vector<u8> data_;
    };
}

#endif