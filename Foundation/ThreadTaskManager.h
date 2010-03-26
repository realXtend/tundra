// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ThreadTaskManager_h
#define incl_Foundation_ThreadTaskManager_h

#include "ThreadTask.h"

namespace Foundation
{
    class Framework;
    
    //! Manager of ThreadTasks.
    /*! Takes ownership of ThreadTasks to handle results from them. Necessary to use ThreadTasks in queued result mode.
        There exists a system-wide ThreadTaskManager in the framework, but nothing prevents you creating your own additional
        ThreadTaskManager and registering tasks to it instead.
     */
    class ThreadTaskManager
    {
        friend class ThreadTask;
        
    public:
        //! Constructor
        /*! \param framework Framework, needed for sending events
         */
        explicit ThreadTaskManager(Framework* framework);
        
        //! Destructor
        ~ThreadTaskManager();
        
        //! Adds a ThreadTask
        /*! \param task Task to add
            To not lose any queued results, adding the task to the manager should always be done before adding work requests to the task.
         */
        void AddThreadTask(ThreadTaskPtr task);
        
        //! Removes a ThreadTask
        /*! \param task Task to remove
         */
        void RemoveThreadTask(ThreadTaskPtr task);
        
        //! Removes a ThreadTask by task description
        /*! If many tasks with same description, removes the first one
            \param task_description Task description to remove by
         */
        void RemoveThreadTask(const std::string& task_description);
        
        //! Removes all ThreadTasks
        void RemoveThreadTasks();
        
        //! Gets a ThreadTask by task description
        /*! If many tasks with same description, gets the first one
            \param task_description Task description
         */
        ThreadTaskPtr GetThreadTask(const std::string& task_description);
        
        //! Adds a request by task description
        /*! \param task_description Task description
            \param request Task request
            \return a non-zero request tag if request could be fulfilled, zero if not
            Note: currently simply the first matching ThreadTask will be used; there is no load balancing
         */
        request_tag_t AddRequest(const std::string& task_description, ThreadTaskRequestPtr request);
        
        //! Template version of adding request. Perfoms dynamic_pointer_cast to ThreadTaskRequest from specified class.
        /*! \param task_description Task description
            \param request Task request
            \return a non-zero request tag if request could be fulfilled, zero if not
         */
        template <class T> request_tag_t AddRequest(const std::string& task_description, boost::shared_ptr<T> request)
        {
            return AddRequest(task_description, boost::dynamic_pointer_cast<ThreadTaskRequest>(request));
        }
        
        //! Checks for results and sends them as events. Deletes finished ThreadTasks.
        /*! Framework calls this for the system-wide ThreadTaskManager on each run of the main loop.
         */
        void SendResultEvents();
        
        //! Gets all results. Does not send them as events. Deletes finished ThreadTasks.
        std::vector<ThreadTaskResultPtr> GetResults();
        
        //! Gets results matching a certain task description. Does not send them as events. Deletes finished ThreadTasks matching description.
        std::vector<ThreadTaskResultPtr> GetResults(const std::string& task_description);
        
        //! Gets amount of results in queue
        uint GetNumResults();
        
        //! Gets amount of results in queue for certain task type
        uint GetNumResults(const std::string& task_description);
        
    private:
        //! Queues a result. Called from ThreadTask work thread.
        /*! \param result Result to queue
         */
        void QueueResult(ThreadTaskResultPtr result);
        
        //! Owned ThreadTasks
        std::vector<ThreadTaskPtr> tasks_;
        
        //! Result queue
        std::list<ThreadTaskResultPtr> results_;
        
        //! Result queue mutex
        Mutex result_mutex_;
        
        //! Framework
        Framework* framework_;
    };
}

#endif // incl_Foundation_ThreadTaskManager_h
