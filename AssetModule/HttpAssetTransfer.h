// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_HttpAssetTransfer_h
#define incl_Asset_HttpAssetTransfer_h

#include "CoreTypes.h"

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPResponse.h>

namespace Asset
{
    //! Stores data related to an Http asset transfer that is in progress. Not necessary to clients of the AssetModule.
    class HttpAssetTransfer
    {
    private:
        /**
         *  Bytes read from http server per Update method call per http request
         */
        static const int BUFFER_SIZE = 1024; 

        /**
         *  Timeout in ms for http get
         */
        static const int HTTP_TIMEOUT_MS = 2000;

    public:
        //! Constructor
        HttpAssetTransfer();

        //! Destructor
        ~HttpAssetTransfer();
        
        //! Receives an asset data 
        /*! Also resets elapsed time
            \param data Pointer to data
            \param size Size of data packet
         */
        void ReceiveData(const u8* data, uint size);

        //! Receives an asset metadata 
        /*! Also resets elapsed time
            \param data Pointer to data
            \param size Size of data packet
         */
        void ReceiveMetadata(const u8* data, uint size);
        
        //! Assembles continuous asset data to a buffer
        /*! Call GetReceivedContinuous() (or GetReceived() if you know the transfer is complete)
            first to know how big the buffer must be
            \param buffer Pointer to buffer that will receive data
         */
        void AssembleData(u8* buffer) const;
        
        //! Assembles continuous asset metadata to a buffer
        /*! Call GetReceivedMetadata() if you know the transfer is complete
            first to know how big the buffer must be
            \param buffer Pointer to buffer that will receive data
         */
        void AssembleMetadata(u8* buffer) const;

        //! Sets asset ID
        /*! \param asset_id Asset id
         */
        void SetAssetId(const std::string& asset_id);

        //! Sets asset type
        /*! \param asset_type Asset type
         */
        void SetAssetType(uint asset_type) { asset_type_ = asset_type; }
        
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
        uint GetSize() const; 

        //! Returns total size of data received so far
        uint GetReceived() const { return received_data_.size(); }

        //! Returns asset metadata as string
        std::string GetAssetMetadata();

        //! Returns total size of continuous data from the asset beginning received so far
        uint GetReceivedContinuous() const;

        //! Returns elapsed time since last packet
        f64 GetTime() const { return time_; }

        //! Returns whether transfer is finished (all bytes received)
        bool Ready() const;

        //! Send http GET request and open stream for response data
        void StartTransfer();

        //! Return error state
        bool IsFailed() const;

        //! Read bytes from response stream 
        //! /bug Current implementatation read always fixed amount of bytes (BUFFER_SIZE) and blocks while reading those bytes.
        //!      Hostile server can stop viewer main loop using this security hole
        //!      We have to change this so that we only read available bytes.
        void Update(f64 frametime);

        //! Send http POST request to store asset to asset service
        void SendHttpPostAssetRequest(const std::string &host, const std::string &json_data);        

    private:
        /**
         *  Send http request with given uri.
         *  Will cancel any exist request for same asset (there is 2 request per asset: metadata and data requests)
         */
        void SendHttpGetAssetRequest(const std::string &resource_uri);

        //! Downloaded binary data of asset
        typedef std::vector<u8> DataVector;

        //! Asset ID
        std::string asset_id_; 

        //! uri for asset data
        std::string asset_data_uri_;

        //! uri for asset metadata
        std::string asset_metadata_uri_;

        //! Asset type
        uint asset_type_;

        ////! Expected http response data size
        uint response_size_;

        //! Received bytes
        uint received_count_;

        //! Received data
        DataVector received_data_;

        //! Received metadata
        DataVector received_metadata_;

        //! Elapsed time since last packet
        f64 time_;

        //! List of request tags associated with this transfer
        RequestTagVector tags_;

        //! Http session from asset fetch
        Poco::Net::HTTPClientSession http_session_;

        //! Http response for http_request_ 
        Poco::Net::HTTPResponse http_response_;

        //! input stream for reading http response data
        std::istream* response_stream_;

        //! error flag, true if http get has been failded
        bool failed_;

        //! response stream read buffer
        const u8 *buffer_;

        //! true if asset metadata was successfully fetched from server
        bool metadata_fetched_;

        //! true if asset data was successfully fetched from server
        bool data_fetched_;

        //! true if asset metadata is under progress
        //! false if asset data is under progress
        bool fetching_metadata_;
    };
} // end of namespace: Asset

#endif // incl_Asset_HttpAssetTransfer_h
