
#define _KLS_THREAD_MKSLEEP(_mk_) {const struct timespec t={_mk_/1000000,(_mk_%1000000)*1000};nanosleep(&t,NULL);}

typedef struct{
    void *next;
    void (*f)(void *args);
}_KLS_t_THREAD_TASK;

typedef struct{
    _KLS_t_THREAD_TASK *first, *last;
}_KLS_t_THREAD_QUEUE;

typedef struct _KLS_t_THREAD{
    KLS_t_THREAD_POOL pool;
    _KLS_t_THREAD_TASK *task;
    pthread_t tid;
} * const _KLS_t_THREAD;

typedef struct _KLS_t_THREAD_POOL{
    pthread_mutex_t mtx[1];
    pthread_cond_t cond[2];
    _KLS_t_THREAD_QUEUE *queue;
    unsigned int count;
    unsigned char die, wait;
    unsigned char peak, max;
    struct _KLS_t_THREAD id[1];
} * const _KLS_t_THREAD_POOL;

typedef struct{
    sem_t sem[1];
    pthread_attr_t attr[1];
    void *id;
}_KLS_t_THREAD_HELP;


static char _KLS_threadStatus=0;
static struct{
    pthread_key_t self, stop;
}_KLS_threadKey;


char _KLS_threadInit(){
    if(!_KLS_threadStatus){
        if(pthread_key_create(&_KLS_threadKey.self,NULL)) return 0;
        if(pthread_key_create(&_KLS_threadKey.stop,NULL)){
            pthread_key_delete(_KLS_threadKey.self);
            return 0;
        } _KLS_threadStatus=1;
    }
    return _KLS_threadStatus;
}
void _KLS_threadClose(){
    if(_KLS_threadStatus){
        pthread_key_delete(_KLS_threadKey.self);
        pthread_key_delete(_KLS_threadKey.stop);
    }
}


static void _KLS_threadPoolPush(_KLS_t_THREAD_POOL p,_KLS_t_THREAD_TASK * const t,unsigned char prio){
    if(prio>p->max) prio=p->max;
    if(prio>p->peak) p->peak=prio;
    {
        _KLS_t_THREAD_QUEUE * const q=p->queue+prio;
        if(q->last){
            q->last->next=t;
            q->last=t;
            return;
        }
        q->first=q->last=t;
    }
}

static _KLS_t_THREAD_TASK *_KLS_threadPoolPop(_KLS_t_THREAD_POOL p){
    _KLS_t_THREAD_QUEUE * const q=p->queue+p->peak;
    _KLS_t_THREAD_TASK * const t=q->first;
    if( t && !(q->first=t->next) ){
        q->last=NULL;
        while(p->peak && !p->queue[--p->peak].first);
    }
    return t;
}

static void _KLS_threadPoolClear(_KLS_t_THREAD_POOL p){
    void *t;
    while( (t=_KLS_threadPoolPop(p)) )
        KLS_free(t);
}




static void *_KLS_threadWorker(_KLS_t_THREAD_HELP *h){
    _KLS_t_THREAD self=h->id;
    _KLS_t_THREAD_POOL p=self->pool;
    unsigned char sleep=1;
    if(pthread_setspecific(_KLS_threadKey.self,self)){
        h->id=NULL;
        sem_post(h->sem);
        return NULL;
    }
    sem_post(h->sem);
    while('0'){

        pthread_mutex_lock(p->mtx);
_mark:
        if(p->die==1)
            break;
        if( (self->task=_KLS_threadPoolPop(p)) ){
            pthread_mutex_unlock(p->mtx);
            self->task->f(self->task+1);
            KLS_free(self->task);
            sleep>>=1;
            continue;
        }
        if(p->die)
            break;
        if(p->wait){
            unsigned int i;
            for(i=0;i<p->count;++i)
                if(p->id[i].task)
                    break;
            if(i==p->count)
                pthread_cond_signal(p->cond+1);
        }
        if(sleep){
            pthread_cond_wait(p->cond,p->mtx);
            goto _mark;
        }
        pthread_mutex_unlock(p->mtx);

        _KLS_THREAD_MKSLEEP(1000);
        sleep|=1;

    }
    pthread_mutex_unlock(p->mtx);
    return NULL;
}


static struct _KLS_t_THREAD *_KLS_threadSelf(){
    return _KLS_threadStatus ? pthread_getspecific(_KLS_threadKey.self) : NULL;
}

static void _KLS_threadHelpClose(_KLS_t_THREAD_HELP *h){
    sem_destroy(h->sem);
    pthread_attr_destroy(h->attr);
}

