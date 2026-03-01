/* * * * * * * * * * * * * * * * * */
/* MIT License                     */
/* Copyright (c) 2024 ANSI-Christ  */
/* * * * * * * * * * * * * * * * * */

#ifndef PTHREAD_EXT_H
#define PTHREAD_EXT_H

#include <pthread.h>
#include <stddef.h>
#include "macro.h"



int pthread_policy_set(pthread_t tid,int policy,int priority);
int pthread_policy_get(pthread_t tid,int *policy,int *priority);

const char *pthread_policy_name(int policy);

unsigned int pthread_cores(void);
unsigned int pthread_backtrace(void **array,unsigned int count);



typedef struct _pthread_pool_t pthread_pool_t;

pthread_pool_t *pthread_pool_create(unsigned int count,unsigned char prio);
pthread_pool_t *pthread_pool_create_ex(unsigned int count,unsigned char prio,unsigned int stackSize_kb,void*(*allocator)(size_t),void(*deallocator)(void*));

int pthread_pool_timedwait(pthread_pool_t *pool,const struct timespec *abstime);


void pthread_pool_wait(pthread_pool_t *pool);
void pthread_pool_clear(pthread_pool_t *pool);
void pthread_pool_unpending(pthread_pool_t *pool);
void pthread_pool_banch(pthread_pool_t *pool,unsigned char count);
void pthread_pool_destroy(pthread_pool_t *pool,unsigned char now,unsigned char async);

void *pthread_pool_task(pthread_pool_t *pool,void(*task)(pthread_pool_t *pool,void *args,unsigned int index),...);
void *pthread_pool_task_prio(pthread_pool_t *pool,unsigned char prio,void(*task)(pthread_pool_t *pool,void *args,unsigned int index),...);

unsigned int pthread_pool_count(const pthread_pool_t *pool);

const pthread_t *pthread_pool_array(const pthread_pool_t *pool);




typedef struct{ union{void *p; int i;} r[1], w[1]; } pthread_channel_t;

int pthread_channel_open(pthread_channel_t *channel);
int pthread_channel_pop(pthread_channel_t *channel,void *data,int size);
int pthread_channel_push(pthread_channel_t *channel,const void *data,int size);

void pthread_channel_close(pthread_channel_t *channel);




#define _PTHREAD_SETUP(_index_,_0_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( do{ union _pthread_pool_task_arg{M_TYPEOF(__VA_ARGS__) _; struct{char _[sizeof(M_TYPEOF(__VA_ARGS__))];} x;}; ((union _pthread_pool_task_arg*)_0_->M_JOIN(_,_index_).x)->x=((const union _pthread_pool_task_arg*)&M_JOIN(_a,_index_).x)->x; }while(0); )
#define _PTHREAD_FIELD(_index_,_0_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( union{M_TYPEOF(__VA_ARGS__) _;char x[1];} M_JOIN(_,_index_); )
#define _PTHREAD_ARGUM(_index_,_0_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( const struct{M_TYPEOF(__VA_ARGS__) x;} M_JOIN(_a,_index_)={__VA_ARGS__}; )
#define _PTHREAD_TASK(_1_,_2_,_3_,...) ({\
    void * const _f_=(_3_);\
    pthread_pool_t * const _p_=(_1_);\
    M_FOREACH(_PTHREAD_ARGUM,-,__VA_ARGS__)\
    struct _pthread_pool_task_size{void *n,*f; M_FOREACH(_PTHREAD_FIELD,-,__VA_ARGS__) char size;} * const _t_=(struct _pthread_pool_task_size*)((_p_ && _f_) ? _pthread_pool_task_alloc(_p_,M_OFFSETOF(struct _pthread_pool_task_size,size)) : NULL);\
    if(_t_){\
        struct _pthread_pool_task_args{ M_FOREACH(_PTHREAD_FIELD,-,__VA_ARGS__) char size;};\
        M_ASSERT( sizeof(void*[2]) + M_OFFSETOF(struct _pthread_pool_task_args,size) == M_OFFSETOF(struct _pthread_pool_task_size,size) , pthread_pool_task_bad_align_of_arguments);\
        _t_->n=NULL; _t_->f=_f_; M_FOREACH(_PTHREAD_SETUP,_t_,__VA_ARGS__) _pthread_pool_task_run(_p_,_t_,(_2_));\
    } _t_;\
})
#define pthread_pool_task(_1_,_3_,...) _PTHREAD_TASK((_1_),0,(_3_),__VA_ARGS__)
#define pthread_pool_task_prio(_1_,_2_,_3_,...) _PTHREAD_TASK((_1_),(_2_),(_3_),__VA_ARGS__)
void _pthread_pool_task_run(pthread_pool_t *,void *,unsigned char);
void *_pthread_pool_task_alloc(const pthread_pool_t *p,unsigned int);
extern int nanosleep(const struct timespec*,struct timespec*);
extern int pthread_kill(pthread_t,int);
#endif /* PTHREAD_EXT_H */





