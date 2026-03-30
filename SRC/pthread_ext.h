/* * * * * * * * * * * * * * * * * */
/* MIT License                     */
/* Copyright (c) 2024 ANSI-Christ  */
/* * * * * * * * * * * * * * * * * */

#ifndef PTHREAD_EXT_H
#define PTHREAD_EXT_H

#include <pthread.h>
#include <stddef.h>
#include "macro.h"


unsigned int pthread_cores(void);




typedef struct{void *_[3];}pthread_poolattr_t;

int pthread_poolattr_init(pthread_poolattr_t *attr);

int pthread_poolattr_setpattr(pthread_poolattr_t *attr,pthread_attr_t *pattr);
int pthread_poolattr_setcattr(pthread_poolattr_t *attr,pthread_condattr_t *cattr);
int pthread_poolattr_setmattr(pthread_poolattr_t *attr,pthread_mutexattr_t *mattr);

int pthread_poolattr_getpattr(const pthread_poolattr_t *attr,pthread_attr_t **pattr);
int pthread_poolattr_getcattr(const pthread_poolattr_t *attr,pthread_condattr_t **cattr);
int pthread_poolattr_getmattr(const pthread_poolattr_t *attr,pthread_mutexattr_t **mattr);

void pthread_poolattr_destroy(pthread_poolattr_t *attr);




typedef struct _pthread_pool_t pthread_pool_t;

pthread_pool_t *pthread_pool_create(unsigned int count,unsigned char prio);
pthread_pool_t *pthread_pool_create_ex(unsigned int count,unsigned char prio,const pthread_poolattr_t *attr,void*(*allocator)(size_t),void(*deallocator)(void*));

int pthread_pool_detach(pthread_pool_t *pool,int forced);
int pthread_pool_timedwait(pthread_pool_t *pool,const struct timespec *abstime);

/* task function shall return 0 for release its resources, or else to hold it */
int pthread_pool_task(pthread_pool_t *pool,int(*task)(pthread_pool_t *pool,void *args,unsigned int index),...); /* return 0 on success */
int pthread_pool_task_prio(pthread_pool_t *pool,unsigned char prio,int(*task)(pthread_pool_t *pool,void *args,unsigned int index),...); /* return 0 on success */

void pthread_pool_wait(pthread_pool_t *pool);
void pthread_pool_clear(pthread_pool_t *pool);
void pthread_pool_unpending(pthread_pool_t *pool);
void pthread_pool_banch(pthread_pool_t *pool,unsigned char count);
void pthread_pool_destroy(pthread_pool_t *pool,unsigned char now);

unsigned int pthread_pool_count(const pthread_pool_t *pool);

const pthread_t *pthread_pool_array(const pthread_pool_t *pool);

/* raw API begin */
typedef struct{void *_;int(*f)(pthread_pool_t*,void*,unsigned int);} pthread_pool_raw_task_t;
void pthread_pool_raw_task_queue(pthread_pool_t *pool,void *raw_task,unsigned char prio);
void *pthread_pool_raw_task_create(const pthread_pool_t *pool,unsigned int size);
void *pthread_pool_raw_task_from_arg(void *task_arg);
/* raw API end */




typedef struct{void *_[2];}pthread_groupattr_t;

int pthread_groupattr_init(pthread_groupattr_t *attr);

int pthread_groupattr_setcattr(pthread_groupattr_t *attr,pthread_condattr_t *cattr);
int pthread_groupattr_setmattr(pthread_groupattr_t *attr,pthread_mutexattr_t *mattr);

int pthread_groupattr_getcattr(const pthread_groupattr_t *attr,pthread_condattr_t **cattr);
int pthread_groupattr_getmattr(const pthread_groupattr_t *attr,pthread_mutexattr_t **mattr);

void pthread_groupattr_destroy(pthread_groupattr_t *attr);




typedef struct{
    pthread_mutex_t mtx[1];
    pthread_cond_t cond[1];
    void (*deallocator)(void*);
    unsigned int target, done, reject;
    signed char state, destroy, wait;
}pthread_group_t;

int pthread_group_init(pthread_group_t *g,unsigned int target,const pthread_groupattr_t *attr,void(*deallocator)(void*));

