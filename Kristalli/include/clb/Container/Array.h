/** @file Array.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 
*/
#ifndef Array_h
#define Array_h

#include "clb/Core/Ptr.h"
#include "clb/Memory/Allocator.h"
#include "clb/Memory/StdCMallocHeap.h"

namespace clb
{

template<typename T, typename AllocT = StdCAlloc>
class Array;

/// Sequential iterator for Array.
template<typename T, typename AllocT>
class ArrayIterator
{
private:
	T *ptr;
	Array<T, AllocT> *container;

	ArrayIterator(Array<T, AllocT> &container_, T *ptr_)
	:container(&container_), ptr(ptr_)
	{
		assert(ptr >= container->beginptr());
		assert(ptr <= container->endptr());
	}

	friend class Array<T, AllocT>;

public:
	ArrayIterator(const ArrayIterator &rhs):ptr(rhs.ptr), container(rhs.container) {}
	~ArrayIterator() {}

	inline void operator++()
	{ 
		assert(ptr <= container->endptr());
		assert(ptr >= container->beginptr());
//		if (*this < container->endptr())
		++ptr;
	}
	inline void operator--()
	{ 
		assert(ptr <= container->endptr());
		assert(ptr >= container->beginptr());
//		if (*this > container->beginptr())
		--ptr;
	}

	inline operator T*()
	{ 
		assert(ptr >= container->beginptr());
		assert(ptr <= container->endptr());
		return ptr;
	}

	inline operator const T*() const 
	{ 
		assert(ptr >= container->beginptr());
		assert(ptr <= container->endptr());
		return ptr;
	}

	inline const T &operator*() const
	{ 
		assert(ptr >= container->beginptr());
		assert(ptr <= container->endptr());
		return *ptr; 
	}

	inline T &operator*() 
	{ 
		assert(ptr >= container->beginptr());
		assert(ptr <= container->endptr());
		return *ptr; 
	}

	inline bool operator<(const ArrayIterator &rhs) const { return ptr < rhs.ptr; }
	inline bool operator<=(const ArrayIterator &rhs) const { return ptr <= rhs.ptr; }
	inline bool operator==(const ArrayIterator &rhs) const { return ptr == rhs.ptr; }
	inline bool operator!=(const ArrayIterator &rhs) const { return ptr != rhs.ptr; }
	inline bool operator>(const ArrayIterator &rhs) const { return ptr > rhs.ptr; }
	inline bool operator>=(const ArrayIterator &rhs) const { return ptr >= rhs.ptr; }
};

/// std::vector -equivalent.
template<typename T, typename AllocT>
class Array
{
private:
	T *data;
	AllocT *allocator;
	size_t cap; ///< The number of elements allocated in the data.
	size_t used; ///< The number of elements actually in use.

	static const size_t initialSize = 32;

public:
	typedef ArrayIterator<T, AllocT> iterator;

	Array(AllocT *allocator_ = 0)
	:data(0), cap(0), used(0), allocator(allocator_)
	{ }

	Array(const Array &rhs)
	:allocator(rhs.allocator), data(0), cap(0), used(0)
	{
		reserve(rhs.cap);
		for(int i = 0; i < rhs.size(); ++i)
			data[i] = rhs.data[i];
		used = rhs.used;
	}

	~Array()
	{
		for(size_t i = 0; i < used; ++i)
			data[i] = T();
		DeleteArray(data, allocator);
	}

	const T &front() const { assert(used > 0); return data[0]; }
	const T &back() const { assert(used > 0); return data[used-1]; }
	T &front() { assert(used > 0); return data[0]; }
	T &back() { assert(used > 0); return data[used-1]; }

	/// @return The number of elements in the container.
	size_t size() const { return used; }

	/// @return The number of elements that the container can hold without needing to reserve more.
	size_t capacity() const { return cap; }

	/// Sets the allocator used by this Array.
	void set_allocator(AllocT *allocator_)
	{ 
		assert(cap == 0);
		assert(data == 0);
		allocator = allocator_;
	}

	/// @return The allocator used by this Array, or 0 if none was set.
	AllocT *get_allocator() const { return allocator; }

	const T *beginptr() const { return data; } ///< Pointer to the first element.
	const T *endptr() const { return data + used; } ///< Pointer to one past the last used element.
	const T *capptr() const { return data + cap; } ///< Pointer to the first element beyond allocated memory.

	/// @return An iterator to start of the data.
	iterator begin() { return iterator(*this, data); }

	/// @return A const pointer to start of the data.
//	const  *begin() const { return data; }

	/// @return An iterator to past the last element in the array.
	iterator end() { return iterator(*this, data + used); }

