// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_Profiler_h
#define incl_Foundation_Profiler_h

#ifdef _WINDOWS
#include <Winsock2.h>
#include <Windows.h>
#endif

#include "HighPerfClock.h"

#include "boost/thread.hpp"

#if (defined(_POSIX_C_SOURCE) || defined(_WINDOWS)) && defined(PROFILING)
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

namespace Foundation
{
    class ProfilerNodeTree;

    //! Profiles a block of code
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
            if (supported_) {
                // yield time to other threads, to potentially reduce chance of another thread pre-empting our profiling effort.
                // Commented out because can in itself skew pofiling data.
                // boost::this_thread::yield();
                start_time_ = Core::GetCurrentClockTime();
            }
        }

        void Stop()
        {
            if (supported_) {
                end_time_ = Core::GetCurrentClockTime();
            }
        }

        //! Returns elapsed time between start and stop in seconds
        double ElapsedTimeSeconds()
        {
            if (supported_) {
                time_elapsed_ = end_time_ - start_time_;
                double elapsed_s = (double)time_elapsed_ / (double)Core::GetCurrentClockFreq();
                return (elapsed_s < 0 ? 0 : elapsed_s);
            } else {
                return 0.0;
            }
        }

        static double ElapsedTimeSeconds(const boost::int64_t &start, const boost::int64_t &end)
        {
            if (supported_) {
                double elapsed_s = (double)(end - start) / (double)Core::GetCurrentClockFreq();
                return (elapsed_s < 0 ? 0 : elapsed_s);
            } else {
                return 0.0;
            }
        }

        //! Returns elapsed time in microseconds
        boost::int64_t ElapsedTimeMicroSeconds()
        {
            if (supported_) {
                time_elapsed_ = end_time_ - start_time_;
                boost::int64_t elapsed_us = static_cast<boost::int64_t>(time_elapsed_ * 1000000) / Core::GetCurrentClockFreq();
                return (elapsed_us < 0 ? 0 : (boost::int64_t)0);
            } else {
                return 0.0;
            }
            
        }

    private:
        //! is high frequency perf counter supported in this platform
        static bool supported_;

        //! performance counter frequency
        static boost::int64_t frequency_;
        static boost::int64_t api_overhead_;

        boost::int64_t start_time_;
        boost::int64_t end_time_;
   
        boost::int64_t time_elapsed_;
    };

    class Profiler;

    //! N-ary tree structure for profiling nodes
    class ProfilerNodeTree
    {
        friend class Profiler;
//        ProfilerNodeTree(); // N/I
        ProfilerNodeTree(const ProfilerNodeTree &rhs); // N/I
    public:
        typedef std::list<boost::shared_ptr<ProfilerNodeTree> > NodeList;

        //! constructor that takes a name for the node
        explicit ProfilerNodeTree(const std::string &name) : name_(name), parent_(0), recursion_(0), owner_(0) {}

        //! destructor
        virtual ~ProfilerNodeTree()
        {
            if (owner_)
                RemoveThreadRootBlock();
        }
        
        void RemoveThreadRootBlock();

        //! Resets this node and all child nodes
        virtual void ResetValues()
        {
            for (NodeList::iterator it = children_.begin() ; it != children_.end() ; ++it)
                (*it)->ResetValues();
        }

        //! Add a child for this node
        void AddChild(boost::shared_ptr<ProfilerNodeTree> node)
        {
            children_.push_back(node);
            node->parent_ = this;
        }

        //! Removes the child node.
        void RemoveChild(ProfilerNodeTree *node)
        {
            if (node)
                for(NodeList::iterator iter = children_.begin(); iter != children_.end(); ++iter)
                    if ((*iter).get() == node)
                    {
                        children_.erase(iter);
                        return;
                    }
        }

        //! Returns a child node
        /*!
          \param name Name of the child node
          \return Child node or 0 if the node was not child
        */
        ProfilerNodeTree* GetChild(const std::string &name)
        {
            assert (name != name_);
            for (NodeList::iterator it = children_.begin() ; it != children_.end() ; ++it)
                if ((*it)->name_ == name)
                    return (*it).get();
            return 0;
        }
        //! Returns the name of this node
        const std::string &Name() const { return name_; }

        //! Returns the parent of this node
        ProfilerNodeTree *Parent() { return parent_; }

        //! Returns list of children for introspection
        const NodeList &GetChildren() const { return children_; }

        void MarkAsRootBlock(Profiler *owner) { owner_ = owner; }

    private:
        //! list of all children for this node
        NodeList children_;
        //! cached parent node for easy access
        ProfilerNodeTree *parent_;
        //! If non-null, this node is a root block owned by the given profiler.
        Profiler *owner_;
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
        ProfilerNode(); // N/I
        ProfilerNode(const ProfilerNode &rhs); // N/I
    public:
        //! constructor that takes a name for the node
        explicit ProfilerNode(const std::string &name) : 
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
            elapsed_max_current_(0.0),
            num_called_custom_(0),
            total_custom_(0),
            custom_elapsed_min_(0),
            custom_elapsed_max_(0)
            {
            }

        virtual ~ProfilerNode()
        {
        }

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
        unsigned long num_called_total_;

        //! Number of times this profile was called during last frame
        unsigned long num_called_;

        //! Total time spend in this profile during the execution of the program
        double total_;

        //! Time spend in this profiling block during last frame.
        double elapsed_;

        //! Minimum time spend in this profile during last frame
        double elapsed_min_;

        //! Maximum time spend in this profile during last frame
        double elapsed_max_;

        // Profiling data is also accumulated here, and can be resetted on a custom interval.
        // Mutable since the application can alter these at will, but the above "official"
        // values may not be touched.
        mutable unsigned long num_called_custom_;
        mutable double total_custom_;
        mutable double custom_elapsed_min_;
        mutable double custom_elapsed_max_;

    private:
        unsigned long num_called_current_;
        double elapsed_current_;
        double elapsed_min_current_;
        double elapsed_max_current_;

        ProfilerBlock block_;
    };

    namespace
    {
        //! For boost::thread_specific_ptr, we don't want it doing automatic deletion
        void EmptyDeletor(ProfilerNodeTree *node) { }
/*        void TSPNodeDeletor(ProfilerNodeTree *node)
          { 
          delete node;
          }*/
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
    public://private:
    Profiler()
        :current_node_(&EmptyDeletor),
            root_("Root")
            {
            }
    public:
        ~Profiler();

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

        //! Reset profiling data for the current thread. Don't call directly, use RESETPROFILER macro instead.
        void ThreadedReset();

        ProfilerNodeTree *CreateThreadRootBlock();
        void RemoveThreadRootBlock(ProfilerNodeTree *rootBlock);

        ProfilerNodeTree *GetThreadRootBlock();

        std::string GetThisThreadRootBlockName();

        //! Returns root profiling node for the current thread only, re-entrant
        ProfilerNodeTree *GetOrCreateThreadRootBlock();

        //! Returns root profiling node for all threads.
        ProfilerNodeTree *Lock()
        {
            mutex_.lock();
            return &root_;
        }

        void Release()
        {
            mutex_.unlock();
        }

        ProfilerNodeTree *GetRoot() { return &root_; }

    private:
        //! The single global root node object. This is a dummy root node that doesn't track any
        //! timing statistics, but just contains all the root blocks of each thread as its children.
        //! This root_ node doesn't own any of the memory of any of its children, those are owned 
        //! and freed by each thread separately Namely, freeing all instances inside the 
        //! thread_specific_root_ will cause all blocks to be freed.
        ProfilerNodeTree root_;

        //! Contains the root profile block for each thread.
        boost::thread_specific_ptr<ProfilerNodeTree> thread_specific_root_;
        //! Points to the current topmost profile block in the stack for each thread.
        boost::thread_specific_ptr<ProfilerNodeTree> current_node_;

        //! container for all the root profile nodes for each thread.
        std::list<ProfilerNodeTree*> thread_root_nodes_;

        boost::mutex mutex_;
    };

    //! Used by PROFILE - macro to automatically stop profiling clock when going out of scope
    class ProfilerSection
    {
        friend class Framework;
        ProfilerSection(); // N/I
        ProfilerSection(const ProfilerSection &rhs);
    public:
        explicit ProfilerSection(const std::string &name) : name_(name), destroyed_(false)
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
        static void SetProfiler(Profiler *profiler) { profiler_ = profiler; }

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