int pthread_group_destroy(pthread_group_t *g); /* return 1 if group destroys at this call, else 0 */
int pthread_group_rejected(pthread_group_t *g); /* return 1 if group is rejected and it destroys at this call, -1 if rejected, else 0 */
int pthread_group_reject(pthread_group_t *g,unsigned char by_task); /* return like pthread_group_destroy */
int pthread_group_progress(pthread_group_t *g,unsigned int add_targets); /* return like pthread_group_destroy */
int pthread_group_wait(pthread_group_t *g,unsigned int *done,unsigned int *target); /* 0 = ok, -1 = reject */
int pthread_group_timedwait(pthread_group_t *g,unsigned int *done,unsigned int *target,struct timespec *abstime); /* 0 = ok, -1 = reject, EINVAL, ETIMEDOUT */
int pthread_group_reject_ex(pthread_group_t *g,unsigned char by_task,void(*payload)(void *arg),void *arg); /* return like pthread_group_destroy */
int pthread_group_progress_ex(pthread_group_t *g,unsigned int add_targets,void(*payload)(void *arg),void *arg); /* return like pthread_group_destroy */




typedef struct{union{void *p;int i;}r[1],w[1];} pthread_channel_t;

int pthread_channel_open(pthread_channel_t *channel);
int pthread_channel_pop(pthread_channel_t *channel,void *data,int size);
int pthread_channel_push(pthread_channel_t *channel,const void *data,int size);

void pthread_channel_close(pthread_channel_t *channel);




#define _PTHREAD_SETUP(_index_,_0_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( do{ union _pthread_pool_task_arg{M_TYPEOF(__VA_ARGS__) _; struct{char _[sizeof(M_TYPEOF(__VA_ARGS__))];} x;}; ((union _pthread_pool_task_arg*)_0_->M_JOIN(_,_index_).x)->x=((const union _pthread_pool_task_arg*)&M_JOIN(_a,_index_).x)->x; }while(0); )
#define _PTHREAD_FIELD(_index_,_0_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( union{M_TYPEOF(__VA_ARGS__) _;char x[1];} M_JOIN(_,_index_); )
#define _PTHREAD_ARGUM(_index_,_0_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( const struct{M_TYPEOF(__VA_ARGS__) x;} M_JOIN(_a,_index_)={__VA_ARGS__}; )
#define _PTHREAD_TASK(_1_,_2_,_3_,...) ({\
    int(* const _f_)(pthread_pool_t*,void*,unsigned int)=(int(*)(pthread_pool_t*,void*,unsigned int))(_3_);\
    pthread_pool_t * const _p_=(pthread_pool_t*)(_1_);\
    M_FOREACH(_PTHREAD_ARGUM,-,__VA_ARGS__)\
    struct _pthread_pool_task_size{pthread_pool_raw_task_t t; M_FOREACH(_PTHREAD_FIELD,-,__VA_ARGS__) char size;} * const _t_=(struct _pthread_pool_task_size*)((_p_ && _f_) ? pthread_pool_raw_task_create(_p_,M_OFFSETOF(struct _pthread_pool_task_size,size)) : NULL);\
    const unsigned char _q_=(_2_);\
    if(_t_){\
        struct _pthread_pool_task_args{ M_FOREACH(_PTHREAD_FIELD,-,__VA_ARGS__) char size;};\
        M_ASSERT( sizeof(_t_->t) + M_OFFSETOF(struct _pthread_pool_task_args,size) == M_OFFSETOF(struct _pthread_pool_task_size,size) , pthread_pool_task_bad_align_of_arguments);\
        _t_->t.f=_f_; M_FOREACH(_PTHREAD_SETUP,_t_,__VA_ARGS__) pthread_pool_raw_task_queue(_p_,_t_,_q_);\
    } (_t_?0:-1);\
})
#define pthread_pool_task(_1_,_3_,...) _PTHREAD_TASK((_1_),0,(_3_),__VA_ARGS__)
#define pthread_pool_task_prio(_1_,_2_,_3_,...) _PTHREAD_TASK((_1_),(_2_),(_3_),__VA_ARGS__)
#define pthread_pool_raw_task_from_arg(_1_) ((void*)(((pthread_pool_raw_task_t*)(_1_))-1))
extern int nanosleep(const struct timespec*,struct timespec*);
extern int pthread_kill(pthread_t,int);
extern int pthread_detach(pthread_t);

#endif /* PTHREAD_EXT_H */





#ifdef PTHREAD_EXT_IMPL

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32

