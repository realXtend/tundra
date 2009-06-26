// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_Profiler_h
#define incl_Foundation_Profiler_h

#if defined(_WINDOWS) && defined(PROFILING)
//! Profiles a block of code in current scope. Ends the profiling when it goes out of scope
/*! Name of the profiling block must be unique in the scope, so do not use the name of the function
    as the name of the profiling block!

    \param x Unique name for the profiling block, use without quotes, f.ex. PROFILE(name_of_the_block)
*/
#   define PROFILE(x) Foundation::ProfilerSection x ## __profiler__(#x);

//! Optionally ends the current profiling block
/*! Use when you wish to end a profiling block before it goes out of scope
*/
#   define ELIFORP(x) x ## __profiler__.Destruct();

//! Resets profiling data per frame. Must be called at end of each frame in each thread, otherwise profiling data may be inaccurate or unavailable.
//! \todo Currently RESETPROFILER is called in modules at end of Update(), but that will probably cause mismatched timing data if things are profiled
//!       after the Update() call but still in the same frame, f.ex. when handling events. All profiling data in the main thread should be reset
//!       at the same time, at the end of the main loop. Threads are free to reset whenever they choose, as they have their own frame. -cm
#define RESETPROFILER { Foundation::ProfilerSection::GetProfiler()->ThreadedReset(); }

#else
#   define PROFILE(x)
#   define ELIFORP(x)
#   define RESETPROFILER
#endif

#ifndef _WINDOWS
    typedef int LONGLONG;
#endif



namespace Foundation
{
    class ProfilerNodeTree;

    //! Profiles a block of code using Windows API function QueryPerformanceCounter
    class ProfilerBlock
    {
        friend class ProfilerNode;
        //! default constructor
        ProfilerBlock() {}

    public:
        //! default destructor
        ~ProfilerBlock() {}

        //! Call before using any performance counters
        /*! Returns true if Performance Counter is supported on the current h/w, false otherwise
        */
        static bool QueryCapability();

        void Start()
        {
            if (supported_)
            {
                // yield time to other threads, to potentially reduce chance of another thread pre-empting our profiling effort. Commented out because can in itself skew pofiling data.
                // boost::this_thread::yield();
#ifdef _WINDOWS
                QueryPerformanceCounter(&start_time_);
#endif
            }
	    }

        void Stop()
        {
            if (supported_)
            {
#ifdef _WINDOWS
                QueryPerformanceCounter(&end_time_);
#endif
            }
        }

        //! Returns elapsed time between start and stop in seconds
        double ElapsedTimeSeconds()
        {
#ifdef _WINDOWS
            if (supported_)
            {
                time_elapsed_.QuadPart = end_time_.QuadPart - start_time_.QuadPart - api_overhead_.QuadPart;		
                double elapsed_s = (double)time_elapsed_.QuadPart / (double)frequency_.QuadPart;
         
                return (elapsed_s < 0 ? 0 : elapsed_s);
            }
#endif
            return 0.0;
	    }

        //! Returns elapsed time in microseconds
        LONGLONG ElapsedTimeMicroSeconds()
        {
#ifdef _WINDOWS
            if (supported_)
            {
                time_elapsed_.QuadPart = end_time_.QuadPart - start_time_.QuadPart - api_overhead_.QuadPart;		
                LONGLONG elapsed_ms = static_cast<LONGLONG>(time_elapsed_.QuadPart * 1.e+6) / frequency_.QuadPart;

                return (elapsed_ms < 0 ? 0 : (LONGLONG)0);
            }
#endif
            return 0;
	    }

    private:
        //! is high frequency perf counter supported in this platform
        static bool supported_;

#ifdef _WINDOWS 
        //! performance counter frequency
        static LARGE_INTEGER frequency_;

        //! Time taken to call QueryPerformanceCounter() api function
        static LARGE_INTEGER api_overhead_;

        LARGE_INTEGER start_time_;
        LARGE_INTEGER end_time_;
   
        LARGE_INTEGER time_elapsed_;
#endif
    };

    //! N-ary tree structure for profiling nodes
    class ProfilerNodeTree
    {
        friend class Profiler;
        ProfilerNodeTree();
        ProfilerNodeTree(const ProfilerNodeTree &rhs);
    public:
        typedef std::list<ProfilerNodeTree*> NodeList;

        //! constructor that takes a name for the node
        ProfilerNodeTree(const std::string &name) : name_(name), parent_(NULL), recursion_(0), auto_delete_children(true) { }

