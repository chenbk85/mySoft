#ifndef _THREAD_LOCK_
#define _THREAD_LOCK_

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

class thread_lock
{
public:

	inline thread_lock(void)
	{
#ifdef _WIN32
		::InitializeCriticalSection(&m_lock);
#else
		::pthread_mutex_init(&m_lock,NULL);
#endif
	}

	inline ~thread_lock(void)
	{
#ifdef _WIN32
		::DeleteCriticalSection(&m_lock);
#else
		::pthread_mutex_destroy(&m_lock);
#endif
	}

	inline void lock(void)
	{
#ifdef _WIN32
		::EnterCriticalSection(&m_lock);
#else
		::pthread_mutex_lock(&m_lock);
#endif
	}

	inline void unlock(void)
	{
#ifdef _WIN32
		::LeaveCriticalSection(&m_lock);
#else
		::pthread_mutex_unlock(&m_lock);
#endif
	}

private:
#ifdef _WIN32
	CRITICAL_SECTION m_lock;
#else
	pthread_mutex_t  m_lock;
#endif
};

class auto_lock{
public:
	inline auto_lock(thread_lock& lock):m_lock(lock) { m_lock.lock(); }
	inline ~auto_lock()	{ m_lock.unlock(); }
private:
	thread_lock& m_lock;
};

#endif