#define NOMINMAX
#include <windows.h>
#undef NOMINMAX

int pthread_channel_open(pthread_channel_t * const channel){
    if(channel) return CreatePipe(&channel->r->p,&channel->w->p,NULL,0)-1;
    return -1;
}

void pthread_channel_close(pthread_channel_t * const channel){
    if(channel && channel->r->p){
        CloseHandle(channel->w->p);
        CloseHandle(channel->r->p);
        channel->r->p=NULL;
    }
}

int pthread_channel_push(pthread_channel_t * const channel,const void *data,const int size){
    DWORD count;
    if(channel && channel->r->p && data && size>0 && WriteFile(channel->w->p,data,size,&count,NULL)) return size;
    return -1;
}

int pthread_channel_pop(pthread_channel_t * const channel,void *data,int size){
    if(channel && channel->r->p && data){
        char *p=(char*)data;
        while(size>0){
            DWORD bytes;
            ReadFile(channel->r->p,p,size,&bytes,NULL);
            if(bytes>0){
                size-=bytes;
                p+=bytes;
            }
        } return 0;
    } return -1;
}


#define _pthread_signals(_) _(1) _(2) _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(10) _(11) _(12) _(13) _(14) _(15) _(16) _(17) _(18) _(19) _(20) _(21) _(22) _(23) _(24) _(25) _(26) _(27) _(28) _(29) _(30) _(31) _(32) _(33) _(34) _(35) _(36) _(37) _(38) _(39) _(40)
#define _pthread_iter(_1_) static void _pthread_raise_##_1_(void){raise(_1_);}
_pthread_signals(_pthread_iter)
#undef _pthread_iter

static void *_pthread_raise_func(const int sig){
#define _pthread_iter(_1_) case _1_:return _pthread_raise_##_1_;
    switch(sig){
        _pthread_signals(_pthread_iter)
    } return NULL;
#undef _pthread_iter
}
#undef _pthread_signals

#if defined(_M_IX86) || (defined(_X86_) && !defined(__amd64__))
    #define _CtxCtrlReg(_1_)  &(_1_.Eip)
#endif

#if defined (_M_IA64) || defined(_IA64)
    #define _CtxCtrlReg(_1_)  &(_1_.StIIP)
#endif

#if defined(_MIPS_) || defined(MIPS)
    #define _CtxCtrlReg(_1_)  &(_1_.Fir)
#endif

#if defined(_ALPHA_)
    #define _CtxCtrlReg(_1_)  &(_1_.Fir)
#endif

#ifdef _PPC_
    #define _CtxCtrlReg(_1_)  &(_1_.Iar)
#endif

#if defined(_AMD64_) || defined(__amd64__)
    #define _CtxCtrlReg(_1_)  &(_1_.Rip)
#endif

#if defined(_ARM_) || defined(ARM) || defined(_M_ARM) || defined(_M_ARM64)
    #define _CtxCtrlReg(_1_)  &(_1_.Pc)
#endif

#ifndef _CtxCtrlReg
static void *_CtxCtrlRegf(CONTEXT *c){
    static unsigned int offset=-1;
    if(offset==-1){
        const uintptr_t * const end=(void*)(c+1), f=(uintptr_t)GetThreadContext;
        uintptr_t diff, min=0, *p=(void*)c;
        GetThreadContext(GetCurrentThread(),c);
        for(c->ContextFlags=0,min=~min;p!=end;++p)
            if((diff=(*p>f) ? (*p-f) : (f-*p))<min){
                min=diff; offset=(char*)p-(char*)c;
            }
        c->ContextFlags=CONTEXT_CONTROL;
    } return (char*)c+offset;
}
#define _CtxCtrlReg(_1_) _CtxCtrlRegf(&_1_)
#endif

#ifdef pthread_kill
#undef pthread_kill
#endif

int _pthread_kill_win(pthread_t tid,const int sig){
    void *f;
    if(!sig) return pthread_kill(tid,0);
    else if(pthread_equal(pthread_self(),tid)) return raise(sig);
    else if( (f=_pthread_raise_func(sig)) ){
        CONTEXT c={.ContextFlags=CONTEXT_CONTROL};
        void **x=(void*)_CtxCtrlReg(c), *p=pthread_gethandle(tid);
        SuspendThread(p);
        GetThreadContext(p,&c);
        *x=f;
        SetThreadContext(p,&c);
        c.ContextFlags=CONTEXT_ALL;
        GetThreadContext(p,&c);
        ResumeThread(p);
        return 0;
    } return -1;
}
#undef _CtxCtrlReg

