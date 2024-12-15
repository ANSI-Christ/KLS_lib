#ifndef PTHREAD_EXT_H
#define PTHREAD_EXT_H

#include <pthread.h>
#include <stddef.h>
#include <signal.h>
#include "macro.h"

extern int pthread_signal_resume;
extern int pthread_signal_pause;

int pthread_signal_getmode(int sig);
void pthread_signal_setmode(int sig,int mode);
void pthread_signal_send(pthread_t tid,int sig);
void *pthread_signal_handler(int sig,void(*handler)(int sig));
const char *pthread_signal_name(int sig);

void pthread_pause(pthread_t tid);
void pthread_resume(pthread_t tid);
void pthread_pausable(unsigned char pausable);

const char *pthread_policy_name(int policy);
unsigned char pthread_policy_set(pthread_t tid,int policy,int priority);
unsigned char pthread_policy_get(pthread_t tid,int *policy,int *priority);

unsigned int pthread_cores(void);
unsigned int pthread_backtrace(void **array,unsigned int count);



typedef struct _pthread_pool_t*  pthread_pool_t;

pthread_pool_t pthread_pool_create(unsigned int count,unsigned char prio,size_t stackSize_kb);

void pthread_pool_wait(pthread_pool_t pool);
void pthread_pool_clear(pthread_pool_t pool);
void pthread_pool_destroy(pthread_pool_t *pool);
void pthread_pool_destroy_later(pthread_pool_t *pool);

void *pthread_pool_task(pthread_pool_t pool,void(*task)(void *args,unsigned int index,pthread_pool_t pool),...);
void *pthread_pool_task_prio(pthread_pool_t pool,unsigned char prio,void(*task)(void *args,unsigned int index,pthread_pool_t pool),...);

unsigned int pthread_pool_count(const pthread_pool_t pool);

unsigned char pthread_pool_timedwait(pthread_pool_t pool,unsigned int msec);

const pthread_t *pthread_pool_array(pthread_pool_t pool);






#define _PTHREAD_OFFSET(_s_,_f_) (size_t)(&((__typeof__(_s_)*)0)->_f_)
#define _PTHREAD_STRUCT(...) struct{void *p; void *f; M_FOREACH(__PTHREAD_STRUCT,-,__VA_ARGS__) char size;}
#define __PTHREAD_STRUCT(_index_,_0_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( __typeof__(__VA_ARGS__) M_JOIN(_,_index_); )
#define _PTHREAD_TASK(_id_,_pr_,_f_,...) ({\
    const _PTHREAD_STRUCT(__VA_ARGS__) M_JOIN(_pt_,M_LINE())={NULL,(_f_),__VA_ARGS__};\
    M_ASSERT( sizeof(struct{void *p[2];}) + _PTHREAD_OFFSET(struct{M_FOREACH(__PTHREAD_STRUCT,-,__VA_ARGS__) char size;},size) == _PTHREAD_OFFSET(M_JOIN(_pt_,M_LINE()),size), pthread_pool_task_bad_align_of_arguments);\
    extern void *_pthread_pool_task(void *pool,const void *task,const unsigned int size,unsigned char prio);\
    _pthread_pool_task(_id_,&M_JOIN(_pt_,M_LINE()),_PTHREAD_OFFSET(M_JOIN(_pt_,M_LINE()),size),(_pr_));\
})
#define pthread_pool_task(_1_,_3_,...) _PTHREAD_TASK((_1_),0,(_3_),__VA_ARGS__)
#define pthread_pool_task_prio(_1_,_2_,_3_,...) _PTHREAD_TASK((_1_),(_2_),(_3_),__VA_ARGS__)

#endif /* PTHREAD_EXT_H */





#ifdef PTHREAD_EXT_IMPL

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define _PTHREAD_MKSLEEP(_mk_) {const struct timespec t={_mk_/1000000,(_mk_%1000000)*1000};nanosleep(&t,NULL);}

typedef struct{
    void *next;
    void (*f)(void *args,unsigned int index,void *pool);
}_pthread_pool_task_t;

typedef struct{
    _pthread_pool_task_t *first, *last;
}_pthread_pool_queue_t;

typedef struct _pthread_pool_t{
    pthread_mutex_t mtx[1];
    pthread_cond_t cond[2];
    pthread_t *tid;
    unsigned int count, busy, size;
    unsigned char die, wait;
    unsigned char peak, max;
    _pthread_pool_queue_t queue[1];
} * const _pthread_pool_t;



