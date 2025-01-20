/* * * * * * * * * * * * * * * * * */
/* MIT License                     */
/* Copyright (c) 2024 ANSI-Christ  */
/* * * * * * * * * * * * * * * * * */

#ifndef CLASS_H
#define CLASS_H

#include "macro.h"

#define _CLASS_DECL CLASS_BEGIN__

#define CLASS struct

#define CLASS_END(_name_,...) \
    _CLASS_ARGS_DEF(_name_)\
    CLASS _name_{void(* const destructor)(void * const); _CLASS_LOOP(_CLASS_END(_name_))};\
    struct M_JOIN(_,_name_){M_IF(_CLASS_ABS(_name_))(const void *M_JOIN(M_JOIN(_,M_LINE()),pad)[1][1][1],void *(* const constructor)(void * const _CLASS_ARGS_STD(_name_))); _CLASS_LOOP(_CLASS_END(_name_))};\
    extern const struct M_JOIN(_,_name_) *_name_(void)

#define CLASS_COMPILE(_name_) \
    void M_JOIN(_dtor_,_name_)(void * const self){\
        _CLASS_LOOP(_CLASS_DESTRUCT(_name_,self))\
    }\
    void *M_JOIN(_ctor_,_name_)(void * const self _CLASS_ARGS_VAR(_name_) ){\
        if(self){\
            extern void *M_JOIN(_impl_,_name_)(const char,CLASS _name_ * const _CLASS_ARGS_STD(_name_) );\
            M_IF(_CLASS_EXT(_name_))(\
                _CLASS_CLEAR(((char * const )self)+sizeof(CLASS _CLASS_EXT(_name_)),sizeof(CLASS _name_)-sizeof(CLASS _CLASS_EXT(_name_))) ,\
                _CLASS_CLEAR(self,sizeof(CLASS _name_))\
            )\
            if( _CLASS_CAST(void*(* const)(const char,void * const _CLASS_ARGS_VAR(_name_))) (M_JOIN(_impl_,_name_)) (1,self _CLASS_ARGS_CALL(_name_,)) )\
                *(void(**)(void*))self=M_JOIN(_dtor_,_name_);\
            else{ M_JOIN(_dtor_,_name_)(self); return (void*)0; }\
        } return self;\
    }\
    const struct M_JOIN(_,_name_) *_name_(void){\
        typedef union { void *ctor; struct M_JOIN(_,_name_) ret[1]; struct{char _[sizeof(struct M_JOIN(_,_name_))];} data; } M_JOIN(_t_u,_name_);\
        static M_JOIN(_t_u,_name_) c={(void*)1};\
        if(c.ctor==(void*)1){\
            M_WHEN(_CLASS_EXT(_name_))( _CLASS_EXT(_name_)(); )\
            {\
                M_JOIN(_t_u,_name_) tmp; _CLASS_ARGS_UNION(_name_) zero={0};\
                if( M_JOIN(_ctor_,_name_)(&tmp _CLASS_ARGS_CALL(_name_,zero)) )\
                    M_JOIN(_dtor_,_name_)(&tmp);\
                tmp.ctor=c.ctor; c.data=tmp.data; if(0)(void)zero;\
                c.ctor=(void*)M_IF(_CLASS_ABS(_name_))(0,M_JOIN(_ctor_,_name_));\
            }\
        } return c.ret;\
    }\
    void *M_JOIN(_impl_,_name_)(const char M_JOIN(M_JOIN(_,M_LINE()),ctr),CLASS _name_ * const self _CLASS_ARGS_STD(_name_) ){\
        if(!M_JOIN(M_JOIN(_,M_LINE()),ctr)){_CLASS_DTR(_name_)}\
        if(M_JOIN(M_JOIN(_,M_LINE()),ctr)){_CLASS_SUPER(_name_) {_CLASS_CTR_BODY(_name_)} _CLASS_COMPILE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

#define __CLASS_ARGS_STD(_index_,_name_,...)    M_WHEN(M_IS_ARG(__VA_ARGS__))( ,__VA_ARGS__ )
#define __CLASS_ARGS_VAR(_index_,_name_,...)    M_WHEN(M_IS_ARG(__VA_ARGS__))( ,const struct M_JOIN(M_JOIN(_,_index_),_name_) M_JOIN(_,_index_) )
#define __CLASS_ARGS_CALL(_index_,_name_,...)   M_WHEN(M_IS_ARG(__VA_ARGS__))( ,M_WHEN(M_IS_ARG(_name_))(_name_.)M_JOIN(_,_index_) )
#define __CLASS_ARGS_DEF(_index_,_name_,...)    M_WHEN(M_IS_ARG(__VA_ARGS__))( struct M_JOIN(M_JOIN(_,_index_),_name_){__VA_ARGS__;}; )
#define __CLASS_ARGS_UNION(_index_,_name_,...)  M_WHEN(M_IS_ARG(__VA_ARGS__))( struct M_JOIN(M_JOIN(_,_index_),_name_) M_JOIN(_,_index_); )

#define _CLASS_ARGS_STD(_name_)   M_FOREACH(__CLASS_ARGS_STD,_name_,_CLASS_CTR_ARGS(_name_))
#define _CLASS_ARGS_VAR(_name_)   M_FOREACH(__CLASS_ARGS_VAR,_name_,_CLASS_CTR_ARGS(_name_))
#define _CLASS_ARGS_DEF(_name_)   M_FOREACH(__CLASS_ARGS_DEF,_name_,_CLASS_CTR_ARGS(_name_))
#define _CLASS_ARGS_UNION(_name_) const union{char init; M_FOREACH(__CLASS_ARGS_UNION,_name_,_CLASS_CTR_ARGS(_name_))}
#define _CLASS_ARGS_CALL(_name_,_storage_) M_FOREACH(__CLASS_ARGS_CALL,_storage_,_CLASS_CTR_ARGS(_name_))

#define _CLASS_COMPILE(...) {M_FOREACH(__CLASS_CTR_BODY,-,__VA_ARGS__)}}else{M_FOREACH(__CLASS_DTR,-,__VA_ARGS__)}return self; (void)self;}

#define _CLASS_CAST(_type_) ( (_type_) __CLASS_CAST
#define __CLASS_CAST(_ptr_) ({ const union{const void * const _; void *p;} M_JOIN(_ccst,M_LINE())={(const void * const)(_ptr_)}; M_JOIN(_ccst,M_LINE()).p; }) )

#define _CLASS_CLEAR(_v_,_s_) \
    if(_s_){\
        union{const void *_;char *c;void **i;} _1_={(const void*)(_v_)};\
        unsigned int _2_=(_s_)/sizeof(*_1_.i);\
        unsigned char _3_=(_s_)&(sizeof(*_1_.i)-1);\
        while(_2_) _1_.i[--_2_]=(void*)0;\
        _1_.i+=(_s_)/sizeof(*_1_.i);\
        while(_3_) _1_.c[--_3_]=0;\
    }

#define _CLASS_SUPER(_name_) \
    M_WHEN(_CLASS_EXT(_name_))(\
        extern void *M_JOIN(_ctor_,_CLASS_EXT(_name_))(void * const _CLASS_ARGS_VAR(_CLASS_EXT(_name_)));\
        void *(* const super)(void * const _CLASS_ARGS_STD(_CLASS_EXT(_name_)))=_CLASS_CAST(void *(* const)(void * const _CLASS_ARGS_STD(_CLASS_EXT(_name_))))(M_JOIN(_ctor_,_CLASS_EXT(_name_)));\
    )

#define __CLASS_DESTRUCT() _CLASS_DESTRUCT
#define _CLASS_DESTRUCT(_name_,_self_) \
    {extern void *M_JOIN(_impl_,_name_)(const char,CLASS _name_ * const _CLASS_ARGS_STD(_name_) );\
    _CLASS_CAST(void(* const)(const char,void * const)) (M_JOIN(_impl_,_name_)) (0,_self_);}\
    M_WHEN(_CLASS_EXT(_name_))( M_OBSTRUCT(__CLASS_DESTRUCT)()(_CLASS_EXT(_name_),_self_) )

#define __CLASS_END() _CLASS_END
#define _CLASS_END(_name_,...)\
    M_WHEN(_CLASS_EXT(_name_))( M_OBSTRUCT(__CLASS_END)()(_CLASS_EXT(_name_),_CLASS_EXT(_name_)) ) \
    _CLASS_PBL(_name_)\
    M_IF(M_IS_ARG(__VA_ARGS__))(\
        M_WHEN(M_IS_ARG(M_PEAK(_CLASS_PRV(_name_))))( struct{_CLASS_PRV(_name_)}M_JOIN(_pad,__VA_ARGS__)[1][1][1]; ) , \
        _CLASS_PRV(_name_)\
    )

#endif // CLASS_H