static unsigned int _pthread_cores(void){
    SYSTEM_INFO sys; GetSystemInfo(&sys);
    return sys.dwNumberOfProcessors>1 ? sys.dwNumberOfProcessors : 1;
}

#else /* end _WIN32 */

extern int pthread_attr_getdetachstate(const pthread_attr_t *,int *);

static int _pthread_pipe(int fd[2]){
    fd[0]=fd[1]=-1; return pipe(fd);
}

int pthread_channel_open(pthread_channel_t * const channel){
    if(channel){
        int fd[2];
        if(_pthread_pipe(fd)){
            channel->r->i=channel->w->i=-1;
            return -1;
        }
        channel->r->i=fd[0];
        channel->w->i=fd[1];
        return 0;
    }
    return -1;
}

void pthread_channel_close(pthread_channel_t * const channel){
    if(channel && channel->r->i!=-1){
        close(channel->w->i);
        close(channel->r->i);
        channel->r->i=-1;
    }
}

int pthread_channel_push(pthread_channel_t * const channel,const void *data,int size){
    if(channel && channel->r->i!=-1 && data && size>0) return write(channel->w->i,data,size);
    return -1;
}

int pthread_channel_pop(pthread_channel_t * const channel,void *data,int size){
    if(channel && channel->r->i!=-1 && data){
        char *p=(char*)data;
        while(size>0){
            const int bytes=read(channel->r->i,p,size);
            if(bytes>0){
                size-=bytes;
                p+=bytes;
            }
        } return 0;
    } return -1;
}

#ifdef _SC_NPROCESSORS_CONF

static unsigned int _pthread_cores(void){
    const long int c=sysconf(_SC_NPROCESSORS_CONF);
    return c>1?c:1;
}

#else

#include <sys/sysctl.h>
static unsigned int _pthread_cores(void){
    int cores=1; size_t len=sizeof(cores);
    int mib[2]={CTL_HW,HW_NCPU};
    if(sysctl(mib,2,&cores,&len,NULL,0))
        return 1;
    return cores>1?cores:1;
}

#endif

#endif /* end not _WIN32 */


int pthread_poolattr_init(pthread_poolattr_t * const attr){
    if(!attr) return EINVAL;
    attr->_[0]=attr->_[1]=attr->_[2]=NULL; return 0;
}

void pthread_poolattr_destroy(pthread_poolattr_t * const attr){
    if(!attr) return;
    if(attr->_[0]) pthread_attr_destroy((pthread_attr_t*)attr->_[0]);
    if(attr->_[1]) pthread_mutexattr_destroy((pthread_mutexattr_t*)attr->_[1]);
    if(attr->_[2]) pthread_condattr_destroy((pthread_condattr_t*)attr->_[2]);
}

int pthread_poolattr_setpattr(pthread_poolattr_t * const attr,pthread_attr_t * const pattr){
    if(!attr) return EINVAL;
    attr->_[0]=pattr; return 0;
}

int pthread_poolattr_setmattr(pthread_poolattr_t * const attr,pthread_mutexattr_t * const mattr){
    if(!attr) return EINVAL;
    attr->_[1]=mattr; return 0;
}

int pthread_poolattr_setcattr(pthread_poolattr_t * const attr,pthread_condattr_t * const cattr){
    if(!attr) return EINVAL;
    attr->_[2]=cattr; return 0;
}

int pthread_poolattr_getpattr(const pthread_poolattr_t * const attr,pthread_attr_t ** const pattr){
    if(!attr) return EINVAL;
    *pattr=(pthread_attr_t*)attr->_[0]; return 0;
}

int pthread_poolattr_getmattr(const pthread_poolattr_t * const attr,pthread_mutexattr_t ** const mattr){
    if(!attr) return EINVAL;
    *mattr=(pthread_mutexattr_t*)attr->_[1]; return 0;
}

int pthread_poolattr_getcattr(const pthread_poolattr_t * const attr,pthread_condattr_t ** const cattr){
    if(!attr) return EINVAL;
    *cattr=(pthread_condattr_t*)attr->_[2]; return 0;
}



