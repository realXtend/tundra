/** @file 
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief Template implementation for the intrusive smart shared pointer class.
*/
#ifndef SharedPtr_inl
#define SharedPtr_inl

#include <algorithm>

namespace clb
{

/** This constructor takes the ownership of the raw pointer and starts managing the 
	reference count on it, assuming that it is the first pointer to access it.

	Thus, when invoking this constructor you need to be sure that the passed pointer
	is not being reference-counted someplace else, because the reference count is stored
	in the referencing smart pointers rather than the object itself. 
	@param ptr_ Raw pointer to an object that is not being refCounted anywhere else. */
template<typename T>
SharedPtr<T>::SharedPtr(T *ptr_)
:dataPtr(ptr_)
{
	AddRef();
}

/** This constructor increases reference on the existing SharedPtr, and initializes
	this to point to the same object and refCount as the existing one. This is the 
	safe way to copy pointers. 
	@param rhs The pointer to make copy of. */
template<typename T>
SharedPtr<T>::SharedPtr(const SharedPtr<T> &rhs)
:dataPtr(rhs.dataPtr)
{
	AddRef();
}

/** This constructor allows to convert SharedPtr's from Derived types to SharedPtr's
	of Base types, or more specifically, from any type A to type B, as long as there
	is an implicit conversion operator available for the specified types. 
	@param rhs The pointer to make copy of. */
template<typename T> // type of this
	template<typename T2> // type of rhs
SharedPtr<T>::SharedPtr(const SharedPtr<T2> &rhs)
:dataPtr(dynamic_cast<T*>(const_cast<T2*>(rhs.ptr())))	// Note: Having to resort to const_cast instead of static_cast here. :O
{
	AddRef();
}

/** Decreases refCount on old object and modifies this to point to the new object,
	increasing it's refCount. 
	@param rhs The pointer to make copy of. 
	@return Reference to this. */
template<typename T>
SharedPtr<T> &SharedPtr<T>::operator=(const SharedPtr<T> &rhs)
{
	// No task if the pointers already point to the same object.
	if (dataPtr == rhs.dataPtr)
		return *this;

	SharedPtr<T> copy(*this);
//	if (rhs.dataPtr)
//		rhs.dataPtr->AddRef();

	// Decrease refCount on old object (possibly deallocated the data).
	Release();

	// Assign the new.
	dataPtr = rhs.dataPtr;

	// Increase reference count for the new.
	AddRef();

	return *this;
}

/** Decreases refCount on old object (possibly frees it) and takes the ownership of 
	the new object, making the assumption that this is the first SharedPtr to count
	reference to the object.
	@param ptr_ Raw pointer to an object that is not being refCounted anywhere else. 
	@return Reference to this. */
template<typename T>
SharedPtr<T> &SharedPtr<T>::operator=(T *ptr_)
{
	if (dataPtr == ptr_) // This and the given pointers point to same object.
		return *this;

	// Decrease refCount on old object (possibly releases).
	Release();

	// Assign the new.
	dataPtr = ptr_;

	AddRef();

	return *this;
}

/** Decreases refCount on the object and frees it if it's no longer used. */
template<typename T>
SharedPtr<T>::~SharedPtr()
{
	Release();
}


template<typename T>
void SharedPtr<T>::AddRef()
{
	if (dataPtr)
		dataPtr->AddRef();
}

/** If the reference count drops to zero, the object is freed. */
template<typename T>
void SharedPtr<T>::Release()
{
	if (!dataPtr) // if refCount is zero, this pointer wasn't even initialized.
		return;

	dataPtr->DecRef();

	assert(dataPtr->RefCount() >= 0 && "Reference count was decreased to a negative value!!");

	// Free the object if no users left
	if (dataPtr->RefCount() == 0)
		delete dataPtr;

	dataPtr = 0;
}

/** @return The reference count of the object this SharedPtr is pointing at, or 
	0 if the SharedPtr isn't pointing to anything. */
template<typename T>
unsigned int SharedPtr<T>::RefCount() const
{
	if (dataPtr)
		return dataPtr->RefCount();
	else
		return 0;
}

/** Causes an assert failure if the pointer is null. If reading a null pointer
	should be considered a legal operation, use .ptr() instead. */
template<typename T>
SharedPtr<T>::operator T*()
{
	assert(dataPtr != 0 && dataPtr->RefCount() > 0 && "Tried to dereference a null pointer!");
	return dataPtr;
}

/** @return Const pointer to the object or 0. */
template<typename T>
const T *SharedPtr<T>::ptr() const
{
	return dataPtr;
}

/** @return Pointer to the object or 0. */
template<typename T>
T *SharedPtr<T>::ptr()
{
	return dataPtr;
}

/** Causes an assert failure if the pointer is null. 
	@return Const pointer to the object. */
template<typename T>
const T *SharedPtr<T>::operator->() const
{
	assert(dataPtr != 0 && dataPtr->RefCount() > 0 && "Tried to dereference a null pointer!");
	return dataPtr;
}

/** Causes an assert failure if the pointer is null. 
	@return Pointer to the object. */
template<typename T>
T *SharedPtr<T>::operator->()
{
	assert(dataPtr != 0 && dataPtr->RefCount() > 0 && "Tried to dereference a null pointer!");
	return dataPtr;
}

/** Causes an assert failure if the pointer is null.
	@return Const reference to the object. */
template<typename T>
const T &SharedPtr<T>::operator*() const
{
	assert(dataPtr != 0 && dataPtr->RefCount() > 0 && "Tried to dereference a null pointer!");
	return *dataPtr;
}

/** Causes an assert failure if the pointer is null.
	@return Reference to the object. */
template<typename T>
T &SharedPtr<T>::operator*()
{
	assert(dataPtr != 0 && dataPtr->RefCount() > 0 && "Tried to dereference a null pointer!");
	return *dataPtr;
}

/** Causes an assert failure if the pointer is null.
	Call this function only for ArrayPtrs or objects you know are laid contiguously
	in memory. There is no bounds checking, since SharedPtr is not aware of the amount
	of objects.
	@param index Index of the array to access.
	@return Const reference to the element specified by index. */
/*
template<typename T>
const T &SharedPtr<T>::operator[](int index) const
{
	assert(dataPtr != 0 && dataPtr->RefCount() > 0 && "Tried to dereference a null pointer!");

	return dataPtr[index];
}
*/
/** Causes an assert failure if the pointer is null.
	Call this function only for ArrayPtrs or objects you know are laid contiguously
	in memory. There is no bounds checking, since SharedPtr is not aware of the amount
	of objects.
	@param index Index of the array to access.
	@return Reference to the element specified by index. */
/*
template<typename T>
T &SharedPtr<T>::operator[](int index)
{
	assert(dataPtr != 0 && dataPtr->RefCount() > 0 && "Tried to dereference a null pointer!");

	return dataPtr[index];
}
*/
/** @return true if the pointer is good, false if it's null. */
template<typename T>
SharedPtr<T>::operator bool() const
{
	return dataPtr != 0;
}

/** @return true if the pointer is good, false if it's null. */
template<typename T>
SharedPtr<T>::operator bool()
{
	return dataPtr != 0;
}

}

#endif
