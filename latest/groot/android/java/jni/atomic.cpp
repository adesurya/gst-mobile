#include "atomic.h"

#if defined(_MSC_VER)
#include <windows.h>
#endif

namespace eau
{
namespace atomic 
{
    void memfence() 
    {
#if HAS_ATOMICS == 0
       return;
#  warning HAS_ATOMICS == 0, no memory fency operations for your platform!
#elif defined(__GNUC__)
        __sync_synchronize();
#elif defined(_MSC_VER)
        MemoryBarrier();
#else
#  error No memory fence implementation for your platform!
#endif
    }

    cas_t cmpandswap(volatile cas_t* ptr, cas_t new_val, cas_t old_val) 
    {
#if HAS_ATOMICS == 0
        cas_t result = *ptr;
        if (result == old_val)
            *ptr = new_val;
        return result;
#elif defined(__GNUC__)
        return __sync_val_compare_and_swap(ptr, old_val, new_val);
#elif defined(_MSC_VER)
        return InterlockedCompareExchange(ptr, new_val, old_val);
#else
#  error No compare-and-swap implementation for your platform!
#endif
    }

    cas_t inc(volatile cas_t* ptr) 
    {
#if HAS_ATOMICS == 0
        ++(*ptr);
        return *ptr;
#elif defined(__GNUC__)
        return __sync_add_and_fetch(ptr, 1);
#elif defined(_MSC_VER)
        return InterlockedIncrement(ptr);
#else
#  error No atomic increment implementation for your platform!
#endif
    }

    cas_t dec(volatile cas_t* ptr) 
    {
#if HAS_ATOMICS == 0
        --(*ptr);
        return *ptr;
#elif defined(__GNUC__)
        return __sync_sub_and_fetch(ptr, 1);
#elif defined(_MSC_VER)
        return InterlockedDecrement(ptr);
#else
#  error No atomic decrement implementation for your platform!
#endif
    }

    cas_t add(volatile cas_t* ptr, cas_t val) 
    {
#if HAS_ATOMICS == 0
        *ptr += val;
        return *ptr;
#elif defined(__GNUC__)
        return __sync_add_and_fetch(ptr, val);
#elif defined(_MSC_VER)
        return InterlockedExchangeAdd(ptr, val) + val;
#else
#  error No atomic add implementation for your platform!
#endif
    }

    cas_t mul(volatile cas_t* ptr, cas_t val) 
    {
        cas_t original, result;
        do {
            original = *ptr;
            result = original * val;
        } while (cmpandswap(ptr, result, original) != original);

        return result;
    }

    cas_t div(volatile cas_t* ptr, cas_t val) 
    {
        cas_t original, result;
        do {
            original = *ptr;
            result = original / val;
        } while (cmpandswap(ptr, result, original) != original);

        return result;
    }

} // namespace atomic
}
