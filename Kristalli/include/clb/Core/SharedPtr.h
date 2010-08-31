/** @file SharedPtr.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief SharedPtr, intrusive reference counting for multiple pointer owners.

	Tutorial 2 deals with basic usage on shared pointers.
*/

#ifndef SharedPtr_h
#define SharedPtr_h

#include "clbAssert.h"

#include <malloc.h>
#include <cstdlib>

/// Smart pointer for dynamic single object allocations on the heap.
#define Ptr(type) clb::SharedPtr< type > 

namespace clb
{

/// Objects that require reference count tracking derive publicly from this.
class RefCountable
{
private:
	unsigned int refCount;

public:
	RefCountable():refCount(0) {}
//	~RefCountable() {} // Can't put any functionality here since not virtual.

	void AddRef() { ++refCount; }
	void DecRef() { assert(refCount > 0); --refCount; }
	unsigned int RefCount() { return refCount; }
};

/** @brief SharedPtr is an intrusive refcount-tracked single-object lifetime-manager.

SharedPtr wraps raw pointers to objects and manages reference 
counting on the number of users on the pointer. When all users abandon the object,
FreeFunc::free(T*) is called to clean up the memory used by the resource. */
template<typename T>
class SharedPtr
{
private:
	T *dataPtr;			///< Pointer to the actual data.

	void AddRef();		///< Increases reference count.
	void Release();	///< Decreases reference count and deletes the object if 0.

public:
	/// Constructs a zero pointer.
	SharedPtr():dataPtr(0) {}

	/// Constructs from a raw pointer.
	SharedPtr(T *ptr);

	/// Constructs from an existing SharedPtr.
	SharedPtr(const SharedPtr<T> &rhs);

	/// Template copy constructor for types that allow conversions to other types.
	template<typename T2>
	SharedPtr(const SharedPtr<T2> &rhs);

	/// Assignment from another SharedPtr.
	SharedPtr<T> &operator=(const SharedPtr<T> &rhs);

	/// Assignment from a raw ptr.
	SharedPtr<T> &operator=(T *ptr);

	// Releases use of the object.
	~SharedPtr();

	unsigned int RefCount() const; ///< Returns the reference count of the currently held object.

	const T *ptr() const; ///< Const access to the pointer.
	const T *operator->() const; ///< Const dereference and access object.
	const T &operator*() const; ///< Const dereference object.

	operator T*(); ///< Implicit conversion to a raw pointer.

	T *ptr(); ///< Returns a pointer to the object or 0 if empty.
	T *operator->(); ///< Dereference and access object.
	T &operator*(); ///< Dereference object.

	operator bool() const; ///< Test if pointer is good.

	operator bool(); ///< Test if pointer is good.
};

/** Equality comparison for two SharedPtr. Note that the types must match.
@param a First operand
@param b Second operand
@return true if the objects point to the same object, false otherwise. */
template<typename T>
bool operator==(const SharedPtr<T> &a, const SharedPtr<T> &b)
{
	if (a.ptr() == b.ptr())
		return true;
	else
		return false;
}

}

#endif