static void _pthread_pool_push(_pthread_pool_t p,_pthread_pool_task_t * const t,unsigned char prio){
    if(prio>p->peak) p->peak=prio;
    {_pthread_pool_queue_t * const q=p->queue + prio;
    if(q->last) q->last->next=t;
    else q->first=t;
    q->last=t;}
    ++p->size;
}

static void *_pthread_pool_pop(_pthread_pool_t p){
    _pthread_pool_queue_t * const q=p->queue+p->peak;
    _pthread_pool_task_t * const t=q->first;
    if(t && !(q->first=t->next)){
        q->last=NULL;
        while(p->peak && !p->queue[--p->peak].first);
    } return t;
}

static void _pthread_pool_clear(_pthread_pool_t p){
    void *t; while( (t=_pthread_pool_pop(p)) ) free(t);
    p->size=0;
}

static unsigned int _pthread_pool_index(const _pthread_pool_t p){
    unsigned int i=p->count-1;
    const pthread_t tid=pthread_self(), * const tids=p->tid;
    while(!pthread_equal(tid,tids[i])) --i;
    return i;
}

static void *_pthread_pool_worker(_pthread_pool_t p){
    const unsigned int index=_pthread_pool_index(p);
    unsigned char i, sleep=0, busy=1;
    _pthread_pool_task_t *t[4];

    while('0'){
        pthread_mutex_lock(p->mtx);
_mark:
        if(p->die==1)
            break;
        if( (t[0]=_pthread_pool_pop(p)) ){
            const unsigned int s=(--p->size)/p->count;
            const unsigned char c=(s>3?3:s);
            for(i=0,p->size-=c; i<c;)
                t[++i]=_pthread_pool_pop(p);
            if(busy&1){busy<<=1; ++p->busy;}

            pthread_mutex_unlock(p->mtx);

            for(i=0;i<=c;++i){
                t[i]->f(t[i]+1,index,p);
                free(t[i]);
            }
            sleep|=64;
            continue;
        }
        if(p->die)
            break;
        if(busy&2){
            busy>>=1; --p->busy;
        }
        if(p->wait && !p->busy)
            pthread_cond_broadcast(p->cond+1);
        if(!sleep){
            pthread_cond_wait(p->cond,p->mtx);
            goto _mark;
        }
        pthread_mutex_unlock(p->mtx);

        _PTHREAD_MKSLEEP(1000);
        sleep>>=1;

    }
    pthread_mutex_unlock(p->mtx);
    return NULL;
}

