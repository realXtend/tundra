// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "HttpAssetTransfer.h"
#include "AssetModule.h"

#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPStream.h>

namespace Asset
{
    HttpAssetTransfer::HttpAssetTransfer() :
        buffer_(0),
        asset_id_(""),
        asset_data_uri_(""),
        asset_metadata_uri_(""),
        asset_type_(0),
        response_size_(0),
        received_count_(0),
        time_(0.0),
        response_stream_(0),
        failed_(false),
        metadata_fetched_(false),
        data_fetched_(false),
        fetching_metadata_(true)
    {
        buffer_ = new u8[BUFFER_SIZE];
    }

    void HttpAssetTransfer::StartTransfer()
    {
        SendHttpGetAssetRequest(asset_metadata_uri_);
    }
    
    HttpAssetTransfer::~HttpAssetTransfer()
    {
        if (buffer_)
        {
            delete [] buffer_;
            buffer_ = 0;
        }
    }

    void HttpAssetTransfer::SendHttpGetAssetRequest(const std::string &resource_uri)
    {
        if (http_session_.connected())
        {
            http_session_.abort();
            http_session_.detachSocket();
        }

        received_count_ = 0;

        Poco::URI uri(resource_uri);
        std::string path(uri.getPathAndQuery());
        if (path.empty())
            path = "/";

        http_session_.setHost(uri.getHost());
        http_session_.setPort(uri.getPort());
        Poco::Timespan time_out(HTTP_TIMEOUT_MS*1000);
        http_session_.setTimeout(time_out);

        Poco::Net::HTTPRequest request;
        request.setMethod(Poco::Net::HTTPRequest::HTTP_GET);
        request.setURI(path);
        request.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);

