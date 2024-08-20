
#define _KLS_THREAD_MKSLEEP(_mk_) {const struct timespec t={_mk_/1000000,(_mk_%1000000)*1000};nanosleep(&t,NULL);}

typedef struct{
    void *next;
    void (*f)(void *args);
}_KLS_t_THREAD_TASK;

typedef struct{
    _KLS_t_THREAD_TASK *first, *last;
}_KLS_t_THREAD_QUEUE;

typedef struct _KLS_t_THREAD{
    sem_t request[1], answer[1];
    KLS_t_THREAD_POOL pool;
    _KLS_t_THREAD_TASK *task;
    pthread_t tid;
} * const _KLS_t_THREAD;

typedef struct _KLS_t_THREAD_POOL{
    pthread_mutex_t mtx[1];
    _KLS_t_THREAD_QUEUE *queue;
    unsigned int count;
    unsigned char die, wait;
    unsigned char peak, max;
    struct _KLS_t_THREAD id[1];
} * const _KLS_t_THREAD_POOL;



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




static void *_KLS_threadWorker(_KLS_t_THREAD self){
    _KLS_t_THREAD_POOL p=self->pool;
    char deepSleep=1;
    if(pthread_setspecific(_KLS_threadKey.self,self)){
        p->die=1; sem_post(self->answer);
        return NULL;
    }
    sem_post(self->answer);

    while(p->die!=1){

        pthread_mutex_lock(p->mtx);
        self->task=_KLS_threadPoolPop(p);
        pthread_mutex_unlock(p->mtx);

        if(self->task){
            self->task->f(self->task+1);
            KLS_free(self->task);
            deepSleep=0;
            continue;
        }

        if(p->wait) {sem_trywait(self->answer);sem_post(self->answer);}
        if(p->die) return NULL;

        if(deepSleep){
            sem_wait(self->request);
            deepSleep=0; continue;
        }

        _KLS_THREAD_MKSLEEP(1000);
        deepSleep=1;
    }
    return NULL;
}


static struct _KLS_t_THREAD *_KLS_threadSelf(){
    return _KLS_threadStatus ? pthread_getspecific(_KLS_threadKey.self) : NULL;
}

void _KLS_threadPoolWake(_KLS_t_THREAD id,unsigned int i){
    while(i){
        sem_trywait(id[--i].request);
        sem_post(id[i].request);
    }
}

static char _KLS_threadStart(_KLS_t_THREAD_POOL p,void *attr){
    _KLS_t_THREAD id=p->id+p->count;
    if(sem_init(id->request,0,0)){
        return -1;
    }
    if(sem_init(id->answer,0,0)){
        sem_destroy(id->request);
        return -2;
    }
    id->pool=p;
    if(pthread_create(&id->tid,attr,(void*)_KLS_threadWorker,id)){
        sem_destroy(id->request);
        sem_destroy(id->answer);
        return -3;
    }
    while(sem_wait(id->answer));
    if(p->die){
        pthread_join(id->tid,NULL);
        sem_destroy(id->request);
        sem_destroy(id->answer);
        return -4;
    }
    return 0;
}

static char _KLS_threadAttr(void *a,size_t s){
    do{
        if(pthread_attr_init(a)) return -1;
        if(s && pthread_attr_setstacksize(a,s<<10)) break;
        if(pthread_attr_setinheritsched(a,PTHREAD_INHERIT_SCHED)) break;
        #ifdef PTHREAD_FPU_ENABLED
        if(pthread_setfpustate(a,PTHREAD_FPU_ENABLED)) break;
        #endif
        return 0;
    }while(0);
    pthread_attr_destroy(a);
    return -2;
}

KLS_t_THREAD_POOL KLS_threadPoolCreate(unsigned int count,unsigned char prio,size_t stackSize_kb){
    if(!_KLS_threadStatus) return NULL;
    if(!count) count=KLS_sysInfoCores();
    if(!count) return NULL;
    {
        pthread_attr_t a[1];
        if(_KLS_threadAttr(a,stackSize_kb)) return NULL;
        {
            const unsigned int sizeQueue=sizeof(_KLS_t_THREAD_QUEUE)*(prio+1);
            KLS_t_THREAD_POOL p=KLS_malloc(KLS_OFFSET(*p,id)+sizeof(struct _KLS_t_THREAD)*count+sizeQueue);
            while(p){
                if(pthread_mutex_init(p->mtx,NULL)){
                    KLS_freeData(p);
                    break;
                }
                p->max=prio;
                p->die=p->wait=p->peak=0;
                p->queue=(void*)(p->id+count);
                memset(p->queue,0,sizeQueue);
                for(p->count=0;p->count<count;++p->count)
                    if(_KLS_threadStart(p,a)){
                        KLS_threadPoolDestroy(&p);
                        break;
                    }
                break;
            }
            pthread_attr_destroy(a);
            return p;
        }
    }
}

static void _KLS_threadPoolDestroy(KLS_t_THREAD_POOL *pool,KLS_byte die){
    if(pool && *pool){
        _KLS_t_THREAD_POOL p=*pool;
        unsigned int i=p->count;
        p->die=die;
        _KLS_threadPoolWake(p->id,i);
        while(i){
            pthread_join(p->id[--i].tid,NULL);
            sem_destroy(p->id[i].request);
            sem_destroy(p->id[i].answer);
        }
        pthread_mutex_destroy(p->mtx);
        _KLS_threadPoolClear(p);
        KLS_freeData(*pool);
    }
}

void KLS_threadPoolDestroy(KLS_t_THREAD_POOL *pool)      { _KLS_threadPoolDestroy(pool,1); }
void KLS_threadPoolDestroyLater(KLS_t_THREAD_POOL *pool) { _KLS_threadPoolDestroy(pool,2); }

void KLS_threadPoolWait(KLS_t_THREAD_POOL pool){
    while(!KLS_threadPoolWaitTime(pool,10000));
}

KLS_byte KLS_threadPoolWaitTime(KLS_t_THREAD_POOL pool,unsigned int msec){
    if(pool){
        const unsigned count=pool->count;
        unsigned int i;
        struct timespec t;
        clock_gettime(CLOCK_REALTIME,&t);
        KLS_timespecAdd(&t,msec/1000,(msec%1000)*1000000);
        pool->wait=1;
        _KLS_threadPoolWake(pool->id,count);
        for(i=0;i<count;++i)
            while(sem_timedwait(pool->id[i].answer,&t)){
                if(errno==ETIMEDOUT){
                    pool->wait=0;
                    return 0;
                }
            }
        pthread_mutex_lock(pool->mtx);
        for(i=0;i<count;++i)
            if(pool->id[i].task)
                break;
        pthread_mutex_unlock(pool->mtx);
        pool->wait=0;
        return i==count;
    } return -1;
}

char _KLS_threadPoolTask(void *pool,void *task, unsigned char prio){
    _KLS_t_THREAD_POOL p=pool;
    if(p && !p->die && task){
        pthread_mutex_lock(p->mtx);
        _KLS_threadPoolPush(p,task,prio);
        pthread_mutex_unlock(p->mtx);
        _KLS_threadPoolWake(p->id,p->count);
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

pthread_t KLS_threadPosix(KLS_t_THREAD id){
    return id ? id->id->tid : pthread_self();
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

