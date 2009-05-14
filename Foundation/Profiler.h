// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_Profiler_h
#define incl_Foundation_Profiler_h

#if defined(_WINDOWS) && defined(PROFILING)
//! Profiles a block of code in current scope. Ends the profiling when it goes out of scope
/*! Name of the profiling block must be unique in the scope, so do not use the name of the function
    as the name of the profiling block!

    \param x Unique name for the profiling block, use without quotes, f.ex. START_PROFILE(name_of_the_block)
*/
#   define PROFILE(x) Foundation::ProfilerSection x ## __profiler__(#x);

//! Optionally ends the current profiling block
/*! Use when you wish to end a profiling block before it goes out of scope
*/
#   define ELIFORP(x)   x ## __profiler__.Destruct();

#else
#   define PROFILE(x)
#   define ELIFORP(x)
#endif

#ifndef _WINDOWS
    typedef int LONGLONG;
#endif

namespace Foundation
{
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

    class ProfilerNodeTree
    {
        ProfilerNodeTree();
    public:
        typedef std::list<ProfilerNodeTree*> NodeList;

        ProfilerNodeTree(const std::string &name, ProfilerNodeTree *parent) : name_(name), parent_(parent) {}
        virtual ~ProfilerNodeTree()
        {
            for (NodeList::iterator it = children_.begin() ; 
                 it != children_.end() ;
                 ++it)
            {
                delete *it;
            }
        }
        
        void AddChild(ProfilerNodeTree *node)
        {
            children_.push_back(node);
        }
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
        const std::string &Name() const { return name_; }
        ProfilerNodeTree *Parent() { return parent_; }

    private:
        ProfilerNodeTree *parent_;
        NodeList children_;
        const std::string name_;
    };

    //! Data container for profiling data for a profiling block
    class ProfilerNode : public ProfilerNodeTree
    {
    public:
        ProfilerNode(const std::string &name, ProfilerNodeTree *parent) : 
          ProfilerNodeTree(name, parent),
          num_called_total_(0),
          total_(0.0),
          elapsed_current_(0.0),
          elapsed_(0.0) 
          {}
        virtual ~ProfilerNode() {}

        //! Number of times this profile has been called during the execution of the program
        Core::ulong num_called_total_;

        //! Total time spend in this profile
        double total_;

        //! Average time spend in this profile
        //double average_;

        double elapsed_current_;

        //! Time spend in this profiling block during last frame.
        double elapsed_;

        ProfilerBlock block_;
    };
    

    //! Profiler can be used to measure execution time of a block of code.
    /*!
        Do not use this class directly for profiling, use instead PROFILE
        and ELIFORP macros.
    */
    class Profiler : public ProfilerNodeTree
    {
    public:
        Profiler() : ProfilerNodeTree(std::string("Root"), NULL) { current_node_ = this; ProfilerBlock::QueryCapability(); }
        ~Profiler() {}

        //! Start a profiling block.
        /*!
            Normally you don't use this directly, instead you use the macro PROFILE.
            However if you want profiling that lasts out of scope, you can use this directly,
            you also need to call matching Profiler::EndBlock()
        */
        void StartBlock(const std::string &name);
        //! End the profiling block
        void EndBlock(const std::string &name);

    private:
        //! Cached node topmost in stack
        ProfilerNodeTree *current_node_;
    };

    class ProfilerSection
    {
    public:
        ProfilerSection(const std::string &name) : name_(name), destroyed_(false)
        {
            GetProfiler().StartBlock(name);
        }

        ~ProfilerSection()
        {
            if (!destroyed_)
            {
                Destruct();
            }
        }

        __inline void Destruct()
        {
            GetProfiler().EndBlock(name_);
            destroyed_ = true;
        }

        __inline static Profiler &GetProfiler()
        {
            static Profiler profiler;
            return profiler;
        }

    private:
        const std::string name_;

        bool destroyed_;
    };
}

#endif
