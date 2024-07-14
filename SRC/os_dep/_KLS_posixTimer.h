
struct _SYS_t_TIMER{
    timer_t t;
    void (*f)(SYS_t_TIMER t,void *arg);
    void *arg;
};

void _SYS_timerCall(SYS_t_TIMER t){
    t->f(t,t->arg);
}

#ifdef SA_SIGINFO

    #ifndef _SYS_TIMER_SIGNUM
    #ifdef SIGCHLD
        #define _SYS_TIMER_SIGNUM SIGCHLD
    #endif
    #endif

    #ifndef _SYS_TIMER_SIGNUM
    #ifdef SIGVTALRM
        #define _SYS_TIMER_SIGNUM SIGVTALRM
    #endif
    #endif
    
    #ifndef _SYS_TIMER_SIGNUM
    #ifdef SIGUSR1
        #define _SYS_TIMER_SIGNUM SIGUSR1
    #endif
    #endif

    #ifndef _SYS_TIMER_SIGNUM
        #define _SYS_TIMER_SIGNUM SIGALRM
    #endif

void _SYS_timerSignal(int sig,siginfo_t *i,void *c){
    if(i && i->si_code==SI_TIMER) _SYS_timerCall(i->si_value.sival_ptr);
}

SYS_t_TIMER SYS_timerCreate(void(*callback)(SYS_t_TIMER timer,void *arg),void *arg){
    SYS_t_TIMER t=KLS_malloc(sizeof(*t));
    if(t){
        struct sigevent e={
            .sigev_value.sival_ptr=t,
            .sigev_notify=SIGEV_SIGNAL,
            .sigev_signo=_SYS_TIMER_SIGNUM,
        };
        t->arg=arg;
        t->f=callback;
        if(timer_create(CLOCK_REALTIME,&e,&t->t))
            KLS_freeData(t);
    }
    return t;
}

KLS_byte SYS_timerStart(SYS_t_TIMER timer,unsigned int msDelay,unsigned int msInterval,void(*callback)(SYS_t_TIMER timer,void *arg),void *arg){
    if(timer){
        SYS_timerStop(timer);
        if(arg) timer->arg=arg;
        if(callback) timer->f=callback;
        if(timer->f){
            struct itimerspec i={.it_interval={msInterval/1000,(msInterval%1000)*10000000}, .it_value={msDelay/1000,!msDelay+(msDelay%1000)*10000000}};
            {struct sigaction s={.sa_flags=SA_SIGINFO|SA_NOMASK|SA_RESTART, .sa_sigaction=(void*)_SYS_timerSignal};
            sigaction(_SYS_TIMER_SIGNUM,&s,NULL);}
            timer_settime(timer->t,0,&i,NULL);
            return 1;
        }
    } return 0;
}

#else

SYS_t_TIMER SYS_timerCreate(void(*callback)(SYS_t_TIMER timer,void *arg),void *arg){
    SYS_t_TIMER t=KLS_malloc(sizeof(*t));
    if(t){
        pthread_attr_t a[1];
        struct sigevent s={
            .sigev_value.sival_ptr=t,
            .sigev_notify=SIGEV_THREAD,
            .sigev_notify_function=(void*)_SYS_timerCall,
            .sigev_notify_attributes=(void*)a,
        };
        if(pthread_attr_init(a)) s.sigev_notify_attributes=NULL;
        pthread_attr_setinheritsched(a,PTHREAD_INHERIT_SCHED);
        #ifdef PTHREAD_FPU_ENABLED
        pthread_setfpustate(a,PTHREAD_FPU_ENABLED);
        #endif
        t->arg=arg;
        t->f=callback;
        if(timer_create(CLOCK_REALTIME,&s,&t->t))
            KLS_freeData(t);
        pthread_attr_destroy(a);
    }
    return t;
}

KLS_byte SYS_timerStart(SYS_t_TIMER timer,unsigned int msDelay,unsigned int msInterval,void(*callback)(SYS_t_TIMER timer,void *arg),const void *arg){
    if(timer){
        SYS_timerStop(timer);
        if(arg) memcpy(&timer->arg,&arg,sizeof(arg));
        if(callback) timer->f=callback;
        if(timer->f){
            struct itimerspec i={.it_interval={msInterval/1000,(msInterval%1000)*10000000},.it_value={msDelay/1000,!msDelay+(msDelay%1000)*10000000}};
            timer_settime(timer->t,0,&i,NULL);
            return 1;
        }
    } return 0;
}

#endif

void SYS_timerStop(SYS_t_TIMER timer){
    if(timer){
        struct itimerspec i={{0,0},{0,0}};
        timer_settime(timer->t,0,&i,NULL);
    }
}

void SYS_timerDestroy(SYS_t_TIMER *timer){
    if(timer && *timer){
        timer_delete(timer[0]->t);
        KLS_freeData(*timer);
    }
}