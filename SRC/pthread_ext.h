/* * * * * * * * * * * * * * * * * */
/* MIT License                     */
/* Copyright (c) 2024 ANSI-Christ  */
/* * * * * * * * * * * * * * * * * */

#ifndef PTHREAD_EXT_H
#define PTHREAD_EXT_H

#include <pthread.h>

unsigned int pthread_cores(void);




typedef struct{void *_[5];}pthread_poolattr_t;

int pthread_poolattr_init(pthread_poolattr_t *attr);

int pthread_poolattr_setalign(pthread_poolattr_t *attr,unsigned int align);
int pthread_poolattr_setcattr(pthread_poolattr_t *attr,pthread_condattr_t *cattr);
int pthread_poolattr_setmattr(pthread_poolattr_t *attr,pthread_mutexattr_t *mattr);
int pthread_poolattr_setpattr(pthread_poolattr_t *attr,pthread_attr_t *pattr,unsigned int count); /* pattr is shared by all threads of pool if count < 2 */

int pthread_poolattr_getalign(const pthread_poolattr_t *attr,unsigned int *align);
int pthread_poolattr_getcattr(const pthread_poolattr_t *attr,pthread_condattr_t **cattr);
int pthread_poolattr_getmattr(const pthread_poolattr_t *attr,pthread_mutexattr_t **mattr);
int pthread_poolattr_getpattr(const pthread_poolattr_t *attr,pthread_attr_t **pattr,unsigned int *count);

void pthread_poolattr_destroy(pthread_poolattr_t *attr);




typedef struct _pthread_pool_t pthread_pool_t;

int pthread_pool_create(pthread_pool_t **pool,const pthread_poolattr_t *attr,unsigned int count,unsigned char max_prio);

int pthread_pool_detach(pthread_pool_t *pool,int forced);
int pthread_pool_timedwait(pthread_pool_t *pool,const struct timespec *abstime);

typedef struct{ void *_padding; void(*task)(pthread_pool_t *pool,void *composite_task,unsigned int index); } pthread_pool_task_t;
/* composite task structure must include pthread_pool_task_t as first field. */
int pthread_pool_task(pthread_pool_t *pool,void *composite_task,unsigned char prio);
int pthread_pool_soft(pthread_pool_t *pool,void *composite_task,unsigned char prio);
void pthread_pool_urgent(pthread_pool_t *pool,void *composite_task);

void pthread_pool_wait(pthread_pool_t *pool);
void pthread_pool_clear(pthread_pool_t *pool);
void pthread_pool_reject(pthread_pool_t *pool);
void pthread_pool_destroy(pthread_pool_t *pool,unsigned char now);

unsigned int pthread_pool_count(const pthread_pool_t *pool);
unsigned int pthread_pool_pending(const pthread_pool_t *pool);

unsigned char pthread_pool_batch(pthread_pool_t *pool,unsigned char count);

const pthread_t *pthread_pool_threads(const pthread_pool_t *pool);




typedef struct{void *_[2];}pthread_groupattr_t;

int pthread_groupattr_init(pthread_groupattr_t *attr);

int pthread_groupattr_setcattr(pthread_groupattr_t *attr,pthread_condattr_t *cattr);
int pthread_groupattr_setmattr(pthread_groupattr_t *attr,pthread_mutexattr_t *mattr);

int pthread_groupattr_getcattr(const pthread_groupattr_t *attr,pthread_condattr_t **cattr);
int pthread_groupattr_getmattr(const pthread_groupattr_t *attr,pthread_mutexattr_t **mattr);

void pthread_groupattr_destroy(pthread_groupattr_t *attr);




typedef struct{pthread_mutex_t _1;pthread_cond_t _2;int _3[4];char _4[2];}pthread_group_t;

int pthread_group_init(pthread_group_t *group,const pthread_groupattr_t *attr,unsigned int target);

