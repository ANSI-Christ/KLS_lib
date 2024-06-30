#ifndef CLASS_H
#define CLASS_H

#include "macro.h"

#define _CLASS_DECL CLASS_BEGIN__

#define CLASS struct

#define CLASS_END(_name_,...) \
    CLASS _name_{void(*const destructor)(void*); _CLASS_LOOP(_CLASS_END(_name_))};\
    struct M_JOIN(_,_name_){M_IF(M_FOREACH(_CLASS_ABS,M_JOIN(_CLASS_DECL,_name_)))(void *_##__LINE__##pad[1][1][1],CLASS _name_*(*constructor)(void*)); _CLASS_LOOP(_CLASS_END(_name_))};\
    extern const struct M_JOIN(_,_name_) *_name_()

#define CLASS_COMPILE(_name_) \
    void *M_JOIN(_add_,_name_)(CLASS _name_*,char);\
    void M_JOIN(_dtor2_,_name_)(CLASS _name_ *self){ M_FOREACH(_CLASS_DTR,M_JOIN(_CLASS_DECL,_name_)) return; (void)self;}\
    void *M_JOIN(_ctor2_,_name_)(CLASS _name_ *self){ M_FOREACH(_CLASS_CTR,M_JOIN(_CLASS_DECL,_name_)) return self;}\
    void M_JOIN(_dtor_,_name_)(void *self){\
        M_JOIN(_dtor2_,_name_)(self);\
        M_JOIN(_add_,_name_)(self,0);\
        M_WHEN(_CLASS_BASE(_name_))({\
            extern void M_JOIN(_dtor_,_CLASS_BASE(_name_))(void*);\
            M_JOIN(_dtor_,_CLASS_BASE(_name_))(self);\
        })\
    }\
    void *M_JOIN(_ctor_,_name_)(void *self){\
        if(self){\
            M_WHEN(_CLASS_BASE(_name_))(\
                extern void *M_JOIN(_ctor_,_CLASS_BASE(_name_))(void*);\
                if(!M_JOIN(_ctor_,_CLASS_BASE(_name_))(self)) return 0;\
            )\
            if(M_JOIN(_ctor2_,_name_)(self) && M_JOIN(_add_,_name_)(self,1)){\
                *(void**)self=M_JOIN(_dtor_,_name_);\
                return self;\
            }\
            M_JOIN(_dtor_,_name_)(self);\
        } return 0;\
    }\
    const struct M_JOIN(_,_name_) *_name_(){\
        static struct M_JOIN(_,_name_) c[1]={{M_IF(M_FOREACH(_CLASS_ABS,M_JOIN(_CLASS_DECL,_name_)))({{{(void*)1}}},(void*)1)}};\
        if(*(void**)c==(void*)1){\
            *(void**)c=(void*)0;\
            M_WHEN(_CLASS_BASE(_name_))( _CLASS_BASE(_name_)(); )\
            _ctor_##_name_(c); _dtor_##_name_(c);\
            *(void**)c=(void*)M_IF(M_FOREACH(_CLASS_ABS,M_JOIN(_CLASS_DECL,_name_)))(0,_ctor_##_name_);\
        } return c;\
    }\
    void *M_JOIN(_add_,_name_)(CLASS _name_ *self,const char _____) _CLASS_COMPILE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _CLASS_LOOP(...)  _CLASS_LOOP0(_CLASS_LOOP0(_CLASS_LOOP0(__VA_ARGS__)))
#define _CLASS_LOOP0(...) _CLASS_LOOP1(_CLASS_LOOP1(_CLASS_LOOP1(__VA_ARGS__)))
#define _CLASS_LOOP1(...) _CLASS_LOOP2(_CLASS_LOOP2(_CLASS_LOOP2(__VA_ARGS__)))
#define _CLASS_LOOP2(...) _CLASS_LOOP3(_CLASS_LOOP3(_CLASS_LOOP3(__VA_ARGS__)))
#define _CLASS_LOOP3(...) __VA_ARGS__

#define _CLASS_INH_from(...) __VA_ARGS__
#define _CLASS_INH_public(...)
#define _CLASS_INH_private(...)
#define _CLASS_INH_constructor(...)
#define _CLASS_INH_destructor(...)
#define _CLASS_INH_abstract

#define _CLASS_PBL_from(...)
#define _CLASS_PBL_public(...) __VA_ARGS__
#define _CLASS_PBL_private(...)
#define _CLASS_PBL_constructor(...)
#define _CLASS_PBL_destructor(...)
#define _CLASS_PBL_abstract

#define _CLASS_PRV_from(...)
#define _CLASS_PRV_public(...)
#define _CLASS_PRV_private(...) __VA_ARGS__
#define _CLASS_PRV_constructor(...)
#define _CLASS_PRV_destructor(...)
#define _CLASS_PRV_abstract

#define _CLASS_CTR_from(...)
#define _CLASS_CTR_public(...)
#define _CLASS_CTR_private(...)
#define _CLASS_CTR_constructor(...) __VA_ARGS__
#define _CLASS_CTR_destructor(...)
#define _CLASS_CTR_abstract

#define _CLASS_DTR_from(...)
#define _CLASS_DTR_public(...)
#define _CLASS_DTR_private(...)
#define _CLASS_DTR_constructor(...)
#define _CLASS_DTR_destructor(...) __VA_ARGS__
#define _CLASS_DTR_abstract

#define _CLASS_ABS_from(...)
#define _CLASS_ABS_public(...)
#define _CLASS_ABS_private(...)
#define _CLASS_ABS_constructor(...)
#define _CLASS_ABS_destructor(...)
#define _CLASS_ABS_abstract   1

#define _CLASS_INH(...) M_WHEN(M_IS_ARG(__VA_ARGS__))( _CLASS_INH_##__VA_ARGS__ )
#define _CLASS_PBL(...) M_WHEN(M_IS_ARG(__VA_ARGS__))( _CLASS_PBL_##__VA_ARGS__ )
#define _CLASS_PRV(...) M_WHEN(M_IS_ARG(__VA_ARGS__))( _CLASS_PRV_##__VA_ARGS__ )
#define _CLASS_CTR(...) M_WHEN(M_IS_ARG(__VA_ARGS__))( _CLASS_CTR_##__VA_ARGS__ )
#define _CLASS_DTR(...) M_WHEN(M_IS_ARG(__VA_ARGS__))( _CLASS_DTR_##__VA_ARGS__ )
#define _CLASS_ABS(...) M_WHEN(M_IS_ARG(__VA_ARGS__))( _CLASS_ABS_##__VA_ARGS__ )

#define _CLASS_BASE(_name_) M_FOREACH(_CLASS_INH,M_JOIN(_CLASS_DECL,_name_))

#define _CLASS_COMPILE(...) {if(_____){M_FOREACH(_CLASS_CTR,__VA_ARGS__)}else{M_FOREACH(_CLASS_DTR,__VA_ARGS__)} return self;(void)self;}

#define __CLASS_END() _CLASS_END
#define _CLASS_END(_name_,...)\
    M_WHEN(_CLASS_BASE(_name_))( M_OBSTRUCT(__CLASS_END)()(_CLASS_BASE(_name_),_CLASS_BASE(_name_)) ) \
    M_FOREACH(_CLASS_PBL,M_JOIN(_CLASS_DECL,_name_))\
    M_IF(M_IS_ARG(__VA_ARGS__))(\
        M_WHEN(M_IS_ARG(M_PEEK(M_FOREACH(_CLASS_PRV,M_JOIN(_CLASS_DECL,_name_)))))( struct{M_FOREACH(_CLASS_PRV,M_JOIN(_CLASS_DECL,_name_))}_pad##__VA_ARGS__[1][1][1]; ) , \
        M_FOREACH(_CLASS_PRV,M_JOIN(_CLASS_DECL,_name_))\
    )

#endif // CLASS_H
