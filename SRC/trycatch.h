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
#define FINALLY(...) _FINALLY(__VA_ARGS__)
#define DEBUG(...)   TRY(__VA_ARGS__)CATCH()(printf("\nDEBUG[%s:%d] %s at %s\n",M_FILE(),M_LINE(),EXCEPTION->type,EXCEPTION->where); getchar();)

extern void(*TryCatchSignal)(void);     /* by default nothing  */
extern void(*TryCatchTerminate)(void);  /* by default exit(-1) */

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

struct _TRYCATCH{struct _TRYCATCH *next; jmp_buf *jmp; struct EXCEPTION_INFO info[1]; void *data; char buffer[95], finally;};
struct _TRYCATCH *_TryCatch(char);
#define EXCEPTION ((const struct EXCEPTION_INFO*)((const struct _EXCEPTION_DONT_EXISTS*)_1tc_->info))
#define _THROW_INFO M_FILE() ":" M_STRING(M_LINE())
#define _THROW() ({\
    struct _TRYCATCH * const _1_=_TryCatch(0);\
    if(_1_){\
        _EXCEPTION_CLEAR(_1_)\
        if(_1_->info->type){\
            if(_1_->jmp) longjmp(*_1_->jmp,1);\
            printf("\nterminate called after throwing an instance of \'%s\' at [%s]\n\n",_1_->info->type,_1_->info->where);\
        }else puts("\nterminate called after throwing without an active excepion at [" _THROW_INFO "]\n");\
    }else puts("\nterminate called after throwing without an active excepion at [" _THROW_INFO "]\n");\
    TryCatchTerminate();\
})
#define _THROW0() ({\
    struct _TRYCATCH * const _1_=_TryCatch(0);\
    if(_1_){\
        _EXCEPTION_POP(_1_)\
        _EXCEPTION_CLEAR(_1_)\
        if(_1_->info->type){\
            if(_1_->jmp) longjmp(*_1_->jmp,1);\
            printf("\nterminate called after throwing an instance of \'%s\' at [%s]\n\n",_1_->info->type,_1_->info->where);\
        }else puts("\nterminate called after throwing without an active excepion at [" _THROW_INFO "]\n");\
    }else puts("\nterminate called after throwing without an active excepion at [" _THROW_INFO "]\n");\
    TryCatchTerminate();\
})
#define _THROW1(_type_,...) ({\
    struct _TRYCATCH * const _1_=_TryCatch(0);\
    if(_1_){\
        _EXCEPTION_CLEAR(_1_)\
        if(_1_->jmp){\
            _1_->info->type=M_STRING(_type_); _1_->info->where=_THROW_INFO;\
            if(_1_->data!=_1_->buffer){free(_1_->data); _1_->data=_1_->buffer;}\
            M_IF(M_IS_ARG(M_PEAK(__VA_ARGS__)))(\
                M_EXTRACT( if( sizeof(_type_)<=sizeof(_1_->buffer) || (_1_->data=malloc(sizeof(_type_))) ){\
                    const union{struct{_type_ _;}_; struct{char _[sizeof(_type_)];} data,*pdata; void *p;} _2_={{__VA_ARGS__}}, _3_={.p=_1_->data};\
                    *_3_.pdata=_2_.data; longjmp(*_1_->jmp,1);\
                }) , \
                M_EXTRACT( longjmp(*_1_->jmp,1); )\
            )\
        }\
    }puts("\nterminate called after throwing an instance of \'" M_STRING(_type_) "\' at [" _THROW_INFO "]\n");\
    TryCatchTerminate();\
})
#define __TRY(_decl_,...) ({\
    jmp_buf _2tc_, *_3tc_;\
    _decl_;\
    char _4tc_;\
    if(!_1tc_){puts("\n\nTRY FAULT at [" _THROW_INFO "]\n");TryCatchTerminate();}\
    _3tc_=_1tc_->jmp;\
    _1tc_->jmp=&_2tc_;\
    _4tc_=setjmp(_2tc_);\
    if(!_4tc_){\
        _EXCEPTION_PUSH(_1tc_)\
        do{__VA_ARGS__}while(!5);\
        _EXCEPTION_POP(_1tc_)\
    }\
    _1tc_->jmp=_3tc_;\
    (_1tc_->finally=_4tc_);\
})
#define _FINALLY(...) do{ struct _TRYCATCH * const _1tc_=_TryCatch(1); if(_1tc_->finally){ _1tc_->finally=0; { _EXCEPTION_PUSH(_1tc_) do{ struct _EXCEPTION_DONT_EXISTS{char _;}; __VA_ARGS__ }while(!1); _EXCEPTION_POP(_1tc_) } } }while(!3);
#define _EXCEPTION_PUSH(_1_) struct _TRYCATCH _5tc_[1]={*_1_};  _1_->next=_5tc_; if(_1_->data==_1_->buffer) _5tc_->data=_5tc_->buffer; else _1_->data=_1_->buffer; _EXCEPTION_DBG(_1_,push)
#define _EXCEPTION_POP(_1_)  if(_1_->next){if(_1_->data!=_1_->buffer) free(_1_->data); if(_1_->next->data==_1_->next->buffer) _1_->next->data=_1_->buffer; *_1_=*_1_->next; } _EXCEPTION_DBG(_1_,pop)
#define _EXCEPTION_CLEAR(_1_) do{struct _TRYCATCH *_6tc_=_1_->next; for(;_6tc_ && _1_->jmp==_6tc_->jmp;_6tc_=_6tc_->next)if(_6tc_->data!=_6tc_->buffer) free(_6tc_->data); _1_->next=_6tc_; }while(0); _EXCEPTION_DBG(_1_,clear)
#define _EXCEPTION_DBG(_1_,...) /* do{struct _TRYCATCH *_6tc_=_1_->next; puts(M_STRING(__VA_ARGS__) " " _THROW_INFO); for(;_6tc_;_6tc_=_6tc_->next){ printf("%s at [%s] | next %p, jmp %p\n",_6tc_->info->type,_6tc_->info->where,_6tc_->next,_6tc_->jmp); } puts("\n"); }while(0); */