typedef struct __pthread_pool_task_t{
    struct __pthread_pool_task_t *next;
    int(*f)(pthread_pool_t *pool,void *args,unsigned int index);
}_pthread_pool_task_t;

typedef struct{
    _pthread_pool_task_t *first, *last;
}_pthread_pool_queue_t;

struct _pthread_pool_t{
    pthread_mutex_t mtx[1];
    pthread_cond_t cond[2];
    void*(*allocator)(size_t);
    void(*deallocator)(void*);
    _pthread_pool_task_t *reject;
    unsigned int count, busy, size;
    unsigned char ctrl, peak, max, banch;
    _pthread_pool_queue_t queue[1];
};

typedef struct{
    pthread_pool_t *p;
    unsigned int i;
}_pthread_pool_initializer_t;

#define _pthread_pool_pad(_N_) ((M_PADDING(_pthread_pool_queue_t,pthread_t)*(_N_))%M_ALIGNOF(pthread_t))
#define _pthread_pool_tids(_p_) ((pthread_t*)(((char*)(_p_->queue+1+(unsigned int)_p_->max))+_pthread_pool_pad(1+(unsigned int)_p_->max)))

static void _pthread_pool_push(pthread_pool_t * const p,_pthread_pool_task_t * const t,unsigned char prio){
    if(prio>p->peak) p->peak=prio;
    {_pthread_pool_queue_t * const q=p->queue + prio;
    if(q->last) q->last->next=t;
    else q->first=t;
    q->last=t;}
    ++p->size;
}

static _pthread_pool_task_t *_pthread_pool_pop(pthread_pool_t * const p){
    _pthread_pool_queue_t * const q=p->queue+p->peak;
    _pthread_pool_task_t * const t=q->first;
    if(t && !(q->first=t->next)){
        q->last=NULL;
        while(p->peak && !p->queue[--p->peak].first);
    } return t;
}

static void _pthread_pool_reject(pthread_pool_t * const p){
    if(p->size){
        unsigned int c=p->peak;
        _pthread_pool_queue_t * const q=p->queue+c;
        while(c){
            _pthread_pool_queue_t * const i=p->queue+(--c);
            if(i->first){
                q->last->next=i->first;
                q->last=i->last;
                i->first=i->last=NULL;
            }
        }
        p->reject=q->last;
        if(p->peak!=p->max){
            _pthread_pool_queue_t * const m=p->queue+(p->peak=p->max);
            *m=*q; q->first=q->last=NULL;
        }
    }
}

static void _pthread_pool_release(pthread_pool_t * const p){
    pthread_mutex_destroy(p->mtx);
    pthread_cond_destroy(p->cond);
    pthread_cond_destroy(p->cond+1);
    p->deallocator(p);
}

static void *_pthread_pool_worker(_pthread_pool_initializer_t * const arg){
    pthread_pool_t * const p=arg->p;
    const unsigned int index=arg->i;
    unsigned int busy=0;
    void(* const del)(void*)=p->deallocator;

    del(arg);

    pthread_mutex_lock(p->mtx);
    while('0'){
        _pthread_pool_task_t *t=_pthread_pool_pop(p);
        if(t){
            pthread_pool_t *_p;
            _pthread_pool_task_t *i=t;
            unsigned int c=(--p->size)/p->count;
            if(c>p->banch){c=p->banch;}
            if(p->reject){
                unsigned int j=0;
                for(_p=NULL;;){
                    if(i==p->reject){p->reject=NULL; break;}
                    if(j==c){break;} ++j;
                    i=i->next=_pthread_pool_pop(p);
                } p->size-=j;
            }else
                for(_p=((p->ctrl & 2)?NULL:p),p->size-=c;c;--c)
                    i=i->next=_pthread_pool_pop(p);
            if(!busy) {busy=1; ++p->busy;}

            pthread_mutex_unlock(p->mtx);
            i->next=NULL;
            do{
                i=t->next;
                if(!t->f(_p,t+1,index)) del(t);
            }while( (t=i) );
            pthread_mutex_lock(p->mtx);
        }else{
            if(busy) {busy=0; --p->busy;}
            if(!p->busy) pthread_cond_broadcast(p->cond+1);
            if(p->ctrl & 1) break;
            pthread_cond_wait(p->cond,p->mtx);
        }
    }
    busy=p->ctrl & 12;
    busy|=(busy & 4) && !--p->count;
    pthread_mutex_unlock(p->mtx);

    if(!index && !(busy & 4)){
        pthread_t * const tid=_pthread_pool_tids(p);
        unsigned int i=p->count;
        while(--i) pthread_join(tid[i],NULL);
        busy|=(busy & 8)>>3;
    }
    if(busy & 1) _pthread_pool_release(p);

    return NULL;
}