static char _KLS_threadHelpInit(_KLS_t_THREAD_HELP *h,size_t s){
    do{
        if(sem_init(h->sem,0,0)) return -1;
        if(pthread_attr_init(h->attr)){
            sem_destroy(h->sem);
            return -2;
        }
        if(s && pthread_attr_setstacksize(h->attr,s<<10)) break;
        if(pthread_attr_setinheritsched(h->attr,PTHREAD_INHERIT_SCHED)) break;
        #ifdef PTHREAD_FPU_ENABLED
        if(pthread_setfpustate(h->attr,PTHREAD_FPU_ENABLED)) break;
        #endif
        return 0;
    }while(0);
    _KLS_threadHelpClose(h);
    return -3;
}

static char _KLS_threadStart(_KLS_t_THREAD_POOL p,_KLS_t_THREAD_HELP *h){
    _KLS_t_THREAD id=h->id=p->id+p->count;
    id->pool=p;
    if(pthread_create(&id->tid,h->attr,(void*)_KLS_threadWorker,h))
        return -1;
    while(sem_wait(h->sem));
    if(!h->id){
        pthread_join(id->tid,NULL);
        return -2;
    }
    return 0;
}

KLS_t_THREAD_POOL KLS_threadPoolCreate(unsigned int count,unsigned char prio,size_t stackSize_kb){
    if(!_KLS_threadStatus) return NULL;
    if(!count) count=KLS_sysInfoCores();
    if(!count) return NULL;
    {
        _KLS_t_THREAD_HELP h[1];
        if(_KLS_threadHelpInit(h,stackSize_kb))
            return NULL;
        {
            const unsigned int sizeQueue=sizeof(_KLS_t_THREAD_QUEUE)*(prio+1);
            KLS_t_THREAD_POOL p=KLS_malloc(KLS_OFFSET(*p,id)+sizeof(struct _KLS_t_THREAD)*count+sizeQueue);
            while(p){
                if(pthread_mutex_init(p->mtx,NULL)){
                    KLS_freeData(p);
                    break;
                }
                if(pthread_cond_init(p->cond,NULL)){
                    pthread_mutex_destroy(p->mtx);
                    KLS_freeData(p);
                    break;
                }
                if(pthread_cond_init(p->cond+1,NULL)){
                    pthread_mutex_destroy(p->mtx);
                    pthread_cond_destroy(p->cond);
                    KLS_freeData(p);
                    break;
                }
                p->max=prio;
                p->die=p->wait=p->peak=0;
                p->queue=(void*)(p->id+count);
                memset(p->queue,0,sizeQueue);
                for(p->count=0;p->count<count;++p->count)
                    if(_KLS_threadStart(p,h)){
                        KLS_threadPoolDestroy(&p);
                        break;
                    }
                break;
            }
            _KLS_threadHelpClose(h);
            return p;
        }
    }
}

static void _KLS_threadPoolDestroy(KLS_t_THREAD_POOL *pool,KLS_byte die){
    if(pool && *pool){
        _KLS_t_THREAD_POOL p=*pool;
        unsigned int i=p->count;
        pthread_mutex_lock(p->mtx);
        p->die=die;
        pthread_cond_broadcast(p->cond);
        pthread_mutex_unlock(p->mtx);

        while(i) pthread_join(p->id[--i].tid,NULL);

        pthread_mutex_destroy(p->mtx);
        pthread_cond_destroy(p->cond);
        pthread_cond_destroy(p->cond+1);
        _KLS_threadPoolClear(p);
        KLS_freeData(*pool);
    }
}

void KLS_threadPoolDestroy(KLS_t_THREAD_POOL *pool)      { _KLS_threadPoolDestroy(pool,1); }
void KLS_threadPoolDestroyLater(KLS_t_THREAD_POOL *pool) { _KLS_threadPoolDestroy(pool,2); }

void KLS_threadPoolWait(KLS_t_THREAD_POOL pool){
    if(!pool) return;
    pthread_mutex_lock(pool->mtx);
    pool->wait=1;
    pthread_cond_broadcast(pool->cond);
    while(pthread_cond_wait(pool->cond+1,pool->mtx));
    pool->wait=0;
    pthread_mutex_unlock(pool->mtx);
}

KLS_byte KLS_threadPoolWaitTime(KLS_t_THREAD_POOL pool,unsigned int msec){
    if(pool){
        KLS_byte ret;
        struct timespec t;
        clock_gettime(CLOCK_REALTIME,&t);
        KLS_timespecAdd(&t,msec/1000,(msec%1000)*1000000);

        pthread_mutex_lock(pool->mtx);
        pool->wait=1;
        pthread_cond_broadcast(pool->cond);
        ret=!pthread_cond_timedwait(pool->cond+1,pool->mtx,&t);
        pool->wait=0;
        pthread_mutex_unlock(pool->mtx);

        return ret;
    } return -1;
}

