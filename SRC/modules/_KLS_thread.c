
typedef struct{
    void *next;
    void (*f)(void *args);
}_KLS_t_THREAD_TASK;

typedef struct{
    _KLS_t_THREAD_TASK *first, *last;
}_KLS_t_THREAD_TASKS;

void _KLS_threadTasksPush(_KLS_t_THREAD_TASKS * const t,_KLS_t_THREAD_TASK *task){
    if(t->last){
        t->last->next=task;
        t->last=task;
        return;
    }
    t->first=t->last=task;
}

_KLS_t_THREAD_TASK *_KLS_threadTasksPop(_KLS_t_THREAD_TASKS * const t){
    _KLS_t_THREAD_TASK * const p=t->first;
    if( p && !(t->first=p->next) )
        t->last=NULL;
    return p;
}

void _KLS_threadTasksClear(_KLS_t_THREAD_TASKS * const t){
    void *p;
    while( (p=_KLS_threadTasksPop(t)) )
        KLS_free(p);
}



#define _KLS_THREAD_FIELDS \
    sem_t request[1], answer[1];\
    KLS_t_THREAD_POOL pool;\
    _KLS_t_THREAD_TASK *task;\
    pthread_t tid;

#define _KLS_THREAD_SHARE_FIELDS \
    pthread_mutex_t mtx[1];\
    _KLS_t_THREAD_TASKS queue[1];\
    char die,wait

typedef struct _KLS_t_THREAD{
    _KLS_THREAD_FIELDS;
} * const _KLS_t_THREAD;

