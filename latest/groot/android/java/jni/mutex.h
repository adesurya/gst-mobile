#ifndef _EAU_MUTEX_H_
#define _EAU_MUTEX_H_

namespace eau
{
    /**
     * usage: several macros to control its implemention
     *      HAVE_PTHREAD_H - use pthead
     *      WIN32 - use windows CRITICAL_SECTION
     */

    class MutexImpl
    {
    public:
        explicit MutexImpl(bool recursive=true);
        ~MutexImpl();

    public:
        bool acquire();
        bool release();
        bool tryacquire();
        
    private:
        MutexImpl(const MutexImpl &original);
        void operator =(const MutexImpl &);

    private:
        void* _data;
    };

    class SmartMutex : public MutexImpl
    {
    public:
        explicit SmartMutex(bool recursive=true) : MutexImpl(recursive) {}
        
        bool acquire()
        {
            return MutexImpl::acquire();
        }

        bool release()
        {
            return MutexImpl::release();
        }

        bool tryacquire()
        {
            return MutexImpl::tryacquire();
        }

    private:
        SmartMutex(const SmartMutex & original);
        void operator =(const SmartMutex &);
    };

    typedef SmartMutex Mutex;

    class SmartScopedLock 
    {
    public:
        SmartScopedLock(SmartMutex &mtx) : _mtx(mtx)
        {
            _mtx.acquire();
        }
    
        ~SmartScopedLock() 
        {
            _mtx.release();
        }

    private:
        SmartMutex &_mtx;
    };

    typedef SmartScopedLock ScopedLock;
    typedef SmartScopedLock AutoLock;

} // namespace eau

#endif