char _KLS_threadPoolTask(void *pool,void *task, unsigned char prio){
    _KLS_t_THREAD_POOL p=pool;
    if(p && !p->die && task){
        pthread_mutex_lock(p->mtx);
        _KLS_threadPoolPush(p,task,prio);
        pthread_cond_signal(p->cond);
        pthread_mutex_unlock(p->mtx);
        return 1;
    }
    KLS_free(task); return 0;
}

void KLS_threadPoolClear(KLS_t_THREAD_POOL pool){
    if(pool){
        pthread_mutex_lock(pool->mtx);
        _KLS_threadPoolClear(pool);
        pthread_mutex_unlock(pool->mtx);
    }
}

KLS_t_THREAD_POOL KLS_threadPoolSelf(){
    const _KLS_t_THREAD t=_KLS_threadSelf();
    return t ? t->pool : NULL;
}

unsigned int KLS_threadPoolNum(){
    const _KLS_t_THREAD t=_KLS_threadSelf();
    return t ? (unsigned int)(t-t->pool->id)+1 : 0;
}

unsigned int KLS_threadPoolCount(const KLS_t_THREAD_POOL pool){
    return pool ? pool->count : 0;
}

const pthread_t *KLS_threadPoolPosix(KLS_t_THREAD_POOL pool,unsigned int num){
    return (pool && (num-1<pool->count)) ? &pool->id[num-1].tid : NULL;
}


/***********************************************************************/
/***********************************************************************/


#ifndef _KLS_THREAD_SIGNAL_PAUSE
    #define _KLS_THREAD_SIGNAL_PAUSE  SIGINT
#endif

#ifndef _KLS_THREAD_SIGNAL_CONTINUE
    #define _KLS_THREAD_SIGNAL_CONTINUE  SIGCONT
#endif

void _KLS_threadPauser(int sig){
    KLS_signalSetMode(sig,SIG_UNBLOCK);
    KLS_signalSetHandler(sig,_KLS_threadPauser);
    if(sig==_KLS_THREAD_SIGNAL_PAUSE){
        pthread_setspecific(_KLS_threadKey.stop, ((char*)pthread_getspecific(_KLS_threadKey.stop))+1 );
        while(pthread_getspecific(_KLS_threadKey.stop)) _KLS_THREAD_MKSLEEP(1000000);
        return;
    }
    pthread_setspecific(_KLS_threadKey.stop, ((char*)pthread_getspecific(_KLS_threadKey.stop))-1 );
}

void KLS_threadPausable(KLS_byte pausable){
    static char mask=4;
    if(mask&4) mask=((KLS_signalGetMode(_KLS_THREAD_SIGNAL_CONTINUE)==SIG_BLOCK)<<1) | (KLS_signalGetMode(_KLS_THREAD_SIGNAL_PAUSE)==SIG_BLOCK);
    if(pausable){
        KLS_signalSetHandler(_KLS_THREAD_SIGNAL_PAUSE,_KLS_threadPauser);
        KLS_signalSetHandler(_KLS_THREAD_SIGNAL_CONTINUE,_KLS_threadPauser);
        KLS_signalSetMode(_KLS_THREAD_SIGNAL_PAUSE,SIG_UNBLOCK);
        KLS_signalSetMode(_KLS_THREAD_SIGNAL_CONTINUE,SIG_UNBLOCK);
    }else{
        KLS_signalSetMode(_KLS_THREAD_SIGNAL_PAUSE,(mask&1)?SIG_BLOCK:SIG_UNBLOCK);
        KLS_signalSetMode(_KLS_THREAD_SIGNAL_CONTINUE,(mask&2)?SIG_BLOCK:SIG_UNBLOCK);
        KLS_signalSetHandler(_KLS_THREAD_SIGNAL_PAUSE,SIG_DFL);
        KLS_signalSetHandler(_KLS_THREAD_SIGNAL_CONTINUE,SIG_DFL);
    }
}

void KLS_threadPause(pthread_t tid){
    KLS_signalSend(tid,_KLS_THREAD_SIGNAL_PAUSE);
}

void KLS_threadResume(pthread_t tid){
    KLS_signalSend(tid,_KLS_THREAD_SIGNAL_CONTINUE);
}

KLS_byte KLS_threadPolicySet(pthread_t tid,int policy,int priority){
    struct sched_param s[1]={{.sched_priority=priority}};
    return !pthread_setschedparam(tid,policy,s);
}

KLS_byte KLS_threadPolicyGet(pthread_t tid,int *policy,int *priority){
    struct sched_param s[1];
    return !pthread_getschedparam(tid,policy,s) && ((*priority=s->sched_priority) || 1);
}

const char *KLS_threadPolicyName(int policy){
    #define _THRPLC(_1_) case KLS_THREAD_POLICY_ ## _1_: return #_1_;
    switch(policy){ _THRPLC(OTHER) _THRPLC(FIFO) _THRPLC(RR) }
    return "unknown";
    #undef _THRPLC
}