        try
        {
            http_session_.sendRequest(request);
            std::istream &s = http_session_.receiveResponse(http_response_);
            Poco::Net::HTTPResponse::HTTPStatus status = http_response_.getStatus();
            switch (status)
            {
            case Poco::Net::HTTPResponse::HTTP_OK:
                response_stream_ = &s;
                response_size_ = http_response_.getContentLength();
                break;

            default:
                std::string reason = http_response_.getReasonForStatus(status);
                std::stringstream error;
                error << "Http GET failed for: ";
                error << resource_uri << std::endl;
                error << "Reason: " << reason;
                AssetModule::LogError(error.str());
                failed_ = true;
                return;
            }
        }
        catch (const Poco::Exception &e)
        {
            std::stringstream error;
            error << "Http GET failed for: ";
            error << resource_uri << std::endl;
            error << "Reason: " << e.displayText();
            AssetModule::LogError(error.str());
            failed_ = true;
            return;
        }
    }

    void HttpAssetTransfer::SendHttpPostAssetRequest(const std::string &host, const std::string &json_data)
    {
        std::string ASSET_UPLOADING_CONTENT_TYPE = "application/json";
        int content_length = json_data.length();
        std::istringstream stream(json_data);

        int send_count = 0; //\todo make member var

        Poco::URI uri(host);
        std::string path(uri.getPathAndQuery());
        if (path.empty())
            path = "/";

        http_session_.setHost(uri.getHost());
        http_session_.setPort(uri.getPort());

        Poco::Timespan time_out(HTTP_TIMEOUT_MS*1000);

        Poco::Net::HTTPRequest request;
        request.setMethod(Poco::Net::HTTPRequest::HTTP_POST);
        std::string t = uri.toString();
        request.setURI(uri.getPath());
        request.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
        request.setContentType(ASSET_UPLOADING_CONTENT_TYPE);
        request.setContentLength(content_length);
        request.setKeepAlive(false);

        try
        {
            std::ostream &request_body = http_session_.sendRequest(request);
            request_body.write(json_data.c_str(), json_data.length());
        
            std::istream &s = http_session_.receiveResponse(http_response_);
            
            Poco::Net::HTTPResponse::HTTPStatus status = http_response_.getStatus();
            switch (status)
            {
            case Poco::Net::HTTPResponse::HTTP_OK:
                response_stream_ = &s;
                response_size_ = http_response_.getContentLength();
                break;

            default:
                std::string reason = http_response_.getReasonForStatus(status);
                std::stringstream error;
                error << "Http POST failed for: ";
                error << host << std::endl;
                error << "Reason: " << reason;
                AssetModule::LogError(error.str());
                failed_ = true;
                return;
            }
        }
        catch (Poco::Net::MessageException &e)
        {
            std::string u = request.getURI();
            std::string m = request.getMethod();
            std::string error = e.message();
            
        }
        catch (Poco::Exception &e)
        {
            std::stringstream error;
            error << "Http POST failed for: ";
            error << host << std::endl;
            error << "Reason: " << e.displayText();
            AssetModule::LogError(error.str());
            failed_ = true;
            return;
        }
    }

    bool HttpAssetTransfer::Ready() const
    {
        if (metadata_fetched_ && data_fetched_)
            return true;
        else
            return false;
    }

    uint HttpAssetTransfer::GetSize() const
    {
        if (!metadata_fetched_)
            return 0;
        else
            return response_size_;
    }
    
    uint HttpAssetTransfer::GetReceivedContinuous() const
    {
        return received_data_.size();
    }
    
    void HttpAssetTransfer::ReceiveData(const u8* data, uint size)
    {
        time_ = 0.0;
        
        if (!size)
        {
            AssetModule::LogWarning("Trying to store zero bytes of data");
            return;
        }
        
        int new_size = received_data_.size() + size;
        received_data_.resize(new_size);
        memcpy(&received_data_[received_data_.size()-size], data, size);
        received_count_ += size;
    }
    
    void HttpAssetTransfer::ReceiveMetadata(const u8* data, uint size)
    {
        time_ = 0.0;
        
        if (!size)
        {
            AssetModule::LogWarning("Trying to store zero bytes of metadata");
            return;
        }
        
        int new_size = received_metadata_.size() + size;
        received_metadata_.resize(new_size);
        memcpy(&received_metadata_[received_metadata_.size()-size], data, size);
        received_count_ += size;
    }

    void HttpAssetTransfer::AssembleData(u8* buffer) const
    {
        memcpy(buffer, &received_data_[0], received_data_.size());
    }

    std::string HttpAssetTransfer::GetAssetMetadata()
    {
        std::string metadata(received_metadata_.begin(), received_metadata_.end());
        return metadata;
    }

    void HttpAssetTransfer::AssembleMetadata(u8* buffer) const
    {
        memcpy(buffer, &received_metadata_[0], received_metadata_.size());
    }

    void HttpAssetTransfer::SetAssetId(const std::string& asset_id)
    {
        asset_id_ = asset_id;
        asset_data_uri_ = asset_id+"/data";
        asset_metadata_uri_ = asset_id+"/metadata";
    }

    bool HttpAssetTransfer::IsFailed() const
    {
        return failed_;
    }

    void HttpAssetTransfer::Update(f64 frametime)
    {
        int received = 0;

        response_stream_->rdbuf()->pubsync();
        int count = response_stream_->rdbuf()->in_avail();
        count = BUFFER_SIZE; //\hack, in_avail method doesn't work and returns always zero
            
        if (count > BUFFER_SIZE)
            count = BUFFER_SIZE;

        response_stream_->read((char*)(buffer_), count);
        received = response_stream_->gcount();

        if (fetching_metadata_)
            ReceiveMetadata(buffer_, received);
        else
            ReceiveData(buffer_, received);
        

        if (response_stream_->eof())
        {
            if (received_count_ == response_size_)
            {
                if (fetching_metadata_)
                    metadata_fetched_ = true;
                else
                    data_fetched_ = true;
            }
            else
            {
                std::stringstream error;
                error << "Cannot read all data from http response stream. Received " << received_count_ << "/" << response_size_ << "bytes";
                AssetModule::LogError(error.str());
                failed_ = true;
            }

            if (metadata_fetched_ && !data_fetched_)
            {
                fetching_metadata_ = false;
                SendHttpGetAssetRequest(asset_data_uri_);
            }
        }
    }
}