typedef struct _KLS_t_THREAD_POOL{
    _KLS_THREAD_SHARE_FIELDS;
    unsigned int count;
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

char _KLS_threadAttr(void *a,size_t s){
    do{
        if(!_KLS_threadStatus) return 0;
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

void *_KLS_thread(_KLS_t_THREAD self){
    _KLS_t_THREAD_POOL share=self->pool;
    if(pthread_setspecific(_KLS_threadKey.self,self)){
        share->die=1;
        sem_post(self->answer);
        return NULL;
    }
    sem_post(self->answer);
    while('0'){
        while(sem_wait(self->request));

        if(share->die==1) break;

        pthread_mutex_lock(share->mtx);
        self->task=_KLS_threadTasksPop(share->queue);
        pthread_mutex_unlock(share->mtx);

        if(self->task){
            self->task->f(self->task+1);
            KLS_free(self->task);
            continue;
        }

        if(share->wait) sem_post(self->answer);

        if(share->die) return NULL;

    }
    return NULL;
}

void *_KLS_threadFree(_KLS_t_THREAD id,char flags){
    if(flags & 1) sem_destroy(id->request);
    if(flags & 2) sem_destroy(id->answer);
    if(id->pool<id) return NULL;
    if(flags & 4) pthread_mutex_destroy(id->pool->mtx);
    if(flags & 8) _KLS_threadTasksClear(id->pool->queue);
    return KLS_free(id);
}

KLS_t_THREAD _KLS_threadNew(void *attr,KLS_t_THREAD_POOL pool){
    struct _KLS_t_THREAD_SINGLE{
        _KLS_THREAD_FIELDS;
        _KLS_THREAD_SHARE_FIELDS;
        char end;
    } *id;
    if(pool){
        id=(void*)(pool->id+pool->count);
        id->pool=pool;
    }else if( (id=KLS_malloc(KLS_OFFSET(*id,end))) ){
        memset(id,0,KLS_OFFSET(*id,end));
        id->pool=(void*)&id->mtx;
        if(pthread_mutex_init(id->mtx,NULL)) return _KLS_threadFree((void*)id,0);
    }else return NULL;
    if(sem_init(id->request,0,0)) return _KLS_threadFree((void*)id,4);
    if(sem_init(id->answer,0,0)) return _KLS_threadFree((void*)id,5);
    if(pthread_create(&id->tid,attr,(void*)_KLS_thread,(void*)id)) return _KLS_threadFree((void*)id,7);
    while(sem_wait(id->answer));
    if(id->die) return _KLS_threadFree((void*)id,7);
    return (void*)id;
}

KLS_t_THREAD KLS_threadCreate(size_t stackSize_kb){
    pthread_attr_t a[1];
    if(_KLS_threadAttr(a,stackSize_kb)){
        KLS_t_THREAD t=_KLS_threadNew(a,NULL);
        pthread_attr_destroy(a);
        return t;
    }
    return NULL;
}

char _KLS_threadTaskAddQueue(_KLS_t_THREAD id,void *task){
    if(id && !id->pool->die && task){
        pthread_mutex_lock(id->pool->mtx);
        _KLS_threadTasksPush(id->pool->queue,task);
        pthread_mutex_unlock(id->pool->mtx);
        return 1;
    }
    KLS_free(task); return 0;
}

KLS_byte _KLS_threadTask(void *id,void *task){
    if(_KLS_threadTaskAddQueue(id,task)){
        sem_post(((KLS_t_THREAD)id)->request);
        return 1;
    } return 0;
}

void KLS_threadClear(KLS_t_THREAD id){
    if(id){
        while(!sem_trywait(id->request));
        pthread_mutex_lock(id->pool->mtx);
        _KLS_threadTasksClear(id->pool->queue);
        pthread_mutex_unlock(id->pool->mtx);
    }
}

void _KLS_threadDestroy(KLS_t_THREAD *id,KLS_byte die){
    if(id && *id){
        _KLS_t_THREAD t=*id;
        t->pool->die=die;
        sem_post(t->request);
        pthread_join(t->tid,NULL);
        *id=_KLS_threadFree(t,15);
    }
}

void KLS_threadDestroy(KLS_t_THREAD *id){ _KLS_threadDestroy(id,1); }
void KLS_threadDestroyLater(KLS_t_THREAD *id){ _KLS_threadDestroy(id,2); }

void _KLS_threadWaitPost(_KLS_t_THREAD id,const unsigned int count){
    unsigned int i;
    for(i=0;i<count;++i) while(!sem_trywait(id[i].answer));
    for(i=0,id->pool->wait=1;i<count;++i) sem_post(id[i].request);
}

void KLS_threadWait(KLS_t_THREAD id){
    if(id){
        _KLS_threadWaitPost(id,1);
        while(sem_wait(id->answer));
        id->pool->wait=0;
    }
}

KLS_byte KLS_threadWaitTime(KLS_t_THREAD id,unsigned int msec){
    if(id){
        struct timespec t[1];
        _KLS_threadWaitPost(id,1);
        clock_gettime(CLOCK_REALTIME,t);
        KLS_timespecAdd(t,msec/1000,(msec%1000)*1000000);
        while(sem_timedwait(id->answer,t))
            if(errno==ETIMEDOUT){
                id->pool->wait=0;
                return 0;
            }
        id->pool->wait=0;
        return 1;
    } return -1;
}

KLS_t_THREAD KLS_threadSelf(){
    return _KLS_threadStatus ? pthread_getspecific(_KLS_threadKey.self) : NULL;
}

pthread_t KLS_threadPosix(KLS_t_THREAD id){
    return id ? id->tid : pthread_self();
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



KLS_t_THREAD_POOL KLS_threadPoolCreateExt(unsigned int count,size_t stackSize_kb){
    KLS_t_THREAD_POOL p=NULL;
    pthread_attr_t a[1];
    if(count && _KLS_threadAttr(a,stackSize_kb)){
        const unsigned int size=KLS_OFFSET(*p,id)+sizeof(struct _KLS_t_THREAD)*(count);
        if( (p=KLS_malloc(size)) ){
            memset(p,0,size);
            if(pthread_mutex_init(p->mtx,NULL)){
                pthread_attr_destroy(a);
                return KLS_free(p);
            }
            for(;p->count<count;++p->count)
                if( !_KLS_threadNew(a,p) ){
                    KLS_threadPoolDestroy(&p);
                    break;
                }
        }
        pthread_attr_destroy(a);
    }
    return p;
}

KLS_t_THREAD_POOL KLS_threadPoolCreate(){
    return KLS_threadPoolCreateExt(KLS_sysInfoCores(),0);
}

unsigned int KLS_threadPoolCount(const KLS_t_THREAD_POOL pool){
    return pool ? pool->count : 0;
}

void _KLS_threadPoolDestroy(KLS_t_THREAD_POOL *pool,KLS_byte die){
    if(pool && *pool){
        unsigned int i;
        _KLS_t_THREAD_POOL p=*pool;
        p->die=die;
        for(i=0;i<p->count;++i) sem_post(p->id[i].request);
        for(i=0;i<p->count;++i) pthread_join(p->id[i].tid,NULL);
        for(i=0;i<p->count;++i) _KLS_threadFree(p->id+i,3);
        pthread_mutex_destroy(p->mtx);
        _KLS_threadTasksClear(p->queue);
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
        struct timespec t[1];
        _KLS_threadWaitPost(pool->id,count);
        clock_gettime(CLOCK_REALTIME,t);
        KLS_timespecAdd(t,msec/1000,(msec%1000)*1000000);
        for(i=0;i<count;++i)
            while(sem_timedwait(pool->id[i].answer,t))
                if(errno==ETIMEDOUT){
                    pool->wait=0;
                    return 0;
                }
        pool->wait=0;
        pthread_mutex_lock(pool->mtx);
        for(i=0;i<count;++i)
            if(pool->id[i].task)
                break;
        pthread_mutex_unlock(pool->mtx);
        return i==count;
    } return -1;
}

KLS_byte _KLS_threadPoolTask(void *pool,void *task){
    if(_KLS_threadTaskAddQueue(pool ? ((KLS_t_THREAD_POOL)pool)->id : NULL,task)){
        unsigned int i=((KLS_t_THREAD_POOL)pool)->count;
        while(i) sem_post(((KLS_t_THREAD_POOL)pool)->id[--i].request);
        return 1;
    } return 0;
}

void _KLS_threadPoolClear(KLS_t_THREAD_POOL pool){
    if(pool){
        unsigned int i=pool->count;
        while(i--) while(!sem_trywait(pool->id[i].request));
        pthread_mutex_lock(pool->mtx);
        _KLS_threadTasksClear(pool->queue);
        pthread_mutex_unlock(pool->mtx);
    }
}

KLS_t_THREAD_POOL KLS_threadPoolSelf(){
    const _KLS_t_THREAD t=KLS_threadSelf();
    return t ? t->pool : NULL;
}

unsigned int KLS_threadPoolSelfNum(){
    const _KLS_t_THREAD t=KLS_threadSelf();
    if(t && t->pool<t) return 1+(unsigned int)(t-t->pool->id);
    return 0;
}

KLS_t_THREAD KLS_threadPoolThreads(KLS_t_THREAD_POOL pool){
    return pool ? pool->id : NULL;
}

#undef _KLS_THREAD_POOL_ID
#undef _KLS_THREAD_FIELDS
#undef _KLS_THREAD_SHARE_FIELDS


#ifndef _KLS_THREAD_SIGNAL_PAUSE
    #define _KLS_THREAD_SIGNAL_PAUSE  SIGINT
#endif

#ifndef _KLS_THREAD_SIGNAL_CONTINUE
    #define _KLS_THREAD_SIGNAL_CONTINUE  SIGCONT
#endif

#define _KLS_THRSIG_P _KLS_THREAD_SIGNAL_PAUSE
#define _KLS_THRSIG_C _KLS_THREAD_SIGNAL_CONTINUE

void _KLS_threadPauser(int sig){
    KLS_signalSetMode(sig,SIG_UNBLOCK);
    KLS_signalSetHandler(sig,_KLS_threadPauser);
    if(sig==_KLS_THRSIG_P){
        pthread_setspecific(_KLS_threadKey.stop, ((char*)pthread_getspecific(_KLS_threadKey.stop))+1 );
        while(pthread_getspecific(_KLS_threadKey.stop))
            select(0,NULL,NULL,NULL,NULL);
        return;
    }
    pthread_setspecific(_KLS_threadKey.stop, ((char*)pthread_getspecific(_KLS_threadKey.stop))-1 );
}

void KLS_threadPausable(KLS_byte pausable){
    static char mask=4;
    if(mask&4) mask=((KLS_signalGetMode(_KLS_THRSIG_C)==SIG_BLOCK)<<1) | (KLS_signalGetMode(_KLS_THRSIG_P)==SIG_BLOCK);
    if(pausable){
        KLS_signalSetHandler(_KLS_THRSIG_P,_KLS_threadPauser);
        KLS_signalSetHandler(_KLS_THRSIG_C,_KLS_threadPauser);
        KLS_signalSetMode(_KLS_THRSIG_P,SIG_UNBLOCK);
        KLS_signalSetMode(_KLS_THRSIG_C,SIG_UNBLOCK);
    }else{
        KLS_signalSetMode(_KLS_THRSIG_P,(mask&1)?SIG_BLOCK:SIG_UNBLOCK);
        KLS_signalSetMode(_KLS_THRSIG_C,(mask&2)?SIG_BLOCK:SIG_UNBLOCK);
        KLS_signalSetHandler(_KLS_THRSIG_P,SIG_DFL);
        KLS_signalSetHandler(_KLS_THRSIG_C,SIG_DFL);
    }
}

void KLS_threadPause(pthread_t tid){
    KLS_signalSend(tid,_KLS_THRSIG_P);
}

void KLS_threadResume(pthread_t tid){
    KLS_signalSend(tid,_KLS_THRSIG_C);
}

#undef _KLS_THRSIG_P
#undef _KLS_THRSIG_C
