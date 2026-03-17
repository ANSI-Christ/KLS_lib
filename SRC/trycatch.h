/* * * * * * * * * * * * * * * * * */
/* MIT License                     */
/* Copyright (c) 2024 ANSI-Christ  */
/* * * * * * * * * * * * * * * * * */

#ifndef TRY_CATCH_H
#define TRY_CATCH_H

#include <setjmp.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "macro.h"

/* if the compiler does not support the declaration in a loop, then compile with the flag -DTRY_CATCH_NO_LOOP_DECL */

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

extern const struct EXCEPTION_INFO{
    const char *type;
    const char *where;
} * const EXCEPTION;

#define TRY(...)     _TRY(__VA_ARGS__)
#define CATCH(...)   M_IF(M_IS_ARG(M_PEAK(__VA_ARGS__)))(_CATCH1,_CATCH0)(__VA_ARGS__)
#define THROW(...)   M_IF(M_IS_ARG(M_PEAK(__VA_ARGS__)))(_THROW1,_THROW0)(__VA_ARGS__)
#define DEBUG(...)   TRY(__VA_ARGS__)CATCH()(printf("\nDEBUG[%s:%d] %s at %s\n",M_FILE(),M_LINE(),EXCEPTION->type,EXCEPTION->where); getchar();)

extern void(*TryCatchSignal)(void);     /* by default nothing  */
extern void(*TryCatchTerminate)(void);  /* by default exit(-1) */

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

extern const char * const _TryCatchMsg[];
struct _TRYCATCH{ jmp_buf *jmp; struct EXCEPTION_INFO info[1]; void *data; union{long double ald; double ad; char c[96];}buffer[1]; };
struct _TRYCATCH *_TryCatch(char);
#define EXCEPTION ((const struct EXCEPTION_INFO*)((const struct _EXCEPTION_DONT_EXISTS*)_1tc_->info))
#define _THROW_INFO M_FILE() ":" M_STRING(M_LINE())
#define _THROW0() ({\
    struct _TRYCATCH * const _1_=_TryCatch(0);\
    if(_1_ && _1_->info->type){\
        if(_1_->jmp) longjmp(*_1_->jmp,1);\
        printf(_TryCatchMsg[1],_1_->info->type,_1_->info->where);\
    }else printf(_TryCatchMsg[2],_THROW_INFO);\
    TryCatchTerminate();\
})
#define _THROW1(_type_,...) ({\
    struct _TRYCATCH * const _1_=_TryCatch(0);\
    if(_1_ && _1_->jmp){\
        _1_->info->type=M_STRING(_type_); _1_->info->where=_THROW_INFO;\
        if(_1_->data!=_1_->buffer){free(_1_->data); _1_->data=_1_->buffer;}\
        M_IF(M_IS_ARG(M_PEAK(__VA_ARGS__)))(\
            M_EXTRACT( if( sizeof(_type_)<=sizeof(_1_->buffer) || (_1_->data=malloc(sizeof(_type_))) ){\
                const union _tccast_{_type_ _; struct{char _[sizeof(_type_)];}x; } _2_={__VA_ARGS__};\
                ((union _tccast_*)_1_->data)->x=_2_.x; longjmp(*_1_->jmp,1);\
            }) , \
            M_EXTRACT( longjmp(*_1_->jmp,1); )\
        )\
    }printf(_TryCatchMsg[1],M_STRING(_type_),_THROW_INFO);\
    TryCatchTerminate();\
})
#define __TRY(_decl_,...) ({\
    jmp_buf _2tc_, *_3tc_;\
    _decl_;\
    int _4tc_;\
    if(!_1tc_){printf(_TryCatchMsg[0],_THROW_INFO);TryCatchTerminate();}\
    _3tc_=_1tc_->jmp;\
    _1tc_->jmp=&_2tc_;\
    _4tc_=setjmp(_2tc_);\
    if(!_4tc_) do{__VA_ARGS__}while(!5);\
    _1tc_->jmp=_3tc_; _4tc_;\
})

