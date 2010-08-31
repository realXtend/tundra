/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef LocklessQueue_h
#define LocklessQueue_h

#include "clb/Algorithm/Alignment.h"

/** @internal A simple lockfree queue that supports one publisher and one subscriber. The queue
	only supports POD types as elements and has a fixed upper size. */
template<typename T>
class SimpleLockfreeQueue
{
private:
	T *data;
	unsigned long maxElementsMask;
	volatile unsigned long head;
	volatile unsigned long tail;

	SimpleLockfreeQueue(const SimpleLockfreeQueue<T> &rhs);
	void operator=(const SimpleLockfreeQueue<T> &rhs);

public:
	explicit SimpleLockfreeQueue(size_t maxElements)
	:head(0), tail(0)
	{
		assert(IS_POW2(maxElements));
		data = new T[maxElements];
		maxElementsMask = maxElements - 1;
	}

	~SimpleLockfreeQueue()
	{
		delete[] data;
	}

    /// Only accessible from the producer thread.
	bool Insert(const T &value)
	{
		unsigned long head_ = head;
		unsigned long nextHead = (head_ + 1) & maxElementsMask;
		if (nextHead == tail)
			return false;
		data[head_] = value;
		head = nextHead; // Publishes the new value to the queue.

		return true;
	}

    /// Only accessible from the consumer thread.
	T *Front()
	{
		if (head == tail)
			return 0;
		return &data[tail];
	}

    /// Accessible from either thread.
	unsigned long Size() const
	{
		unsigned long head_ = head;
		unsigned long tail_ = tail;
		if (head_ >= tail_)
			return head_ - tail_;
		else
			return maxElementsMask + 1 - (tail_ - head_);
	}

    /// Returns the number of elements that can still be added to the queue, without Insert() failing.
    /// Accessible from either thread.
	unsigned long SpaceLeft() const
	{
        return maxElementsMask + 1 - Size();
	}

    /// Only accessible from the consumer thread.
	void PopFront()
	{
		assert(head != tail);
		if (head == tail)
			return;
		unsigned long tail_ = (tail + 1) & maxElementsMask;
		tail = tail_;
	}
};

#endif