        //! destructor
        virtual ~ProfilerNodeTree()
        {
            if (auto_delete_children)
                RemoveAndDeleteChildren();
        }
        
        //! Removes and deletes all child nodes
        virtual void RemoveAndDeleteChildren()
        {
            for (NodeList::iterator it = children_.begin() ; 
                 it != children_.end() ;
                 ++it)
            {
                (*it)->RemoveAndDeleteChildren();
                delete *it;
            }
            children_.clear();
        }

        //! Resets this node and all child nodes
        virtual void ResetValues()
        {
            for (NodeList::iterator it = children_.begin() ; 
                 it != children_.end() ;
                 ++it)
            {
                (*it)->ResetValues();
            }
        }
        
        //! Add a child for this node
        void AddChild(ProfilerNodeTree *node)
        {
            children_.push_back(node);
            node->parent_ = this;
        }

        //! Removes the child node. Does not delete it.
        void RemoveChild(ProfilerNodeTree *node)
        {
            if (node)
            {
                NodeList::iterator it = std::find(children_.begin(), children_.end(), node);
                assert (it != children_.end());
                children_.erase(it);
            }
        }

        //! Removes and deletes the child node and all its children.
        void RemoveAndDestroyChild(ProfilerNodeTree *node)
        {
            if (node)
            {
                NodeList::iterator it = std::find(children_.begin(), children_.end(), node);
                assert (it != children_.end());
                (*it)->RemoveAndDeleteChildren();
                delete *it;
                children_.erase(it);
            }
        }

        //! Returns a child node
        /*!
            \param name Name of the child node
            \return Child node or NULL if the node was not child
        */
        ProfilerNodeTree* GetChild(const std::string &name)
        {
            assert (name != name_);

            for (NodeList::iterator it = children_.begin() ; 
                 it != children_.end() ; 
                 ++it)
            {
                if ((*it)->name_ == name)
                {
                    return *it;
                }
            }

            return NULL;
        }
        //! Returns the name of this node
        const std::string &Name() const { return name_; }

        //! Returns the parent of this node
        ProfilerNodeTree *Parent() { return parent_; }

        //! Returns list of children for introspection
        const NodeList &GetChildren() const { return children_; }

        //! Set true to auto delete children when this node is deleted
        bool auto_delete_children;

    private:
        //! list of all children for this node
        NodeList children_;
        //! cached parent node for easy access
        ProfilerNodeTree *parent_;
        //! Name of this node
        const std::string name_;

        //! helper counter for recursion
        int recursion_;
    };
    typedef boost::shared_ptr<ProfilerNodeTree> ProfilerNodeTreePtr;

    //! Data container for profiling data for a profiling block
    class ProfilerNode : public ProfilerNodeTree
    {
        friend class Profiler;
        ProfilerNode();
    public:
        //! constructor that takes a name for the node
        ProfilerNode(const std::string &name) : 
          ProfilerNodeTree(name),
          num_called_total_(0),
          num_called_(0),
          num_called_current_(0),
          total_(0.0),
          elapsed_current_(0.0),
          elapsed_(0.0) ,
          elapsed_min_(0.0),
          elapsed_max_(0.0),
          elapsed_min_current_(0.0),
          elapsed_max_current_(0.0)
          { }

        virtual ~ProfilerNode() {}

        void ResetValues()
        {
            num_called_ = num_called_current_;
            num_called_current_ = 0;

            elapsed_ = elapsed_current_;
            elapsed_current_ = 0;

            elapsed_min_ = elapsed_min_current_;
            elapsed_min_current_ = 0;

            elapsed_max_ = elapsed_max_current_;
            elapsed_max_current_ = 0;

            ProfilerNodeTree::ResetValues();
        }

        //! Number of times this profile has been called during the execution of the program
        Core::ulong num_called_total_;

        //! Number of times this profile was called during last frame
        Core::ulong num_called_;

        //! Total time spend in this profile during the execution of the program
        double total_;

        //! Time spend in this profiling block during last frame.
        double elapsed_;

        //! Minimum time spend in this profile during last frame
        double elapsed_min_;

        //! Maximum time spend in this profile during last frame
        double elapsed_max_;

    private:
        Core::ulong num_called_current_;
        double elapsed_current_;
        double elapsed_min_current_;
        double elapsed_max_current_;


        ProfilerBlock block_;
    };

    namespace
    {
        //! For boost::thread_specific_ptr, we don't want it doing automatic deletion
        void EmptyDeletor(ProfilerNodeTree *node) { }
        void TSPNodeDeletor(ProfilerNodeTree *node)
        { 
            if (node)
            {
                node->RemoveAndDeleteChildren();
                delete node;
            }
        }
    }
    