pthread_pool_t *pthread_pool_create(const unsigned int count,const unsigned char prio){
    return pthread_pool_create_ex(count,prio,NULL,(void*(*)(size_t))0,(void(*)(void*))0);
}

pthread_pool_t *pthread_pool_create_ex(unsigned int count,const unsigned char prio,const pthread_poolattr_t * const attr,void*(*allocator)(size_t),void(*deallocator)(void*)){
    pthread_attr_t *pattr=NULL;
    pthread_condattr_t *cattr=NULL;
    pthread_mutexattr_t *mattr=NULL;
    int detached=PTHREAD_CREATE_JOINABLE;

    pthread_poolattr_getpattr(attr,&pattr);
    if(pattr && pthread_attr_getdetachstate(pattr,&detached))
        return NULL;

    pthread_poolattr_getmattr(attr,&mattr);
    pthread_poolattr_getcattr(attr,&cattr);
    if(!allocator) allocator=malloc;
    if(!deallocator) deallocator=free;

    if(count || (count=pthread_cores())){
        const size_t size=sizeof(_pthread_pool_queue_t)*(1+(unsigned int)prio) + sizeof(pthread_t)*count + _pthread_pool_pad(1+(unsigned int)prio);
        pthread_pool_t * const p=(pthread_pool_t*)allocator(M_OFFSETOF(struct _pthread_pool_t,queue) + size);
        if(p){
            if(pthread_mutex_init(p->mtx,mattr)){
                deallocator(p); return NULL;
            }
            if(pthread_cond_init(p->cond,cattr)){
                pthread_mutex_destroy(p->mtx);
                deallocator(p); return NULL;
            }
            if(pthread_cond_init(p->cond+1,cattr)){
                pthread_mutex_destroy(p->mtx);
                pthread_cond_destroy(p->cond);
                deallocator(p); return NULL;
            }
            p->allocator=allocator;
            p->deallocator=deallocator;
            p->reject=NULL;
            p->count=p->busy=p->size=0;
            p->ctrl=(detached==PTHREAD_CREATE_DETACHED)*4;
            p->peak=0;
            p->max=prio;
            p->banch=0;
            memset(p->queue,0,size);

            {pthread_t * const tid=_pthread_pool_tids(p);
            for(;p->count<count;++p->count){
                _pthread_pool_initializer_t * const _i=(_pthread_pool_initializer_t*)allocator(sizeof(*_i));
                if(!_i){
                    pthread_pool_destroy(p,1); return NULL;
                }
                _i->p=p; _i->i=p->count;
                if(pthread_create(tid+p->count,pattr,(void*(*)(void*))_pthread_pool_worker,_i)){
                    deallocator(_i); pthread_pool_destroy(p,1); return NULL;
                }
            }}
            return p;
        }
    } return NULL;
}

int pthread_pool_detach(pthread_pool_t * const p,const int forced){
    int err=EINVAL;
    pthread_mutex_lock(p->mtx);
    if( !(p->ctrl & 12) && ( !(err=pthread_detach(_pthread_pool_tids(p)[0])) || forced) )
        p->ctrl|=8;
    pthread_mutex_unlock(p->mtx);
    return err;
}

void pthread_pool_destroy(pthread_pool_t * const p,const unsigned char now){
    if(p){
        unsigned int i=1|((now!=0)<<1);
        pthread_mutex_lock(p->mtx);
        i=(p->ctrl|=i);
        pthread_cond_broadcast(p->cond);
        pthread_mutex_unlock(p->mtx);
        if(i & 12) return;
        pthread_join(_pthread_pool_tids(p)[0],NULL);
        _pthread_pool_release(p);
    }
}

void pthread_pool_unpending(pthread_pool_t * const p){
    pthread_mutex_lock(p->mtx);
    _pthread_pool_reject(p);
    pthread_mutex_unlock(p->mtx);
}

void pthread_pool_wait(pthread_pool_t * const p){
    pthread_mutex_lock(p->mtx);
    while(p->busy || p->size) pthread_cond_wait(p->cond+1,p->mtx);
    pthread_mutex_unlock(p->mtx);
}