static char _pthread_attr_init(void *a,size_t s){
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

pthread_pool_t pthread_pool_create(unsigned int count,unsigned char prio,size_t stackSize_kb){
    pthread_attr_t attr[1];
    if( (count || (count=pthread_cores())) && _pthread_attr_init(attr,stackSize_kb) ){
        const unsigned int queueSize=sizeof(_pthread_pool_queue_t)*(1+(unsigned int)prio);
        pthread_pool_t p=malloc( _PTHREAD_OFFSET(*p,queue) + queueSize + sizeof(pthread_t)*count);
        while(p){
            if(pthread_mutex_init(p->mtx,NULL)){
                free(p); p=NULL;
                break;
            }
            if(pthread_cond_init(p->cond,NULL)){
                pthread_mutex_destroy(p->mtx);
                free(p); p=NULL;
                break;
            }
            if(pthread_cond_init(p->cond+1,NULL)){
                pthread_mutex_destroy(p->mtx);
                pthread_cond_destroy(p->cond);
                free(p); p=NULL;
                break;
            }

            p->max=prio;
            p->die=p->wait=p->peak=0;
            p->count=p->size=p->busy=0;
            memset(p->queue,0,queueSize);
            p->tid=(void*)(p->queue+1+prio);

            while(p->count<count)
                if(pthread_create(p->tid+p->count++,attr,(void*)_pthread_pool_worker,p)){
                    --p->count; pthread_pool_destroy(&p); break;
                }
            break;
        }
        pthread_attr_destroy(attr);
        return p;
    } return NULL;
}

static void _pthread_pool_destroy(pthread_pool_t *pool,char die){
    if(pool && *pool){
        _pthread_pool_t p=*pool;
        unsigned int i=p->count;
        pthread_mutex_lock(p->mtx);
        p->die=die;
        pthread_cond_broadcast(p->cond);
        pthread_mutex_unlock(p->mtx);

        while(i) pthread_join(p->tid[--i],NULL);

        pthread_mutex_destroy(p->mtx);
        pthread_cond_destroy(p->cond);
        pthread_cond_destroy(p->cond+1);
        _pthread_pool_clear(p);
        free(*pool); *pool=NULL;
    }
}

void pthread_pool_destroy(pthread_pool_t *pool){_pthread_pool_destroy(pool,1);}
void pthread_pool_destroy_later(pthread_pool_t *pool){_pthread_pool_destroy(pool,2);}

void pthread_pool_wait(pthread_pool_t pool){
    if(!pool) return;
    pthread_mutex_lock(pool->mtx);
    pool->wait=1;
    pthread_cond_signal(pool->cond);
    while(pthread_cond_wait(pool->cond+1,pool->mtx));
    pool->wait=0;
    pthread_mutex_unlock(pool->mtx);
}

unsigned char pthread_pool_timedwait(pthread_pool_t pool,unsigned int msec){
    if(pool){
        unsigned char ret;
        struct timespec t;
        clock_gettime(CLOCK_REALTIME,&t);
        t.tv_sec+=msec/1000;
        t.tv_nsec+=(msec%1000)*1000000;
        t.tv_sec+=(t.tv_nsec/1000000000);
        t.tv_nsec%=1000000000;

        pthread_mutex_lock(pool->mtx);
        pool->wait=1;
        pthread_cond_signal(pool->cond);
        ret=!pthread_cond_timedwait(pool->cond+1,pool->mtx,&t);
        pool->wait=0;
        pthread_mutex_unlock(pool->mtx);

        return ret;
    } return -1;
}

void *_pthread_pool_task(void *pool,const void *task,const unsigned int size,unsigned char prio){
    _pthread_pool_t p=pool;
    if( p && !p->die && ((void**)task)[1] && (pool=malloc(size)) ){
        memcpy(pool,task,size);
        if(prio>p->max) prio=p->max;
        pthread_mutex_lock(p->mtx);
        _pthread_pool_push(p,pool,prio);
        pthread_cond_signal(p->cond);
        pthread_mutex_unlock(p->mtx);
        return pool;
    } return NULL;
}

void pthread_pool_clear(pthread_pool_t pool){
    if(pool){
        pthread_mutex_lock(pool->mtx);
        _pthread_pool_clear(pool);
        pthread_mutex_unlock(pool->mtx);
    }
}

unsigned int pthread_pool_count(const pthread_pool_t pool){
    return pool ? pool->count : 0;
}

const pthread_t *pthread_pool_array(pthread_pool_t pool){
    return pool ? pool->tid : NULL;
}


#ifdef __WIN32

#define NOMINMAX
#include <windows.h>

#define _pthread_kill(_1_,_2_,_sig_) static void _pthread_kill_##_sig_(void){raise(_sig_);}
M_FOREACH(_pthread_kill,-,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40)
#undef _pthread_kill

static void *_pthread_killFunc(int sig){
#define _pthread_kill(_1_,_2_,_sig_) case _sig_:return _pthread_kill_##_sig_;
    switch(sig){
        M_FOREACH(_pthread_kill,-,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40)
    } return NULL;
#undef _pthread_kill
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

void pthread_signal_send(pthread_t tid,int sig){
    void *f=_pthread_killFunc(sig);
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
    }
}

int pthread_signal_getmode(int sig){
    return SIG_UNBLOCK;(void)sig;
}

void pthread_signal_setmode(int sig,int mode){
    return; (void)sig;(void)mode;
}

unsigned int pthread_backtrace(void **array,unsigned int count){
    const int c=CaptureStackBackTrace(0,count,array,NULL);
    return c>0?c:0;
}

unsigned int pthread_cores(void){
    SYSTEM_INFO sys; GetSystemInfo(&sys);
    return sys.dwNumberOfProcessors>1 ? sys.dwNumberOfProcessors : 1;
}

#define _PTHREAD_CONT SIGBREAK

#else /* end __WIN32 */

extern int backtrace(void**,int);

void pthread_signal_send(pthread_t tid,int sig){
    pthread_kill(tid,sig);
}

int pthread_signal_getmode(int sig){
    sigset_t s[1];
    sigemptyset(s);
    pthread_sigmask(0,NULL,s);
    return sigismember(s,sig) ? SIG_UNBLOCK : SIG_BLOCK;
}

void pthread_signal_setmode(int sig,int mode){
    sigset_t s[1];
    sigemptyset(s);
    sigaddset(s,sig);
    pthread_sigmask(mode,s,NULL);
    return;
}

unsigned int pthread_backtrace(void **array,unsigned int count){
    const int c=backtrace(array,count);
    return c>0?c:0;
}

unsigned int pthread_cores(void){
    const long int c=sysconf(_SC_NPROCESSORS_CONF);
    return c>1?c:1;
}

#define _PTHREAD_CONT SIGCONT

#endif /* end not __WIN32 */


int pthread_signal_resume=_PTHREAD_CONT;
int pthread_signal_pause=SIGINT;

static pthread_key_t _pthreadKey;
static unsigned char _pthreadStatus=0;
static pthread_once_t _pthreadOnce=PTHREAD_ONCE_INIT;

static void _pthread_fini(void){
    if(_pthreadStatus) pthread_key_delete(_pthreadKey);
}

static void _pthread_init(void){
    if(!pthread_key_create(&_pthreadKey,NULL)){
        extern int atexit(void(*)(void));
        _pthreadStatus=1 | ((pthread_signal_getmode(pthread_signal_resume)==SIG_BLOCK)<<2) | ((pthread_signal_getmode(pthread_signal_pause)==SIG_BLOCK)<<1);
        atexit(_pthread_fini);
    }
}

static void _pthread_holder(int sig){
    pthread_signal_handler(sig,_pthread_holder);
    pthread_signal_setmode(sig,SIG_UNBLOCK);
    if(sig==pthread_signal_pause){
        const char * const p=pthread_getspecific(_pthreadKey);
        pthread_setspecific(_pthreadKey,p+1);
        if(!p) while(pthread_getspecific(_pthreadKey)) _PTHREAD_MKSLEEP(1000000);
        return;
    }
    pthread_setspecific(_pthreadKey, ((char*)pthread_getspecific(_pthreadKey))-1 );
}

void pthread_pausable(unsigned char pausable){
    if(_pthreadStatus || (!pthread_once(&_pthreadOnce,_pthread_init) && _pthreadStatus)){
        if(pausable){
            pthread_signal_handler(pthread_signal_pause,_pthread_holder);
            pthread_signal_handler(pthread_signal_resume,_pthread_holder);
            pthread_signal_setmode(pthread_signal_pause,SIG_UNBLOCK);
            pthread_signal_setmode(pthread_signal_resume,SIG_UNBLOCK);
        }else{
            pthread_signal_setmode(pthread_signal_pause,(_pthreadStatus&2)?SIG_BLOCK:SIG_UNBLOCK);
            pthread_signal_setmode(pthread_signal_resume,(_pthreadStatus&4)?SIG_BLOCK:SIG_UNBLOCK);
            pthread_signal_handler(pthread_signal_pause,SIG_DFL);
            pthread_signal_handler(pthread_signal_resume,SIG_DFL);
        }
    }
}

void pthread_pause(pthread_t tid){
    if(_pthreadStatus || (!pthread_once(&_pthreadOnce,_pthread_init) && _pthreadStatus))
        pthread_signal_send(tid,pthread_signal_pause);
}

void pthread_resume(pthread_t tid){
    if(_pthreadStatus || (!pthread_once(&_pthreadOnce,_pthread_init) && _pthreadStatus))
        pthread_signal_send(tid,pthread_signal_resume);
}

unsigned char pthread_policy_set(pthread_t tid,int policy,int priority){
    struct sched_param s[1]; s->sched_priority=priority;
    return !pthread_setschedparam(tid,policy,s);
}

unsigned char pthread_policy_get(pthread_t tid,int *policy,int *priority){
    struct sched_param s[1];
    return !pthread_getschedparam(tid,policy,s) && ((*priority=s->sched_priority) || 1);
}

const char *pthread_policy_name(int policy){
    #define _THRPLC(_1_) case SCHED_ ## _1_: return #_1_;
    switch(policy){ _THRPLC(OTHER) _THRPLC(FIFO) _THRPLC(RR) }
    return "unknown";
    #undef _THRPLC
}

void *pthread_signal_handler(int sig,void(*handler)(int sig)){
    return signal(sig,handler);
}

const char *pthread_signal_name(int sig){
    #define _PSIG_CASE(_1_) case _1_: return #_1_
    switch(sig){
        #ifdef SIGHUP
        _PSIG_CASE(SIGHUP);
        #endif
        #ifdef SIGINT
        _PSIG_CASE(SIGINT);
        #endif
        #ifdef SIGQUIT
        _PSIG_CASE(SIGQUIT);
        #endif
        #ifdef SIGILL
        _PSIG_CASE(SIGILL);
        #endif
        #ifdef SIGTRAP
        _PSIG_CASE(SIGTRAP);
        #endif
        #ifdef SIGABRT
        _PSIG_CASE(SIGABRT);
        #endif
        #ifdef SIGEMT
        _PSIG_CASE(SIGEMT);
        #endif
        #ifdef SIGFPE
        _PSIG_CASE(SIGFPE);
        #endif
        #ifdef SIGKILL
        _PSIG_CASE(SIGKILL);
        #endif
        #ifdef SIGBUS
        _PSIG_CASE(SIGBUS);
        #endif
        #ifdef SIGSEGV
        _PSIG_CASE(SIGSEGV);
        #endif
        #ifdef SIGSYS
        _PSIG_CASE(SIGSYS);
        #endif
        #ifdef SIGPIPE
        _PSIG_CASE(SIGPIPE);
        #endif
        #ifdef SIGALRM
        _PSIG_CASE(SIGALRM);
        #endif
        #ifdef SIGTERM
        _PSIG_CASE(SIGTERM);
        #endif
        #ifdef SIGUSR1
        _PSIG_CASE(SIGUSR1);
        #endif
        #ifdef SIGUSR2
        _PSIG_CASE(SIGUSR2);
        #endif
        #ifdef SIGCHLD
        _PSIG_CASE(SIGCHLD);
        #endif
        #ifdef SIGPWR
        _PSIG_CASE(SIGPWR);
        #endif
        #ifdef SIGWINCH
        _PSIG_CASE(SIGWINCH);
        #endif
        #ifdef SIGURG
        _PSIG_CASE(SIGURG);
        #endif
        #ifdef SIGPOLL
        _PSIG_CASE(SIGPOLL);
        #endif
        #ifdef SIGSTOP
        _PSIG_CASE(SIGSTOP);
        #endif
        #ifdef SIGTSTP
        _PSIG_CASE(SIGTSTP);
        #endif
        #ifdef SIGCONT
        _PSIG_CASE(SIGCONT);
        #endif
        #ifdef SIGTTIN
        _PSIG_CASE(SIGTTIN);
        #endif
        #ifdef SIGTTOU
        _PSIG_CASE(SIGTTOU);
        #endif
        #ifdef SIGVTALRM
        _PSIG_CASE(SIGVTALRM);
        #endif
        #ifdef SIGPROF
        _PSIG_CASE(SIGPROF);
        #endif
        #ifdef SIGXCPU
        _PSIG_CASE(SIGXCPU);
        #endif
        #ifdef SIGXFSZ
        _PSIG_CASE(SIGXFSZ);
        #endif
        #ifdef SIGWAITING
        _PSIG_CASE(SIGWAITING);
        #endif
        #ifdef SIGLWP
        _PSIG_CASE(SIGLWP);
        #endif
        #ifdef SIGFREEZE
        _PSIG_CASE(SIGFREEZE);
        #endif
        #ifdef SIGTHAW
        _PSIG_CASE(SIGTHAW);
        #endif
        #ifdef SIGCANCEL
        _PSIG_CASE(SIGCANCEL);
        #endif
        #ifdef SIGLOST
        _PSIG_CASE(SIGLOST);
        #endif
        #ifdef SIGXRES
        _PSIG_CASE(SIGXRES);
        #endif
        #ifdef SIGJVM1
        _PSIG_CASE(SIGJVM1);
        #endif
        #ifdef SIGJVM2
        _PSIG_CASE(SIGJVM2);
        #endif
    }
    return "???";
    #undef PSIG_CASE
}

#undef _PTHREAD_CONT
#undef _PTHREAD_MKSLEEP

#endif /*PTHREAD_EXT_IMPL*/
