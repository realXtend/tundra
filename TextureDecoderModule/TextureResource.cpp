// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "TextureResource.h"

namespace TextureDecoder
{
    TextureResource::TextureResource(const std::string& id) : 
        TextureInterface(id),
        width_(0),
        height_(0),
        components_(0),
        level_(-1)
    {
    }

    TextureResource::TextureResource(const std::string& id, uint width, uint height, uint components) : 
        TextureInterface(id),
        width_(width),
        height_(height),
        components_(components),
        level_(-1)
    {
        data_.resize(width * height * components);
    }

    TextureResource::~TextureResource()
    {
    }

    void TextureResource::SetSize(uint width, uint height, uint components)
    {
        width_ = width;
        height_ = height;
        components_ = components;

        data_.resize(width * height * components);
    }

    static const std::string texture_resource_name("Texture");

    const std::string& TextureResource::GetType() const
    {
        return texture_resource_name;
    }

    const std::string& TextureResource::GetTypeStatic()
    {
        return texture_resource_name;
    }
    
    bool TextureResource::IsValid() const
    {
        return data_.size() > 0;
    }
}