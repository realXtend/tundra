// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetTransfer_h
#define incl_Asset_AssetTransfer_h

#include "RexUUID.h"

namespace Asset
{
    //! An asset transfer in progress
    class AssetTransfer
    {
    public:
        //! constructor
        AssetTransfer();
        //! destructor
        ~AssetTransfer();
        
        //! receives an asset data packet
        /*! also resets elapsed time
            \param packet_index packet number starting from 0
            \param data pointer to data
            \param size size of data packet
         */
        void ReceiveData(Core::uint packet_index, const Core::u8* data, Core::uint size);
        
        //! assembles continuous asset data to a buffer
        /*! call GetReceivedContinuous() (or GetReceived() if you know the transfer is complete)
            first to know how big the buffer must be
            \param buffer pointer to buffer that will receive data
         */
        void AssembleData(Core::u8* buffer) const;
        
        //! sets asset ID
        /*! \param asset_id asset UUID
         */
        void SetAssetId(const RexTypes::RexUUID& asset_id) { asset_id_ = asset_id; }
        
        //! sets asset type
        /*! \param asset_type asset type
         */
        void SetAssetType(Core::uint asset_type) { asset_type_ = asset_type; }
        
        //! sets asset size
        /*! called when asset transfer header received
            \param size asset size in bytes
         */
        void SetSize(Core::uint size) { size_ = size; }
        
        //! adds elapsed time
        /*! \param delta_time amount of time to add
         */
        void AddTime(Core::f64 delta_time) { time_ += delta_time; }
        
        //! resets elapsed time
        void ResetTime() { time_ = 0.0; }
        
        //! returns asset ID
        const RexTypes::RexUUID& GetAssetId() const { return asset_id_; }
        
        //! returns asset type
        Core::uint GetAssetType() const { return asset_type_; }
        
        //! returns expected asset size
        Core::uint GetSize() const { return size_; }
        
        //! returns total size of data received so far
        Core::uint GetReceived() const { return received_; }
        
        //! returns total size of continuous data from the asset beginning received so far
        Core::uint GetReceivedContinuous() const;
        
        //! returns elapsed time since last packet
        Core::f64 GetTime() const { return time_; }
        
        //! returns whether transfer is finished (all bytes received)
        bool Ready() const;
        
    private:
        typedef std::map<Core::uint, std::vector<Core::u8> > DataPacketMap;
        
        //! asset ID
        RexTypes::RexUUID asset_id_;
        
        //! asset type
        Core::uint asset_type_;
        
        //! expected size
        Core::uint size_;
        
        //! received bytes
        Core::uint received_;
        
        //! map of data packets
        DataPacketMap data_packets_;
        
        //! elapsed time since last packet
        Core::f64 time_;
    };
}

#endif