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
/* if there are no sigsetjmp / siglongjmp, then compile with the flag -DTRY_CATCH_NO_SIGJMP */

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
#ifdef __WIN32
#ifndef TRY_CATCH_NO_SIGJMP
#define TRY_CATCH_NO_SIGJMP
#endif
#endif
#ifdef TRY_CATCH_NO_SIGJMP
#define _TC_BUF      jmp_buf
#define _TC_SAV(_1_) setjmp(_1_)
#define _TC_JMP(_1_) longjmp((_1_),1)
#else
#define _TC_BUF      sigjmp_buf
#define _TC_SAV(_1_) sigsetjmp(_1_,1)
#define _TC_JMP(_1_) siglongjmp((_1_),1)
#endif
struct _TRYCATCH{ _TC_BUF *jmp; struct EXCEPTION_INFO info[1]; void *data; char buffer[96];};
struct _TRYCATCH *_TryCatch(char);
#define EXCEPTION ((const struct EXCEPTION_INFO*)((const struct _EXCEPTION_DONT_EXISTS*)_1tc_->info))
#define _THROW_INFO M_FILE() ":" M_STRING(M_LINE())
#define _THROW0() ({\
    struct _TRYCATCH * const _1_=_TryCatch(0);\
    if(_1_ && _1_->info->type){\
        if(_1_->jmp) _TC_JMP(*_1_->jmp);\
        printf("\nterminate called after throwing an instance of \'%s\' at [%s]\n\n",_1_->info->type,_1_->info->where);\
    }else puts("\nterminate called after throwing without an active excepion at [" _THROW_INFO "]\n");\
    TryCatchTerminate();\
})
#define _THROW1(_type_,...) ({\
    struct _TRYCATCH * const _1_=_TryCatch(0);\
    if(_1_ && _1_->jmp){\
        _1_->info->type=M_STRING(_type_); _1_->info->where=_THROW_INFO;\
        if(_1_->data!=_1_->buffer){free(_1_->data); _1_->data=_1_->buffer;}\
        M_IF(M_IS_ARG(M_PEAK(__VA_ARGS__)))(\
            M_EXTRACT( if( sizeof(_type_)<=sizeof(_1_->buffer) || (_1_->data=malloc(sizeof(_type_))) ){\
                const union{struct{_type_ _;}_; struct{char _[sizeof(_type_)];} data,*pdata; void *p;} _2_={{__VA_ARGS__}}, _3_={.p=_1_->data};\
                *_3_.pdata=_2_.data; _TC_JMP(*_1_->jmp);\
            }) , \
            M_EXTRACT( _TC_JMP(*_1_->jmp); )\
        )\
    }puts("\nterminate called after throwing an instance of \'" M_STRING(_type_) "\' at [" _THROW_INFO "]\n");\
    TryCatchTerminate();\
})
#define __TRY(_decl_,...) ({\
    _TC_BUF _2tc_, *_3tc_;\
    _decl_;\
    char _4tc_;\
    if(!_1tc_){puts("\n\nTRY FAULT at [" _THROW_INFO "]\n");TryCatchTerminate();}\
    _3tc_=_1tc_->jmp;\
    _1tc_->jmp=&_2tc_;\
    _4tc_=_TC_SAV(_2tc_);\
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

static pthread_key_t _TryCatchKey;
static unsigned char _TryCatchInit;

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
