#include "StableHeaders.h"
#include "TextureRequest.h"

namespace TextureDecoder
{
    TextureRequest::TextureRequest() :
        requested_(false),
        decode_requested_(false),
        size_(0),
        received_(0),
        width_(0),
        height_(0),
        levels_(-1),
        decoded_level_(-1),
        next_level_(5)
    {
    }
    
    TextureRequest::TextureRequest(const std::string& id) : 
        id_(id),
        requested_(false),
        decode_requested_(false),
        size_(0),
        received_(0),
        width_(0),
        height_(0),
        levels_(-1),
        decoded_level_(-1),
        next_level_(5)
    {
    }
    
    TextureRequest::~TextureRequest()
    {
    }
   
    void TextureRequest::UpdateSizeReceived(Core::uint size, Core::uint received)
    {
        size_ = size;
        received_ = received;

        // If has all data, can decode the max. quality level
        if ((size_) && (received >= size_))
            next_level_ = 0;
    }
     
    void TextureRequest::SetSize(Core::uint width, Core::uint height, Core::uint components)
    {
        width_ = width;
        height_ = height;
        components_ = components;
    }

    void TextureRequest::SetLevels(int levels)
    {
        levels_ = levels;
    }

    void TextureRequest::SetNextLevelToDecode()
    {
        if (next_level_ > 0)
            next_level_--;
    }

    void TextureRequest::DecodeSuccess()
    {
        decoded_level_ = next_level_;
    }

    bool TextureRequest::HasEnoughData()
    {
        return received_ >= EstimateDataSize(next_level_);
    }

    Core::uint TextureRequest::EstimateDataSize(int level)
    {
        if (level < 0) level = 0;
        
        // If nothing known of the image yet, assume the first packet will tell us everything
        if ((!width_) || (!height_) || (!components_))
            return 600;
        
        // If max quality level, we require the full size downloaded
        if (!level)
            return size_ ? size_ : 600;
        
        // Assume about 85% compression ratio
        Core::uint estimate = (Core::uint)((width_ >> level) * (height_ >> level) * components_ * 0.15f);
        
        if (estimate > size_) 
            estimate = size_;
        if (estimate < 600)
            estimate = 600;
            
        return estimate;
    }
}