void pthread_pool_clear(pthread_pool_t * const p){
    pthread_mutex_lock(p->mtx);
    if(p->busy || p->size){
        p->ctrl|=2;
        do{ pthread_cond_wait(p->cond+1,p->mtx); } while(p->busy || p->size);
        p->ctrl&=~2;
    }
    pthread_mutex_unlock(p->mtx);
}

int pthread_pool_timedwait(pthread_pool_t * const p,const struct timespec *abstime){
    #define _CASE_ERR case EINVAL: err=EINVAL; goto _mark; case ETIMEDOUT: err=ETIMEDOUT; goto _mark;
    int err=0;
    pthread_mutex_lock(p->mtx);
    while(p->busy || p->size)
        switch(pthread_cond_timedwait(p->cond+1,p->mtx,abstime)){
            case -1: switch(errno){_CASE_ERR} break;
            _CASE_ERR
        }
_mark:
    pthread_mutex_unlock(p->mtx);
    return err;
    #undef _CASE_ERR
}

void *pthread_pool_raw_task_create(const pthread_pool_t * const p,const unsigned int size){
    return (p->ctrl & 3) ? NULL : (size ? p->allocator(size) : (void*)1);
}

void pthread_pool_raw_task_queue(pthread_pool_t * const p,void * const t,unsigned char prio){
    if(prio>p->max) prio=p->max;
    ((_pthread_pool_task_t*)t)->next=NULL;
    pthread_mutex_lock(p->mtx);
    _pthread_pool_push(p,(_pthread_pool_task_t*)t,prio);
    pthread_cond_signal(p->cond);
    pthread_mutex_unlock(p->mtx);
}

void pthread_pool_banch(pthread_pool_t * const p,const unsigned char count){
    pthread_mutex_lock(p->mtx);
    p->banch=count;
    pthread_mutex_unlock(p->mtx);
}

unsigned int pthread_pool_count(const pthread_pool_t * const p){
    return p->count;
}

const pthread_t *pthread_pool_array(const pthread_pool_t * const p){
    return _pthread_pool_tids(p);
}

#undef _pthread_pool_tids
#undef _pthread_pool_pad

int pthread_groupattr_init(pthread_groupattr_t * const attr){
    if(!attr) return EINVAL;
    attr->_[0]=attr->_[1]=NULL; return 0;
}

void pthread_groupattr_destroy(pthread_groupattr_t * const attr){
    if(!attr) return;
    if(attr->_[0]) pthread_mutexattr_destroy((pthread_mutexattr_t*)attr->_[0]);
    if(attr->_[1]) pthread_condattr_destroy((pthread_condattr_t*)attr->_[1]);
}

int pthread_groupattr_setmattr(pthread_groupattr_t * const attr,pthread_mutexattr_t * const mattr){
    if(!attr) return EINVAL;
    attr->_[0]=mattr; return 0;
}

int pthread_groupattr_setcattr(pthread_groupattr_t * const attr,pthread_condattr_t * const cattr){
    if(!attr) return EINVAL;
    attr->_[1]=cattr; return 0;
}

int pthread_groupattr_getmattr(const pthread_groupattr_t * const attr,pthread_mutexattr_t ** const mattr){
    if(!attr) return EINVAL;
    *mattr=(pthread_mutexattr_t*)attr->_[0]; return 0;
}

int pthread_groupattr_getcattr(const pthread_groupattr_t * const attr,pthread_condattr_t ** const cattr){
    if(!attr) return EINVAL;
    *cattr=(pthread_condattr_t*)attr->_[1]; return 0;
}


static void _pthread_group_reset(pthread_group_t * const g,const unsigned int target){
    g->target=target; g->done=g->reject=0; g->state=0; g->wait=1;
}

int pthread_group_init(pthread_group_t * const g,const unsigned int target,const pthread_groupattr_t * const attr,void(* const deallocator)(void*)){
    if(g){
        int err;
        pthread_condattr_t *cattr=NULL;
        pthread_mutexattr_t *mattr=NULL;
        pthread_groupattr_getmattr(attr,&mattr);
        pthread_groupattr_getcattr(attr,&cattr);
        if( (err=pthread_mutex_init(g->mtx,mattr)) )
            return err;
        if( (err=pthread_cond_init(g->cond,cattr)) ){
            pthread_mutex_destroy(g->mtx);
            return err;
        }
        _pthread_group_reset(g,target);
        g->deallocator=deallocator;
        g->destroy=0;
        return 0;
    } return EINVAL;
}

