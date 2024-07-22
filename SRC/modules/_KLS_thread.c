
static pthread_key_t _KLS_threadKey;
static KLS_byte _KLS_threadStatus=0;
KLS_byte _KLS_threadInit(){ if(!_KLS_threadStatus) _KLS_threadStatus=!pthread_key_create(&_KLS_threadKey,NULL); return _KLS_threadStatus; }
void _KLS_threadClose(){ if(_KLS_threadStatus) pthread_key_delete(_KLS_threadKey); }

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


typedef struct{
    void (*f)(void *args);
    void *args;
}_KLS_t_THREAD_TASK;

struct _KLS_t_THREAD{
    pthread_t tid;
    pthread_mutex_t *mtx;
    KLS_t_QUEUE *queue;
    sem_t request, answer;
    KLS_t_QUEUE _queue;
    pthread_mutex_t _mtx;
    unsigned int poolNumber;
    KLS_byte die, wait;
};

typedef struct{
    void *mtx, *queue, *id;
    pthread_t thread;
    sem_t sem;
}_KLS_t_THREAD_HELP;

void _KLS_threadQueueDeleter(_KLS_t_THREAD_TASK *t){ KLS_free(t->args); }

KLS_byte _KLS_threadSync(_KLS_t_THREAD_HELP *help,KLS_t_THREAD id){
    #define _THR_RET(_code_) sem_post(&help->sem); return _code_
    if(sem_init(&id->request,0,0)) { _THR_RET(1); }
    if(sem_init(&id->answer,0,0)) { sem_destroy(&id->request); _THR_RET(2); }
    if(help->mtx) id->mtx=help->mtx;
    else if(pthread_mutex_init((id->mtx=&id->_mtx),NULL)) { sem_destroy(&id->request); sem_destroy(&id->answer); _THR_RET(3); }
    if(pthread_setspecific(_KLS_threadKey,(help->id=id))) { pthread_mutex_destroy(&id->_mtx); sem_destroy(&id->request); sem_destroy(&id->answer); _THR_RET(4); }
    if(help->queue) id->queue=help->queue;
    else *(id->queue=&id->_queue)=KLS_queueNew(KLS_FIFO,sizeof(_KLS_t_THREAD_TASK),(void*)_KLS_threadQueueDeleter);
    _THR_RET(0);
    #undef _THR_RET
}

void *_KLS_thread(_KLS_t_THREAD_HELP *help){
    struct _KLS_t_THREAD self={pthread_self()};
    _KLS_t_THREAD_TASK task={NULL};
    if(_KLS_threadSync(help,&self))
        return NULL;
    while('0'){
        while(sem_wait(&self.request));

        if(self.die==1) break;

        pthread_mutex_lock(self.mtx);
        if(!KLS_queuePop(self.queue,&task)){
            if(self.die){
                pthread_mutex_unlock(self.mtx);
                break;
            }
            if(self.wait){
                self.wait=0;
                sem_post(&self.answer);
            }
        }
        pthread_mutex_unlock(self.mtx);

        if(task.f){
            task.f(task.args);
            KLS_free(task.args);
            task.f=NULL;
        }
    }
    sem_destroy(&self.request);
    sem_destroy(&self.answer);
    if(self.mtx==&self._mtx) pthread_mutex_destroy(self.mtx);
    if(self.queue==&self._queue) KLS_queueClear(self.queue);
    return NULL;
}

KLS_t_THREAD _KLS_threadCreate(void *attr,void *mtx,void *queue){
    _KLS_t_THREAD_HELP help={mtx,queue};
    if(sem_init(&help.sem,0,0) || (pthread_create(&help.thread,attr,(void*)_KLS_thread,&help) && !sem_destroy(&help.sem)) )
        return NULL;
    while(sem_wait(&help.sem));
    sem_destroy(&help.sem);
    return help.id;
}

KLS_t_THREAD KLS_threadCreate(size_t stackSize_kb){
    pthread_attr_t a[1];
    if(_KLS_threadAttr(a,stackSize_kb)){
        KLS_t_THREAD t=_KLS_threadCreate(a,NULL,NULL);
        pthread_attr_destroy(a);
        return t;
    }
    return NULL;
}

KLS_byte _KLS_threadTaskAddQueue(KLS_t_THREAD id,void *task,void *args,unsigned int argsCount){
    KLS_byte ret=0;
    if(id && task && (args || !argsCount)){
        _KLS_t_THREAD_TASK t={.f=task,.args=args};
        pthread_mutex_lock(id->mtx);
        ret=KLS_queuePush(id->queue,&t);
        pthread_mutex_unlock(id->mtx);
    }
    if(!ret) KLS_free(args);
    return ret;
}

KLS_byte _KLS_threadTask(KLS_t_THREAD id,void *task,void *args,unsigned int argsCount){
    if(_KLS_threadTaskAddQueue(id,task,args,argsCount)){
        sem_post(&id->request);
        return 1;
    } return 0;
}

void _KLS_threadDestroy(KLS_t_THREAD *id,KLS_byte die){
    if(id && *id){
        pthread_t tid=(*id)->tid;
        (*id)->die=die;
        sem_post(&(*id)->request);
        pthread_join(tid,NULL);
        *id=NULL;
    }
}

