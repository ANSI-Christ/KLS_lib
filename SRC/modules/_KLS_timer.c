
struct _KLS_t_TIMER{
    void(*f)(void *arg);
    void *arg;
    struct timespec time;
    unsigned int msInterval;
    char run;
};

static struct{
    pthread_mutex_t mtx[1];
    sem_t sem[1];
    pthread_t tid;
    KLS_t_LIST list[1];
    char create, mtxDel;
}_KLS_timerGlob={.mtx={PTHREAD_MUTEX_INITIALIZER}};

#define KLS_TIMER_CLOCKID    CLOCK_REALTIME

void *_KLS_timerThread(void *arg){
    KLS_TYPEOF(_KLS_timerGlob) * const g=&_KLS_timerGlob;
    struct timespec tWait={0,0},tMin={0,0},t;
    KLS_t_TIMER timer;
    if(KLS_threadPolicyGet(g->tid,(void*)&t,(void*)&tMin) && *(int*)&tMin!=0)
        KLS_threadPolicySet(g->tid,*(int*)&t,99);
    while(sem_wait(g->sem));
    while('0'){
        while(sem_timedwait(g->sem,&tWait))
            if(errno==ETIMEDOUT) break;
        pthread_mutex_lock(g->mtx);
        if(!(timer=g->list->first)){
            g->create=0;
            sem_destroy(g->sem);
            pthread_mutex_unlock(g->mtx);
            if(g->mtxDel) pthread_mutex_destroy(g->mtx);
            break;
        }
        clock_gettime(KLS_TIMER_CLOCKID,&tWait);
        KLS_timespecNorm(&tWait);
        tMin.tv_sec=tWait.tv_sec+3600;
        while(timer && timer->run){
            t=timer->time;
            KLS_timespecSub(&t,tWait.tv_sec,tWait.tv_nsec);
            if( !(t.tv_sec|t.tv_nsec) ){
                timer->f(timer->arg);
                if(!timer->msInterval){
                    KLS_t_TIMER next=KLS_listNext(timer);
                    timer->run=0; KLS_listMoveAfter(g->list,timer,g->list->last);
                    timer=next; continue;
                }
                timer->time=tWait;
                KLS_timespecAdd(&timer->time,timer->msInterval/1000,(timer->msInterval%1000)*1000000);
            }
            if(timer->time.tv_sec<tMin.tv_sec || (timer->time.tv_sec==tMin.tv_sec && timer->time.tv_nsec<tMin.tv_nsec))
                tMin=timer->time;
            timer=KLS_listNext(timer);
        }
        pthread_mutex_unlock(g->mtx);
        tWait=tMin;
    }
    return NULL;
    (void)arg;
}

void _KLS_timerClose(){
    const char mtxDel=0;
    KLS_TYPEOF(_KLS_timerGlob) * const g=&_KLS_timerGlob;
    if(g->create){
        pthread_mutex_lock(g->mtx);
        KLS_listClear(g->list);
        if(mtxDel)g->mtxDel=1;
        sem_post(g->sem);
        pthread_mutex_unlock(g->mtx);
    }else if(mtxDel)pthread_mutex_destroy(g->mtx);
}

KLS_byte _KLS_timerInit(){
    KLS_TYPEOF(_KLS_timerGlob) * const g=&_KLS_timerGlob;
    if(!g->create){
        pthread_attr_t a[1];
        do{
            if(sem_init(g->sem,0,0)) break;
            if(pthread_attr_init(a)) break;
            if(pthread_attr_setstacksize(a,40<<10)) break;
            if(pthread_attr_setdetachstate(a,PTHREAD_CREATE_DETACHED)) break;
            if(pthread_attr_setinheritsched(a,PTHREAD_INHERIT_SCHED)) break;
            #ifdef PTHREAD_FPU_ENABLED
            if(pthread_setfpustate(a,PTHREAD_FPU_ENABLED)) break;
            #endif
            if( (g->create=!pthread_create(&g->tid,a,_KLS_timerThread,NULL)) )
                *g->list=KLS_listNew(sizeof(struct _KLS_t_TIMER),NULL);
        }while(0);
        pthread_attr_destroy(a);
        if(!g->create) sem_destroy(g->sem);
    } return g->create;
}

KLS_t_TIMER KLS_timerCreate(void(*callback)(void *arg),void *arg){
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

KLS_byte KLS_timerStart(KLS_t_TIMER timer,unsigned int msDelay,unsigned msInterval,void(*callback)(void *arg),void *arg){
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
        clock_gettime(KLS_TIMER_CLOCKID,&timer->time);
        KLS_timespecAdd(&timer->time,msDelay/1000,(msDelay%1000)*1000000);
        pthread_mutex_unlock(g->mtx);
        while(!sem_trywait(g->sem));
        sem_post(g->sem);
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
        pthread_mutex_unlock(g->mtx);
        while(!sem_trywait(g->sem));
        sem_post(g->sem);
        return 1;
    } return 0;
}

void KLS_timerDestroy(KLS_t_TIMER *timer){
    if(timer && *timer){
        KLS_TYPEOF(_KLS_timerGlob) * const g=&_KLS_timerGlob;
        pthread_mutex_lock(g->mtx);
        KLS_listRemove(g->list,*timer);
        if(!g->list->size) sem_post(g->sem);
        pthread_mutex_unlock(g->mtx);
        *timer=NULL;
    }
}

#undef KLS_TIMER_CLOCKID
