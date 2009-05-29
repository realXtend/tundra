#include "StableHeaders.h"

#include "Poco/URIStreamOpener.h"
#include "Poco/StreamCopier.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/Net/FTPStreamFactory.h"


#include "HttpAssetTransfer.h"
#include "AssetModule.h"

namespace Asset
{
    HttpAssetTransfer::HttpAssetTransfer() :
        size_(0),
        received_(0),
        time_(0.0),
		packet_id_(0),
		response_stream_(NULL)
    {
    }

	void HttpAssetTransfer::StartTransfer()
	{
		Poco::URI uri(asset_id_);
		std::string path(uri.getPathAndQuery());
		if (path.empty())
			path = "/";

		http_session_.setHost(uri.getHost());
		http_session_.setPort(uri.getPort());

		http_request_.setMethod(Poco::Net::HTTPRequest::HTTP_GET);
		http_request_.setURI(path);
		http_request_.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
		http_session_.sendRequest(http_request_);
		http_session_.receiveResponse(http_response_);
		
		int data_size = http_response_.getContentLength();
		SetSize(data_size);

		response_stream_ = Poco::URIStreamOpener::defaultOpener().open(uri);
	}
    
    HttpAssetTransfer::~HttpAssetTransfer()
    {
    }
    
    bool HttpAssetTransfer::Ready() const
    {
        if (!size_) 
            return false; // No header received, size not known yet
        
        return received_ >= size_;
    }
    
    Core::uint HttpAssetTransfer::GetReceivedContinuous() const
    {
        Core::uint size = 0;
        
        DataPacketMap::const_iterator i = data_packets_.begin();
        
        Core::uint expected_index = 0;
      
        while (i != data_packets_.end())
        {
            if (i->first != expected_index)
                break;
            
            size += i->second.size();
            
            ++expected_index;
            ++i;
        }
        
        return size;
    }
    
    void HttpAssetTransfer::ReceiveData(Core::uint packet_index, const Core::u8* data, Core::uint size)
    {
        time_ = 0.0;
        
        if (!size)
        {
            AssetModule::LogWarning("Trying to store zero bytes of data");
            return;
        }
        
        if (!data_packets_[packet_index].size())
        {
            data_packets_[packet_index].resize(size);
            memcpy(&data_packets_[packet_index][0], data, size);
            received_ += size;
        }
        else
        {
            AssetModule::LogWarning("Already received asset data packet index " + Core::ToString<Core::uint>(packet_index));
        }
    }
    
    void HttpAssetTransfer::AssembleData(Core::u8* buffer) const
    {
        DataPacketMap::const_iterator i = data_packets_.begin();
        
        Core::uint expected_index = 0;
      
        while (i != data_packets_.end())
        {
            if (i->first != expected_index)
                break;
            
            memcpy(buffer, &i->second[0], i->second.size());
            buffer += i->second.size();
            
            ++expected_index;
            ++i;
        }
    }


	std::istream* HttpAssetTransfer::GetResponseStream()
	{
		return response_stream_;
	}


	int HttpAssetTransfer::GetNextPacketId()
	{
		int i = packet_id_;
		packet_id_++;
		return i;
	}

}