    //! Profiler can be used to measure execution time of a block of code.
    /*!
        Do not use this class directly for profiling, use instead PROFILE
        and ELIFORP macros.

        Threadsafety: all profiling related functions are re-entrant so can
              be safely used from any thread. Profiling data needs to be reset
              per frame, so ThreadedReset() should be called from within the 
              profiled thread. The profiling data won't show up otherwise.

              Lock() and Release() functions should not be used, they are for
              reporting profiling data. They are threadsafe because the
              variables that are accessed during reporting are ones that are only
              written to during Reset() or ResetThread and that is protected by a lock.
              Otherwise for thread safety boost::thread_specific_ptr is used to store
              thread specific profiling data. 
              
              Locks are not used when dealing with profiling blocks, as they might skew
              the data too much.

        \todo A memory leak around here somewhere of several kilobytes.
    */
    class Profiler
    {
        friend class Framework;
    private:
        Profiler()
        {
#ifdef PROFILING
            //! we don't want thread_specific_ptr to delete this one automatically
            current_node_ = new boost::thread_specific_ptr<ProfilerNodeTree>(&EmptyDeletor);
            //root_ = new boost::thread_specific_ptr<ProfilerNodeTree>(&TSPNodeDeletor);

            all_nodes_ = boost::shared_ptr<ProfilerNodeTree>(new ProfilerNodeTree("Root"));
            all_nodes_->auto_delete_children = false;

            ProfilerBlock::QueryCapability();
#endif            
        }
    public:
        ~Profiler()
        {
#ifdef PROFILING
            current_node_->release();
            delete current_node_;
#endif  
        }

        //! Start a profiling block.
        /*!
            Normally you don't use this directly, instead you use the macro PROFILE.
            However if you want profiling that lasts out of scope, you can use this directly,
            you also need to call matching Profiler::EndBlock()

            Can be called multiple times with the same name without calling EndBlock() for
            recursion support.

            Re-entrant.
        */
        void StartBlock(const std::string &name);

        //! End the profiling block
        /*! Each StartBlock() should have a matching EndBlock(). Recursion is supported.
            
            Re-entrant.
        */
        void EndBlock(const std::string &name);

        //! Reset profiling data (should be called between frames from main thread only)
        void Reset();

        //! Reset profiling data for threads other than main thread. Don't call directly, use RESETPROFILER macro instead.
        void ThreadedReset();

        //! Returns root profiling node for the current thread only, re-entrant
        ProfilerNodeTree *GetRoot()
        { 
            if (!root_.get())
                root_.reset(new ProfilerNodeTree("Root"));
            return root_.get();
        }

        //! Returns root profiling node for all threads, you must call Release() after you are done!
        ProfilerNodeTreePtr Lock()
        {
            mutex_.lock();
            return all_nodes_;
        }

        void Release()
        {
            mutex_.unlock();
        }

    private:
        //! Root profiler node
        boost::thread_specific_ptr<ProfilerNodeTree> root_;
        //! Cached node topmost in stack, specific to each thread
        boost::thread_specific_ptr<ProfilerNodeTree> *current_node_;

        //! container for all nodes
        boost::shared_ptr<ProfilerNodeTree> all_nodes_;
        Core::Mutex mutex_;
    };

    //! Used by PROFILE - macro to automatically stop profiling clock when going out of scope
    class ProfilerSection
    {
        friend class Framework;
        ProfilerSection();
    public:
        ProfilerSection(const std::string &name) : name_(name), destroyed_(false)
        {
            assert (profiler_ && "Trying to profile before profiler initialized.");
            GetProfiler()->StartBlock(name);
        }

        ~ProfilerSection()
        {
            if (!destroyed_)
            {
                Destruct();
            }
        }

        //! Explicitly destroy this section before it runs out of scope
        __inline void Destruct()
        {
            assert (profiler_ && "Trying to profile before profiler initialized.");

            GetProfiler()->EndBlock(name_);
            destroyed_ = true;
        }
        static Profiler *GetProfiler() { return profiler_; }
        //! This should only be called once per translation unit. it contains some side-effects too
        static void SetProfiler(Profiler *profiler) { profiler_ = profiler; ProfilerBlock::QueryCapability(); }

    private:
        //! Parent profiler used by this section
        static Profiler *profiler_;

        //! Name of this profiling section
        const std::string name_;

        //! True if this section has explicitly been destroyed before it run out of scope
        bool destroyed_;
    };
}

#endif
