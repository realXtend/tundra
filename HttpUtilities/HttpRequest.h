// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_HttpUtilities_HttpRequest_h__
#define incl_HttpUtilities_HttpRequest_h__

#include "CoreTypes.h"

namespace HttpUtilities
{
    //! Performs a blocking http request
    class HttpRequest
    {
    public:
        //! Http methods
        enum Method
        {
            Get,
            Put,
            Post
        };
        
        HttpRequest();
        ~HttpRequest();
        
        //! Sets url. If protocol not specified, http:// is used. If path is empty, / will be used.
        /*! \param url Url
         */
        void SetUrl(const std::string& url);
        
        //! Sets method. Default method is get
        /*! \param method Method
         */
        void SetMethod(Method method);
        
        //! Sets request data from a vector
        /*! \param content_type Content type string
            \param data Data
         */
        void SetRequestData(const std::string& content_type, const std::vector<u8>& data);
        
        //! Sets request data from a string
        /*! \param content_type Content type string
            \param data Data
         */
        void SetRequestData(const std::string& content_type, const std::string& data);
        
        //! Sets request data from a C string. Null terminator is not included.
        /*! \param content_type Content type string
            \param data Data
         */
        void SetRequestData(const std::string& content_type, const char* data);
        
        //! Sets timeout for request. Default is 5 seconds.
        /*! \param seconds Timeout in seconds
         */
        void SetTimeout(float seconds);
        
        //! Performs the request
        void Perform();
        
        //! Returns url
        const std::string& GetUrl() const { return url_; }
        
        //! Returns method
        Method GetMethod() const { return method_; }
        
        //! Returns request data
        const std::vector<u8>& GetRequestData() const { return request_data_; }
        
        //! Returns timeout
        float GetTimeout() const { return timeout_; }
        
        //! Returns if succeeded
        bool GetSuccess() const { return success_; }
        
        //! Returns reason string if unsuccessful
        const std::string& GetReason() const { return reason_; }
        
        //! Returns reply data
        const std::vector<u8>& GetResponseData() const { return response_data_; }
        
    private:
        //! Url
        std::string url_;
        //! Method.
        Method method_;
        //! Http connection timeout
        float timeout_;
        //! Request data to be send
        std::vector<u8> request_data_;
        //! Request data content type
        std::string content_type_;
        //! Reply data
        std::vector<u8> response_data_;
        //! Request success
        bool success_;
        //! Error reason
        std::string reason_;
    };
}

#endif // incl_HttpUtilities_HttpRequest_h__