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

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

extern const struct EXCEPTION_INFO{
    const char *type;
    const char *where;
} * const EXCEPTION;

#define TRY(...)     if( _TRY(__VA_ARGS__) );else for(;;THROW()) if(!5);
#define CATCH(...)   M_OVERLOAD(_CATCH,__VA_ARGS__)(__VA_ARGS__)
#define FINALLY(...) if(_TryCatch(0)->final){ _EXCEPTION_SAVE _5tc_->final=0; do{__VA_ARGS__}while(0); _EXCEPTION_RESTORE }
#define THROW(...)   M_IF(M_COUNT(__VA_ARGS__))(_THROW1,_THROW0)(__VA_ARGS__)
#define DEBUG(...)   TRY(__VA_ARGS__)CATCH()(printf("\nDEBUG[%s:%d] %s at %s\n",M_FILE(),M_LINE(),EXCEPTION->type,EXCEPTION->where); getchar();)

extern void(*TryCatchSignal)(void);     /* by default nothing  */
extern void(*TryCatchTerminate)(void);  /* by default exit(-1) */

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

struct _TRYCATCH{ jmp_buf *jmp; struct EXCEPTION_INFO info; void *data; char buffer[95], final;};
struct _TRYCATCH *_TryCatch(char);
#define EXCEPTION ((const struct EXCEPTION_INFO*)((const struct _EXCEPTION_DONT_EXISTS*)_6tc_))
#define _EXCEPTION_SAVE struct _EXCEPTION_DONT_EXISTS{char _;}; struct _TRYCATCH * const _5tc_=_TryCatch(0); const struct EXCEPTION_INFO _6tc_[1]={_5tc_->info};
#define _EXCEPTION_RESTORE _5tc_->info= _6tc_[0];
#define _CATCH(...) do{__VA_ARGS__}while(0); _5tc_->final=1; _EXCEPTION_RESTORE break;}
#define _CATCH0() else{ _EXCEPTION_SAVE _CATCH
#define _CATCH1(_type_) else if( !strcmp(_TryCatch(0)->info.type,M_STRING(_type_)) ) { _EXCEPTION_SAVE _CATCH
#define _CATCH2(_type_,_var_) else if( !strcmp(_TryCatch(0)->info.type,M_STRING(_type_)) ) { _EXCEPTION_SAVE _type_ _var_= *(_type_*)(_5tc_->data); _CATCH
#define _THROW_INFO M_FILE() ":" M_STRING(M_LINE())
#define _THROW0() ({\
    struct _TRYCATCH * const _1_=_TryCatch(0);\
    if(_1_){\
        if(_1_->info.type){\
            if(_1_->jmp) longjmp(*_1_->jmp,1);\
            printf("\nterminate called after throwing an instance of \'%s\' at [%s]\n\n",_1_->info.type,_1_->info.where);\
        }else puts("\nterminate called after throwing at [" _THROW_INFO "]\n");\
    }else puts("\nterminate called after throwing at [" _THROW_INFO "]\n");\
    TryCatchTerminate();\
})
#define _THROW1(_type_,...) ({\
    struct _TRYCATCH * const _1_=_TryCatch(0);\
    if(_1_ && _1_->jmp){\
        _1_->info.type=M_STRING(_type_); _1_->info.where=_THROW_INFO;\
        if(_1_->data!=_1_->buffer){free(_1_->data); _1_->data=_1_->buffer;}\
        M_IF(M_IS_ARG(M_PEAK(__VA_ARGS__)))(\
            M_EXTRACT( if( sizeof(_type_)<=sizeof(_1_->buffer) || (_1_->data=malloc(sizeof(_type_))) ){\
                const union{struct{_type_ _;}_; struct{char _[sizeof(_type_)];} data,*pdata; void *p;} _2_={{__VA_ARGS__}}, _3_={.p=_1_->data};\
                *_3_.pdata=_2_.data; longjmp(*_1_->jmp,1);\
            }) , \
            M_EXTRACT( longjmp(*_1_->jmp,1); )\
        )\
    }puts("\nterminate called after throwing an instance of \'" M_STRING(_type_) "\' at [" _THROW_INFO "]\n");\
    TryCatchTerminate();\
})
#define _TRY(...) ({\
    jmp_buf _1tc_, *_2tc_;\
    struct _TRYCATCH * const _3tc_=_TryCatch(1);\
    char _4tc_;\
    if(!_3tc_){puts("\n\nTRY FAULT at [" _THROW_INFO "]\n");TryCatchTerminate();}\
    _2tc_=_3tc_->jmp;\
    _3tc_->jmp=&_1tc_;\
    _4tc_=!setjmp(_1tc_);\
    if(_4tc_) do{__VA_ARGS__}while(!5);\
    _3tc_->jmp=_2tc_;\
    _3tc_->final=0;\
    _4tc_;\
})

#endif /* TRY_CATCH_H */



#ifdef TRY_CATCH_IMPL

#include <pthread.h>

static pthread_key_t _TryCatchKey;
static unsigned char _TryCatchInit;

static void _TryCatchDeleter(struct _TRYCATCH *s){
    if(s){
        pthread_setspecific(_TryCatchKey,NULL);
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
            s->jmp=NULL; s->data=s->buffer;
            if(TryCatchSignal) TryCatchSignal();
        }
    }
    return s;
}

static void _TryCatchTerminator(void){exit(-1);}
void(*TryCatchTerminate)(void)=_TryCatchTerminator;
void(*TryCatchSignal)(void)=NULL;

#endif
