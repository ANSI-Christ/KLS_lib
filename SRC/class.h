#ifndef CLASS_H
#define CLASS_H

#include "macro.h"

#define _CLASS_DECL CLASS_BEGIN__

#define CLASS struct

#define CLASS_END(_name_,...) \
    _CLASS_ARGS_DEF(_name_)\
    CLASS _name_{void(* const destructor)(void*); _CLASS_LOOP(_CLASS_END(_name_))};\
    struct M_JOIN(_,_name_){M_IF(_CLASS_ABS(_name_))(void *_##__LINE__##pad,CLASS _name_*(* const constructor)(void* _CLASS_ARGS_STD(_name_))); _CLASS_LOOP(_CLASS_END(_name_))};\
    extern const struct M_JOIN(_,_name_) *_name_()

#define CLASS_COMPILE(_name_) \
    void *M_JOIN(_impl_,_name_)(const char,CLASS _name_* _CLASS_ARGS_STD(_name_) );\
    void M_JOIN(_dtor_,_name_)(void* self){\
        void(* const f)(const char,void*)=(void*)M_JOIN(_impl_,_name_);\
        f(0,self);\
        M_WHEN(_CLASS_EXT(_name_))({\
            extern void M_JOIN(_dtor_,_CLASS_EXT(_name_))(void*);\
            M_JOIN(_dtor_,_CLASS_EXT(_name_))(self);\
        })\
    }\
    void *M_JOIN(_ctor_,_name_)(void *self _CLASS_ARGS_VAR(_name_) ){\
        if(self){\
            void*(* const f)(const char,void* _CLASS_ARGS_VAR(_name_))=(void*)M_JOIN(_impl_,_name_);\
            M_IF(_CLASS_EXT(_name_))(\
                _CLASS_CLEAR(((char*)self)+sizeof(CLASS _CLASS_EXT(_name_)),sizeof(CLASS _name_)-sizeof(CLASS _CLASS_EXT(_name_))) ,\
                _CLASS_CLEAR(self,sizeof(CLASS _name_))\
            )\
            if(f(1,self _CLASS_ARGS_CALL(_name_)))\
                *(void**)self=M_JOIN(_dtor_,_name_);\
            else{ M_JOIN(_dtor_,_name_)(self); self=(void*)0; }\
        } return self;\
    }\
    const struct M_JOIN(_,_name_) *_name_(){\
        static struct M_JOIN(_,_name_) c[1]={{(void*)1}};\
        if(*(void**)c==(void*)1){\
            *(void**)c=(void*)0;\
            M_WHEN(_CLASS_EXT(_name_))( _CLASS_EXT(_name_)(); )\
            { _CLASS_ARGS_ZERO(_name_) _ctor_##_name_(c _CLASS_ARGS_CALL(_name_)); }\
            _dtor_##_name_(c);\
            *(void**)c=(void*)M_IF(_CLASS_ABS(_name_))(0,_ctor_##_name_);\
        } return c;\
    }\
    void *M_JOIN(_impl_,_name_)(const char _##__LINE__##ctr,CLASS _name_ *self _CLASS_ARGS_STD(_name_) ){\
        if(!_##__LINE__##ctr){_CLASS_DTR(_name_)}\
        if(_##__LINE__##ctr){_CLASS_SUPER(_name_) {_CLASS_CTR_BODY(_name_)} _CLASS_COMPILE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _CLASS_LOOP(...)  _CLASS_LOOP0(_CLASS_LOOP0(_CLASS_LOOP0(__VA_ARGS__)))
#define _CLASS_LOOP0(...) _CLASS_LOOP1(_CLASS_LOOP1(_CLASS_LOOP1(__VA_ARGS__)))
#define _CLASS_LOOP1(...) _CLASS_LOOP2(_CLASS_LOOP2(_CLASS_LOOP2(__VA_ARGS__)))
#define _CLASS_LOOP2(...) _CLASS_LOOP3(_CLASS_LOOP3(_CLASS_LOOP3(__VA_ARGS__)))
#define _CLASS_LOOP3(...) __VA_ARGS__

#define _CLASS_EXT_extends(...) __VA_ARGS__
#define _CLASS_EXT_public(...)
#define _CLASS_EXT_private(...)
#define _CLASS_EXT_constructor(...) M_SKIP
#define _CLASS_EXT_destructor(...) M_SKIP
#define _CLASS_EXT_abstract

#define _CLASS_PBL_extends(...)
#define _CLASS_PBL_public(...) __VA_ARGS__
#define _CLASS_PBL_private(...)
#define _CLASS_PBL_constructor(...) M_SKIP
#define _CLASS_PBL_destructor(...) M_SKIP
#define _CLASS_PBL_abstract

#define _CLASS_PRV_extends(...)
#define _CLASS_PRV_public(...)
#define _CLASS_PRV_private(...) __VA_ARGS__
#define _CLASS_PRV_constructor(...) M_SKIP
#define _CLASS_PRV_destructor(...) M_SKIP
#define _CLASS_PRV_abstract

#define _CLASS_CTR1_extends(...)
#define _CLASS_CTR1_public(...)
#define _CLASS_CTR1_private(...)
#define _CLASS_CTR1_constructor(...) __VA_ARGS__ M_SKIP
#define _CLASS_CTR1_destructor(...) M_SKIP
#define _CLASS_CTR1_abstract

#define _CLASS_CTR2_extends(...)
#define _CLASS_CTR2_public(...)
#define _CLASS_CTR2_private(...)
#define _CLASS_CTR2_constructor(...) M_EXTRACT
#define _CLASS_CTR2_destructor(...) M_SKIP
#define _CLASS_CTR2_abstract

#define _CLASS_DTR_extends(...)
#define _CLASS_DTR_public(...)
#define _CLASS_DTR_private(...)
#define _CLASS_DTR_constructor(...) M_SKIP
#define _CLASS_DTR_destructor(...) M_EXTRACT
#define _CLASS_DTR_abstract

#define _CLASS_ABS_extends(...)
#define _CLASS_ABS_public(...)
#define _CLASS_ABS_private(...)
#define _CLASS_ABS_constructor(...) M_SKIP
#define _CLASS_ABS_destructor(...) M_SKIP
#define _CLASS_ABS_abstract   1

#define __CLASS_EXT(_1_,_2_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( M_JOIN(_CLASS_EXT_,__VA_ARGS__) )
#define __CLASS_PBL(_1_,_2_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( M_JOIN(_CLASS_PBL_,__VA_ARGS__) )
#define __CLASS_PRV(_1_,_2_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( M_JOIN(_CLASS_PRV_,__VA_ARGS__) )
#define __CLASS_DTR(_1_,_2_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( M_JOIN(_CLASS_DTR_,__VA_ARGS__) )
#define __CLASS_ABS(_1_,_2_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( M_JOIN(_CLASS_ABS_,__VA_ARGS__) )
#define __CLASS_CTR_ARGS(_1_,_2_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( M_JOIN(_CLASS_CTR1_,__VA_ARGS__) )
#define __CLASS_CTR_BODY(_1_,_2_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( M_JOIN(_CLASS_CTR2_,__VA_ARGS__) )

#define _CLASS_EXT(_name_) M_FOREACH(__CLASS_EXT,-,M_JOIN(_CLASS_DECL,_name_))
#define _CLASS_PBL(_name_) M_FOREACH(__CLASS_PBL,-,M_JOIN(_CLASS_DECL,_name_))
#define _CLASS_PRV(_name_) M_FOREACH(__CLASS_PRV,-,M_JOIN(_CLASS_DECL,_name_))
#define _CLASS_DTR(_name_) M_FOREACH(__CLASS_DTR,-,M_JOIN(_CLASS_DECL,_name_))
#define _CLASS_ABS(_name_) M_FOREACH(__CLASS_ABS,-,M_JOIN(_CLASS_DECL,_name_))
#define _CLASS_CTR_ARGS(_name_) M_FOREACH(__CLASS_CTR_ARGS,-,M_JOIN(_CLASS_DECL,_name_))
#define _CLASS_CTR_BODY(_name_) M_FOREACH(__CLASS_CTR_BODY,-,M_JOIN(_CLASS_DECL,_name_))

#define __CLASS_ARGS_STD(_index_,_name_,...)   M_WHEN(M_IS_ARG(__VA_ARGS__))( ,__VA_ARGS__ )
#define __CLASS_ARGS_VAR(_index_,_name_,...)   M_WHEN(M_IS_ARG(__VA_ARGS__))( ,struct M_JOIN(_##_index_,_name_) _##_index_ )
#define __CLASS_ARGS_CALL(_index_,_name_,...)  M_WHEN(M_IS_ARG(__VA_ARGS__))( ,M_JOIN(_,_index_) )
#define __CLASS_ARGS_DEF(_index_,_name_,...)   M_WHEN(M_IS_ARG(__VA_ARGS__))( struct M_JOIN(_##_index_,_name_){__VA_ARGS__;}; )
#define __CLASS_ARGS_ZERO1(_index_,_name_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( struct M_JOIN(_##_index_,_name_) _##_index_; )
#define __CLASS_ARGS_ZERO2(_index_,_name_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( _CLASS_CLEAR(&_##_index_,sizeof(_##_index_)); )

#define _CLASS_ARGS_STD(_name_)  M_FOREACH(__CLASS_ARGS_STD,_name_,_CLASS_CTR_ARGS(_name_))
#define _CLASS_ARGS_VAR(_name_)  M_FOREACH(__CLASS_ARGS_VAR,_name_,_CLASS_CTR_ARGS(_name_))
#define _CLASS_ARGS_CALL(_name_) M_FOREACH(__CLASS_ARGS_CALL,_name_,_CLASS_CTR_ARGS(_name_))
#define _CLASS_ARGS_DEF(_name_)  M_FOREACH(__CLASS_ARGS_DEF,_name_,_CLASS_CTR_ARGS(_name_))
#define _CLASS_ARGS_ZERO(_name_) M_FOREACH(__CLASS_ARGS_ZERO1,_name_,_CLASS_CTR_ARGS(_name_)) M_FOREACH(__CLASS_ARGS_ZERO2,-,_CLASS_CTR_ARGS(_name_))

#define _CLASS_COMPILE(...) {M_FOREACH(__CLASS_CTR_BODY,-,__VA_ARGS__)}}else{M_FOREACH(__CLASS_DTR,-,__VA_ARGS__)}return self; (void)self;}

#define _CLASS_CLEAR(_v_,_s_) \
    if(_s_){\
        union{void *v;char *c;int *i;} _1_={.v=_v_}; int _2_=_s_; \
        while(_2_ & (sizeof(*_1_.i)-1)){ --_2_; *_1_.c=0; ++_1_.c; } _2_/=sizeof(*_1_.i);\
        while(_2_){ --_2_; *_1_.i=0; ++_1_.i; }\
    }

#define _CLASS_SUPER(_name_) \
    M_WHEN(_CLASS_EXT(_name_))(\
        extern void *M_JOIN(_ctor_,_CLASS_EXT(_name_))(void* _CLASS_ARGS_VAR(_CLASS_EXT(_name_)));\
        void *(* const super)(void* _CLASS_ARGS_STD(_CLASS_EXT(_name_)))=(void*)M_JOIN(_ctor_,_CLASS_EXT(_name_));\
    )

#define __CLASS_END() _CLASS_END
#define _CLASS_END(_name_,...)\
    M_WHEN(_CLASS_EXT(_name_))( M_OBSTRUCT(__CLASS_END)()(_CLASS_EXT(_name_),_CLASS_EXT(_name_)) ) \
    _CLASS_PBL(_name_)\
    M_IF(M_IS_ARG(__VA_ARGS__))(\
        M_WHEN(M_IS_ARG(M_PEEK(_CLASS_PRV(_name_))))( struct{_CLASS_PRV(_name_)}_pad##__VA_ARGS__[1][1][1]; ) , \
        _CLASS_PRV(_name_)\
    )

#endif // CLASS_H
