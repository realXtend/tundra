// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "Profiler.h"
#include "CoreDefines.h"
#include "CoreStringUtils.h"
#include "HighPerfClock.h"
#include "MemoryLeakCheck.h"
#include "Math/MathFunc.h"

#include <QThread>

#include <iostream>
#include <utility>

bool ProfilerBlock::QueryCapability()
{
#if defined(_WINDOWS)
    LARGE_INTEGER frequency;
    BOOL result = QueryPerformanceFrequency(&frequency);
    return result != 0;
#elif defined(ANDROID) || defined(_POSIX_C_SOURCE)
    return true;
#else
#warning "ProfilerBlock::QueryCapability not implemented for current platform!"
    return false;
#endif
}

void Profiler::StartBlock(const std::string &name)
{
#ifdef PROFILING
    // Get the current topmost profiling node in the stack, or 
    // if none exists, get the root node or create a new root node.
    // This will be the parent node of the new block we're starting.
    ProfilerNodeTree *parent = current_node_;
    if (!parent)
    {
        parent = GetOrCreateThreadRootBlock();
        current_node_ = parent;
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
        parent->AddChild(shared_ptr<ProfilerNodeTree>(node));
    }

    assert (parent->recursion_ >= 0);

    // If a recursive call, just increment recursion count, the timer has already
    // been started so no need to touch it. Otherwise, start the timer for the current
    // block.
    if (parent == node)
        parent->recursion_++; // handle recursion
    else
    {
        current_node_ = node;

        checked_static_cast<ProfilerNode*>(node)->block_.Start();
    }
#endif
}

void Profiler::EndBlock(const std::string &name)
{
#ifdef PROFILING
    using namespace std;

    ProfilerNodeTree *treeNode = current_node_;
    if (!treeNode)
        return;
    assert (treeNode->Name() == name && "New profiling block started before old one ended!");
    UNREFERENCED_PARAM(name)
    ProfilerNode* node = checked_static_cast<ProfilerNode*>(treeNode);
    node->block_.Stop();
    node->num_called_total_++;
    node->num_called_current_++;

    double elapsed = node->block_.ElapsedTimeSeconds();

    node->elapsed_current_ += elapsed;
    node->elapsed_min_current_ = (EqualAbs(node->elapsed_min_current_, 0.0) ? elapsed : (elapsed < node->elapsed_min_current_ ? elapsed : node->elapsed_min_current_));
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
        current_node_ = node->Parent();
#endif
}

void ProfilerQObj::BeginBlock(const QString &name)
{
#ifdef PROFILING
    Framework *fw = Framework::Instance();
    Profiler *p = fw ? fw->GetProfiler() : 0;
    if (p)
        p->StartBlock(name.toStdString());
#endif
}

void ProfilerQObj::EndBlock()
{
#ifdef PROFILING
    Framework *fw = Framework::Instance();
    Profiler *p = fw ? fw->GetProfiler() : 0;
    if (p)
    {
        ProfilerNodeTree *treeNode = p->current_node_;
        if (!treeNode)
            return;
        p->EndBlock(treeNode->Name());
    }
#endif
}

ProfilerNodeTree *Profiler::GetThreadRootBlock()
{
    return thread_specific_root_;
}

ProfilerNodeTree *Profiler::GetOrCreateThreadRootBlock()
{
#ifdef PROFILING // If not profiling, never create the root block so the getter will always return 0.
    if (!thread_specific_root_)
        return CreateThreadRootBlock();
#endif
    return thread_specific_root_;
}

std::string Profiler::GetThisThreadRootBlockName()
{
    char str[256];
    sprintf(str, "Thread%p", QThread::currentThreadId());
    return str;
}

ProfilerNodeTree *FindBlockByName(ProfilerNodeTree *parent, const char *name)
{
    if (!parent)
        return 0;
    if (parent->Name() == name)
        return parent;
    ProfilerNodeTree::NodeList &children = parent->GetChildren();
    for(ProfilerNodeTree::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        ProfilerNodeTree *node = FindBlockByName((*iter).get(), name);
        if (node)
            return node;
    }
    return 0;
}

ProfilerNodeTree *Profiler::FindBlockByName(const char *name)
{
    return ::FindBlockByName(GetThreadRootBlock(), name);
}

float ProfilerNode::TotalCustomSpentInChildren() const
{
    const ProfilerNodeTree::NodeList &children = GetChildren();
    float timeSpentInChildren = 0.f;
    bool steppedIntoChildren = false;
    for(ProfilerNodeTree::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        ProfilerNodeTree *node = iter->get();

        const ProfilerNode *timings_node = dynamic_cast<const ProfilerNode*>(node);
        if (timings_node)
        {
            timeSpentInChildren += timings_node->total_custom_;
            if (timings_node->num_called_custom_ > 0)
                steppedIntoChildren = true;
        }
    }
    if (!steppedIntoChildren)
        return -1.f;
    return timeSpentInChildren;
}

ProfilerNodeTree *Profiler::CreateThreadRootBlock()
{
#ifdef PROFILING
    ProfilerBlock::QueryCapability();
    
    std::string rootObjectName = GetThisThreadRootBlockName();

    ProfilerNodeTree *root = new ProfilerNodeTree(rootObjectName);
    assert(!thread_specific_root_);
    thread_specific_root_ = root;

    // Each thread root block is added as a child of a dummy node root_ owned by
    // this Profiler. The root_ object doesn't own the memory of its children,
    // but just weakly refers to them an allows easy access for printing the
    // profiling data in each thread.
    mutex_.lock();
    root_.AddChild(shared_ptr<ProfilerNodeTree>(root, &EmptyDeletor));
    root->MarkAsRootBlock(this);
    thread_root_nodes_.push_back(root);
    mutex_.unlock();
    return root;
#else
    return 0;
#endif
}

void Profiler::RemoveThreadRootBlock(ProfilerNodeTree *rootBlock)
{
#ifdef PROFILING
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
#endif
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
    if (owner_)
        owner_->RemoveThreadRootBlock(this);
}

void Profiler::Reset()
{
    // We are going down.. tell all root blocks that they don't need to notify back to the Profiler that they've been deleted.
    // i.e. 'detach' all the (thread specific) root blocks so that they delete themselves at their leisure when their threads die.
    mutex_.lock();
    for(std::list<ProfilerNodeTree*>::iterator iter = thread_root_nodes_.begin(); iter != thread_root_nodes_.end(); ++iter)
        (*iter)->MarkAsRootBlock(0);
    mutex_.unlock();
}

Profiler::~Profiler()
{
    Reset();
}
