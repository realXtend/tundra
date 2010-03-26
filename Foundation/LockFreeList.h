// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_LockFreeList_h
#define incl_Foundation_LockFreeList_h

template<typename T>
struct LockFreeListNode
{
    T value;

    volatile LockFreeListNode<T> *next;
};

/** Implements a linked list that has threadsafe inserts:
    - There's no inserts in-between, only push_backs (call Insert to do a push_back).
    - There are no functions that allow doing a delete/erase on the list nodes. Everything
      is freed once the list is destroyed.
    - Only one thread can act as a producer to this thread. This is the only thread
      that may call Insert().
    - The actual elements in the list are not protected in any way. Use mutexes manually
      if you need to shield the individual nodes.
    - There is no limitation on how many threads can read the list.
    
    Deliberately not following the naming of std::list so that there is no confusion that
    this doesn't operate like a standard list container. */
template<typename T>
class LockFreeList
{
    LockFreeList(const LockFreeList &); // N/I
    void operator =(const LockFreeList &); // N/I
public:
    typedef LockFreeListNode<T> node_t;

    LockFreeList()
    :root(0), tail(0)
    {
    }
    
    /// Inserts a new node at the back of the list. To be thread-safe, this
    /// function may only be called from the list producer thread, not from any other threads.
    void PushBack(const T &value)
    {
        node_t *newNode = new node_t;
        newNode->value = value;
        newNode->next = 0;

        if (root == 0)
        {
            assert(tail == 0);
            root = newNode;
            tail = newNode;
        }
        else
        {
            tail->next = newNode;
        }
    }

    /// @return The first node in the list, thread-safe.
    node_t *Begin() const { return root; }

    /// Deletes all nodes in the list. Not thread-safe!
    void Clear()
    {
        node_t *node = root;
        while(node)
        {
            node_t *next = node->next;
            delete node;
            node = next;
        }
        root = tail = 0;
    }

private:
    /// The first item in the list. Write access is on the producer thread only.
    volatile node_t *root;
    /// The last item in the list. Write access is on the producer thread only.
    volatile node_t *tail;
};

#endif