int pthread_group_destroy(pthread_group_t *group);
int pthread_group_rejected(pthread_group_t *group);
int pthread_group_reject(pthread_group_t *group,unsigned char by_task);
int pthread_group_progress(pthread_group_t *group,unsigned int add_targets);
int pthread_group_wait(pthread_group_t *group,unsigned int *done,unsigned int *target);
int pthread_group_timedwait(pthread_group_t *group,unsigned int *done,unsigned int *target,struct timespec *abstime);
int pthread_group_reject_ex(pthread_group_t *group,unsigned char by_task,void(*payload)(void *arg),void *arg);
int pthread_group_progress_ex(pthread_group_t *group,unsigned int add_targets,void(*payload)(void *arg),void *arg);




typedef struct{union{void *p;int i;}r[1],w[1];} pthread_channel_t;

int pthread_channel_open(pthread_channel_t *channel);
int pthread_channel_pop(pthread_channel_t *channel,void *data,int size);
int pthread_channel_push(pthread_channel_t *channel,const void *data,int size);

void pthread_channel_close(pthread_channel_t *channel);




extern int pthread_kill(pthread_t,int);
extern int pthread_detach(pthread_t);

#endif /* PTHREAD_EXT_H */





#ifdef PTHREAD_EXT_IMPL

#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifdef offsetof
    #define _PTHREAD_OFFSETOF offsetof
#else
    #define _PTHREAD_OFFSETOF(_1_,_2_) ((size_t)&((_1_*)0)->_2_)
#endif

#ifdef _WIN32

#define NOMINMAX
#include <windows.h>
#undef NOMINMAX

int pthread_channel_open(pthread_channel_t * const c){
    return CreatePipe(&c->r->p,&c->w->p,NULL,0)==0 ? -1 : 0;
}

void pthread_channel_close(pthread_channel_t * const c){
    if(c->r->p){
        CloseHandle(c->w->p);
        CloseHandle(c->r->p);
        c->r->p=NULL;
    }
}

int pthread_channel_push(pthread_channel_t * const c,const void *data,const int size){
    DWORD count;
    if(c->r->p && data && size>0 && WriteFile(c->w->p,data,size,&count,NULL)) return size;
    return -1;
}

