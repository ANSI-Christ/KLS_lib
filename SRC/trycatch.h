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

void TryCatchClose(void);
unsigned char TryCatchInit(void);

void TryCatchSetTerminator(void(*f)(void *arg),void *arg); /*by default exit(-1)*/
void TryCatchSetSignalInitializer(void(*f)(void *arg),void *arg);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

struct _EXCEPTION{ const char *type, *where; void *data; };
struct _TRYCATCH{ jmp_buf *jmp; struct _EXCEPTION e[1]; char buffer[95], final;};
struct _TRYCATCH *_TryCatch(void);
void _TryCatchTerminate(void);
#define _CATCH(...) {__VA_ARGS__ break;}}
#define _CATCH0() else{ _CATCH
#define _CATCH1(_type_) else if( !strcmp(EXCEPTION->type,M_STRING(_type_)) ) { _CATCH
#define _CATCH2(_type_,_var_) else if( !strcmp(EXCEPTION->type,M_STRING(_type_)) ) { _type_ _var_= *(_type_*)(EXCEPTION->data); _CATCH
#define _THROW_INFO M_FILE() ":" M_STRING(M_LINE())
#define _THROW0() ({\
    struct _TRYCATCH *_1_=_TryCatch();\
    if(_1_){\
        if(_1_->e->type){\
            if(_1_->jmp) longjmp(*_1_->jmp,1);\
            printf("\nterminate called after throwing an instance of \'%s\' at [%s]\n\n",_1_->e->type,_1_->e->where);\
        }else puts("\nterminate called after throwing at [" _THROW_INFO "]\n");\
    }else puts("\nterminate called after throwing at [" _THROW_INFO "]\n");\
    _TryCatchTerminate();\
})
#define _THROW1(_type_,...) ({\
    struct _TRYCATCH *_1_=_TryCatch();\
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
    _TryCatchTerminate();\
})
#define _TRY(...) ({\
    jmp_buf _1tc_;\
    struct _TRYCATCH *_2tc_=_TryCatch();\
    void *_3tc_; char _4tc_;\
    if(!_2tc_){puts("\n\nTRY FAULT at [" _THROW_INFO "]\n");_TryCatchTerminate();}\
    _3tc_=_2tc_->jmp;\
    _2tc_->jmp=&_1tc_;\
    _4tc_=!setjmp(_1tc_);\
    if(_4tc_) do{__VA_ARGS__}while(!5);\
    _2tc_->jmp=_3tc_;\
    _2tc_->final=!_4tc_;\
    _4tc_;\
})

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef TRY_CATCH_IMPL

static struct{
    void(*sig_f)(void *sig_arg);
    void *sig_arg;
    void(*term_f)(void *term_arg);
    void *term_arg;
    pthread_key_t key;
    unsigned char init;
}_tcInit;

void TryCatchSetSignalInitializer(void(*f)(void *arg),void *arg){
    _tcInit.sig_f=(void*)f; _tcInit.sig_arg=arg;
}

void TryCatchSetTerminator(void(*f)(void *arg),void *arg){
    _tcInit.term_f=(void*)f; _tcInit.term_arg=arg;
}

void _TryCatchTerminate(void){
    if(!_tcInit.term_f) exit(-1);
    _tcInit.term_f(_tcInit.term_arg);
}

void _TryCatchDeleter(struct _TRYCATCH *s){
    if(s){
        pthread_setspecific(_tcInit.key,NULL);
        if(s->e->data!=s->buffer) free(s->e->data);
        free(s);
    }
}

unsigned char TryCatchInit(void){
    if(!_tcInit.init){
        _tcInit.init=!pthread_key_create(&_tcInit.key,(void*)_TryCatchDeleter);
        return _tcInit.init;
    } return 2;
}

void TryCatchClose(void){
    if(_tcInit.init){
        _TryCatchDeleter(pthread_getspecific(_tcInit.key));
        pthread_key_delete(_tcInit.key); _tcInit.init=0;
    }
}

struct _TRYCATCH *_TryCatch(void){
    struct _TRYCATCH *s=NULL;
    if(_tcInit.init && !(s=pthread_getspecific(_tcInit.key)) && (s=malloc(sizeof(*s))) ){
        if(pthread_setspecific(_tcInit.key,s)){
            free(s); s=NULL;
        }else{
            memset(s,0,sizeof(*s));
            if(_tcInit.sig_f) _tcInit.sig_f(_tcInit.sig_arg);
        }
    }
    return s;
}

#endif

#endif // TRY_CATCH_H
