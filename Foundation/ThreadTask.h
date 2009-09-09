// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ThreadTask_h
#define incl_Foundation_ThreadTask_h

#include "EventDataInterface.h"

namespace Foundation
{
    class ThreadTaskManager;
    
    //! Base class for a threaded work request. Subclass and add needed variables.
    class ThreadTaskRequest
    {
    public:
        ThreadTaskRequest()  {}
        virtual ~ThreadTaskRequest() {}
    };

    typedef boost::shared_ptr<ThreadTaskRequest> ThreadTaskRequestPtr;
    
    //! Base class for a threaded work result. Subclass and add needed variables.
    class ThreadTaskResult : public EventDataInterface
    {
    public:
        std::string task_description_;
    };
    
    typedef boost::shared_ptr<ThreadTaskResult> ThreadTaskResultPtr;
    
    //! A class for performing threaded work.
    /*! Subclass to use and implement the Work() function. The work can either be 
        - one-shot, use SetResult() and terminate work thread
        - continuous, use QueueResult() to queue results to the thread task manager, while work thread keeps running
          In this case a task manager is needed to post results to, otherwise results will be lost
     */
    class ThreadTask
    {
    public:
        //! Constructor
        /*! \param task_description Description of the work this thread will be doing. Should be unique,
            if work requests are to be communicated via the foundation's default ThreadTaskManager
         */
        ThreadTask(const std::string& task_description);
        
        //! Destructor
        /*! Calls Stop(). Note that in subclass destructors, it would be safest to call Stop() first, at least before
            accessing any data that the still running work thread may also be accessing
         */
        virtual ~ThreadTask();
        
        //! Returns task description.
        const std::string& GetTaskDescription() { return task_description_; }
        
        //! Adds a work request and starts the work thread if not running
        void AddRequest(ThreadTaskRequestPtr request);
        
        template <class T> void AddRequest(boost::shared_ptr<T> request)
        {
            AddRequest(boost::dynamic_pointer_cast<ThreadTaskRequest>(request));
        }
        
        //! Gets final result from a completed (stopped) work thread. If result not yet ready, returns null
        ThreadTaskResultPtr GetResult() const;
        
        template <class T> boost::shared_ptr<T> GetResult() const
        {
            return boost::dynamic_pointer_cast<T>(GetResult());
        }
        
        //! Checks if work thread is currently running
        bool IsRunning() const { return running_; }

        //! Checks if work thread has been run & finished
        bool HasFinished() const { return finished_; }
        
        //! Commands the work thread to stop after current iteration is complete (continuous tasks only)
        void Stop();
        
        //! Thread entry point
        void operator()();
        
        //! Sets task manager. Needs to be set to use queued results, otherwise they will be lost
        void SetThreadTaskManager(ThreadTaskManager* manager) { task_manager_ = manager; }
        
    protected:
        //! Performs work thread activity.
        /*! Note: if doing a loop, check the keep_running_ variable and terminate when it turns false
         */
        virtual void Work() = 0;
        
        //! Waits for request queue to contain at least one item, or keep_running_ to become false
        /*! \return true if a request did arrive, false if keep_running_ becomes false
         */
        bool WaitForRequests();
        
        //! Gets the next request from the request queue. Returns 0 if queue empty
        ThreadTaskRequestPtr GetNextRequest();
        
        template <class T> boost::shared_ptr<T> GetNextRequest()
        {
            return boost::dynamic_pointer_cast<T>(GetNextRequest());
        }
        
        //! Queues result to ThreadTaskManager. Call from work thread. If manager not set, does nothing and logs error.
        void QueueResult(ThreadTaskResultPtr result);
        
        template <class T> void QueueResult(boost::shared_ptr<T> result)
        {
            QueueResult(boost::dynamic_pointer_cast<ThreadTaskResult>(result));
        }
        
        //! Sets final result. Call from work thread. Work thread should terminate after this.
        void SetResult(ThreadTaskResultPtr result);
        
        template <class T> void SetResult(boost::shared_ptr<T> result)
        {
            SetResult(boost::dynamic_pointer_cast<ThreadTaskResult>(result));
        }
        
        //! Task description
        std::string task_description_;
        //! Mutex for request queue
        Core::Mutex request_mutex_;
        //! Mutex for result
        Core::Mutex result_mutex_;
        //! Condition for request queue
        Core::Condition request_condition_;
        //! Request queue
        std::list<ThreadTaskRequestPtr> requests_;
        //! Work thread
        Core::Thread thread_;
        //! Thread task manager, collects queued results
        ThreadTaskManager* task_manager_;
        //! Final result, available when work finished
        ThreadTaskResultPtr result_;
        //! Keep running-flag
        bool keep_running_;
        //! Running flag
        bool running_;
        //! Finished flag
        bool finished_;
    };
    
    typedef boost::shared_ptr<ThreadTask> ThreadTaskPtr;
}

//! Events
namespace Task
{
    namespace Events
    {
        //! Sent when a work request is completed with either success or failure. Uses the event data structure ThreadTaskResult and its subclasses
        static const Core::event_id_t REQUEST_COMPLETED = 1;
    }
}

#endif // incl_Foundation_ThreadTask_h