int pthread_channel_pop(pthread_channel_t * const c,void *data,int size){
    if(c->r->p && data){
        char *p=(char*)data;
        while(size>0){
            DWORD bytes;
            ReadFile(c->r->p,p,size,&bytes,NULL);
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
        const size_t * const end=(void*)(c+1), f=(size_t)GetThreadContext;
        size_t min=0, *p=(void*)c;
        GetThreadContext(GetCurrentThread(),c);
        for(c->ContextFlags=0,min=~min;p!=end;++p){
            const size_t diff=((*p>f) ? (*p-f) : (f-*p));
            if(diff<min){
                min=diff; offset=(char*)p-(char*)c;
            }
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

int pthread_channel_open(pthread_channel_t * const c){
    int fd[2];
    if(pipe(fd)){
        c->r->i=c->w->i=-1;
        return -1;
    }
    c->r->i=fd[0];
    c->w->i=fd[1];
    return 0;
}

void pthread_channel_close(pthread_channel_t * const c){
    if(c->r->i!=-1){
        close(c->w->i);
        close(c->r->i);
        c->r->i=-1;
    }
}

int pthread_channel_push(pthread_channel_t * const c,const void *data,int size){
    if(c->r->i!=-1 && data && size>0) return write(c->w->i,data,size);
    return -1;
}

int pthread_channel_pop(pthread_channel_t * const c,void *data,int size){
    if(c->r->i!=-1 && data){
        char *p=(char*)data;
        while(size>0){
            const int bytes=read(c->r->i,p,size);
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
    attr->_[0]=NULL; attr->_[1]=NULL; attr->_[2]=NULL; attr->_[3]=NULL;
    {const union{void *_; unsigned int *i;} p={(void*)&attr->_[4]}; *p.i=256;}
    return 0;
}

void pthread_poolattr_destroy(pthread_poolattr_t * const attr){
    if(!attr) return;
    if(attr->_[0]) pthread_mutexattr_destroy((pthread_mutexattr_t*)attr->_[0]);
    if(attr->_[1]) pthread_condattr_destroy((pthread_condattr_t*)attr->_[1]);
    if(attr->_[2]){
        const union{void *_; unsigned int *i;} p={(void*)&attr->_[3]};
        unsigned int i=*p.i;
        while(i--) pthread_attr_destroy(((pthread_attr_t*)attr->_[2])+i);
    }
}

int pthread_poolattr_setmattr(pthread_poolattr_t * const attr,pthread_mutexattr_t * const mattr){
    if(!attr) return EINVAL;
    attr->_[0]=mattr; return 0;
}

int pthread_poolattr_setcattr(pthread_poolattr_t * const attr,pthread_condattr_t * const cattr){
    if(!attr) return EINVAL;
    attr->_[1]=cattr; return 0;
}

int pthread_poolattr_setpattr(pthread_poolattr_t * const attr,pthread_attr_t * const pattr,const unsigned int count){
    if(!attr) return EINVAL;
    attr->_[2]=pattr;
    {const union{void *_; unsigned int *i;} p={(void*)&attr->_[3]}; *p.i=(pattr?count:0);}
    return 0;
}

int pthread_poolattr_setalign(pthread_poolattr_t * const attr,const unsigned int align){
    if(!attr || !align || (align & (align-1)) ) return EINVAL;
    {const union{void *_; unsigned int *i;} p={(void*)&attr->_[4]}; *p.i=align;}
    return 0;
}

int pthread_poolattr_getmattr(const pthread_poolattr_t * const attr,pthread_mutexattr_t ** const mattr){
    if(!attr) return EINVAL;
    *mattr=(pthread_mutexattr_t*)attr->_[0]; return 0;
}

int pthread_poolattr_getcattr(const pthread_poolattr_t * const attr,pthread_condattr_t ** const cattr){
    if(!attr) return EINVAL;
    *cattr=(pthread_condattr_t*)attr->_[1]; return 0;
}

int pthread_poolattr_getpattr(const pthread_poolattr_t * const attr,pthread_attr_t ** const pattr,unsigned int * const count){
    if(!attr) return EINVAL;
    *pattr=(pthread_attr_t*)attr->_[2];
    {const union{void *_; unsigned int *i;} p={(void*)&attr->_[3]}; *count=*p.i;}
    return 0;
}

int pthread_poolattr_getalign(const pthread_poolattr_t * const attr,unsigned int * const align){
    if(!attr) return EINVAL;
    {const union{void *_; unsigned int *i;} p={(void*)&attr->_[4]}; *align=*p.i;}
    return 0;
}


typedef struct __pthread_pool_task_t{
    struct __pthread_pool_task_t *next;
    void(*f)(pthread_pool_t *pool,void *args,unsigned int index);
}_pthread_pool_task_t;

typedef struct{
    _pthread_pool_task_t *first, *last;
}_pthread_pool_queue_t;

struct _pthread_pool_t{
    pthread_mutex_t mtx[1];
    pthread_cond_t cond[2];
    _pthread_pool_task_t *reject;
    unsigned int count, busy, size;
    unsigned char ctrl, peak, max, batch;
    _pthread_pool_queue_t queue[1];
};

typedef struct{
    pthread_pool_t * const pool;
    pthread_attr_t * const attr;
    pthread_t * const tid;
    const unsigned int inc, count;
    int err;
}_pthread_pool_initializer_t;

typedef struct{char _; pthread_t t;}_pthread_pool_align_t;

#define _pthread_pool_pad(_N_) ((_PTHREAD_OFFSETOF(_pthread_pool_align_t,t) - ((_PTHREAD_OFFSETOF(struct _pthread_pool_t,queue)+sizeof(_pthread_pool_queue_t)*(_N_)) % _PTHREAD_OFFSETOF(_pthread_pool_align_t,t))) % _PTHREAD_OFFSETOF(_pthread_pool_align_t,t) )
#define _pthread_pool_tids(_p_) ((pthread_t*)(((char*)(_p_->queue+1+(unsigned int)_p_->max))+_pthread_pool_pad(1+(unsigned int)_p_->max)))

static void _pthread_pool_append(pthread_pool_t * const p,_pthread_pool_task_t * const t,const unsigned char prio){
    _pthread_pool_queue_t * const q=p->queue+prio;
    if(q->last) q->last->next=t;
    else q->first=t;
    q->last=t;
}

static void _pthread_pool_prepend(pthread_pool_t * const p,_pthread_pool_task_t * const t,const unsigned char prio){
    _pthread_pool_queue_t * const q=p->queue+prio;
    if( !(t->next=q->first) ) q->last=t;
    q->first=t;
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
            p->queue[(p->peak=p->max)]=*q;
            q->first=NULL; q->last=NULL;
        }
    }
}

static void _pthread_pool_release(pthread_pool_t * const p){
    pthread_mutex_destroy(p->mtx);
    pthread_cond_destroy(p->cond);
    pthread_cond_destroy(p->cond+1);
    free( (p->ctrl & 16) ? ((void**)p)[-1] : p );
}

static void *_pthread_pool_worker(_pthread_pool_initializer_t * const cfg){
    pthread_pool_t * const p=cfg->pool;
    const unsigned int index=p->count++;
    int busy=(p->count!=cfg->count ? pthread_create(cfg->tid+p->count,cfg->attr+p->count*cfg->inc,(void*(*)(void*))_pthread_pool_worker,cfg) : 1);

    pthread_mutex_lock(p->mtx);
    if(busy){cfg->err=busy; pthread_cond_signal(p->cond+1);}
    for(busy=0;;){
        _pthread_pool_task_t *t=_pthread_pool_pop(p);
        if(t){
            pthread_pool_t *_p;
            _pthread_pool_task_t *i=t;
            unsigned int c=(--p->size)/p->count;
            if(!busy) {busy=1; ++p->busy;}
            if(c>p->batch){c=p->batch;}
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
            pthread_mutex_unlock(p->mtx);
            i->next=NULL;
            do{
                i=t->next;
                t->f(_p,t,index);
            }while( (t=i) );
            pthread_mutex_lock(p->mtx);
        }else{
            if(busy){busy=0; if(!--p->busy) pthread_cond_broadcast(p->cond+1);}
            if( (p->ctrl & 1) && !p->busy) break;
            pthread_cond_wait(p->cond,p->mtx);
        }
    }
    busy=p->ctrl & 12;
    busy|=(busy & 4) && !--p->count;
    if(p->ctrl & 32){p->ctrl&=~32; pthread_cond_broadcast(p->cond);}
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

int pthread_pool_create(pthread_pool_t ** const pool,const pthread_poolattr_t * const attr,unsigned int count,const unsigned char prio){
    pthread_attr_t *pattr=NULL;
    pthread_condattr_t *cattr=NULL;
    pthread_mutexattr_t *mattr=NULL;
    int err, detached=PTHREAD_CREATE_JOINABLE;
    unsigned int pattrs=0, align=256;

    pthread_poolattr_getpattr(attr,&pattr,&pattrs);
    if(pattr && (err=pthread_attr_getdetachstate(pattr,&detached))){
        *pool=(pthread_pool_t*)1; return err;
    }

    pthread_poolattr_getalign(attr,&align);
    pthread_poolattr_getmattr(attr,&mattr);
    pthread_poolattr_getcattr(attr,&cattr);

    if( (count || (count=pthread_cores())) && (pattrs<2 || pattrs>=count) ){
        const size_t qsize=sizeof(_pthread_pool_queue_t)*(1+(unsigned int)prio), tsize=_pthread_pool_pad(1+(unsigned int)prio) + sizeof(pthread_t)*count;
        const size_t palign=align-1, asize=(_PTHREAD_OFFSETOF(struct _pthread_pool_t,queue) + qsize + tsize + palign) & ~palign;
        void * const _p=malloc(asize + palign);
        if(_p){
            pthread_pool_t * const p=(pthread_pool_t*)((((size_t)_p)+palign) & ~palign);
            if( (err=pthread_mutex_init(p->mtx,mattr)) ){
                free(_p); *pool=(pthread_pool_t*)2; return err;
            }
            if( (err=pthread_cond_init(p->cond,cattr)) ){
                pthread_mutex_destroy(p->mtx);
                free(_p); *pool=(pthread_pool_t*)3; return err;
            }
            if( (err=pthread_cond_init(p->cond+1,cattr)) ){
                pthread_mutex_destroy(p->mtx);
                pthread_cond_destroy(p->cond);
                free(_p); *pool=(pthread_pool_t*)3; return err;
            }
            p->reject=NULL;
            p->count=p->busy=p->size=0;
            p->ctrl=p->peak=0;
            p->max=prio;
            p->batch=0;
            if(detached==PTHREAD_CREATE_DETACHED) p->ctrl|=4;
            if(((size_t)_p) & palign){p->ctrl|=16; ((void**)p)[-1]=_p;}
            memset(p->queue,0,qsize);

            {_pthread_pool_initializer_t cfg[1]={{p,pattr,_pthread_pool_tids(p),pattrs>1,count,0}};
            if( !(err=pthread_create(cfg->tid,cfg->attr,(void*(*)(void*))_pthread_pool_worker,cfg)) ){
                pthread_mutex_lock(p->mtx);
                while(!cfg->err) pthread_cond_wait(p->cond+1,p->mtx);
                pthread_mutex_unlock(p->mtx);
                if(p->count==count){*pool=p; return 0;}
                err=cfg->err;
            }
            pthread_pool_destroy(p,1);}
            *pool=(pthread_pool_t*)4; return err;
        } *pool=(pthread_pool_t*)5; return ENOMEM;
    } *pool=(pthread_pool_t*)6; return EINVAL;
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
        unsigned int i=1|32|((now!=0)<<1);
        pthread_mutex_lock(p->mtx);
        i=(p->ctrl|=i);
        pthread_cond_signal(p->cond);
        pthread_mutex_unlock(p->mtx);
        if(i & 12) return;
        pthread_join(_pthread_pool_tids(p)[0],NULL);
        _pthread_pool_release(p);
    }
}

void pthread_pool_reject(pthread_pool_t * const p){
    pthread_mutex_lock(p->mtx);
    _pthread_pool_reject(p);
    pthread_mutex_unlock(p->mtx);
}

void pthread_pool_wait(pthread_pool_t * const p){
    pthread_mutex_lock(p->mtx);
    while(p->busy|p->size) pthread_cond_wait(p->cond+1,p->mtx);
    pthread_mutex_unlock(p->mtx);
}

void pthread_pool_clear(pthread_pool_t * const p){
    pthread_mutex_lock(p->mtx);
    if(p->busy|p->size){
        p->ctrl|=2;
        do{ pthread_cond_wait(p->cond+1,p->mtx); } while(p->busy|p->size);
        p->ctrl&=~2;
    }
    pthread_mutex_unlock(p->mtx);
}

int pthread_pool_timedwait(pthread_pool_t * const p,const struct timespec *abstime){
    #define _CASE_ERR case EINVAL: err=EINVAL; goto _mark; case ETIMEDOUT: err=ETIMEDOUT; goto _mark;
    int err=0;
    pthread_mutex_lock(p->mtx);
    while(p->busy|p->size)
        switch(pthread_cond_timedwait(p->cond+1,p->mtx,abstime)){
            case -1: switch(errno){_CASE_ERR} break;
            _CASE_ERR
        }
_mark:
    pthread_mutex_unlock(p->mtx);
    return err;
    #undef _CASE_ERR
}

int pthread_pool_task(pthread_pool_t * const p,void * const t,unsigned char prio){
    int err;
    ((_pthread_pool_task_t*)t)->next=NULL;
    if(prio>p->max) prio=p->max;
    pthread_mutex_lock(p->mtx);
    if( !(err=p->ctrl & 3) ){
        if(prio>p->peak) p->peak=prio;
        _pthread_pool_append(p,(_pthread_pool_task_t*)t,prio);
        if(++p->size>=p->busy && p->count!=p->busy) pthread_cond_signal(p->cond);
    }
    pthread_mutex_unlock(p->mtx);
    if(err & 1) return EINVAL;
    if(err & 2) return EAGAIN;
    return 0;
}

int pthread_pool_soft(pthread_pool_t * const p,void * const t,unsigned char prio){
    int err;
    ((_pthread_pool_task_t*)t)->next=NULL;
    if(prio>p->max) prio=p->max;
    pthread_mutex_lock(p->mtx);
    if( !(err=p->ctrl & 3) ){
        if(prio>p->peak) p->peak=prio;
        _pthread_pool_append(p,(_pthread_pool_task_t*)t,prio);
        if(++p->size>p->busy && p->count!=p->busy) pthread_cond_signal(p->cond);
    }
    pthread_mutex_unlock(p->mtx);
    if(err & 1) return EINVAL;
    if(err & 2) return EAGAIN;
    return 0;
}

void pthread_pool_urgent(pthread_pool_t * const p,void * const t){
    ((_pthread_pool_task_t*)t)->next=NULL;
    pthread_mutex_lock(p->mtx);
    _pthread_pool_prepend(p,(_pthread_pool_task_t*)t,(p->peak=p->max));
    if(++p->size>=p->busy && p->count!=p->busy) pthread_cond_signal(p->cond);
    pthread_mutex_unlock(p->mtx);
}

unsigned char pthread_pool_batch(pthread_pool_t * const p,unsigned char count){
    const unsigned char sav=p->batch;
    if(count){
        --count;
        pthread_mutex_lock(p->mtx);
        p->batch=count;
        pthread_mutex_unlock(p->mtx);
    } return sav+1;
}

unsigned int pthread_pool_count(const pthread_pool_t * const p){
    return p->count;
}

unsigned int pthread_pool_pending(const pthread_pool_t * const p){
    return p->size;
}

const pthread_t *pthread_pool_threads(const pthread_pool_t * const p){
    return _pthread_pool_tids(p);
}

#undef _pthread_pool_tids
#undef _pthread_pool_pad
#undef _PTHREAD_OFFSETOF

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


typedef struct{
    pthread_mutex_t mtx[1];
    pthread_cond_t cond[1];
    unsigned int target, done, reject;
    int state; char destroy, wait;
}_pthread_group_t;

static void _pthread_group_reset(_pthread_group_t * const g,const unsigned int target){
    g->target=target; g->done=g->reject=0; g->state=0; g->wait=1;
}

int pthread_group_init(pthread_group_t * const _g,const pthread_groupattr_t * const attr,const unsigned int target){
    if(_g){
        _pthread_group_t * const g=(_pthread_group_t*)_g;
        pthread_condattr_t *cattr=NULL;
        pthread_mutexattr_t *mattr=NULL;
        int err;
        pthread_groupattr_getmattr(attr,&mattr);
        pthread_groupattr_getcattr(attr,&cattr);
        if( (err=pthread_mutex_init(g->mtx,mattr)) )
            return err;
        if( (err=pthread_cond_init(g->cond,cattr)) ){
            pthread_mutex_destroy(g->mtx);
            return err;
        }
        _pthread_group_reset(g,target);
        g->destroy=0;
        return 0;
    } return EINVAL;
}

static void _pthread_group_destroy(_pthread_group_t * const g){
    pthread_mutex_destroy(g->mtx);
    pthread_cond_destroy(g->cond);
}

int pthread_group_destroy(pthread_group_t * const _g){
    _pthread_group_t * const g=(_pthread_group_t*)_g;
    int err=0;
    pthread_mutex_lock(g->mtx);
    g->state=EINTR;
    if(g->done+g->reject==g->target) err=ENOENT;
    else g->destroy=1;
    pthread_mutex_unlock(g->mtx);
    if(err) _pthread_group_destroy(g);
    return err;
}

int pthread_group_wait(pthread_group_t * const _g,unsigned int * const done,unsigned int * const target){
    _pthread_group_t * const g=(_pthread_group_t*)_g;
    int err; unsigned int v[2];
    pthread_mutex_lock(g->mtx);
    if(g->target) while(g->wait) pthread_cond_wait(g->cond,g->mtx);
    v[0]=g->done; v[1]=g->target; err=g->state;
    _pthread_group_reset(g,0);
    pthread_mutex_unlock(g->mtx);
    if(done) *done=v[0];
    if(target) *target=v[1];
    return err;
}

int pthread_group_timedwait(pthread_group_t * const _g,unsigned int * const done,unsigned int * const target,struct timespec * const abstime){
    _pthread_group_t * const g=(_pthread_group_t*)_g;
    int err=0; unsigned int v[2];
    pthread_mutex_lock(g->mtx);
    if(g->target) while(g->wait)
        #define _CASE_ERR case EINVAL: err=EINVAL; goto _mark; case ETIMEDOUT: err=ETIMEDOUT; goto _mark;
        switch(pthread_cond_timedwait(g->cond,g->mtx,abstime)){
            case -1: switch(errno){ _CASE_ERR }
            _CASE_ERR
        }
        #undef _CASE_ERR
_mark:
    v[0]=g->done; v[1]=g->target;
    if(!err){
        err=g->state;
       _pthread_group_reset(g,0);
    }
    pthread_mutex_unlock(g->mtx);
    if(done) *done=v[0];
    if(target) *target=v[1];
    return err;
}

int pthread_group_progress_ex(pthread_group_t * const _g,const unsigned int add_targets,void(* const f)(void *arg),void * const arg){
    _pthread_group_t * const g=(_pthread_group_t*)_g;
    int err=0;
    pthread_mutex_lock(g->mtx);
    if(f) f(arg);
    g->done+=!!g->target;
    g->target+=add_targets;
    if(g->done+g->reject==g->target){
        if(g->destroy) err=ENOENT;
        else{
            g->wait=0;
            pthread_cond_broadcast(g->cond);
        }
    }
    pthread_mutex_unlock(g->mtx);
    if(err) _pthread_group_destroy(g);
    return err;
}

int pthread_group_progress(pthread_group_t * const g,const unsigned int add_targets){
    return pthread_group_progress_ex(g,add_targets,NULL,NULL);
}

int pthread_group_reject_ex(pthread_group_t * const _g,const unsigned char by_task,void(* const f)(void *arg),void * const arg){
    _pthread_group_t * const g=(_pthread_group_t*)_g;
    int err=0;
    pthread_mutex_lock(g->mtx);
    if(f) f(arg);
    g->state=EINTR;
    if(by_task){
        if(++g->reject+g->done==g->target){
            if(g->destroy) err=ENOENT;
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
    if(err) _pthread_group_destroy(g);
    return err;
}

int pthread_group_reject(pthread_group_t * const g,const unsigned char by_task){
    return pthread_group_reject_ex(g,by_task,NULL,NULL);
}

int pthread_group_rejected(pthread_group_t * const _g){
    _pthread_group_t * const g=(_pthread_group_t*)_g;
    if(g->state){
        const int e=pthread_group_reject(_g,1);
        return e ? e : EINTR;
    } return 0;
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
