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
#include <pthread.h>

#include "macro.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TRY(...)     if( _TRY(__VA_ARGS__) );else for(;;THROW()) if(!5);
#define CATCH(...)   M_OVERLOAD(_CATCH,__VA_ARGS__)(__VA_ARGS__)
#define FINALLY(...) if( ({char *_1_=&_TryCatch()->final, _2_=*_1_; *_1_=0; _2_;}) ){__VA_ARGS__}
#define THROW(...)   M_IF(M_COUNT(__VA_ARGS__))(_THROW1,_THROW0)(__VA_ARGS__)
#define EXCEPTION    ((const struct _EXCEPTION*)(_TryCatch()->e))
#define DEBUG(...)   TRY(__VA_ARGS__)CATCH()(printf("\nDEBUG[%s:%d] %s at %s\n",M_FILE(),M_LINE(),EXCEPTION->type,EXCEPTION->where); getchar();)

extern void(*TryCatchSignal)(void);     /* by default nothing  */
extern void(*TryCatchTerminate)(void);  /* by default exit(-1) */

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

struct _EXCEPTION{ const char *type, *where; void *data; };
struct _TRYCATCH{ jmp_buf *jmp; struct _EXCEPTION e[1]; char buffer[95], final;};
struct _TRYCATCH *_TryCatch(void);
#define _CATCH(...) {__VA_ARGS__ break;}}
#define _CATCH0() else{ _CATCH
#define _CATCH1(_type_) else if( !strcmp(EXCEPTION->type,M_STRING(_type_)) ) { _CATCH
#define _CATCH2(_type_,_var_) else if( !strcmp(EXCEPTION->type,M_STRING(_type_)) ) { _type_ _var_= *(_type_*)(EXCEPTION->data); _CATCH
#define _THROW_INFO M_FILE() ":" M_STRING(M_LINE())
#define _THROW0() ({\
    struct _TRYCATCH * const _1_=_TryCatch();\
    if(_1_){\
        if(_1_->e->type){\
            if(_1_->jmp) longjmp(*_1_->jmp,1);\
            printf("\nterminate called after throwing an instance of \'%s\' at [%s]\n\n",_1_->e->type,_1_->e->where);\
        }else puts("\nterminate called after throwing at [" _THROW_INFO "]\n");\
    }else puts("\nterminate called after throwing at [" _THROW_INFO "]\n");\
    TryCatchTerminate();\
})
#define _THROW1(_type_,...) ({\
    struct _TRYCATCH * const _1_=_TryCatch();\
    if(_1_ && _1_->jmp){\
        _1_->e->type=M_STRING(_type_); _1_->e->where=_THROW_INFO;\
        if(_1_->e->data!=_1_->buffer){free(_1_->e->data); _1_->e->data=_1_->buffer;}\
        M_IF(M_IS_ARG(M_PEAK(__VA_ARGS__)))(\
            M_EXTRACT( if( sizeof(_type_)<=sizeof(_1_->buffer) || (_1_->e->data=malloc(sizeof(_type_))) ){\
                struct M_JOIN(_tc,M_LINE()){char _[sizeof(_type_)];};\
                const struct{_type_ _;} _2_={__VA_ARGS__};\
                *((struct M_JOIN(_tc,M_LINE())*)_1_->e->data)=*(const struct M_JOIN(_tc,M_LINE())*)&_2_;\
                longjmp(*_1_->jmp,1);\
            }) , \
            M_EXTRACT( longjmp(*_1_->jmp,1); )\
        )\
    }puts("\nterminate called after throwing an instance of \'" M_STRING(_type_) "\' at [" _THROW_INFO "]\n");\
    TryCatchTerminate();\
})
#define _TRY(...) ({\
    jmp_buf _1tc_, *_2tc_;\
    struct _TRYCATCH * const _3tc_=_TryCatch();\
    char _4tc_;\
    if(!_3tc_){puts("\n\nTRY FAULT at [" _THROW_INFO "]\n");TryCatchTerminate();}\
    _2tc_=_3tc_->jmp;\
    _3tc_->jmp=&_1tc_;\
    _4tc_=!setjmp(_1tc_);\
    if(_4tc_) do{__VA_ARGS__}while(!5);\
    _3tc_->jmp=_2tc_;\
    _3tc_->final=!_4tc_;\
    _4tc_;\
})

#endif /* TRY_CATCH_H */



#ifdef TRY_CATCH_IMPL

static pthread_key_t _TryCatchKey;
static unsigned char _TryCatchInit;

static void _TryCatchDeleter(struct _TRYCATCH *s){
    if(s){
        pthread_setspecific(_TryCatchKey,NULL);
        if(s->e->data!=s->buffer) free(s->e->data);
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

struct _TRYCATCH *_TryCatch(void){
    static pthread_once_t once=PTHREAD_ONCE_INIT;
    struct _TRYCATCH *s=NULL;
    if((_TryCatchInit || (!pthread_once(&once,_TryCatchOnce) && _TryCatchInit)) && !(s=(struct _TRYCATCH*)pthread_getspecific(_TryCatchKey)) && (s=(struct _TRYCATCH*)malloc(sizeof(*s))) ){
        if(pthread_setspecific(_TryCatchKey,s)){
            free(s); s=NULL;
        }else{
            memset(s,0,sizeof(*s));
            if(TryCatchSignal) TryCatchSignal();
        }
    }
    return s;
}

static void _TryCatchTerminator(void){exit(-1);}
void(*TryCatchTerminate)(void)=_TryCatchTerminator;
void(*TryCatchSignal)(void)=NULL;

#endif
