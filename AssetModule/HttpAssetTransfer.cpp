#include "StableHeaders.h"

#include "Poco/StreamCopier.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"

#include "HttpAssetTransfer.h"
#include "AssetModule.h"

#include "Poco/Net/HTTPStream.h"

namespace Asset
{
    HttpAssetTransfer::HttpAssetTransfer() :
        size_(0),
        received_(0),
        time_(0.0),
		response_stream_(NULL),
		failed_(false)
    {
    }

	void HttpAssetTransfer::StartTransfer()
	{
		buffer_ = new Core::u8[BUFFER_SIZE];

		Poco::URI uri(asset_id_);
		std::string path(uri.getPathAndQuery());
		if (path.empty())
			path = "/";

		http_session_.setHost(uri.getHost());
		http_session_.setPort(uri.getPort());
		Poco::Timespan time_out(HTTP_TIMEOUT_MS*1000);
		http_session_.setTimeout(time_out);
		
		http_request_.setMethod(Poco::Net::HTTPRequest::HTTP_GET);
		http_request_.setURI(path);
		http_request_.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);

		try
		{
			http_session_.sendRequest(http_request_);
			std::istream &s = http_session_.receiveResponse(http_response_);
			response_stream_ = &s;

			int data_size = http_response_.getContentLength();
			SetSize(data_size);
		}
		catch (Poco::Exception e)
		{
			failed_ = true;
			return;
		}
	}
    
    HttpAssetTransfer::~HttpAssetTransfer()
    {
		if (buffer_)
		{
			delete [] buffer_;
			buffer_ = NULL;
		}
    }
    
    bool HttpAssetTransfer::Ready() const
    {
		if (!response_stream_)
			return false;

		if (response_stream_->eof())
			return true;
		else
			return false;
    }
    
    Core::uint HttpAssetTransfer::GetReceivedContinuous() const
    {
		return received_data_.size();
    }
    
    void HttpAssetTransfer::ReceiveData(const Core::u8* data, Core::uint size)
    {
        time_ = 0.0;
        
        if (!size)
        {
            AssetModule::LogWarning("Trying to store zero bytes of data");
            return;
        }
        
		int new_size = received_data_.size() + size;
		received_data_.resize(new_size);
		memcpy(&received_data_[0], data, size);
        received_ += size;
    }
    
    void HttpAssetTransfer::AssembleData(Core::u8* buffer) const
    {
        memcpy(buffer, &received_data_[0], received_data_.size());
    }

	bool HttpAssetTransfer::IsFailed()
	{
		return failed_;
	}

	void HttpAssetTransfer::Update(Core::f64 frametime)
	{
		int received = 0;

		response_stream_->rdbuf()->pubsync();
		int count = response_stream_->rdbuf()->in_avail();
		count = BUFFER_SIZE; //\hack, in_avail method doesn't work and returns always zero
			
		if (count > BUFFER_SIZE)
			count = BUFFER_SIZE;

		response_stream_->read((char*)(buffer_), count);
		received = response_stream_->gcount();
		ReceiveData(buffer_, received);
	}
}
