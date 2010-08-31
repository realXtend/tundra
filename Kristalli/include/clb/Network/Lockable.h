/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef Lockable_h
#define Lockable_h

#include <windows.h>

template<typename T>
class Lockable;

/// @internal Wraps mutex-lock acquisition and releasing into a RAII-style object that automatically releases the lock when the scope
/// is exited.
template<typename T>
class Lock
{
	Lockable<T> *lockedObject;
	T *value;

	void TearDown() { lockedObject = 0; value = 0; }

public:
	explicit Lock(Lockable<T> *lockedObject_)
	:lockedObject(lockedObject_), value(&lockedObject->Lock())
	{
	}

	Lock(const Lock<T> &rhs)
	:lockedObject(rhs.lockedObject), value(rhs.value)
	{
		assert(this != &rhs);
		rhs.TearDown();
	}

	Lock<T> &operator=(const Lock<T> &rhs)
	{
		if (&rhs == this)
			return *this;

		lockedObject = rhs.lockedObject();
		value = rhs.value;

		rhs.TearDown();
	}

	~Lock()
	{
		if (lockedObject)
			lockedObject->Unlock();
	}

	T *operator ->() const { return value; }
	T &operator *() { return *value; }
};

/// @internal Wraps mutex-lock acquisition and releasing to const data into a RAII-style object 
/// that automatically releases the lock when the scope is exited.
template<typename T>
class ConstLock
{
	const Lockable<T> *lockedObject;
	const T *value;

	void TearDown() { lockedObject = 0; value = 0; }

public:
	explicit ConstLock(const Lockable<T> *lockedObject_)
	:lockedObject(lockedObject_), value(&lockedObject->Lock())
	{
	}

	ConstLock(const ConstLock<T> &rhs)
	:lockedObject(rhs.lockedObject), value(rhs.value)
	{
		assert(this != &rhs);
		rhs.TearDown();
	}

	ConstLock<T> &operator=(const ConstLock<T> &rhs)
	{
		if (&rhs == this)
			return *this;

		lockedObject = rhs.lockedObject();
		value = rhs.value;

		rhs.TearDown();
	}

	~ConstLock()
	{
		if (lockedObject)
			lockedObject->Unlock();
	}

	const T *operator ->() const { return value; }
	const T &operator *() const { return *value; }
};

/// Stores an object of type T behind a mutex-locked shield. To access the object, one has to acquire a lock to it first, and remember
/// to free the lock when done. Use @see Lock and @see ConstLock to manage the locks in a RAII-style manner.
template<typename T>
class Lockable
{
private:
	T value;
	mutable CRITICAL_SECTION lockObject;

	void operator=(const Lockable<T> &);
	Lockable(const Lockable<T> &);

public:
	typedef Lock<T> LockType;
	typedef ConstLock<T> ConstLockType;

	Lockable()
	{
		InitializeCriticalSection(&lockObject);
	}
/*
	Lockable(const Lockable<T> &other)
	{		
		InitializeCriticalSection(&lockObject);
		value = other.Lock();
		other.Unlock();
	}
*/
	explicit Lockable(const T &value_)
	:value(value_)
	{
		InitializeCriticalSection(&lockObject);
	}

	~Lockable()
	{
		DeleteCriticalSection(&lockObject);
	}
/*
	Lockable &operator=(const Lockable<T> &other)
	{
		if (this == &other)
			return *this;

		this->Lock();
		value = other.Lock();
		other.Unlock();
		this->Unlock();
	}
*/
	T &Lock()
	{
		EnterCriticalSection(&lockObject);
		return value;
	}

	const T &Lock() const
	{
		EnterCriticalSection(&lockObject);
		return value;
	}

	void Unlock() const
	{
		LeaveCriticalSection(&lockObject);
	}

	LockType Acquire()
	{
		return LockType(this);
	}

	ConstLockType Acquire() const
	{
		return ConstLockType(this);
	}
};

#endif
