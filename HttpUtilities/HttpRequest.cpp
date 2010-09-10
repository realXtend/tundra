// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "HttpRequest.h"
#include "ForwardDefines.h"

#include "curl/curl.h"

namespace HttpUtilities
{

    // Writer callback for cURL.
    size_t WriteCallback(char *data, size_t size, size_t nmemb, std::vector<u8>* buffer)
    {  
        if (buffer)
        {
            buffer->insert(buffer->end(), data, data + size * nmemb);
            return size * nmemb;
        }
        else
            return 0;
    }

    HttpRequest::HttpRequest() :
        method_(Get),
        success_(false),
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
    
    void HttpRequest::SetTimeout(float seconds)
    {
        if (seconds < 0.0f)
            seconds = 0.0f;
        timeout_ = seconds;
    }
    
    void HttpRequest::SetMethod(Method method)
    {
        method_ = method;
    }
    
    void HttpRequest::SetRequestData(const std::string& content_type, const std::vector<u8>& data)
    {
        request_data_ = data;
        content_type_ = content_type;
    }
    
    void HttpRequest::SetRequestData(const std::string& content_type, const char* data)
    {
        if (!data)
            return;
        
        uint size = strlen(data);
        request_data_.resize(size);
        memcpy(&request_data_[0], data, size);
        
        content_type_ = content_type;
    }
    
    void HttpRequest::SetRequestData(const std::string& content_type, const std::string& data)
    {
        uint size = data.length();
        request_data_.resize(size);
        memcpy(&request_data_[0], data.c_str(), size);
        
        content_type_ = content_type;
    }
    
    void HttpRequest::Perform()
    {
        success_ = false;
        reason_ = std::string();
        response_data_.clear();
        
        CURL* curl = curl_easy_init();
        if (!curl)
        {
            Foundation::RootLogError("Null curl handle");
            return;
        }
        
        CURLcode result;
        curl_slist *headers = 0;
        char curlerror[256];
        
        if (request_data_.size())
        {
            std::string content_type_str = "Content-Type: " + content_type_;
            headers = curl_slist_append(headers, content_type_str.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, &request_data_[0]);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request_data_.size());
            if (method_ == Put)
                curl_easy_setopt(curl, CURLOPT_PUT, 1);
            if (method_ == Post)
                curl_easy_setopt(curl, CURLOPT_POST, 1);
        }
        
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, (int)timeout_);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data_);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlerror);
        
        result = curl_easy_perform(curl);
        if (result != CURLE_OK)
            reason_ = std::string(curlerror);
        else
            success_ = true;

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}