#ifdef TRY_CATCH_NO_LOOP_DECL

#define _CATCH0() else{ struct _TRYCATCH * const _1tc_=_TryCatch(0); _CATCH_ELSE
#define _CATCH1(_type_,...) else if( ({ struct _TRYCATCH * const _1tc_=_TryCatch(0); if(!strcmp(_1tc_->info->type,M_STRING(_type_))){ M_WHEN(M_IS_ARG(M_PEAK(__VA_ARGS__)))( _type_ __VA_ARGS__= *(_type_*)(_1tc_->data); ) _CATCH_ELIF
#define _CATCH_DO(...) do{ struct _EXCEPTION_DONT_EXISTS{char _;}; __VA_ARGS__ }while(!1); break; if(_1tc_->jmp) continue;
#define _CATCH_ELIF(...) _CATCH_DO(__VA_ARGS__) } 0;}) );
#define _CATCH_ELSE(...) _CATCH_DO(__VA_ARGS__) }
#define _TRY(...) for(;__TRY(struct _TRYCATCH * const _1tc_=_TryCatch(1),__VA_ARGS__);THROW())if(!1);

#else

#define _CATCH(...) do{ struct _EXCEPTION_DONT_EXISTS{char _;}; __VA_ARGS__ }while(!1); break; }
#define _CATCH0() else{ _CATCH
#define _CATCH1(_type_,...) else if(!strcmp(_1tc_->info->type,M_STRING(_type_))){ M_WHEN(M_IS_ARG(M_PEAK(__VA_ARGS__)))( _type_ __VA_ARGS__= *(_type_*)(_1tc_->data); ) _CATCH
#define _TRY(...) for(struct _TRYCATCH * const _1tc_=_TryCatch(1);__TRY(,__VA_ARGS__);THROW())if(!1);

#endif

#endif /* TRY_CATCH_H */



#ifdef TRY_CATCH_IMPL

#include <pthread.h>

const char * const _TryCatchMsg[]={
    "\nTRY FAULT at [%s]\n\n",
    "\nterminate called after throwing an instance of \'%s\' at [%s]\n\n",
    "\nterminate called after throwing without an active excepion at [%s]\n\n"
};
static pthread_key_t _TryCatchKey;
static unsigned char _TryCatchInit=0;

static void _TryCatchDeleter(struct _TRYCATCH *s){
    if(s){
        if(s->data!=s->buffer) free(s->data);
        free(s);
    }
}

static void _TryCatchExit(void){
    _TryCatchDeleter((struct _TRYCATCH*)pthread_getspecific(_TryCatchKey));
    pthread_key_delete(_TryCatchKey);
}

static void _TryCatchOnce(void){
    extern int atexit(void(*)(void));
    if( (_TryCatchInit=!pthread_key_create(&_TryCatchKey,(void(*)(void*))_TryCatchDeleter)) )
        atexit(_TryCatchExit);
}

struct _TRYCATCH *_TryCatch(const char alloc){
    static pthread_once_t once=PTHREAD_ONCE_INIT;
    struct _TRYCATCH *s=NULL;
    if((_TryCatchInit || (!pthread_once(&once,_TryCatchOnce) && _TryCatchInit)) && !(s=(struct _TRYCATCH*)pthread_getspecific(_TryCatchKey)) && alloc && (s=(struct _TRYCATCH*)malloc(sizeof(*s))) ){
        if(pthread_setspecific(_TryCatchKey,s)){
            free(s); s=NULL;
        }else{
            s->jmp=NULL; s->info->type=NULL; s->data=s->buffer;
            if(TryCatchSignal) TryCatchSignal();
        }
    }
    return s;
}

static void _TryCatchTerminator(void){exit(-1);}
void(*TryCatchTerminate)(void)=_TryCatchTerminator;
void(*TryCatchSignal)(void)=NULL;

#endif
