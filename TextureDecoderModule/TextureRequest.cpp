#include "StableHeaders.h"
#include "TextureRequest.h"

namespace TextureDecoder
{
    TextureRequest::TextureRequest() :
        requested_(false),
        size_(0),
        received_(0),
        width_(0),
        height_(0),
        levels_(-1),
        decoded_level_(-1),
        next_level_(5)
    {
    }
    
    TextureRequest::TextureRequest(const std::string& asset_id) : 
        asset_id_(asset_id),
        requested_(false),
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