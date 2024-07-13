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

#define TRY(...)     if( ({jmp_buf _1tc_; struct _TRYCATCH *_2tc_=_TryCatch(); void *_3tc_=_2tc_->jmp; char _4tc_; _2tc_->jmp=&_1tc_; if( (_4tc_=!setjmp(_1tc_)) )do{__VA_ARGS__}while(!5); _2tc_->jmp=_3tc_; _2tc_->final=!_4tc_; _4tc_;}) );else for(;;THROW()) if(!5);
#define CATCH(...)   M_OVERLOAD(_CATCH,__VA_ARGS__)(__VA_ARGS__)
#define FINALLY(...) if( ({char *_1tc_=&_TryCatch()->final; char _2tc_=*_1tc_; *_1tc_=0; _2tc_;}) ){__VA_ARGS__}
#define THROW(...)   M_IF(M_COUNT(__VA_ARGS__))(_THROW1,_THROW0)(__VA_ARGS__)
#define EXCEPTION    ((const struct _EXCEPTION*)(_TryCatch()->e))
#define DEBUG(...)   TRY(__VA_ARGS__)CATCH()(printf("\nDEBUG[%s:%d] %s at %s\n",M_FILE(),M_LINE(),EXCEPTION->type,EXCEPTION->where); getchar();)

unsigned char TryCatchInit();

void TryCatchClose();
void TryCatchSetSignalInitializer(void(*f)(void *arg),void *arg);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

struct _EXCEPTION{ const char *type, *where; void *data; };
struct _TRYCATCH{ jmp_buf *jmp; struct _EXCEPTION e[1]; char buffer[95], final;};
struct _TRYCATCH *_TryCatch();
#define _CATCH(...) {__VA_ARGS__ break;}}
#define _CATCH0() else{ _CATCH
#define _CATCH1(_type_) else if( !strcmp(EXCEPTION->type,M_STRING(_type_)) ) { _CATCH
#define _CATCH2(_type_,_var_) else if( !strcmp(EXCEPTION->type,M_STRING(_type_)) ) { _type_ _var_= *(_type_*)(EXCEPTION->data); _CATCH
#define _THROW_INFO M_FILE() ":" M_STRING(M_LINE())
#define _THROW_EXIT() exit(-1) // pthread_exit(NULL)
#define _THROW0() ({\
    struct _TRYCATCH *_1_=_TryCatch();\
    if(_1_){\
        if(_1_->e->type){\
            if(_1_->jmp) longjmp(*_1_->jmp,1);\
            else printf("\nterminate called after throwing an instance of \'%s\' at [%s]\n\n",_1_->e->type,_1_->e->where);\
        }else printf("\nterminate called after throwing at [" _THROW_INFO "]\n\n");\
    }else printf("\nterminate called after throwing at [" _THROW_INFO "]\n\n");\
    _THROW_EXIT();\
})
#define _THROW1(_type_,...) ({\
    struct _TRYCATCH *_1_=_TryCatch();\
    if(_1_ && _1_->jmp){\
        _1_->e->type=M_STRING(_type_); _1_->e->where=_THROW_INFO;\
        if(_1_->e->data!=_1_->buffer){free(_1_->e->data); _1_->e->data=_1_->buffer;}\
        M_IF(M_COUNT(__VA_ARGS__))(\
            M_EXTRACT( if( sizeof(_type_)<=sizeof(_1_->buffer) || (_1_->e->data=malloc(sizeof(_type_))) ) { *(_type_*)(_1_->e->data)=(_type_)__VA_ARGS__; longjmp(*_1_->jmp,1);} ),\
            longjmp(*_1_->jmp,1);\
        )\
    }printf("\nterminate called after throwing an instance of \'" M_STRING(_type_) "\' at [" _THROW_INFO "]\n\n");\
    _THROW_EXIT();\
})

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef TRY_CATCH_IMPL

static struct{
    void(*f)(void *arg);
    void *arg;
    pthread_key_t key;
    unsigned char init;
}_tcInit;

void TryCatchSetSignalInitializer(void(*f)(void *arg),void *arg){
    _tcInit.f=f; _tcInit.arg=arg;
}

void _TryCatchDeleter(struct _TRYCATCH *s){
    if(s){
        pthread_setspecific(_tcInit.key,NULL);
        if(s->e->data!=s->buffer) free(s->e->data);
        free(s);
    }
}

unsigned char TryCatchInit(){
    if(!_tcInit.init) _tcInit.init=!pthread_key_create(&_tcInit.key,(void*)_TryCatchDeleter);
    return _tcInit.init;
}

void TryCatchClose(){
    if(_tcInit.init){
        _TryCatchDeleter(pthread_getspecific(_tcInit.key));
        pthread_key_delete(_tcInit.key); _tcInit.init=0;
    }
}

struct _TRYCATCH *_TryCatch(){
    struct _TRYCATCH *s=NULL;
    if(_tcInit.init && !(s=pthread_getspecific(_tcInit.key)) && (s=malloc(sizeof(*s))) ){
        if(pthread_setspecific(_tcInit.key,s)){
            free(s); s=NULL;
        }else{
            memset(s,0,sizeof(*s));
            if(_tcInit.f) _tcInit.f(_tcInit.arg);
        }
    }
    return s;
}

#endif

#endif // TRY_CATCH_H