#ifdef TRY_CATCH_NO_LOOP_DECL

#define _CATCH0() else{ struct _TRYCATCH * const _1tc_=_TryCatch(0); _CATCH_ELSE
#define _CATCH1(_type_,...) else if( ({ struct _TRYCATCH * const _1tc_=_TryCatch(0); if(!strcmp(_1tc_->info->type,M_STRING(_type_))){ M_WHEN(M_IS_ARG(M_PEAK(__VA_ARGS__)))( _type_ __VA_ARGS__= *(_type_*)(_1tc_->data); ) _CATCH_ELIF
#define _CATCH_DO(...) _EXCEPTION_PUSH(_1tc_) do{ struct _EXCEPTION_DONT_EXISTS{char _;}; __VA_ARGS__ }while(!1); _EXCEPTION_CLEAR(_1tc_) break;
#define _CATCH_ELIF(...) _CATCH_DO(__VA_ARGS__) } 0;}) );
#define _CATCH_ELSE(...) _CATCH_DO(__VA_ARGS__) }
#define _TRY(...) for(;__TRY(struct _TRYCATCH * const _1tc_=_TryCatch(1),__VA_ARGS__);_THROW())if(!1);

#else

#define _CATCH(...) _EXCEPTION_PUSH(_1tc_) do{ struct _EXCEPTION_DONT_EXISTS{char _;}; __VA_ARGS__ }while(!1); _EXCEPTION_POP(_1tc_) break; }
#define _CATCH0() else{ _CATCH
#define _CATCH1(_type_,...) else if(!strcmp(_1tc_->info->type,M_STRING(_type_))){ M_WHEN(M_IS_ARG(M_PEAK(__VA_ARGS__)))( _type_ __VA_ARGS__= *(_type_*)(_1tc_->data); ) _CATCH
#define _TRY(...) for(struct _TRYCATCH * const _1tc_=_TryCatch(1);__TRY(,__VA_ARGS__);_THROW())if(!1);

#endif

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
            s->next=NULL; s->jmp=NULL; s->info->type=NULL; s->data=s->buffer; s->finally=0;
            if(TryCatchSignal) TryCatchSignal();
        }
    }
    return s;
}

static void _TryCatchTerminator(void){exit(-1);}
void(*TryCatchTerminate)(void)=_TryCatchTerminator;
void(*TryCatchSignal)(void)=NULL;

#endif