#ifdef PTHREAD_EXT_IMPL

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


typedef struct __pthread_pool_task_t{
    struct __pthread_pool_task_t *next;
    void (*f)(void *p,void *a,unsigned int i);
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

#define _pthread_pool_tids(_p_) ((pthread_t*)(p->queue+1+p->max))

typedef struct{
    pthread_pool_t *p;
    unsigned int i;
}_pthread_pool_initializer_t;

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
            m->first=q->first; m->last=q->last;
            q->first=q->last=NULL;
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
                t->f(_p,t+1,index);
                del(t); t=i;
            }while(t);
            pthread_mutex_lock(p->mtx);
        }else{
            if(busy) {busy=0; --p->busy;}
            if(!p->busy) pthread_cond_broadcast(p->cond+1);
            if(p->ctrl & 1) break;
            pthread_cond_wait(p->cond,p->mtx);
        }
    }
    busy=p->ctrl & 4;
    busy|=busy && !--p->count;
    pthread_mutex_unlock(p->mtx);

    if(busy) pthread_detach(pthread_self());
    if(busy & 1) _pthread_pool_release(p);
    return NULL;
}

static char _pthread_attr_init(pthread_attr_t *a,size_t s){
    do{
        if(pthread_attr_init(a)) return 0;
        if(s && pthread_attr_setstacksize(a,s<<10)) break;
        if(pthread_attr_setinheritsched(a,PTHREAD_INHERIT_SCHED)) break;
        #ifdef PTHREAD_FPU_ENABLED
        if(pthread_setfpustate(a,PTHREAD_FPU_ENABLED)) break;
        #endif
        return 1;
    }while(0);
    pthread_attr_destroy(a);
    return 0;
}

pthread_pool_t *pthread_pool_create(const unsigned int count,const unsigned char prio){
    return pthread_pool_create_ex(count,prio,0,(void*(*)(size_t))0,(void(*)(void*))0);
}

pthread_pool_t *pthread_pool_create_ex(unsigned int count,const unsigned char prio,const unsigned int stackSize_kb,void*(*allocator)(size_t),void(*deallocator)(void*)){
    pthread_attr_t attr[1];
    if( (count || (count=pthread_cores())) && _pthread_attr_init(attr,stackSize_kb) && (allocator || (allocator=malloc)) && (deallocator || (deallocator=free)) ){
        const size_t size=sizeof(_pthread_pool_queue_t)*(1+(unsigned int)prio) + sizeof(pthread_t)*count;
        pthread_pool_t *p=(pthread_pool_t*)allocator(M_OFFSETOF(*p,queue) + size);
        while(p){
            if(pthread_mutex_init(p->mtx,NULL)){
                deallocator(p); p=NULL;
                break;
            }
            if(pthread_cond_init(p->cond,NULL)){
                pthread_mutex_destroy(p->mtx);
                deallocator(p); p=NULL;
                break;
            }
            if(pthread_cond_init(p->cond+1,NULL)){
                pthread_mutex_destroy(p->mtx);
                pthread_cond_destroy(p->cond);
                deallocator(p); p=NULL;
                break;
            }
            p->allocator=allocator;
            p->deallocator=deallocator;
            p->reject=NULL;
            p->count=p->busy=p->size=0;
            p->ctrl=p->peak=0;
            p->max=prio;
            p->banch=3;
            memset(p->queue,0,size);

            {pthread_t * const tid=_pthread_pool_tids(p);
            for(;p->count<count;++p->count){
                _pthread_pool_initializer_t * const _i=(_pthread_pool_initializer_t*)allocator(sizeof(*_i));
                if(!_i){
                    pthread_pool_destroy(p,1,0); p=NULL; break;
                }
                _i->p=p; _i->i=p->count;
                if(pthread_create(tid+p->count,attr,(void*(*)(void*))_pthread_pool_worker,_i)){
                    deallocator(_i); pthread_pool_destroy(p,1,0); p=NULL; break;
                }
            }}
            break;
        }
        pthread_attr_destroy(attr);
        return p;
    } return NULL;
}

void pthread_pool_destroy(pthread_pool_t * const p,const unsigned char now,const unsigned char async){
    if(p){
        unsigned int i=1|((now!=0)<<1)|((async!=0)<<2);
        pthread_mutex_lock(p->mtx);
        p->ctrl|=i; pthread_cond_broadcast(p->cond);
        pthread_mutex_unlock(p->mtx);
        if(async) return;
        {pthread_t * const tid=_pthread_pool_tids(p);
        for(i=p->count;i;) pthread_join(tid[--i],NULL);}
        _pthread_pool_release(p);
    }
}

void pthread_pool_unpending(pthread_pool_t * const p){
    if(!p) return;
    pthread_mutex_lock(p->mtx);
    _pthread_pool_reject(p);
    pthread_mutex_unlock(p->mtx);
}

void pthread_pool_wait(pthread_pool_t * const p){
    if(!p) return;
    pthread_mutex_lock(p->mtx);
    while(p->busy || p->size) pthread_cond_wait(p->cond+1,p->mtx);
    pthread_mutex_unlock(p->mtx);
}

