
#define _KLS_THREAD_MKSLEEP(_mk_) {const struct timespec t={_mk_/1000000,(_mk_%1000000)*1000};nanosleep(&t,NULL);}

typedef struct{
    void *next;
    void (*f)(void *args,unsigned int index,void *pool);
}_KLS_t_THREAD_TASK;

typedef struct{
    _KLS_t_THREAD_TASK *first, *last;
}_KLS_t_THREAD_QUEUE;

typedef struct _KLS_t_THREAD_POOL{
    pthread_mutex_t mtx[1];
    pthread_cond_t cond[2];
    pthread_t *tid;
    unsigned int count, busy, size;
    unsigned char die, wait;
    unsigned char peak, max;
    _KLS_t_THREAD_QUEUE queue[1];
} * const _KLS_t_THREAD_POOL;



static void _KLS_threadPoolPush(_KLS_t_THREAD_POOL p,_KLS_t_THREAD_TASK * const t,unsigned char prio){
    _KLS_t_THREAD_QUEUE * const q=p->queue + (prio>p->peak ? p->peak : prio);
    if(q->last) q->last->next=t;
    else q->first=t;
    q->last=t;
    ++p->size;
}

static void *_KLS_threadPoolPop(_KLS_t_THREAD_POOL p){
    _KLS_t_THREAD_QUEUE * const q=p->queue+p->peak;
    _KLS_t_THREAD_TASK * const t=q->first;
    if(t){
        --p->size;
        if( !(q->first=t->next) ){
            q->last=NULL;
            while(p->peak && !p->queue[--p->peak].first);
        }
    } return t;
}

static void _KLS_threadPoolClear(_KLS_t_THREAD_POOL p){
    void *t; while( (t=_KLS_threadPoolPop(p)) ) KLS_free(t);
}



static unsigned int _KLS_threadIndex(const _KLS_t_THREAD_POOL p){
    unsigned int i=p->count-1;
    const pthread_t tid=pthread_self(), * const tids=p->tid;
    while(!pthread_equal(tid,tids[i])) --i;
    return i;
}

