// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_UDPAssetTransfer_h
#define incl_Asset_UDPAssetTransfer_h

#include "CoreTypes.h"
namespace Asset
{
    //! Stores data related to an UDP asset transfer that is in progress. Not necessary to clients of the AssetModule.
    class UDPAssetTransfer
    {
    public:
        //! Constructor
        UDPAssetTransfer();
        //! Destructor
        ~UDPAssetTransfer();
        
        //! Receives an asset data packet
        /*! Also resets elapsed time
            \param packet_index Packet number, starting from 0
            \param data Pointer to data
            \param size Size of data packet
         */
        void ReceiveData(uint packet_index, const u8* data, uint size);
        
        //! Assembles continuous asset data to a buffer
        /*! Call GetReceivedContinuous() (or GetReceived() if you know the transfer is complete)
            first to know how big the buffer must be
            \param buffer Pointer to buffer that will receive data
         */
        void AssembleData(u8* buffer) const;
        
        //! Sets asset ID
        /*! \param asset_id Asset id
         */
        void SetAssetId(const std::string& asset_id) { asset_id_ = asset_id; }
        
        //! Sets asset type
        /*! \param asset_type Asset type
         */
        void SetAssetType(uint asset_type) { asset_type_ = asset_type; }
        
        //! Sets asset size
        /*! Called when asset transfer header received
            \param size Asset size in bytes
         */
        void SetSize(uint size) { size_ = size; }
        
        //! Adds elapsed time
        /*! \param delta_time Amount of time to add
         */
        void AddTime(f64 delta_time) { time_ += delta_time; }
        
        //! Resets elapsed time
        void ResetTime() { time_ = 0.0; }
        
        //! Inserts a request tag
        void InsertTag(request_tag_t tag) { tags_.push_back(tag); }
        
        //! Inserts several request tags
        void InsertTags(const RequestTagVector tags) { tags_.insert(tags_.end(), tags.begin(), tags.end()); }
        
        //! Clears request tags
        void ClearTags() { tags_.clear(); } 
             
        //! Returns associated request tags
        const RequestTagVector& GetTags() const { return tags_; }
                          
        //! Returns asset ID
        const std::string& GetAssetId() const { return asset_id_; }
        
        //! Returns asset type
        uint GetAssetType() const { return asset_type_; }
        
        //! Returns expected asset size, 0 if unknown
        uint GetSize() const { return size_; }
        
        //! Returns total size of data received so far
        uint GetReceived() const { return received_; }
        
        //! Returns total size of continuous data from the asset beginning received so far
        uint GetReceivedContinuous() const;
        
        //! Returns elapsed time since last packet
        f64 GetTime() const { return time_; }
                        
        //! Returns whether transfer is finished (all bytes received)
        bool Ready() const;
        
    private:
        typedef std::map<uint, std::vector<u8> > DataPacketMap;
        
        //! Asset ID
        std::string asset_id_;
        
        //! Asset type
        uint asset_type_;
        
        //! Expected size
        uint size_;
        
        //! Received bytes
        uint received_;
        
        //! Map of data packets
        DataPacketMap data_packets_;
        
        //! Elapsed time since last packet
        f64 time_;
        
        //! List of request tags associated with this transfer
        RequestTagVector tags_;
    };
}

#endif