void pthread_pool_clear(pthread_pool_t * const p){
    if(!p) return;
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
    if(p && abstime){
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
    } return EINVAL;
    #undef _CASE_ERR
}

void *_pthread_pool_task_alloc(const pthread_pool_t * const p,const unsigned int size){
    return p->ctrl ? NULL : p->allocator(size);
}

void _pthread_pool_task_run(pthread_pool_t * const p,void * const t,unsigned char prio){
    if(prio>p->max) prio=p->max;
    pthread_mutex_lock(p->mtx);
    _pthread_pool_push(p,(_pthread_pool_task_t*)t,prio);
    pthread_cond_signal(p->cond);
    pthread_mutex_unlock(p->mtx);
}

void pthread_pool_banch(pthread_pool_t * const p,unsigned char count){
    if(!p) return;
    if(count) --count;
    pthread_mutex_lock(p->mtx);
    p->banch=count;
    pthread_mutex_unlock(p->mtx);
}

unsigned int pthread_pool_count(const pthread_pool_t * const p){
    return p ? p->count : 0;
}

const pthread_t *pthread_pool_array(const pthread_pool_t * const p){
    return p ? _pthread_pool_tids(p) : NULL;
}

#undef _pthread_pool_tids

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


#define _pthread_iter(_1_,_2_,_sig_) static void _pthread_raise_##_sig_(void){raise(_sig_);}
M_FOREACH(_pthread_iter,-,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40)
#undef _pthread_iter

static void *_pthread_raise_func(const int sig){
#define _pthread_iter(_1_,_2_,_sig_) case _sig_:return _pthread_raise_##_sig_;
    switch(sig){
        M_FOREACH(_pthread_iter,-,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40)
    } return NULL;
#undef _pthread_iter
}

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

int _pthread_kill_win(pthread_t tid,const int sig){
    void *f=_pthread_raise_func(sig);
    if(f){
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

unsigned int pthread_backtrace(void **array,unsigned int count){
    const int c=CaptureStackBackTrace(0,count,array,NULL);
    return c>0?c:0;
}

static unsigned int _pthread_cores(void){
    SYSTEM_INFO sys; GetSystemInfo(&sys);
    return sys.dwNumberOfProcessors>1 ? sys.dwNumberOfProcessors : 1;
}

#else /* end _WIN32 */

static int _pthread_pipe(int fd[2]){
    struct _pipe_t{long fd[2];} s={{-1,-1}};
    const union{void * const _; struct _pipe_t(* const f)(int fd[2]);}f={(void*)pipe};
    fd[0]=fd[1]=-1; s=f.f(fd);
    if(s.fd[0]==-1) return -1;
    if(fd[0]==-1){
        fd[0]=s.fd[0];
        fd[1]=s.fd[1];
    }
    return fd[0]==-1;
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

extern int backtrace(void**,int);

unsigned int pthread_backtrace(void **array,unsigned int count){
    const int c=backtrace(array,count);
    return c>0?c:0;
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

unsigned int pthread_cores(void){
    static unsigned int cores=0;
    if(!cores) cores=_pthread_cores();
    return cores;
}

static int _pthread_policy_checked(const int pol,const int pri){
#ifdef _POSIX_PRIORITY_SCHEDULING
    const int min=sched_get_priority_min(pol), max=sched_get_priority_max(pol);
    if(pri>max) return max;
    if(pri<min) return min;
#endif
    return pri;
}

int pthread_policy_set(pthread_t tid,int policy,int priority){
    struct sched_param s[1]={{0}}; s->sched_priority=_pthread_policy_checked(policy,priority);
    return pthread_setschedparam(tid,policy,s);
}

int pthread_policy_get(pthread_t tid,int *policy,int *priority){
    struct sched_param s[1]={{0}};
    const int err=pthread_getschedparam(tid,policy,s); *priority=s->sched_priority;
    return err;
}

const char *pthread_policy_name(int policy){
    switch(policy){
        #ifdef SCHED_RR
            case SCHED_RR: return "SCHED_RR";
        #endif
        #ifdef SCHED_FIFO
            case SCHED_FIFO: return "SCHED_FIFO";
        #endif
        #ifdef SCHED_IDLE
            case SCHED_IDLE: return "SCHED_IDLE";
        #endif
        #ifdef SCHED_OTHER
            case SCHED_OTHER: return "SCHED_OTHER";
        #endif
        #ifdef SCHED_BATCH
            case SCHED_BATCH: return "SCHED_BATCH";
        #endif
        #ifdef SCHED_DEADLINE
            case SCHED_DEADLINE: return "SCHED_DEADLINE";
        #endif
    }
    return "unknown";
}

#endif /*PTHREAD_EXT_IMPL*/

#ifdef _WIN32
    int _pthread_kill_win(pthread_t,int);
    #ifndef pthread_kill
        #define pthread_kill _pthread_kill_win
    #endif
#endif