static void *_KLS_threadWorker(_KLS_t_THREAD_POOL p){
    const unsigned int index=_KLS_threadIndex(p);
    unsigned char sleep=0, busy=1;
    _KLS_t_THREAD_TASK *t[1+3+1]={NULL};

    while('0'){
        pthread_mutex_lock(p->mtx);
_mark:
        if(p->die==1)
            break;
        if( (t[0]=_KLS_threadPoolPop(p)) ){
            unsigned int i=1, c=p->size/p->count;
            if(c>(KLS_ARRAY_LEN(t)-2)) c=KLS_ARRAY_LEN(t)-2;
            while(c && (t[i]=_KLS_threadPoolPop(p))) ++i,--c;

            if(busy&1){busy<<=1; ++p->busy;}
            pthread_mutex_unlock(p->mtx);

            for(i=0;t[i];++i){
                t[i]->f(t[i]+1,index,p);
                KLS_freeData(t[i]);
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

        _KLS_THREAD_MKSLEEP(1000);
        sleep>>=1;

    }
    pthread_mutex_unlock(p->mtx);
    return NULL;
}


static char _KLS_threadAttrInit(void *a,size_t s){
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

KLS_t_THREAD_POOL KLS_threadPoolCreate(unsigned int count,unsigned char prio,size_t stackSize_kb){
    pthread_attr_t attr[1];
    if( (count || (count=KLS_sysInfoCores())) && _KLS_threadAttrInit(attr,stackSize_kb) ){
        const unsigned int queueSize=sizeof(_KLS_t_THREAD_QUEUE)*(1+(unsigned int)prio);
        KLS_t_THREAD_POOL p=KLS_malloc( KLS_OFFSET(*p,queue) + queueSize + sizeof(pthread_t)*count);
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
            p->count=p->size=p->busy=0;
            memset(p->queue,0,queueSize);
            p->tid=(void*)(p->queue+1+prio);

            while(p->count<count)
                if(pthread_create(p->tid+p->count++,attr,(void*)_KLS_threadWorker,p)){
                    --p->count; KLS_threadPoolDestroy(&p); break;
                }
            break;
        }
        pthread_attr_destroy(attr);
        return p;
    } return NULL;
}

static void _KLS_threadPoolDestroy(KLS_t_THREAD_POOL *pool,KLS_byte die){
    if(pool && *pool){
        _KLS_t_THREAD_POOL p=*pool;
        unsigned int i=p->count;
        pthread_mutex_lock(p->mtx);
        p->die=die;
        pthread_cond_broadcast(p->cond);
        pthread_mutex_unlock(p->mtx);

        while(i) pthread_join(p->tid[--i],NULL);

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
    pthread_cond_signal(pool->cond);
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
        pthread_cond_signal(pool->cond);
        ret=!pthread_cond_timedwait(pool->cond+1,pool->mtx,&t);
        pool->wait=0;
        pthread_mutex_unlock(pool->mtx);

        return ret;
    } return -1;
}

void *_KLS_threadPoolTask(void *pool,const void *task,const unsigned int size,unsigned char prio){
    _KLS_t_THREAD_POOL p=pool;
    if( p && !p->die && ((void**)task)[1] && (pool=KLS_malloc(size)) ){
        memcpy(pool,task,size);
        if(prio>p->max) prio=p->max;
        pthread_mutex_lock(p->mtx);
        _KLS_threadPoolPush(p,pool,prio);
        pthread_cond_signal(p->cond);
        pthread_mutex_unlock(p->mtx);
        return pool;
    } return NULL;
}

void KLS_threadPoolClear(KLS_t_THREAD_POOL pool){
    if(pool){
        pthread_mutex_lock(pool->mtx);
        _KLS_threadPoolClear(pool);
        pthread_mutex_unlock(pool->mtx);
    }
}

unsigned int KLS_threadPoolCount(const KLS_t_THREAD_POOL pool){
    return pool ? pool->count : 0;
}

const pthread_t *KLS_threadPoolPosix(KLS_t_THREAD_POOL pool){
    return pool ? pool->tid : NULL;
}


/***********************************************************************/
/***********************************************************************/

static char _KLS_threadStatus=0;
static struct{
    pthread_key_t stop;
}_KLS_threadKey;

char _KLS_threadInit(void){
    if(!_KLS_threadStatus){
        if(pthread_key_create(&_KLS_threadKey.stop,NULL))
            return 0;
        _KLS_threadStatus=1 | ((KLS_signalGetMode(KLS_thread_sigresume)==SIG_BLOCK)<<2) | ((KLS_signalGetMode(KLS_thread_sigpause)==SIG_BLOCK)<<1);
    } return _KLS_threadStatus;
}
void _KLS_threadClose(void){
    if(_KLS_threadStatus){
        _KLS_threadStatus=0;
        pthread_key_delete(_KLS_threadKey.stop);
    }
}

#ifndef _KLS_THREAD_SIGNAL_PAUSE
    #define _KLS_THREAD_SIGNAL_PAUSE  SIGINT
#endif

#ifndef _KLS_THREAD_SIGNAL_CONTINUE
    #define _KLS_THREAD_SIGNAL_CONTINUE  SIGCONT
#endif

int KLS_thread_sigresume=_KLS_THREAD_SIGNAL_CONTINUE;
int KLS_thread_sigpause=_KLS_THREAD_SIGNAL_PAUSE;

#undef _KLS_THREAD_SIGNAL_PAUSE
#undef _KLS_THREAD_SIGNAL_CONTINUE


void _KLS_threadPauser(int sig){
    KLS_signalSetHandler(sig,_KLS_threadPauser);
    KLS_signalSetMode(sig,SIG_UNBLOCK);
    if(sig==KLS_thread_sigpause){
        /* pthread_setspecific(_KLS_threadKey.stop,((char*)pthread_getspecific(_KLS_threadKey.stop))+1); */
        char * const p=pthread_getspecific(_KLS_threadKey.stop);
        pthread_setspecific(_KLS_threadKey.stop,p+1);
        if(!p) while(pthread_getspecific(_KLS_threadKey.stop)) _KLS_THREAD_MKSLEEP(1000000);
        return;
    }
    pthread_setspecific(_KLS_threadKey.stop, ((char*)pthread_getspecific(_KLS_threadKey.stop))-1 );
}

void KLS_threadPausable(KLS_byte pausable){
    if(pausable){
        KLS_signalSetHandler(KLS_thread_sigpause,_KLS_threadPauser);
        KLS_signalSetHandler(KLS_thread_sigresume,_KLS_threadPauser);
        KLS_signalSetMode(KLS_thread_sigpause,SIG_UNBLOCK);
        KLS_signalSetMode(KLS_thread_sigresume,SIG_UNBLOCK);
    }else{
        KLS_signalSetMode(KLS_thread_sigpause,(_KLS_threadStatus&2)?SIG_BLOCK:SIG_UNBLOCK);
        KLS_signalSetMode(KLS_thread_sigresume,(_KLS_threadStatus&4)?SIG_BLOCK:SIG_UNBLOCK);
        KLS_signalSetHandler(KLS_thread_sigpause,SIG_DFL);
        KLS_signalSetHandler(KLS_thread_sigresume,SIG_DFL);
    }
}

void KLS_threadPause(pthread_t tid){
    KLS_signalSend(tid,KLS_thread_sigpause);
}

void KLS_threadResume(pthread_t tid){
    KLS_signalSend(tid,KLS_thread_sigresume);
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

