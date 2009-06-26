// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Profiler.h"

namespace Foundation
{
    bool ProfilerBlock::supported_ = false;

#ifdef _WINDOWS
    LARGE_INTEGER ProfilerBlock::frequency_;
    LARGE_INTEGER ProfilerBlock::api_overhead_;
#endif

    Profiler *ProfilerSection::profiler_ = NULL;

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
        ProfilerNodeTree *node = current_node_->get();
        if (!node)
        {
            node = GetRoot();
            current_node_->reset(node);
        }

        if (name != (*current_node_)->Name())
            node = (*current_node_)->GetChild(name);

        if (!node)
        {
            node = new ProfilerNode(name);
            (*current_node_)->AddChild(node);
        }

        assert ((*current_node_)->recursion_ >= 0);
        
        if (current_node_->get() == node)
            (*current_node_)->recursion_++; // handle recursion
        else
        {
            current_node_->release();
            current_node_->reset(node);

            checked_static_cast<ProfilerNode*>(current_node_->get())->block_.Start();
        }
    }

    void Profiler::EndBlock(const std::string &name)
    {
        assert ((*current_node_)->Name() == name && "New profiling block started before old one ended!");

        ProfilerNode* node = checked_static_cast<ProfilerNode*>(current_node_->get());
        node->block_.Stop();
        node->num_called_total_++;
        node->num_called_current_++;

        double elapsed = node->block_.ElapsedTimeSeconds();

        node->elapsed_current_ += elapsed;
        node->elapsed_min_current_ = (Core::equals(node->elapsed_min_current_, 0.0) ? elapsed : (elapsed < node->elapsed_min_current_ ? elapsed : node->elapsed_min_current_));
        node->elapsed_max_current_ = elapsed > node->elapsed_max_current_ ? elapsed : node->elapsed_max_current_;
        node->total_ += elapsed;

        assert ((*current_node_)->recursion_ >= 0);

        // need to handle recursion
        if ((*current_node_)->recursion_ > 0)
            (*current_node_)->recursion_--;
        else
        {
            current_node_->release();
            current_node_->reset(node->Parent());
        }
    }

    void Profiler::Reset()
    {
        ProfilerNodeTree *root = new ProfilerNodeTree("MainThread");
        root->AddChild(GetRoot());

        // lock here shouldn't matter, as we are not profiling here
        Core::MutexLock lock(mutex_);

        GetRoot()->ResetValues();

        ProfilerNodeTree *new_root = all_nodes_->GetChild("MainThread");
        all_nodes_->RemoveChild(new_root);
        if (new_root)
        {
            new_root->auto_delete_children = false;
            delete new_root;
        }

        all_nodes_->AddChild(root);
    }

    void Profiler::ThreadedReset()
    {
        const std::string thread("Thread" + Core::ToString(boost::this_thread::get_id()));

        ProfilerNodeTree *root = new ProfilerNodeTree(thread);
        root->AddChild(GetRoot());

        // lock here shouldn't matter, as we are not profiling here
        Core::MutexLock lock(mutex_);

        GetRoot()->ResetValues();

        ProfilerNodeTree *new_root = all_nodes_->GetChild(thread);
        all_nodes_->RemoveChild(new_root);
        if (new_root)
        {
            new_root->auto_delete_children = false;
            delete new_root;
        }
        
        all_nodes_->AddChild(root);
    }
}

