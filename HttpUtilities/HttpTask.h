// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_HttpUtilities_HttpTask_h__
#define incl_HttpUtilities_HttpTask_h__

#include "ThreadTask.h"
#include "HttpRequest.h"

namespace HttpUtilities
{
    //! Data structure for http task requests
    class HttpTaskRequest : public Foundation::ThreadTaskRequest
    {
    public:
        HttpTaskRequest() :
            timeout_(5.0f),
            method_(HttpUtilities::HttpRequest::Get)
        {
        }
        
        //! Url
        std::string url_;
        //! Http method
        HttpUtilities::HttpRequest::Method method_;
        //! Connection timeout
        float timeout_;
        //! Data content type. Only has significance if data exists
        std::string content_type_;
        //! Data to be sent in the request
        std::vector<u8> data_;
    };
    
    typedef boost::shared_ptr<HttpTaskRequest> HttpTaskRequestPtr;

    //! Data structure for http task results
    class HttpTaskResult : public Foundation::ThreadTaskResult
    {
    public:
        //! Success
        bool success_;
        //! Reason for error (if any)
        std::string reason_;
        //! Response data
        std::vector<u8> data_;
        
        bool GetSuccess() const { return success_; }
    };

    typedef boost::shared_ptr<HttpTaskResult> HttpTaskResultPtr;

    // Performs threaded http request(s)
    class HttpTask : public Foundation::ThreadTask
    {
    public:
        //! Constructor with default task description HttpRequest, non-continuous mode
        HttpTask();
        
        //! Constructor with custom task description
        /*! \param task_description Task description
            \param continuous Continuous mode on/off, default off
         */
        HttpTask(const std::string& task_description, bool continuous = false);
        
        //! Sets continuous mode on/off. Default is off (no ThreadTaskManager needed, one-shot request/response)
        /*! \param enable Continuous mode setting
         */
        void SetContinuous(bool enable);
        
        //! Gets continuous mode
        bool GetContinuous() const { return continuous_; }

    protected:
        //! Work function, performs http request(s)
        virtual void Work();

    private:
        //! Continuous mode flag
        bool continuous_;
    };
    
    typedef boost::shared_ptr<HttpTask> HttpTaskPtr;
}

#endif // incl_HttpUtilities_HttpTask_h__