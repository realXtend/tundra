/** @file Smartptr.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief Smartptr, non-intrusive reference counting for multiple pointer owners.

	Tutorial 2 deals with basic usage on shared pointers.
*/

#ifndef SmartPtr_h
#define SmartPtr_h

#include <malloc.h>
#include <cstdlib>

// NOTE THE SLIGHTLY CLUMSY SYNTAX: In lack of a better name, SmartPtr is the macro, Smartptr is the class template.
/// Smart pointer for dynamic single object allocations on the heap.
#define SmartPtr(type) clb::Smartptr<type, clb::DeletePtr<type> >

/// Smart pointer for dynamic arrays allocated on the heap.
#define ArrayPtr(type) clb::Smartptr<type, clb::DeleteArrayPtr<type> >

/// Smart pointer for C-allocated heap memory (malloc, free).
#define CPtr(type) clb::Smartptr<type, clb::FreeCPtr<type> >

/// Smart pointer for handling COM-object references.
#define ComPtr(type) clb::Smartptr<type, clb::ReleasePtr<type> >

namespace clb
{

/** @brief Nonintrusive refcount-tracked single-object lifetime-manager.

	Smartptr wraps raw pointers to objects of type T and manages reference 
	counting on the number of users on the pointer. When all users abandon the object,
	FreeFunc::Free(T*) is called to clean up the memory used by the resource. */
template<typename T, class FreeFunc>
class Smartptr
{
private:
	T *dataPtr;			///< Pointer to the actual data.
	int *refCount;		///< Pointer to a single heap-allocated integer. Shared between all smart pointers.

	void AddRef();		///< Increases reference count.
	void Release();	///< Decreases reference count.

public:
	/// Constructs a zero pointer.
	Smartptr():dataPtr(0),refCount(0) {}

	/// Constructs from a raw pointer.
	Smartptr(T *ptr);

	/// Constructs from an existing Smartptr.
	Smartptr(const Smartptr<T, FreeFunc> &rhs);

	/// Template copy constructor for types that allow conversions to other types.
	template<typename T2, typename FreeFunc2>
	Smartptr(Smartptr<T2, FreeFunc2> &rhs);

	/// Assignment from another Smartptr.
	Smartptr<T, FreeFunc> &operator=(const Smartptr<T, FreeFunc> &rhs);

	// Assignment from a raw Smartptr.
	Smartptr<T, FreeFunc> &operator=(T *ptr);

	// Releases use of the object.
	~Smartptr();

	int RefCount() const; ///< Returns the reference count of the currently held object.
	int *RefCountPtr() const; ///< Returns a pointer to the reference count value.

	const T *ptr() const; ///< Const access to the pointer.
	const T *operator->() const; ///< Const dereference and access object.
	const T &operator*() const; ///< Const dereference object.
	const T &operator[](int index) const; ///< Const access an array element.

	operator T*(); ///< Implicit conversion to a raw pointer.

	T *ptr(); ///< Returns a pointer to the object or 0 if empty.
	T *operator->(); ///< Dereference and access object.
	T &operator*(); ///< Dereference object.
	T &operator[](int index); ///< Access an array element.

	operator bool(); ///< Test if pointer is good.
};

/** Equality comparison for two Smartptr. Note that the types must match.
	@param a First operand
	@param b Second operand
	@return true if the objects point to the same object, false otherwise. */
template<typename T, typename FreeFunc>
bool operator==(const Smartptr<T, FreeFunc> &a, const Smartptr<T, FreeFunc> &b)
{
	if (a.ptr() == b.ptr())
		return true;
	else
		return false;
}

/** @brief A policy object for Smartptr. This class of objects are allocated with C++ new and deallocated 
	with C++ delete. 

	Use the Ptr(type) macro to hold objects in this class. */
template<typename T>
class DeletePtr
{
public:
	static void Free(T *ptr) { delete ptr; } ///< Deallocates by invoking delete.
};

/** @brief DeleteArray is a policy object for Smartptr. This class of objects are 
	allocated with new[] and deallocated with delete[].

	Use the ArrayPtr(type) macro to hold objects in this class. */
template<typename T>		
class DeleteArrayPtr
{
public:
	static void Free(T *ptr) { delete[] ptr; } ///< Deallocates by invoking delete[].
};

/** @brief FreeCPtr is a policy object for Smartptr. This class of objects are 
	allocated with malloc() and deallocated with free(). 
	
	Use the CPtr(type) macro to hold objects in this class. */
template<typename T>
class FreeCPtr
{
public:
	static void Free(T *ptr) { ::free(ptr); } ///< Deallocates by invoking free()
};

/** @brief ReleasePtr is a policy object for Smartptr. This class of objects are
	COM objects, acquired however possible, and deallocated with a call to 
	IUnknown->Release(). 
	
	Use the ComPtr(type) macro to hold objects in this class. */
template<typename T>
class ReleasePtr
{
public:
	static void Free(T *ptr) { ptr->Release(); } ///< Deallocates by invoking IUnknown->Release()
};

}

#endif
