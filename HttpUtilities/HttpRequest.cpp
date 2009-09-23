// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "HttpRequest.h"

#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"

namespace HttpUtilities
{

    HttpRequest::HttpRequest() :
        method_(Get),
        status_(0),
        timeout_(5.0f)
    {
    }
    
    HttpRequest::~HttpRequest()
    {
    }
    
    void HttpRequest::SetUrl(const std::string& url)
    {
        if (url.find("://") == std::string::npos)
            url_ = "http://" + url;
        else
            url_ = url;
    }
    
    void HttpRequest::SetTimeout(Core::Real seconds)
    {
        if (seconds < 0.0f)
            seconds = 0.0f;
        timeout_ = seconds;
    }
    
    void HttpRequest::SetMethod(Method method)
    {
        method_ = method;
    }
    
    void HttpRequest::SetRequestData(const std::string& content_type, const std::vector<Core::u8>& data)
    {
        request_data_ = data;
        content_type_ = content_type;
    }
    
    void HttpRequest::SetRequestData(const std::string& content_type, const char* data)
    {
        if (!data)
            return;
        
        Core::uint size = strlen(data);
        request_data_.resize(size);
        memcpy(&request_data_[0], data, size);
        
        content_type_ = content_type;
    }
    
    void HttpRequest::SetRequestData(const std::string& content_type, const std::string& data)
    {
        Core::uint size = data.length();
        request_data_.resize(size);
        memcpy(&request_data_[0], data.c_str(), size);
        
        content_type_ = content_type;
    }
    
    void HttpRequest::Perform()
    {
        status_ = 0;
        reason_ = std::string();
        response_data_.clear();
        
        try
        {
            Poco::URI uri(url_);
            Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
            float seconds = floor(timeout_);
            float microseconds = (timeout_ - seconds) * 1000000;
            session.setTimeout(Poco::Timespan((long)seconds, (long)microseconds));

            Poco::Net::HTTPRequest request(Poco::Net::HTTPMessage::HTTP_1_1);
            if (uri.getPathAndQuery().empty())
                request.setURI("/");
            else
                request.setURI(uri.getPathAndQuery());
            
            if (request_data_.size())
            {
                request.setContentLength(request_data_.size());
                request.setContentType(content_type_);
            }
            
            switch(method_)
            {
            case Get:
                request.setMethod(Poco::Net::HTTPRequest::HTTP_GET);
                break;

            case Put:
                request.setMethod(Poco::Net::HTTPRequest::HTTP_PUT);
                break;

           case Post:
                request.setMethod(Poco::Net::HTTPRequest::HTTP_POST);
                break;
            }
            
            std::ostream& request_stream = session.sendRequest(request);
            if (request_data_.size())
            {
                std::vector<Core::u8>::const_iterator i = request_data_.begin();
                while (i != request_data_.end())
                {
                    request_stream.put(*i);
                    ++i;
                }
            }
            
            Poco::Net::HTTPResponse response;
            std::istream& response_stream = session.receiveResponse(response);
            reason_ = response.getReason();
            status_ = (int)response.getStatus();
            
            while (response_stream.good())
            {
                int c = response_stream.get();
                if (response_stream.good())
                    response_data_.push_back(c);
            }
        }
        catch (Poco::Exception& e)
        {
            reason_ = e.displayText();
        }
    }
}
