// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Profiler.h"
#include "CoreDefines.h"
#include "CoreMath.h"
#include "CoreStringUtils.h"

namespace Foundation
{
    bool ProfilerBlock::supported_ = false;

#ifdef _WINDOWS
    LARGE_INTEGER ProfilerBlock::frequency_;
    LARGE_INTEGER ProfilerBlock::api_overhead_;
#endif

    Profiler *ProfilerSection::profiler_ = 0;

    bool ProfilerBlock::QueryCapability()
    {
#ifdef _WINDOWS
        BOOL result = QueryPerformanceFrequency(&frequency_);
        supported_ = (result != 0);
#endif
        return supported_;
    }

    void Profiler::StartBlock(const std::string &name)
    {
        // Get the current topmost profiling node in the stack, or 
        // if none exists, get the root node or create a new root node.
        // This will be the parent node of the new block we're starting.
        ProfilerNodeTree *parent = current_node_.get();
        if (!parent)
        {
            parent = GetOrCreateThreadRootBlock();
            current_node_.reset(parent);
        }
        assert(parent);

        // If parent name == new block name, we assume that we're
        // recursively re-entering the same function (with a single
        // profiling block).
        ProfilerNodeTree *node = (name != parent->Name()) ? parent->GetChild(name) : parent;

        // We're entering this PROFILE() block for the first time,
        // need to allocate the memory for it.
        if (!node)
        {
            node = new ProfilerNode(name);
            parent->AddChild(boost::shared_ptr<ProfilerNodeTree>(node));
        }

        assert (parent->recursion_ >= 0);

        // If a recursive call, just increment recursion count, the timer has already
        // been started so no need to touch it. Otherwise, start the timer for the current
        // block.
        if (parent == node)
            parent->recursion_++; // handle recursion
        else
        {
            current_node_.release();
            current_node_.reset(node);

            checked_static_cast<ProfilerNode*>(node)->block_.Start();
        }
    }

    void Profiler::EndBlock(const std::string &name)
    {
        using namespace std;

        ProfilerNodeTree *treeNode = current_node_.get();
        assert (treeNode->Name() == name && "New profiling block started before old one ended!");

        ProfilerNode* node = checked_static_cast<ProfilerNode*>(treeNode);
        node->block_.Stop();
        node->num_called_total_++;
        node->num_called_current_++;

        double elapsed = node->block_.ElapsedTimeSeconds();

        node->elapsed_current_ += elapsed;
        node->elapsed_min_current_ = (equals(node->elapsed_min_current_, 0.0) ? elapsed : (elapsed < node->elapsed_min_current_ ? elapsed : node->elapsed_min_current_));
        node->elapsed_max_current_ = elapsed > node->elapsed_max_current_ ? elapsed : node->elapsed_max_current_;
        node->total_ += elapsed;

        node->num_called_custom_++;
        node->total_custom_ += elapsed;
        node->custom_elapsed_min_ = std::min(node->custom_elapsed_min_, elapsed);
        node->custom_elapsed_max_ = std::max(node->custom_elapsed_max_, elapsed);

        assert (node->recursion_ >= 0);

        // need to handle recursion
        if (node->recursion_ > 0)
            --node->recursion_;
        else
        {
            current_node_.release();
            current_node_.reset(node->Parent());
        }
    }

    ProfilerNodeTree *Profiler::GetThreadRootBlock()
    { 
        return thread_specific_root_.get();
    }

    ProfilerNodeTree *Profiler::GetOrCreateThreadRootBlock()
    { 
        if (!thread_specific_root_.get())
            return CreateThreadRootBlock();

        return thread_specific_root_.get();
    }

    std::string Profiler::GetThisThreadRootBlockName()
    {
        return std::string("Thread" + ToString(boost::this_thread::get_id()));
    }

    ProfilerNodeTree *Profiler::CreateThreadRootBlock()
    {
        std::string rootObjectName = GetThisThreadRootBlockName();

        ProfilerNodeTree *root = new ProfilerNodeTree(rootObjectName);
        thread_specific_root_.reset(root);

        // Each thread root block is added as a child of a dummy node root_ owned by
        // this Profiler. The root_ object doesn't own the memory of its children,
        // but just weakly refers to them an allows easy access for printing the
        // profiling data in each thread.
        mutex_.lock();
        root_.AddChild(boost::shared_ptr<ProfilerNodeTree>(root, &EmptyDeletor));
        root->MarkAsRootBlock(this);
        thread_root_nodes_.push_back(root);
        mutex_.unlock();
        return root;
    }

    void Profiler::RemoveThreadRootBlock(ProfilerNodeTree *rootBlock)
    {
        mutex_.lock();
        for(std::list<ProfilerNodeTree*>::iterator iter = thread_root_nodes_.begin(); iter != thread_root_nodes_.end(); ++iter)
            if (*iter == rootBlock)
            {
                thread_root_nodes_.erase(iter);
                mutex_.unlock();
                return;
            }

        std::cout << "Warning: Tried to delete a nonexisting thread root block!" << std::endl;
        mutex_.unlock();
    }

    void Profiler::ThreadedReset()
    {
        ProfilerNodeTree *root = GetThreadRootBlock();
        if (!root)
            return;

        mutex_.lock();
        root->ResetValues();
        mutex_.unlock();
    }

    void ProfilerNodeTree::RemoveThreadRootBlock()
    {
        assert(owner_);
        owner_->RemoveThreadRootBlock(this);
    }

    Profiler::~Profiler()
    {
        // We are going down.. tell all root blocks that they don't need to notify back to the Profiler that they've been deleted.
        // i.e. 'detach' all the (thread specific) root blocks so that they delete themselves at their leisure when their threads die.
        mutex_.lock();
        for(std::list<ProfilerNodeTree*>::iterator iter = thread_root_nodes_.begin(); iter != thread_root_nodes_.end(); ++iter)
            (*iter)->MarkAsRootBlock(0);
        mutex_.unlock();
    }
}

