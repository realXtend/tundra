// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_HttpAssetTransfer_h
#define incl_Asset_HttpAssetTransfer_h

#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"


namespace Asset
{
    //! Stores data related to an Http asset transfer that is in progress. Not necessary to clients of the AssetModule.
    class HttpAssetTransfer
    {
    public:
        //! Constructor
        HttpAssetTransfer();
        //! Destructor
        ~HttpAssetTransfer();
        
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
        
        //! Inserts a request tag
        void InsertTag(Core::request_tag_t tag) { tags_.push_back(tag); }
        
        //! Inserts several request tags
        void InsertTags(const Core::RequestTagVector tags) { tags_.insert(tags_.end(), tags.begin(), tags.end()); }
        
        //! Clears request tags
        void ClearTags() { tags_.clear(); } 
             
        //! Returns associated request tags
        const Core::RequestTagVector& GetTags() const { return tags_; }
                          
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

		//void Update(Core::f64 frametime);


		//! Send http GET request and open stream for response data
		void StartTransfer();

		//! return stream for reading http response
		std::istream* GetResponseStream();

		//! return next packet id 
		int GetNextPacketId();
        
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
        
        //! List of request tags associated with this transfer
        Core::RequestTagVector tags_;

		//! Http session from asset fetch
		Poco::Net::HTTPClientSession http_session_;

		//! Http request for asset fetch
		Poco::Net::HTTPRequest http_request_;

		//! Http response for http_request_ 
		Poco::Net::HTTPResponse http_response_;

		//! input stream for reading http response data
		std::istream* response_stream_;

		//! For data assembly we need id for every data chunk received from server
		//! This start from 0 and increments by 1 for every time GetNextPacketId is called
		int packet_id_;
    };

} // end of namespace: Asset

#endif // incl_Asset_HttpAssetTransfer_h
