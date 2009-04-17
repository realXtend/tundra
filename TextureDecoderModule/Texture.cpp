// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "Texture.h"

namespace TextureDecoder
{
    Texture::Texture(const std::string& id) : 
        TextureInterface(id),
        width_(0),
        height_(0),
        components_(0)
    {
    }

    Texture::Texture(const std::string& id, Core::uint width, Core::uint height, Core::uint components) : 
        TextureInterface(id),
        width_(width),
        height_(height),
        components_(components)
    {
        data_.resize(width * height * components);
    }

    Texture::~Texture()
    {
    }

    void Texture::SetSize(Core::uint width, Core::uint height, Core::uint components)
    {
        width_ = width;
        height_ = height;
        components_ = components;

        data_.resize(width * height * components);
    }

    const std::string& Texture::GetTypeName()
    {
        static const std::string name("Texture");

        return name;
    }
}