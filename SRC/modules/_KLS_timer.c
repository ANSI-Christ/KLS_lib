
#define KLS_TIMER_CLOCKID  CLOCK_REALTIME
#define _KLS_timerLess(_1_,_2_) (_1_.tv_sec<_2_.tv_sec || (_1_.tv_sec==_2_.tv_sec && _1_.tv_nsec<_2_.tv_nsec))

struct _KLS_t_TIMER{
    void(*f)(void *arg,unsigned int *interval);
    void *arg;
    struct timespec t;
    unsigned int msInterval;
    char run, _size;
};

static struct{
    pthread_mutex_t mtx[1];
    pthread_cond_t cond[1];
    KLS_t_LIST list[1];
    struct timespec t;
}_KLS_timerGlob={{PTHREAD_MUTEX_INITIALIZER}};


static void _KLS_timerExit(void){
    KLS_TYPEOF(_KLS_timerGlob) * const g=&_KLS_timerGlob;
    if(!g->list->sizeElement) return;
    pthread_mutex_lock(g->mtx);
    KLS_listClear(g->list);
    g->t.tv_sec=g->t.tv_nsec=0;
    pthread_cond_signal(g->cond);
    pthread_mutex_unlock(g->mtx);
}
#define PRAGMA_ATEXIT _KLS_timerExit
#include "pragma.h"

static void _KLS_timerPolicy(void){
    pthread_t tid=pthread_self();
    int pol=0, pri=0;
    if(KLS_threadPolicyGet(tid,&pol,&pri) && pol!=0)
        KLS_threadPolicySet(tid,pol,99);
}

static void *_KLS_timerThread(void *arg){
    KLS_TYPEOF(_KLS_timerGlob) * const g=&_KLS_timerGlob;
    struct timespec t, tmp;
    KLS_t_TIMER timer;
    _KLS_timerPolicy();
    while('0'){
        pthread_mutex_lock(g->mtx);
_mark:
        t=g->t;
        switch(pthread_cond_timedwait(g->cond,g->mtx,&t)){
            case -1: if(errno==EINTR) goto _mark; break;
            case 0: if(g->list->first) goto _mark; break;
            case EINTR: goto _mark;
        }
        if(!(timer=g->list->first)){
            g->list->sizeElement=0;
            pthread_cond_destroy(g->cond);
            pthread_mutex_unlock(g->mtx);
            break;
        }
        clock_gettime(KLS_TIMER_CLOCKID,&t);
        KLS_timespecNorm(&t);
        g->t.tv_sec=t.tv_sec+3600;
        while(timer && timer->run){
            tmp=timer->t;
            KLS_timespecSub(&tmp,t.tv_sec,t.tv_nsec);
            if( !(tmp.tv_sec|tmp.tv_nsec) ){
                timer->f(timer->arg,&timer->msInterval);
                if(!timer->msInterval){
                    KLS_t_TIMER next=KLS_listNext(timer);
                    timer->run=0; KLS_listMoveAfter(g->list,timer,g->list->last);
                    timer=next; continue;
                }
                timer->t=t;
                KLS_timespecAdd(&timer->t,timer->msInterval/1000,(timer->msInterval%1000)*1000000);
            }
            if(_KLS_timerLess(timer->t,g->t))
                g->t=timer->t;
            timer=KLS_listNext(timer);
        }
        pthread_mutex_unlock(g->mtx);
    }
    return NULL;
    (void)arg;
}

