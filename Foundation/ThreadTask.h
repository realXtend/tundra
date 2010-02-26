// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ThreadTask_h
#define incl_Foundation_ThreadTask_h

#include "EventDataInterface.h"
#include "CoreTypes.h"
#include "CoreThread.h"

namespace Foundation
{
    class ThreadTaskManager;
    
    //! Base class for a threaded work request. Subclass and add needed variables.
    class ThreadTaskRequest
    {
    public:
        ThreadTaskRequest() {}
        virtual ~ThreadTaskRequest() {}
        
        //! Request tag. Assigned when queuing the request & returned to caller.
        /*! Note: assigned by a ThreadTaskManager, not by ThreadTask itself
         */
        request_tag_t tag_;
    };

    typedef boost::shared_ptr<ThreadTaskRequest> ThreadTaskRequestPtr;
    
    //! Base class for a threaded work result. Subclass and add needed variables.
    class ThreadTaskResult : public EventDataInterface
    {
    public:
        //! Request tag. Should be copied from the request to match the result to request
        request_tag_t tag_;
        
        //! Task description (which kind of task produced the result)
        std::string task_description_;
    };
    
    typedef boost::shared_ptr<ThreadTaskResult> ThreadTaskResultPtr;
    
    //! A class for performing threaded work.
    /*! Subclass to use and implement the Work() function. The work can either be 
        - one-shot, use SetResult() and terminate work thread
        - continuous, use QueueResult() to queue results to the thread task manager, while work thread keeps running
          In this mode a thread task manager is needed to post results to, otherwise results will be lost
     */
    class ThreadTask
    {
        friend class ThreadTaskManager;
        
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
        
        //! Template version of adding a work request. Performs dynamic_pointer_cast from the type specified.
        template <class T> void AddRequest(boost::shared_ptr<T> request)
        {
            return AddRequest(boost::dynamic_pointer_cast<ThreadTaskRequest>(request));
        }
        
        //! Gets final result from a completed (stopped) work thread.
        /*! \return Result, or 0 if thread still running or has not produced a result
         */
        ThreadTaskResultPtr GetResult() const;
        
        //! Template version of getting final result. Performs dynamic_pointer_cast to the type specified.
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
        
    protected:
        //! Performs work thread activity.
        /*! Note: if doing a loop, check ShouldRun() function and terminate when it returns false
         */
        virtual void Work() = 0;
        
        //! Waits for request queue to contain at least one item, or ShouldRun() becomes false
        /*! \return true if a request did arrive, false if ShouldRun() becomes false
         */
        bool WaitForRequests();
        
        //! Gets the next request from the request queue. Returns 0 if queue empty
        ThreadTaskRequestPtr GetNextRequest();
        
        //! Template version of getting next request. Performs dynamic_pointer_cast to the type specified.
        template <class T> boost::shared_ptr<T> GetNextRequest()
        {
            return boost::dynamic_pointer_cast<T>(GetNextRequest());
        }
        
        //! Sets final result for one-shot tasks.
        /*! Call from Work() function, which should return after that.
            \param result Final result
         */
        void SetResult(ThreadTaskResultPtr result);
        
        //! Template version of storing final result.
        /*! Performs dynamic_pointer_cast from specified type to ThreadTaskResult.
            \param result Final result
         */
        template <class T> void SetResult(boost::shared_ptr<T> result)
        {
            SetResult(boost::dynamic_pointer_cast<ThreadTaskResult>(result));
        }
        
        //! Queues result to ThreadTaskManager, for continuous tasks.
        /*! Call from Work() function.
            \param result Result to be queued
            \return True if successful (a thread task manager has been set), false if not
         */
        bool QueueResult(ThreadTaskResultPtr result);
        
        //! Template version of queuing a result.
        /*! Performs dynamic_pointer_cast from specified type to ThreadTaskResult.
            \param result Result to be queued
         */
         
        template <class T> bool QueueResult(boost::shared_ptr<T> result)
        {
            return QueueResult(boost::dynamic_pointer_cast<ThreadTaskResult>(result));
        }
        
        //! Returns thread task manager
        ThreadTaskManager* GetThreadTaskManager() const { return task_manager_; }
        
        //! Whether should keep running the continuous work loop
        bool ShouldRun() const { return keep_running_; }
        
    private:
        //! Sets task manager. Needs to be set to use queued results, otherwise they will be lost
        /*! \param manager Task manager
         */
        void SetThreadTaskManager(ThreadTaskManager* manager) { task_manager_ = manager; }
        
        //! Task description
        std::string task_description_;
        //! Mutex for request queue
        Mutex request_mutex_;
        //! Mutex for result
        Mutex result_mutex_;
        //! Condition for request queue
        Condition request_condition_;
        //! Request queue
        std::list<ThreadTaskRequestPtr> requests_;
        //! Work thread
        Thread thread_;
        //! Final result, available when work finished
        ThreadTaskResultPtr result_;
        //! Thread task manager, collects queued results
        ThreadTaskManager* task_manager_;
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
        //! Sent when a work result has arrived. Uses the event data structure ThreadTaskResult and its subclasses
        static const event_id_t REQUEST_COMPLETED = 1;
    }
}

#endif // incl_Foundation_ThreadTask_h
