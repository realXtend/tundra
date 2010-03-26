#include "StableHeaders.h"
#include "TextureRequest.h"
#include "TextureDecoderModule.h"

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
   
    void TextureRequest::UpdateSizeReceived(uint size, uint received)
    {
        size_ = size;
        received_ = received;

        // If has all data, can decode the max. quality level
        if ((size_) && (received >= size_))
            next_level_ = 0;
    }
     
    bool TextureRequest::HasEnoughData() const
    {
        return received_ >= EstimateDataSize(next_level_);
    }

    uint TextureRequest::EstimateDataSize(int level) const
    {
        if (level < 0) level = 0;
        
        // If nothing known of the image yet, assume the first full packet will tell us everything
        uint estimate = 600;
        
        // If know the dimensions, do data size estimation
        if ((width_) && (height_) && (components_))
            estimate = (uint)((width_ >> level) * (height_ >> level) * components_ * 0.15f);

        // If asset size known, adjust level 0 to require whole asset downloaded, and make sure
        // the estimate is not higher than actual data size
        if (size_)
        {
            if (!level)
                estimate = size_;
                
            if (estimate > size_) 
                estimate = size_;
        }
        
        return estimate;
    }
    
    bool TextureRequest::UpdateWithDecodeResult(DecodeResult* result)
    {
        if (result)
        {
            // Decode no longer pending
            decode_requested_ = false;

            // Update amount of quality levels, should now be known
            levels_ = result->max_levels_;
            
            // See if successfully decoded data
            if (result->texture_)
            {
                // Update texture original dimensions, should now be known
                width_ = result->original_width_;
                height_ =  result->original_height_;
                components_ = result->components_;
                       
                decoded_level_ = next_level_;  
            }
            
            // Set next quality level to decode
            // We do this regardless of success or failure, so that illegal texture data will
            // not cause endless re-decoding attempts
            if (next_level_ > 0)
            {
                next_level_--;
                return false;
            }
        }
        return true;
    }
}