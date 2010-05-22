// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoder_TextureRequest_h
#define incl_TextureDecoder_TextureRequest_h

#include "AssetInterface.h"
#include "ResourceInterface.h"
#include "ThreadTask.h"

namespace TextureDecoder
{
    //! OpenJpeg decode request, used internally by TextureService
    class DecodeRequest : public Foundation::ThreadTaskRequest
    {
    public:
        //! Texture asset ID
        std::string id_;

        //! Source asset data (JPEG2000 stream)
        Foundation::AssetPtr source_;

        //! Quality level to decode, 0 = highest
        int level_;
    };

    typedef boost::shared_ptr<DecodeRequest> DecodeRequestPtr;
    
    //! OpenJpeg decode result, used internally by TextureService
    struct DecodeResult : public Foundation::ThreadTaskResult
    {
    public:
        //! Texture asset id
        std::string id_;
        
        //! Pointer to resulting raw texture, null if decode failed
        Foundation::ResourcePtr texture_;

        //! Decoded quality level, -1 if decode failed
        int level_;
        
        //! Amount of quality levels found in stream
        int max_levels_;

        //! Original texture width 
        uint original_width_;

        //! Original texture height
        uint original_height_;

        //! Amount of components in texture
        uint components_;
    };
    
    typedef boost::shared_ptr<DecodeResult> DecodeResultPtr;

    //! An ongoing texture request, used internally by TextureService
    class TextureRequest
    {
    public:
        //! Constructor
        TextureRequest();
        
        //! Constructor
        /*! \param id Asset ID
         */
        TextureRequest(const std::string& id);
        
        //! Destructor
        ~TextureRequest();

        //! Sets asset request status
        void SetRequested(bool requested) { requested_ = requested; }

        void SetCanceled(bool canceled) { canceled_ = canceled; }

        //! Sets decode request status
        void SetDecodeRequested(bool requested) { decode_requested_ = requested; }

        //! Updates size & received count
        /*! \param size Total size of asset (from asset service)
            \param received Received continuous bytes (from asset service)
         */
        void UpdateSizeReceived(uint size, uint received);

        //! Updates request from decode result
        /*! \param result Decode result
            \return true if highest quality level was successfully decoded and request can be erased
         */
        bool UpdateWithDecodeResult(DecodeResult* result);
 
        //! Inserts a request tag
        void InsertTag(request_tag_t tag) { tags_.push_back(tag); }
        
        //! Inserts several request tags
        void InsertTags(const RequestTagVector tags) { tags_.insert(tags_.end(), tags.begin(), tags.end()); }
        
        //! Clears request tags
        void ClearTags() { tags_.clear(); } 
             
        //! Returns associated request tags
        const RequestTagVector& GetTags() const { return tags_; }
                
        //! Checks if enough data to decode next level
        bool HasEnoughData() const;

        //! Returns asset id
        const std::string& GetId() const { return id_; }

        //! Returns asset request status
        bool IsRequested() const { return requested_; }

        bool IsCanceled() const { return canceled_; }
        
        //! Returns decode request status
        bool IsDecodeRequested() const { return decode_requested_; }

        //! Returns total data size, 0 if unknown
        uint GetSize() const { return size_; }
        
        //! Returns received bytes
        uint GetReceived() const { return received_; }

        //! Returns width, 0 if unknown
        uint GetWidth() const { return width_; }

        //! Returns height, 0 if unknown
        uint GetHeight() const { return height_; }

        //! Returns components, 0 if unknown
        uint GetComponents() const { return components_; }

        //! Returns amount quality levels, -1 if unknown
        int GetLevels() const { return levels_; }
        
        //! Returns last decoded level, -1 if none so far
        int GetDecodedLevel() const { return decoded_level_; }

        //! Returns next level to decode
        int GetNextLevel() const { return next_level_; }
        
        //! List of request tags associated with this transfer
        RequestTagVector tags_;
        
    private:
        //! Estimates needed data size for a given level
        /*! \param level quality level
         */
        uint EstimateDataSize(int level) const;
        
        //! Asset on which this request is based
        std::string id_;

        //! whether asset request has been queued
        bool requested_;

        //! whether decode request has been queued
        bool decode_requested_;

        bool canceled_;

        //! Total data size, 0 if unknown
        uint size_;

        //! Last checked size of download progress (continuous received bytes)
        uint received_;

        //! Texture original width, 0 if unknown
        uint width_;

        //! Texture original height, 0 if unknown
        uint height_;

        //! Components in image, 0 if unknown
        uint components_;

        //! Number of quality levels, -1 if unknown
        int levels_;

        //! Last decoded quality level, 0 = full quality, -1 if none decoded so far
        int decoded_level_;

        //! Next quality level to decode
        int next_level_;     
    };
}
#endif