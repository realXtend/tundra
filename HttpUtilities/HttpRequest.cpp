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
    
    void HttpRequest::SetRequestBody(const std::string& content_type, const std::vector<Core::u8>& body)
    {
        request_body_ = body;
        content_type_ = content_type;
    }
    
    void HttpRequest::SetRequestBody(const std::string& content_type, const char* body)
    {
        if (!body)
            return;
        
        Core::uint size = strlen(body);
        request_body_.resize(size);
        memcpy(&request_body_[0], body, size);
        
        content_type_ = content_type;
    }
    
    void HttpRequest::SetRequestBody(const std::string& content_type, const std::string& body)
    {
        Core::uint size = body.length();
        request_body_.resize(size);
        memcpy(&request_body_[0], body.c_str(), size);
        
        content_type_ = content_type;
    }
    
    void HttpRequest::Perform()
    {
        status_ = 0;
        reason_ = std::string();
        response_body_.clear();
        
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
            
            if (request_body_.size())
            {
                request.setContentLength(request_body_.size());
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
            if (request_body_.size())
            {
                std::vector<Core::u8>::const_iterator i = request_body_.begin();
                while (i != request_body_.end())
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
                    response_body_.push_back(c);
            }
        }
        catch (Poco::Exception& e)
        {
            reason_ = e.displayText();
        }
    }
}