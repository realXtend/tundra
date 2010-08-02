/** @file 
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief Template implementation for nonintrusive shared smart pointer class.
*/

#ifndef SmartPtr_inl
#define SmartPtr_inl

#include <algorithm>

namespace clb
{

/** This constructor takes the ownership of the raw pointer and starts managing the 
	reference count on it, assuming that it is the first pointer to access it.

	Thus, when invoking this constructor you need to be sure that the passed pointer
	is not being reference-counted someplace else, because the reference count is stored
	in the referencing smart pointers rather than the object itself. 
	@param ptr_ Raw pointer to an object that is not being refCounted anywhere else. */
template<typename T, class FreeFunc>
Smartptr<T, FreeFunc>::Smartptr(T *ptr_)
:dataPtr(ptr_)
{
	if (ptr_ == 0) // Don't initialize reference counting if null passed.
		refCount = 0;
	else
		refCount = new int(1); // This object is the first reference to the pointer.
}

/** This constructor increases reference on the existing Smartptr, and initializes
	this to point to the same object and refCount as the existing one. This is the 
	safe way to copy pointers. 
	@param rhs The pointer to make copy of. */
template<typename T, class FreeFunc>
Smartptr<T, FreeFunc>::Smartptr(const Smartptr<T, FreeFunc> &rhs)
:dataPtr(rhs.dataPtr), refCount(rhs.refCount)
{
	// Increase reference count if not null object.
	if (refCount)
	{
		++*refCount;
		assert(*refCount >= 2 && "Created a copy of smart pointer, but the reference count is too low to hold both objects!");
	}
}

/** This constructor allows to convert Smartptr's from Derived types to Smartptr's
	of Base types, or more specifically, from any type A to type B, as long as there
	is an implicit conversion operator available for the specified types. 
	@param rhs The pointer to make copy of. */
template<typename T, class FreeFunc> // type of this
	template<typename T2, typename FreeFunc2> // type of rhs
Smartptr<T, FreeFunc>::Smartptr(Smartptr<T2, FreeFunc2> &rhs)
:dataPtr(static_cast<T*>(rhs.ptr())), refCount(rhs.RefCountPtr())
{
	// Increase reference count if not null object.
	if (refCount)
	{
		++*refCount;
		assert(*refCount >= 2 && "Created a copy of smart pointer, but the reference count is too low to hold both objects!");
	}
}

/** Decreases refCount on old object and modifies this to point to the new object,
	increasing it's refCount. 
	@param rhs The pointer to make copy of. 
	@return Reference to this. */
template<typename T, class FreeFunc >
Smartptr<T, FreeFunc> &Smartptr<T, FreeFunc>::operator=(const Smartptr<T, FreeFunc> &rhs)
{
	// No task if the pointers already point to the same object.
	if (rhs.refCount == this->refCount)
		return *this;

	// Decrease refCount on old object (possibly releases).
	Release();

	// Assign the new.
	dataPtr = rhs.dataPtr;
	refCount = rhs.refCount;

	// Increase reference count for the new.
	if (refCount)
		++*refCount;

	return *this;
}

/** Decreases refCount on old object (possibly frees it) and takes the ownership of 
	the new object, making the assumption that this is the first Smartptr to count
	reference to the object.
	@param ptr_ Raw pointer to an object that is not being refCounted anywhere else. 
	@return Reference to this. */
template<typename T, class FreeFunc >
Smartptr<T, FreeFunc> &Smartptr<T, FreeFunc>::operator=(T *ptr_)
{
	if (dataPtr == ptr_) // This and the given pointers point to same object.
	{
		assert(dataPtr == 0 && "Trying to assign a raw pointer that is already refCounted to a shared pointer"); // passed pointer must be unique or 0, and it wasn't unique.
		return *this;
	}

	// Create a local copy first:
	Smartptr<T, FreeFunc> newPtr(ptr_);

	// Decrease refCount on old object (possibly releases).
	Release();

	// Assign the new.
	dataPtr = newPtr.dataPtr;
	refCount = newPtr.refCount;

	if (refCount) // if not pointing to a null object.
		++*refCount;

	return *this;
}

/** Decreases refCount on the object and frees it if it's no longer used. */
template<typename T, class FreeFunc >
Smartptr<T, FreeFunc>::~Smartptr()
{
	Release();
}


template<typename T, class FreeFunc >
void Smartptr<T, FreeFunc>::AddRef()
{
	if (refCount) // if not pointing to a null object.
		++*refCount;
}

/** If the reference count drops to zero, the object is freed. */
template<typename T, class FreeFunc >
void Smartptr<T, FreeFunc>::Release()
{
	if (!refCount) // if refCount is zero, this pointer wasn't even initialized.
		return;

	assert(dataPtr != 0 && "Tried to release a null pointer!");

	--*refCount;
	assert(*refCount >= 0 && "Reference count was decreased to a negative value!!");

	// Free the object if no users left
	if (*refCount == 0)
	{
		FreeFunc::Free(dataPtr);
		dataPtr = 0;
		delete refCount;
		refCount = 0;
	}
}

/** @return The reference count of the object this Smartptr is pointing at, or 
	0 if the Smartptr isn't pointing to anything. */
template<typename T, class FreeFunc >
int Smartptr<T, FreeFunc>::RefCount() const
{
	if (!refCount)
		return 0;
	else
		return *refCount;
}

/** This is a helper function to allow simpler internal copying. It is of no use
	to client applications.
	@return This object's refCount pointer value. */
template<typename T, class FreeFunc >
int *Smartptr<T, FreeFunc>::RefCountPtr() const
{
	return refCount;
}


/** Causes an assert failure if the pointer is null. If reading a null pointer
	should be considered a legal operation, use .ptr() instead. */
template<typename T, class FreeFunc >
Smartptr<T, FreeFunc>::operator T*()
{
	assert(dataPtr != 0 && *refCount > 0 && "Tried to dereference a null pointer!");
	return dataPtr;
}

/** @return Const pointer to the object or 0. */
template<typename T, class FreeFunc >
const T *Smartptr<T, FreeFunc>::ptr() const
{
	return dataPtr;
}

/** @return Pointer to the object or 0. */
template<typename T, class FreeFunc >
T *Smartptr<T, FreeFunc>::ptr()
{
	return dataPtr;
}

/** Causes an assert failure if the pointer is null. 
	@return Const pointer to the object. */
template<typename T, class FreeFunc >
const T *Smartptr<T, FreeFunc>::operator->() const
{
	assert(dataPtr != 0 && *refCount > 0 && "Tried to dereference a null pointer!");
	return dataPtr;
}

/** Causes an assert failure if the pointer is null. 
	@return Pointer to the object. */
template<typename T, class FreeFunc >
T *Smartptr<T, FreeFunc>::operator->()
{
	assert(dataPtr != 0 && *refCount > 0 && "Tried to dereference a null pointer!");
	return dataPtr;
}

/** Causes an assert failure if the pointer is null.
	@return Const reference to the object. */
template<typename T, class FreeFunc >
const T &Smartptr<T, FreeFunc>::operator*() const
{
	assert(dataPtr != 0 && *refCount > 0 && "Tried to dereference a null pointer!");
	return *dataPtr;
}

/** Causes an assert failure if the pointer is null.
	@return Reference to the object. */
template<typename T, class FreeFunc >
T &Smartptr<T, FreeFunc>::operator*()
{
	assert(dataPtr != 0 && *refCount > 0 && "Tried to dereference a null pointer!");
	return *dataPtr;
}

/** Causes an assert failure if the pointer is null.
	Call this function only for ArrayPtrs or objects you know are laid contiguously
	in memory. There is no bounds checking, since Smartptr is not aware of the amount
	of objects.
	@param index Index of the array to access.
	@return Const reference to the element specified by index. */
template<typename T, class FreeFunc >
const T &Smartptr<T, FreeFunc>::operator[](int index) const
{
	assert(dataPtr != 0 && *refCount > 0 && "Tried to dereference a null pointer!");

	return dataPtr[index];
}

/** Causes an assert failure if the pointer is null.
	Call this function only for ArrayPtrs or objects you know are laid contiguously
	in memory. There is no bounds checking, since Smartptr is not aware of the amount
	of objects.
	@param index Index of the array to access.
	@return Reference to the element specified by index. */
template<typename T, class FreeFunc >
T &Smartptr<T, FreeFunc>::operator[](int index)
{
	assert(dataPtr != 0 && *refCount > 0 && "Tried to dereference a null pointer!");

	return dataPtr[index];
}

/** @return true if the pointer is good, false if it's null. */
template<typename T, class FreeFunc >
Smartptr<T, FreeFunc>::operator bool()
{
	return dataPtr != 0;
}

}

#endif
