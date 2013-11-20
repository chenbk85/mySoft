#ifndef SINGLETON_H
#define SINGLETON_H
/***********************************************************************
*
*  Copyright ( C ) Morningstar, 2008. All rights reserved.
*
*  File:       Singleton.h
*
*  Contents:   
*
*  Classes:    
*
*  Author:     David Chen(david.chen@morningstar.com)
*
***********************************************************************/ 
#include "beyondy/mutex_cond.hpp"
template<class T>
class Singleton
{
public:
	static T& Instance()
	{
		if (m_pInstance == NULL)
		{
			beyondy::auto_lock<typeof m_mutex> guardian(m_mutex);
			if (m_pInstance == NULL)
			{
				MakeInstance();
			}
		}
		return *m_pInstance;
	}

private:

	Singleton()
	{
		m_bDestroyed = true;
	}

	static void MakeInstance()
	{
		if (!m_pInstance)
		{
			m_bDestroyed = false;
		}
		m_pInstance = new T();
		std::atexit(&DestroySingleton);
	}

	static void DestroySingleton()
	{
		//ASSERT(!m_bDestroyed);
		delete m_pInstance;
		m_pInstance = NULL;
		m_bDestroyed = true;
	}
	static beyondy::thread_mutex m_mutex;
	static T* m_pInstance;
	static bool m_bDestroyed;
};

template<class T>
T* Singleton<T>::m_pInstance;

template<class T>
bool Singleton<T>::m_bDestroyed;

template<class T>
beyondy::thread_mutex Singleton<T>::m_mutex = NULL;
#endif
