// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "HttpTask.h"

namespace HttpUtilities
{
    HttpTask::HttpTask() :
        Foundation::ThreadTask("HttpRequest"),
        continuous_(false)
    {
    }

    HttpTask::HttpTask(const std::string& task_description, bool continuous) :
        Foundation::ThreadTask(task_description),
        continuous_(continuous)
    {
    }
    
    void HttpTask::SetContinuous(bool enable)
    {
        continuous_ = enable;
    }

    void HttpTask::Work()
    {
        while (ShouldRun())
        {
            WaitForRequests();
            
            boost::shared_ptr<HttpTaskRequest> request = GetNextRequest<HttpTaskRequest>();
            if (request)
            {
                HttpUtilities::HttpRequest http;
                http.SetUrl(request->url_);
                http.SetMethod(request->method_);
                http.SetTimeout(request->timeout_);
                if (request->data_.size())
                {
                    http.SetRequestData(request->content_type_, request->data_);
                }
                
                http.Perform();
                
                boost::shared_ptr<HttpTaskResult> result(new HttpTaskResult());
                result->tag_ = request->tag_;
                result->success_ = http.GetSuccess();
                result->reason_ = http.GetReason();
                result->data_ = http.GetResponseData();
                
                if (continuous_)
                    QueueResult<HttpTaskResult>(result);
                else
                    SetResult<HttpTaskResult>(result);
            }
            
            if (!continuous_)
                break;
        }
    }
    
}