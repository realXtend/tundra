// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_HttpUtilities_HttpRequest_h__
#define incl_HttpUtilities_HttpRequest_h__

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
        void SetUrl(const std::string& url);
        
        //! Sets method. Default method is get
        void SetMethod(Method method);
        
        //! Sets request data from a vector
        void SetRequestBody(const std::string& content_type, const std::vector<Core::u8>& body);
        
        //! Sets request data from a string
        void SetRequestBody(const std::string& content_type, const std::string& body);
        
        //! Sets request data from a C string. Null terminator is not included.
        void SetRequestBody(const std::string& content_type, const char* body);
        
        //! Sets timeout for request in seconds. Default is 5.
        void SetTimeout(Core::Real seconds);
        
        //! Performs the request
        void Perform();
        
        //! Returns url
        const std::string& GetUrl() const { return url_; }
        
        //! Returns method
        Method GetMethod() const { return method_; }
        
        //! Returns request data
        const std::vector<Core::u8>& GetRequestBody() const { return request_body_; }
        
        //! Returns timeout
        Core::Real GetTimeout() const { return timeout_; }
        
        //! Returns http status after request has been performed.
        /*! Will be 0 if request could not be performed (DNS error, no connection etc.) Will be 200 if successful.
         */
        int GetStatus() const { return status_; }
        
        //! Returns if status is 200 (OK)
        bool GetSuccess() const { return status_ == 200; }
        
        //! Returns reason string that accompanied the status code. 
        /*! If status was 0 (connection failed), will contain the network exception string
         */
        const std::string& GetReason() const { return reason_; }
        
        //! Returns reply data
        const std::vector<Core::u8>& GetResponseBody() const { return response_body_; }
        
    private:
        //! Url
        std::string url_;
        //! Method.
        Method method_;
        //! Http connection timeout
        Core::Real timeout_;
        //! Request data
        std::vector<Core::u8> request_body_;
        //! Request data content type
        std::string content_type_;
        //! Reply data
        std::vector<Core::u8> response_body_;
        //! Reply status code
        int status_;
        //! Reply reason
        std::string reason_;
    };
}

#endif // incl_HttpUtilities_HttpRequest_h__