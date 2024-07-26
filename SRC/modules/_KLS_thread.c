
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



struct _KLS_t_THREAD{
    KLS_t_THREAD_POOL pool;
    pthread_mutex_t *mtx;
    _KLS_t_THREAD_TASKS *queue;
    _KLS_t_THREAD_TASK *task;
    pthread_t tid;
    sem_t request[1], answer[1];
    KLS_byte die, wait;
};

struct _KLS_t_THREAD_POOL{
    KLS_t_THREAD threads;
    pthread_mutex_t mtx[1];
    _KLS_t_THREAD_TASKS queue[1];
    unsigned int count;
};


static pthread_key_t _KLS_threadKey;
static KLS_byte _KLS_threadStatus=0;

KLS_byte _KLS_threadInit(){
    if(!_KLS_threadStatus) _KLS_threadStatus=!pthread_key_create(&_KLS_threadKey,NULL);
    return _KLS_threadStatus;
}
void _KLS_threadClose(){
    if(_KLS_threadStatus) pthread_key_delete(_KLS_threadKey);
}

KLS_byte _KLS_threadAttr(void *a,size_t s){
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

void *_KLS_thread(struct _KLS_t_THREAD * const self){
    if(pthread_setspecific(_KLS_threadKey,self)){
        self->die=1;
        sem_post(self->answer);
        return NULL;
    }
    sem_post(self->answer);
    while('0'){
        while(sem_wait(self->request));

        if(self->die==1) break;

        pthread_mutex_lock(self->mtx);
        if( !(self->task=_KLS_threadTasksPop(self->queue)) ){
            if(self->die){
                pthread_mutex_unlock(self->mtx);
                return NULL;
            }
            if(self->wait){
                self->wait=0;
                sem_post(self->answer);
            }
        }
        pthread_mutex_unlock(self->mtx);

        if(self->task){
            self->task->f(self->task+1);
            KLS_free(self->task);
        }
    }
    return NULL;
}

void *_KLS_threadFree(KLS_t_THREAD id,char flags){
    if(flags & 1) sem_destroy(id->request);
    if(flags & 2) sem_destroy(id->answer);
    if(id->pool) return NULL;
    if(flags & 4) pthread_mutex_destroy(id->mtx);
    if(flags & 8) _KLS_threadTasksClear(id->queue);
    KLS_free(id);
    return NULL;
}

KLS_t_THREAD _KLS_threadNew(void *attr,KLS_t_THREAD_POOL pool){
    struct _KLS_t_THREAD_SINGLE{struct _KLS_t_THREAD id; pthread_mutex_t mtx[1]; _KLS_t_THREAD_TASKS queue[1];} *id;
    if(pool){
        id=(void*)(pool->threads+pool->count);
        id->id.pool=pool;
        id->id.mtx=pool->mtx;
        id->id.queue=pool->queue;
    }else if( (id=KLS_malloc(sizeof(*id))) ){
        memset(id,0,sizeof(*id));
        id->id.mtx=id->mtx;
        id->id.queue=id->queue;
        if(pthread_mutex_init(id->id.mtx,NULL)) return _KLS_threadFree((void*)id,0);
    }else return NULL;
    if(sem_init(id->id.request,0,0)) return _KLS_threadFree((void*)id,4);
    if(sem_init(id->id.answer,0,0)) return _KLS_threadFree((void*)id,5);
    if(pthread_create(&id->id.tid,attr,(void*)_KLS_thread,(void*)id)) return _KLS_threadFree((void*)id,7);
    while(sem_wait(id->id.answer));
    if(id->id.die) return _KLS_threadFree((void*)id,7);
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

KLS_byte _KLS_threadTaskAddQueue(KLS_t_THREAD id,void *task){
    if(id && task){
        pthread_mutex_lock(id->mtx);
        _KLS_threadTasksPush(id->queue,task);
        pthread_mutex_unlock(id->mtx);
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
        pthread_mutex_lock(id->mtx);
        _KLS_threadTasksClear(id->queue);
        pthread_mutex_unlock(id->mtx);
    }
}

void _KLS_threadDestroy(KLS_t_THREAD *id,KLS_byte die){
    if(id && *id){
        KLS_t_THREAD t=*id;
        t->die=die;
        sem_post(t->request);
        pthread_join(t->tid,NULL);
        *id=_KLS_threadFree(t,15);
    }
}

void KLS_threadDestroy(KLS_t_THREAD *id){ _KLS_threadDestroy(id,1); }
void KLS_threadDestroyLater(KLS_t_THREAD *id){ _KLS_threadDestroy(id,2); }

void _KLS_threadWaitPost(KLS_t_THREAD id){
    while(!sem_trywait(id->answer));
    id->wait=1; sem_post(id->request);
}

void KLS_threadWait(KLS_t_THREAD id){
    if(id){
        _KLS_threadWaitPost(id);
        while(sem_wait(id->answer));
    }
}

KLS_byte KLS_threadWaitTime(KLS_t_THREAD id,unsigned int msec){
    if(id){
        struct timespec t[1];
        _KLS_threadWaitPost(id);
        clock_gettime(CLOCK_REALTIME,t);
        KLS_timespecAdd(t,msec/1000,(msec%1000)*1000000);
        while(sem_timedwait(id->answer,t))
            if(errno==ETIMEDOUT)
                return 0;
        return 1;
    } return -1;
}

pthread_t KLS_threadPosix(KLS_t_THREAD id){
    return id ? id->tid : pthread_self();
}

KLS_byte KLS_threadPolicySet(pthread_t tid,int policy,int priority){
    struct sched_param s[1]={{.sched_priority=priority}};
    return !pthread_setschedparam(tid,policy,s);
}

KLS_byte KLS_threadPolicyGet(pthread_t tid,int *policy,int *priority){
    struct sched_param s[1]={};
    return !pthread_getschedparam(tid,policy,s) && ((*priority=s->sched_priority) || 1);
}

const char *KLS_threadPolicyName(int policy){
    #define _THRPLC(_1_) case KLS_THREAD_POLICY_ ## _1_: return #_1_;
    switch(policy){ _THRPLC(OTHER) _THRPLC(FIFO) _THRPLC(RR) }
    return "unknown";
    #undef _THRPLC
}



KLS_t_THREAD_POOL KLS_threadPoolCreate(unsigned int count,size_t stackSize_kb){
    KLS_t_THREAD_POOL p=NULL;
    pthread_attr_t a[1];
    if(count && _KLS_threadAttr(a,stackSize_kb)){
        if( (p=KLS_malloc(sizeof(*p)+sizeof(*p->threads)*(count))) ){
            memset(p,0,sizeof(*p)+sizeof(*p->threads)*(count));
            if(pthread_mutex_init(p->mtx,NULL)){
                KLS_threadPoolDestroy(&p);
                pthread_attr_destroy(a);
                return p;
            }
            for(p->threads=(void*)(p+1);p->count<count;++p->count)
                if( !_KLS_threadNew(a,p) ){
                    KLS_threadPoolDestroy(&p);
                    break;
                }
        }
        pthread_attr_destroy(a);
    }
    return p;
}

KLS_t_THREAD KLS_threadSelf(){
    return pthread_getspecific(_KLS_threadKey);
}

unsigned int KLS_threadPoolCount(const KLS_t_THREAD_POOL pool){
    return pool ? pool->count : 0;
}

void _KLS_threadPoolDestroy(KLS_t_THREAD_POOL *pool,KLS_byte die){
    if(pool && *pool){
        unsigned int i;
        KLS_t_THREAD_POOL p=*pool;
        for(i=0;i<p->count;++i){
            p->threads[i].die=die;
            sem_post(p->threads[i].request);
        }
        for(i=0;i<p->count;++i) pthread_join(p->threads[i].tid,NULL);
        for(i=0;i<p->count;++i) _KLS_threadFree(p->threads+i,3);
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
        for(i=0;i<count;++i)
            _KLS_threadWaitPost(pool->threads+i);
        clock_gettime(CLOCK_REALTIME,t);
        KLS_timespecAdd(t,msec/1000,(msec%1000)*1000000);
        for(i=0;i<count;++i)
            while(sem_timedwait(pool->threads[i].answer,t))
                if(errno==ETIMEDOUT)
                    return 0;
        pthread_mutex_lock(pool->mtx);
        for(i=0;i<count;++i)
            if(pool->threads[i].task)
                break;
        pthread_mutex_unlock(pool->mtx);
        return i==count;
    } return -1;
}

KLS_byte _KLS_threadPoolTask(void *pool,void *task){
    if(_KLS_threadTaskAddQueue(pool?((KLS_t_THREAD_POOL)pool)->threads:NULL,task)){
        unsigned int i=((KLS_t_THREAD_POOL)pool)->count;
        while(i) sem_post(((KLS_t_THREAD_POOL)pool)->threads[--i].request);
        return 1;
    } return 0;
}

void _KLS_threadPoolClear(KLS_t_THREAD_POOL pool){
    if(pool){
        unsigned int i=pool->count;
        while(i) while(!sem_trywait(pool->threads[--i].request));
        pthread_mutex_lock(pool->mtx);
        _KLS_threadTasksClear(pool->queue);
        pthread_mutex_unlock(pool->mtx);
    }
}

KLS_t_THREAD_POOL KLS_threadPoolSelf(){
    KLS_t_THREAD t=KLS_threadSelf();
    if(t) return t->pool;
    return NULL;
}

unsigned int KLS_threadPoolSelfNum(){
    KLS_t_THREAD t=KLS_threadSelf();
    if(t && t->pool) return 1+(unsigned int )(t-t->pool->threads);
    return 0;
}

KLS_t_THREAD KLS_threadPoolAt(KLS_t_THREAD_POOL pool,unsigned int index){
    if(pool && index<pool->count)
        return pool->threads+index;
    return NULL;
}
