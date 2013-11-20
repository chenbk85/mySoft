#ifndef __SMART_PTR_H__
#define __SMART_PTR_H__

#include "thread_lock.h"

class RefCountObjectBase
{
public:
	RefCountObjectBase () {}
	virtual ~RefCountObjectBase () {}
	virtual int _add_ref () = 0;
	virtual int _remove_ref (bool autoDelete) = 0;
	virtual int _ref_count () = 0;
};

class RefCountObject: public RefCountObjectBase
{
public:
	RefCountObject ()
	{
		_refcount = 0;
	}

	virtual ~RefCountObject ()
	{
		_refcount = 0;
	}

	virtual int _add_ref ()
	{
		return _inc_count(1, 0, true);
	}

	virtual int _remove_ref (bool autoDelete)
	{
		return _inc_count(-1, 0, autoDelete);
	}

	virtual int _ref_count ()
	{
		return _refcount;
	}

	virtual int _inc_count (int inc, int threshold, bool autoDelete)
	{
		  int rc = (_refcount += inc);
		  if (autoDelete && rc <= threshold)
		  {
			delete this;
		  }

		  return rc;
	}

protected:
	int _refcount;
};

class MTRefCountObject: public RefCountObject
{
public:
	MTRefCountObject (){}

	virtual ~MTRefCountObject (){}

	virtual int _add_ref ()
	{
		return _inc_count(1, 0, true);
	}

	virtual int _remove_ref (bool autoDelete)
	{
		return _inc_count(-1, 0, autoDelete);
	}

	virtual int _ref_count ()
	{
		return RefCountObject::_ref_count();
	}

	virtual int _inc_count (int inc, int threshold, bool autoDelete)
	{
		_lock.lock();
		int rc = RefCountObject::_inc_count(inc, threshold, autoDelete);
		_lock.unlock();
		return rc;
	}

protected:
	thread_lock _lock;
};

/**
 * @class  TFC_RefCountPtr
 * @brief  Smart Pointer (using ref-count)
 * When instance of RefCountPtr is created, it will call the TYPE's
 * _add_ref(), and when instance of RefCountPtr is destroyed, it will call
 * the TYPE's _remove_ref()
 *
 * @see
 */
template <typename TYPE>
class RefCountPtr
{
public:
  /// Default Constructor
  RefCountPtr (): _ptr(NULL)
  {
  }

  /// Convert Constructor
  RefCountPtr (TYPE* ptr): _ptr(ptr)
  {
    if (_ptr != NULL)
      _ptr->_add_ref();
  }

  /// Copy Constructor
  RefCountPtr (const RefCountPtr<TYPE>& other): _ptr(other._ptr)
  {
    if (_ptr != NULL)
      _ptr->_add_ref();
  }

  /// Destructor
  ~RefCountPtr ()
  {
    if (_ptr != NULL)
      _ptr->_remove_ref(true);
  }

  /// Overload operator "="
  RefCountPtr& operator = (const RefCountPtr<TYPE>& other)
  {
    if (&other != this)
    {
      // release self first
      if (_ptr != NULL)
        _ptr->_remove_ref(true);

      _ptr = other._ptr;

      // modified by PM, add if clause
      if (_ptr != NULL)
        _ptr->_add_ref();
    }

    return *this;
  }

  /// Whether the RefCountPtr is pointing to NULL.
  bool isNull () const
  {
    return (_ptr == NULL);
  }

  /// Overload operator "->"
  TYPE* operator -> () const
  {
    return _ptr;
  }

  /// Return the pointer
  TYPE* ptr () const
  {
    return _ptr;
  }

  /// Return the input pointer
  TYPE* in () const
  {
    return _ptr;
  }

  /// Return the output pointer
  TYPE*& out () const
  {
    if (_ptr != NULL)
    {
      _ptr->_remove_ref(true);
      _ptr = NULL;
    }

    return _ptr;
  }

  /// Return the pointer & release the control of pointed object
  TYPE* retn ()
  {
    TYPE* ptr = _ptr;
    _ptr = NULL;
    if (ptr != NULL)
      ptr->_remove_ref(false);
    return ptr;
  }

  /// Overload operator TYPE* cast
  operator TYPE* () const
  {
    return _ptr;
  }

  bool operator < (const RefCountPtr& other) const
  {
    if (_ptr < other._ptr)
      return true;
    return false;
  }

protected:
  TYPE* _ptr;
};

#endif //__SMART_PTR_H__