void KLS_threadDestroy(KLS_t_THREAD *id){ _KLS_threadDestroy(id,1); }
void KLS_threadDestroyLater(KLS_t_THREAD *id){ _KLS_threadDestroy(id,2); }

void _KLS_threadWaitPost(KLS_t_THREAD id){
    while(!sem_trywait(&id->answer));
    id->wait=1; sem_post(&id->request);
}

void KLS_threadWait(KLS_t_THREAD id){
    if(id){
        _KLS_threadWaitPost(id);
        while(sem_wait(&id->answer));
    }
}

KLS_byte KLS_threadWaitTime(KLS_t_THREAD id,unsigned int msec){
    if(id){
        struct timespec t[1];
        _KLS_threadWaitPost(id);
        clock_gettime(CLOCK_REALTIME,t);
        KLS_timespecAdd(t,msec/1000,(msec%1000)*1000000);
        while(sem_timedwait(&id->answer,t))
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



struct _KLS_t_THREAD_POOL{
    union{ KLS_t_THREAD id; pthread_t tid; } *threads;
    KLS_t_QUEUE tasks;
    pthread_mutex_t mtx;
    unsigned int count;
};

KLS_t_THREAD_POOL KLS_threadPoolCreate(unsigned int count,size_t stackSize_kb){
    KLS_t_THREAD_POOL p=NULL;
    pthread_attr_t a[1];
    if(count && _KLS_threadAttr(a,stackSize_kb)){
        if( (p=KLS_malloc(sizeof(*p)+sizeof(*p->threads)*(count))) ){
            memset(p,0,sizeof(*p)+sizeof(*p->threads)*(count));
            if(pthread_mutex_init(&p->mtx,NULL)){
                KLS_threadPoolDestroy(&p);
                pthread_attr_destroy(a);
                return p;
            }
            p->threads=(void*)(p+1);
            p->tasks=KLS_queueNew(KLS_FIFO,sizeof(_KLS_t_THREAD_TASK),(void*)_KLS_threadQueueDeleter);
            for(;p->count<count;++p->count){
                if( (p->threads[p->count].id=_KLS_threadCreate(a,&p->mtx,&p->tasks)) ){
                    p->threads[p->count].id->poolNumber=p->count+1;
                    continue;
                }
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
            KLS_t_THREAD id=p->threads[i].id;
            id->die=die;
            p->threads[i].tid=id->tid;
            sem_post(&id->request);
        }
        for(i=0;i<p->count;++i)
            pthread_join(p->threads[i].tid,NULL);
        pthread_mutex_destroy(&p->mtx);
        KLS_queueClear(&p->tasks);
        KLS_freeData(*pool);
    }
}

void KLS_threadPoolDestroy(KLS_t_THREAD_POOL *pool)      { _KLS_threadPoolDestroy(pool,1); }
void KLS_threadPoolDestroyLater(KLS_t_THREAD_POOL *pool) { _KLS_threadPoolDestroy(pool,2); }


void KLS_threadPoolWait(KLS_t_THREAD_POOL pool){
    if(pool){
        unsigned int i;
        for(i=0;i<pool->count;++i)
            _KLS_threadWaitPost(pool->threads[i].id);
        for(i=0;i<pool->count;++i)
            while(sem_wait(&pool->threads[i].id->answer));
    }
}

KLS_byte KLS_threadPoolWaitTime(KLS_t_THREAD_POOL pool,unsigned int msec){
    if(pool){
        unsigned int i;
        struct timespec t[1];
        for(i=0;i<pool->count;++i)
            _KLS_threadWaitPost(pool->threads[i].id);
        clock_gettime(CLOCK_REALTIME,t);
        KLS_timespecAdd(t,msec/1000,(msec%1000)*1000000);
        for(i=0;i<pool->count;++i)
            while(sem_timedwait(&pool->threads[i].id->answer,t))
                if(errno==ETIMEDOUT)
                    return 0;
        return 1;
    } return -1;
}

KLS_byte _KLS_threadPoolTask(KLS_t_THREAD_POOL pool,void *task,void *args,unsigned int argsCount){
    if(_KLS_threadTaskAddQueue(pool?pool->threads[0].id:NULL,task,args,argsCount)){
        argsCount=pool->count;
        while(argsCount) sem_post(&pool->threads[--argsCount].id->request);
        return 1;
    } return 0;
}

KLS_t_THREAD_POOL KLS_threadPoolSelf(){
    KLS_t_THREAD t=KLS_threadSelf();
    if(t && t->poolNumber)
        return (void*)( ((KLS_byte*)t->queue)-KLS_OFFSET(struct _KLS_t_THREAD_POOL,tasks) );
    return NULL;
}

unsigned int KLS_threadPoolSelfNum(){
    KLS_t_THREAD t=KLS_threadSelf();
    return t ? t->poolNumber : 0;
}

KLS_t_THREAD KLS_threadPoolAt(KLS_t_THREAD_POOL pool,unsigned int index){
    if(pool && index<pool->count)
        return pool->threads[index].id;
    return NULL;
}