static char _KLS_timerInit(void){
    KLS_TYPEOF(_KLS_timerGlob) * const g=&_KLS_timerGlob;
    if(!g->list->sizeElement){
        pthread_attr_t a[1];
        pthread_t tid[1];
        do{
            if(pthread_attr_init(a)) break;
            if(pthread_cond_init(g->cond,NULL)) break;
            if(pthread_attr_setstacksize(a,40<<10)) break;
            if(pthread_attr_setdetachstate(a,PTHREAD_CREATE_DETACHED)) break;
            if(pthread_attr_setinheritsched(a,PTHREAD_INHERIT_SCHED)) break;
            #ifdef PTHREAD_FPU_ENABLED
            if(pthread_setfpustate(a,PTHREAD_FPU_ENABLED)) break;
            #endif
            g->t.tv_sec=time(NULL)+3600;
            if(!pthread_create(tid,a,_KLS_timerThread,NULL))
                *g->list=KLS_listNew(KLS_OFFSET(struct _KLS_t_TIMER,_size),NULL);
        }while(0);
        pthread_attr_destroy(a);
        if(!g->list->sizeElement) pthread_cond_destroy(g->cond);
    } return g->list->sizeElement>0;
}

KLS_t_TIMER KLS_timerCreate(void(*callback)(void *arg,unsigned int *msInterval),void *arg){
    KLS_TYPEOF(_KLS_timerGlob) * const g=&_KLS_timerGlob;
    KLS_t_TIMER t=NULL;
    pthread_mutex_lock(g->mtx);
    if(_KLS_timerInit() && (t=KLS_listPushBack(g->list,NULL)) ){
        t->arg=arg;
        t->f=(void*)callback;
    }
    pthread_mutex_unlock(g->mtx);
    return t;
}

KLS_byte KLS_timerStart(KLS_t_TIMER timer,unsigned int msDelay,unsigned int msInterval,void(*callback)(void *arg,unsigned int *msInterval),void *arg){
    if(timer && (msDelay|msInterval) && (callback || timer->f)){
        KLS_TYPEOF(_KLS_timerGlob) * const g=&_KLS_timerGlob;
        pthread_mutex_lock(g->mtx);
        if(arg) timer->arg=arg;
        if(callback) timer->f=(void*)callback;
        if(!timer->run){
            timer->run=1;
            KLS_listMoveBefore(g->list,timer,g->list->first);
        }
        timer->msInterval=msInterval;
        clock_gettime(KLS_TIMER_CLOCKID,&timer->t);
        KLS_timespecAdd(&timer->t,msDelay/1000,(msDelay%1000)*1000000);
        if(_KLS_timerLess(timer->t,g->t)){
            g->t=timer->t;
            pthread_cond_signal(g->cond);
        }
        pthread_mutex_unlock(g->mtx);
        return 1;
    } return 0;
}

void KLS_timerStop(KLS_t_TIMER timer){
    if(timer && timer->run){
        KLS_TYPEOF(_KLS_timerGlob) * const g=&_KLS_timerGlob;
        pthread_mutex_lock(g->mtx);
        timer->run=0;
        KLS_listMoveAfter(g->list,timer,g->list->last);
        pthread_mutex_unlock(g->mtx);
    }
}

KLS_byte KLS_timerContinue(KLS_t_TIMER timer){
    if(timer && !timer->run && timer->f){
        KLS_TYPEOF(_KLS_timerGlob) * const g=&_KLS_timerGlob;
        pthread_mutex_lock(g->mtx);
        timer->run=1;
        KLS_listMoveBefore(g->list,timer,g->list->first);
        if(_KLS_timerLess(timer->t,g->t)){
            g->t=timer->t;
            pthread_cond_signal(g->cond);
        }
        pthread_mutex_unlock(g->mtx);
        return 1;
    } return 0;
}

void KLS_timerDestroy(KLS_t_TIMER *timer){
    if(timer && *timer){
        KLS_TYPEOF(_KLS_timerGlob) * const g=&_KLS_timerGlob;
        pthread_mutex_lock(g->mtx);
        KLS_listRemove(g->list,*timer);
        if(!g->list->first) pthread_cond_signal(g->cond);
        pthread_mutex_unlock(g->mtx);
        *timer=NULL;
    }
}

#undef _KLS_timerLess
#undef KLS_TIMER_CLOCKID
