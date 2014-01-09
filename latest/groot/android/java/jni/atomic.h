#ifndef _EAU_ATOMIC_H_
#define _EAU_ATOMIC_H_

namespace eau
{
    /**
     * usage: several macros to control its implemention 
     *      HAS_ATOMICS - whether support atomic operations or not, 
     *      _MSC_VER - windows platform,
     *      __GNUC__ - gnu platform
     */

    namespace atomic 
    {
        void memfence();

#ifdef _MSC_VER
        typedef long        cas_t;
#else
        typedef unsigned int    cas_t;
#endif
        cas_t cmpandswap(volatile cas_t *ptr, cas_t old_val, cas_t new_val);
        cas_t inc(volatile cas_t *ptr);
        cas_t dec(volatile cas_t *ptr);
        cas_t add(volatile cas_t *ptr, cas_t val);
        cas_t mul(volatile cas_t *ptr, cas_t val);
        cas_t div(volatile cas_t *ptr, cas_t val);
    } // namespace atomic

} // namespace eau


#endif
