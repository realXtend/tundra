// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetTransfer_h
#define incl_Asset_AssetTransfer_h

namespace Asset
{
    //! Stores data related to an asset transfer that is in progress. Not necessary to clients of the AssetModule.
    class AssetTransfer
    {
    public:
        //! Constructor
        AssetTransfer();
        //! Destructor
        ~AssetTransfer();
        
        //! Receives an asset data packet
        /*! Also resets elapsed time
            \param packet_index Packet number, starting from 0
            \param data Pointer to data
            \param size Size of data packet
         */
        void ReceiveData(Core::uint packet_index, const Core::u8* data, Core::uint size);
        
        //! Assembles continuous asset data to a buffer
        /*! Call GetReceivedContinuous() (or GetReceived() if you know the transfer is complete)
            first to know how big the buffer must be
            \param buffer Pointer to buffer that will receive data
         */
        void AssembleData(Core::u8* buffer) const;
        
        //! Sets asset ID
        /*! \param asset_id Asset id
         */
        void SetAssetId(const std::string& asset_id) { asset_id_ = asset_id; }
        
        //! Sets asset type
        /*! \param asset_type Asset type
         */
        void SetAssetType(Core::uint asset_type) { asset_type_ = asset_type; }
        
        //! Sets asset size
        /*! Called when asset transfer header received
            \param size Asset size in bytes
         */
        void SetSize(Core::uint size) { size_ = size; }
        
        //! Adds elapsed time
        /*! \param delta_time Amount of time to add
         */
        void AddTime(Core::f64 delta_time) { time_ += delta_time; }
        
        //! Resets elapsed time
        void ResetTime() { time_ = 0.0; }
        
        //! Returns asset ID
        const std::string& GetAssetId() const { return asset_id_; }
        
        //! Returns asset type
        Core::uint GetAssetType() const { return asset_type_; }
        
        //! Returns expected asset size, 0 if unknown
        Core::uint GetSize() const { return size_; }
        
        //! Returns total size of data received so far
        Core::uint GetReceived() const { return received_; }
        
        //! Returns total size of continuous data from the asset beginning received so far
        Core::uint GetReceivedContinuous() const;
        
        //! Returns elapsed time since last packet
        Core::f64 GetTime() const { return time_; }
        
        //! Returns whether transfer is finished (all bytes received)
        bool Ready() const;
        
    private:
        typedef std::map<Core::uint, std::vector<Core::u8> > DataPacketMap;
        
        //! Asset ID
        std::string asset_id_;
        
        //! Asset type
        Core::uint asset_type_;
        
        //! Expected size
        Core::uint size_;
        
        //! Received bytes
        Core::uint received_;
        
        //! Map of data packets
        DataPacketMap data_packets_;
        
        //! Elapsed time since last packet
        Core::f64 time_;
    };
}

#endif