static void _pthread_group_destroy(pthread_group_t * const g){
    pthread_mutex_destroy(g->mtx);
    pthread_cond_destroy(g->cond);
    if(g->deallocator) g->deallocator(g);
}

int pthread_group_destroy(pthread_group_t * const g){
    int del=0;
    pthread_mutex_lock(g->mtx);
    if(g->target){
        g->state=-1;
        if(g->deallocator) g->destroy=1;
        else{
            del=1;
            while(g->wait) pthread_cond_wait(g->cond,g->mtx);
        }
    }else del=1;
    pthread_mutex_unlock(g->mtx);
    if(del) _pthread_group_destroy(g);
    return del;
}

int pthread_group_wait(pthread_group_t * const g,unsigned int * const done,unsigned int * const target){
    int ret;
    pthread_mutex_lock(g->mtx);
    if(g->target) while(g->wait) pthread_cond_wait(g->cond,g->mtx);
    if(done) *done=g->done;
    if(target) *target=g->target;
    ret=g->state;
    _pthread_group_reset(g,0);
    pthread_mutex_unlock(g->mtx);
    return ret;
}

int pthread_group_timedwait(pthread_group_t * const g,unsigned int * const done,unsigned int * const target,struct timespec * const abstime){
    int ret,err=0;
    pthread_mutex_lock(g->mtx);
    if(g->target) while(g->wait)
        #define _CASE_ERR case EINVAL: err=EINVAL; goto _mark; case ETIMEDOUT: err=ETIMEDOUT; goto _mark;
        switch(pthread_cond_timedwait(g->cond,g->mtx,abstime)){
            case -1: switch(errno){ _CASE_ERR }
            _CASE_ERR
        }
        #undef _CASE_ERR
_mark:
    if(done) *done=g->done;
    if(target) *target=g->target;
    if(err){
        ret=err;
    }else{
        ret=g->state;
       _pthread_group_reset(g,0);
    }
    pthread_mutex_unlock(g->mtx);
    return ret;
}

int pthread_group_progress_ex(pthread_group_t * const g,const unsigned int add_targets,void(* const f)(void *arg),void * const arg){
    int del=0;
    pthread_mutex_lock(g->mtx);
    if(f) f(arg);
    g->done+=!!g->target;
    g->target+=add_targets;
    if(g->done+g->reject==g->target){
        if(g->destroy) del=1;
        else{
            g->wait=0;
            pthread_cond_broadcast(g->cond);
        }
    }
    pthread_mutex_unlock(g->mtx);
    if(del) _pthread_group_destroy(g);
    return del;
}

int pthread_group_progress(pthread_group_t * const g,const unsigned int add_targets){
    return pthread_group_progress_ex(g,add_targets,NULL,NULL);
}

int pthread_group_reject_ex(pthread_group_t * const g,const unsigned char by_task,void(* const f)(void *arg),void * const arg){
    int del=0;
    pthread_mutex_lock(g->mtx);
    if(f) f(arg);
    g->state=-1;
    if(by_task){
        if(++g->reject+g->done==g->target){
            if(g->destroy) del=1;
            else{
                g->wait=0;
                pthread_cond_broadcast(g->cond);
            }
        }
    }else{
        if(g->target) while(g->wait) pthread_cond_wait(g->cond,g->mtx);
        _pthread_group_reset(g,0);
    }
    pthread_mutex_unlock(g->mtx);
    if(del) _pthread_group_destroy(g);
    return del;
}

int pthread_group_reject(pthread_group_t * const g,const unsigned char by_task){
    return pthread_group_reject_ex(g,by_task,NULL,NULL);
}

int pthread_group_rejected(pthread_group_t * const g){
    return (g->state ? ((pthread_group_reject(g,1)<<1)-1) : 0);
}

unsigned int pthread_cores(void){
    static unsigned int cores=0;
    if(!cores) cores=_pthread_cores();
    return cores;
}

#endif /*PTHREAD_EXT_IMPL*/


#ifdef _WIN32

#ifdef pthread_kill
#undef pthread_kill
#endif
#define pthread_kill _pthread_kill_win
int _pthread_kill_win(pthread_t,int);

#endif
