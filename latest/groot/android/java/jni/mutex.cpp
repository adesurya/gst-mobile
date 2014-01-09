#include "mutex.h"

using namespace eau;

#if defined(HAVE_PTHREAD_H)

#include <cassert>
#include <pthread.h>
#include <stdlib.h>

static const bool s_pthread_enabled = true;

MutexImpl::MutexImpl(bool recursive) : _data(0)
{
    if (s_pthread_enabled)
    {
        pthread_mutexattr_t attr;
        int rval = pthread_mutexattr_init(&attr);
        int kind = (recursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_NORMAL);
        rval = pthread_mutexattr_settype(&attr, kind);

#if !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
        rval = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
#endif
        
        pthread_mutex_t* mutex = (pthread_mutex_t*)(malloc(sizeof(pthread_mutex_t)));
        rval = pthread_mutex_init(mutex, &attr);
        _data = mutex;
    }
}

MutexImpl::~MutexImpl()
{
    if (s_pthread_enabled)
    {
        pthread_mutex_t* mutex = (pthread_mutex_t*)_data;
        pthread_mutex_destroy(mutex);
    }
}

bool MutexImpl::acquire()
{
    if (s_pthread_enabled)
    {
        pthread_mutex_t* mutex = (pthread_mutex_t*)_data;
        int rval = pthread_mutex_lock(mutex);
        return rval == 0;
    } 
    else 
        return false;
}

bool MutexImpl::release()
{
    if (s_pthread_enabled)
    {
        pthread_mutex_t* mutex = (pthread_mutex_t*)_data;
        int rval = pthread_mutex_unlock(mutex);
        return rval == 0;
    } 
    else 
        return false;
}

bool MutexImpl::tryacquire()
{
    if (s_pthread_enabled)
    {
        pthread_mutex_t* mutex = (pthread_mutex_t*)_data;
        int rval = pthread_mutex_trylock(mutex);
        return rval == 0;
    } 
    else 
        return false;
}

#elif defined(WIN32)

#include <Windows.h>

MutexImpl::MutexImpl(bool /*recursive*/)
{
    _data = new CRITICAL_SECTION;
    InitializeCriticalSection((LPCRITICAL_SECTION)_data);
}

MutexImpl::~MutexImpl()
{
    DeleteCriticalSection((LPCRITICAL_SECTION)_data);
    delete (LPCRITICAL_SECTION)_data;
    _data = 0;
}

bool MutexImpl::acquire()
{
    EnterCriticalSection((LPCRITICAL_SECTION)_data);
    return true;
}

bool MutexImpl::release()
{
    LeaveCriticalSection((LPCRITICAL_SECTION)_data);
    return true;
}

bool MutexImpl::tryacquire()
{
    return TryEnterCriticalSection((LPCRITICAL_SECTION)_data);
}

#else
#warning Neither HAVE_PTHREAD_H nor WIN32 was set in mutex.cpp 

MutexImpl::MutexImpl(bool /*recursive*/)
{
    _data = 0;
}

MutexImpl::~MutexImpl()
{
    _data = 0;
}

bool MutexImpl::acquire()
{
    return false;
}

bool MutexImpl::release()
{
    return false;
}

bool MutexImpl::tryacquire()
{
    return false;
}

#endif
