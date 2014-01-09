#ifndef _REF_COUNT_H_
#define _REF_COUNT_H_

#include "atomic.h"

namespace eau
{
    /**
     * usage:
     *      class A : public RefCount{...}; // class A is one virtual class, cannot new directly
     *      zeroptr<A> pA = new RefCounted<A>(); // you can new one object by this way
     *      pA->func();
     *      // neednot to delete pA
     *
     *      class B : public RefCounted<A>{...}; // class B is one normal class
     *      zeroptr<B> pB = new B();
     *      pB->func();
     *      // neednot to delete pB
     */

    class RefCount
    {
    public:
        virtual int AddRef() = 0;
        virtual int Release() = 0;

    protected:
        virtual ~RefCount() {}
    }; // class RefCount

    template <class T> class RefCounted : public T
    {
    public:
        RefCounted() : _ref_count(0) {}

        template<typename U> 
        explicit RefCounted(U u) : T(u), _ref_count(0) {}

        template<typename U1, typename U2>
        RefCounted(U1 u1, U2 u2) : T(u1, u2), _ref_count(0) {}

        template<typename U1, typename U2, typename U3>
        RefCounted(U1 u1, U2 u2, U3 u3) : T(u1, u2, u3), _ref_count(0) {}

        template<typename U1, typename U2, typename U3, typename U4>
        RefCounted(U1 u1, U2 u2, U3 u3, U4 u4) : T(u1, u2, u3, u4), _ref_count(0) {}
        
        template<typename U1, typename U2, typename U3, typename U4, typename U5>
        RefCounted(U1 u1, U2 u2, U3 u3, U4 u4, U5 u5) : T(u1, u2, u3, u4, u5), _ref_count(0) {}

        virtual int AddRef()
        {
            return (int)atomic::inc(&_ref_count);
        }

        virtual int Release()
        {
            int count = (int)atomic::dec(&_ref_count);
            if (!count) delete this;
            return count;
        }
        
    protected:
        virtual ~RefCounted() {}
        atomic::cas_t _ref_count;
    }; // class RefCounted

} // namespace eau

#endif