	/// @return A const pointer to past the last element in the array.
	const T *end() const { return data + used ; }

	void insert(const T &val) { insert(used, val); }

	/// Inserts an element before the ith element of the Array. Running time is O(n).
	void insert(int i, const T &val)
	{
		if (used >= cap)
			reserve(cap*2);

		// Push all data up one index to make room for the new element.
		for(int j = used+1; j > i; --j)
			data[j] = data[j-1];

		// Add the new element.
		data[i] = val;
		++used;
	}

	void InsertMultiple(int i, const T &val, int numTimes)
	{
		if (used + numTimes > cap)
			reserve((cap+numTimes)*2);

		// Push all data up numTimes slots to make room for the new element.
		for(int j = used+numTimes; j > i+numTimes; --j)
			data[j] = data[j-numTimes];

		// Add the new element.
		for(int j = i; j < i + numTimes; ++j)
			data[j] = val;
		
		used += numTimes;
	}

	/// Inserts an element to the front of the Array. Running time is O(n).
	void push_front(const T &val) { insert(0, val); }

	/// Inserts the given element to back of the Array. Running time is O(1).
	void push_back(const T &val)
	{
		if (used >= cap)
			reserve(cap*2);
		data[used] = val;
		++used;
	}

	void push_back_unsafe(const T &val)
	{
		assert(used < cap);
		data[used++] = val;
	}

	T &push_back_unsafe_pod()
	{
		assert(used < cap);
		return data[used++];
	}

	/// Inserts a default-constructed element to back of the Array. Running time is O(1).
	T &push_back()
	{
		if (used >= cap)
			reserve(cap*2);
		data[used] = T();
		return data[used++];
	}

	/// Erases a range of elements, excluding the element at index end. That is, the interval [start, end[.
	/// Running time is O(n).
	void erase(int start, int end)
	{
		assert(start >= 0);
		assert(end >= 0);
		assert(start <= end);
		assert(end <= used);

		if (start == end)
			return;

		const int items = end-start;
		for(int i = start; i < used-items; ++i)
			data[i] = data[i+items];
		for(int i = used-items; i < used; ++i)
			data[i] = T(); ///< \todo Placement new/delete instead of this crap.

		used -= items;
	}

	/// Removes the ith element from the array. Running time is O(n).
	void erase(int i)
	{
		assert(i >= 0);
		erase((size_t)i);
	}

	/// Removes the ith element from the array. Running time is O(n).
	void erase(size_t i)
	{
		assert(i < used);

		for(size_t j = i; j+1 < used; ++j)
			data[j] = data[j+1];

		data[used-1] = T(); ///< \todo Placement new/delete instead of this crap.
		--used;
	}

	/// Clears the last element of the array. Running time is O(1).
	void pop_back()
	{
		assert(used > 0);

		data[used-1] = T(); ///< \todo Placement new/delete instead of this crap.
		--used; /// \note Doesn't call destructor for the popped object.
	}

	/// Clears the last element of the array. Running time is O(1). Use for pod data that don't need
	/// to be cleared when uninitializing.
	void pop_back_pod()
	{
		assert(used > 0);

		--used; /// \note Doesn't call destructor for the popped object.
	}

	/// Removes all elements in the array. Running time is O(n).
	void clear()
	{
		for(size_t i = 0; i < used; ++i)
			data[i] = T(); ///< \todo Placement new/delete instead of this crap.
		used = 0;
	}

	void clear_pod()
	{
		used = 0;
	}

	/// Enlarges the allocated memory area if it's not big enough to hold newSize elements.
	void reserve(size_t newSize)
	{
		newSize = newSize < initialSize ? initialSize : newSize;

		// only to enlarge.
		if (newSize < cap)
			return;

		const size_t alignment = 4; ///\ todo
		T *newData = NewArray<T, AllocT>(newSize, allocator, alignment);

//		memcpy(newData, data, sizeof(T)*used);
		for(size_t i = 0; i < used; ++i)
			newData[i] = data[i];
		DeleteArray(data, allocator);
		data = newData;
		cap = newSize;
	}

	const T &operator[](int index) const
	{
		assert(index >= 0);
		assert((size_t)index < used);
		return data[index];
	}

	T &operator[](int index)
	{
		assert(index >= 0);
		assert((size_t)index < used);
		return data[index];
	}

	Array<T, AllocT> &operator =(const Array<T, AllocT> &rhs)
	{
		if (this == &rhs)
			return *this;

		reserve(rhs.size());
		for(int i = 0; i < rhs.size(); ++i)
			data[i] = rhs.data[i];
		used = rhs.size();

		return *this;
	}
};

} // ~clb

#endif
