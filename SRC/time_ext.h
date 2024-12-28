/* * * * * * * * * * * * * * * * * */
/* MIT License                     */
/* Copyright (c) 2024 ANSI-Christ  */
/* * * * * * * * * * * * * * * * * */

#ifndef TIME_EXT_H
#define TIME_EXT_H

#include <time.h>
#include <pthread.h>



struct datetime{
    int year;
    unsigned short doy;   /* 1 - 366 */
    unsigned char month;  /* 1 - 12  */
    unsigned char day;    /* 1 - 31  */
    unsigned char dow;    /* 1 - 7   */
    unsigned char hour;   /* 0 - 23  */
    unsigned char minute; /* 0 - 59  */
    unsigned char second; /* 0 - 59  */
};

int timezone_current(void);

int datetime_cmp(const struct datetime *dt1,const struct datetime *dt2);

void datetime_from_epoch(struct datetime *dt,time_t t);

time_t datetime_to_epoch(const struct datetime *dt);

/*format: Y=year, M=month, D=day of month, W=day of week str, w=day of week, y=day of year, h=hour, m=minute, s=second */
char *datetime_string(const struct datetime * const dt,const char *format,char buffer[],unsigned int buffer_size);





struct timer {void *_[4]; char __[sizeof(struct timespec)+sizeof(pthread_t)+sizeof(int)*2];};

int timer_init(struct timer *t,void(*callback)(void *arg,unsigned int *interval_ms,pthread_t tid),void *arg);
int timer_start(struct timer *timer,unsigned int delay_ms,unsigned int interval_ms,void(*callback)(void *arg,unsigned int *interval_ms,pthread_t tid),void *arg);

void timer_stop(struct timer *t);
void timer_close(struct timer *t);




int timespec_sign(const struct timespec *t);
int timespec_cmp(const struct timespec *t1,const struct timespec *t2);

void timespec_current(struct timespec *t);
void timespec_normalize(struct timespec *t);
void timespec_runtime(struct timespec *t,...);
void timespec_change(struct timespec *t,long sec,long nanosec);

double timespec_seconds(const struct timespec *t);
double timespec_milliseconds(const struct timespec *t);
double timespec_microseconds(const struct timespec *t);
double timespec_nanoseconds(const struct timespec *t);




extern int nanosleep(const struct timespec*,struct timespec*);
#define timespec_current(_t_) clock_gettime(CLOCK_REALTIME,(_t_))
#define timespec_runtime(_t_,...) do{\
    struct timespec _rt1[1], * const _rt2=(_t_);\
    timespec_current(_rt1); {__VA_ARGS__} timespec_current(_rt2);\
    timespec_change(_rt2,-_rt1->tv_sec,-_rt1->tv_nsec);\
}while(0)
#define timer_init(_t_,_f_,_a_) _timer_init((_t_),(void*)(_f_),(_a_))
#define timer_start(_t_,_d_,_i_,_f_,_a_) _timer_start((_t_),(_d_),(_i_),(void*)(_f_),(_a_))
extern int _timer_init(void*,void*,const void*);
extern int _timer_start(void*,unsigned int,unsigned int,void*,const void*);

#endif /* TIME_EXT_H */




#ifdef TIME_EXT_IMPL

#include <errno.h>
#include <stdio.h>

#ifdef __WIN32

#define NOMINMAX
#include <windows.h>

int timezone_current(void){
    _tzset(); return -(int)timezone;
}

#else

int timezone_current(void){
    tzset(); return -(int)timezone;
}

#endif

void timespec_normalize(struct timespec * const t){
    t->tv_sec+=t->tv_nsec/1000000000;
    if( (t->tv_nsec%=1000000000)<0){
        t->tv_nsec += 1000000000;
        --t->tv_sec;
    }
}

void timespec_change(struct timespec * const t,const long sec,const long nanosec){
    t->tv_sec+=sec;
    t->tv_nsec+=nanosec;
    timespec_normalize(t);
}

int timespec_sign(const struct timespec * const t){
    const union{float d;int u;}x={(float)timespec_seconds(t)};
    return x.u;
}

int timespec_cmp(const struct timespec * const t1,const struct timespec * const t2){
    const struct timespec t={t1->tv_sec-t2->tv_sec,t1->tv_nsec-t2->tv_nsec};
    return timespec_sign(&t);
}

double timespec_seconds(const struct timespec * const t){
    return t->tv_sec + t->tv_nsec/1000000000.;
}

double timespec_milliseconds(const struct timespec *t){
    return t->tv_sec*1000. + t->tv_nsec/1000000.;
}

double timespec_microseconds(const struct timespec *t){
    return t->tv_sec*1000000. + t->tv_nsec/1000.;
}

double timespec_nanoseconds(const struct timespec *t){
    return t->tv_sec*1000000000 + t->tv_nsec;
}


void datetime_from_epoch(struct datetime * const dt,time_t t){
    dt->second=t%60;  t/=60;
    dt->minute=t%60;  t/=60;
    dt->hour=t%24;    t/=24;
    if( !(dt->dow=(t > -5 ? (t+4) % 7 : (t+5) % 7 + 6)) )
        dt->dow=7;
    {
        const int era = ((t += 719468) >= 0 ? t : t - 146096) / 146097;
        const unsigned doe = (t - era * 146097);
        const unsigned yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;
        const unsigned doy = doe - (365*yoe + yoe/4 - yoe/100);
        const unsigned mp = (5*doy + 2)/153;
        const unsigned d = doy - (153*mp+2)/5 + 1;
        const unsigned m = mp + (mp < 10 ? 3 : -9);
        const int y = (int)yoe + era * 400 + (m < 3);
        dt->year=y;
        dt->month=m;
        dt->day=d;
        dt->doy=doy + ( m<3 ? -305 : 60 + ((y&3)==0 && (y%100!=0 || y%400==0)) );
    }
}

time_t datetime_to_epoch(const struct datetime * const dt){
    const unsigned char m=dt->month, d=dt->day;
    const int y=dt->year-(m<3);
    const int era = (y > -1 ? y : y-399) / 400;
    const unsigned int yoe = y - era * 400;
    const unsigned int doy = (153*(m + (m > 2 ? -3 : 9)) + 2)/5 + d-1;
    const int doe = yoe * 365 + yoe/4 - yoe/100 + doy;
    time_t t = era * 146097 + doe - 719468;
    t *= 3600*24; t += 3600*dt->hour + 60*dt->minute + dt->second;
    return t;
}

int datetime_cmp(const struct datetime * const dt1,const struct datetime * const dt2){
    const time_t t1=datetime_to_epoch(dt1), t2=datetime_to_epoch(dt2);
    if(t1<t2) return -1;
    return t1>t2;
}

char *datetime_string(const struct datetime * const dt,const char *format,char buffer[],unsigned int buffer_size){
    int l;
    char *p=buffer;
    if(--buffer_size)
        while(buffer_size){
            switch(*(format++)){
                case 0: *p=0; return buffer;
                case 'Y': l=snprintf(p,buffer_size,"%u",dt->year); break;
                case 'M': l=snprintf(p,buffer_size,"%0*u",2,dt->month); break;
                case 'D': l=snprintf(p,buffer_size,"%0*u",2,dt->day); break;
                case 'W':{
                    const char *da[7]={"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
                    l=snprintf(p,buffer_size,"%s",da[dt->dow]);
                } break;
                case 'h': l=snprintf(p,buffer_size,"%0*u",2,dt->hour); break;
                case 'm': l=snprintf(p,buffer_size,"%0*u",2,dt->minute); break;
                case 's': l=snprintf(p,buffer_size,"%0*u",2,dt->second); break;
                case 'w': l=snprintf(p,buffer_size,"%u",dt->dow); break;
                case 'y': l=snprintf(p,buffer_size,"%0*u",3,dt->doy); break;
                default:  l=1; *p=format[-1]; break;
            }
            if(l<1) break;
            p+=l; buffer_size-=l;
        }
    *p=0;
    return buffer;
}

typedef struct _timer_struct_t{
    struct _timer_struct_t *prev, *next;
    void *f;
    const void *arg;
    struct timespec t;
    unsigned int i;
    char run,init;
    pthread_t tid;
} * const _timer_cptr_t;

static struct _global_timer_t{
    pthread_mutex_t mtx[1];
    pthread_cond_t cond[1];
    struct _timer_struct_t *first, *last;
    struct timespec t;
}_timer_global={{PTHREAD_MUTEX_INITIALIZER}};

#define TIMER_GLOBAL_LINK(_name_) struct _global_timer_t * const _name_=&_timer_global

static void _timer_link(_timer_cptr_t t){
    TIMER_GLOBAL_LINK(g);
    if(g->last){
        g->last->next=t;
        t->prev=g->last;
    }else g->first=t;
    g->last=t;
}

static void _timer_unlink(_timer_cptr_t t){
    TIMER_GLOBAL_LINK(g);
    if(t->prev) t->prev->next=t->next;
    else g->first=t->next;
    if(t->next) t->next->prev=t->prev;
    else g->last=t->prev;
}

static void _timer_link_end(_timer_cptr_t t){
    TIMER_GLOBAL_LINK(g);
    if(t!=g->last){
        _timer_unlink(t);
        g->last->next=t;
        t->prev=g->last;
        g->last=t;
        t->next=(_timer_cptr_t)0;
    }
}

static void _timer_link_beg(_timer_cptr_t t){
    TIMER_GLOBAL_LINK(g);
    if(t!=g->first){
        _timer_unlink(t);
        g->first->prev=t;
        t->next=g->first;
        g->first=t;
        t->prev=(_timer_cptr_t)0;
    }
}

static void _timer_sched(void){
    const pthread_t tid=pthread_self();
    struct sched_param pri; int pol;
    if(!pthread_getschedparam(tid,&pol,&pri)){
        pri.sched_priority=99;
        pthread_setschedparam(tid,pol,&pri);
    }
}

static void *_timer_thread_worker(void *arg){
    TIMER_GLOBAL_LINK(g);
    struct timespec t;
    struct _timer_struct_t *timer;
    _timer_sched();

    while('0'){
        pthread_mutex_lock(g->mtx);
_mark:
        t=g->t;
        switch(pthread_cond_timedwait(g->cond,g->mtx,&t)){
            case -1: if(errno==EINTR) goto _mark; break;
            case 0: if(g->first) goto _mark; break;
            case EINTR: goto _mark;
        }
        if(!(timer=g->first)){
            g->t.tv_sec=0;
            pthread_cond_destroy(g->cond);
            pthread_mutex_unlock(g->mtx);
            break;
        }
        timespec_current(&t);
        g->t.tv_sec=t.tv_sec+3600;
        while(timer && timer->run){
            if(timespec_cmp(&timer->t,&t)<1){
                ((void(*)(const void*,unsigned int*,pthread_t))(timer->f))(timer->arg,&timer->i,timer->tid);
                if(!timer->i){
                    _timer_cptr_t next=timer->next;
                    timer->run=0;
                    _timer_link_end(timer);
                    timer=next;
                    continue;
                }
                timer->t=t;
                timespec_change(&timer->t,timer->i/1000,(timer->i%1000)*1000000);
            }
            if(timespec_cmp(&timer->t,&g->t)<0)
                g->t=timer->t;
            timer=timer->next;
        }
        pthread_mutex_unlock(g->mtx);
    }
    return NULL;
    (void)arg;
}

static char _timer_thread_init(void){
    TIMER_GLOBAL_LINK(g);
    if(!g->t.tv_sec){
        pthread_attr_t a[1];
        pthread_t tid[1];
        do{
            if(pthread_attr_init(a)) break;
            if(pthread_cond_init(g->cond,NULL)) break;
            if(pthread_attr_setstacksize(a,20<<10)) break;
            if(pthread_attr_setdetachstate(a,PTHREAD_CREATE_DETACHED)) break;
            if(pthread_attr_setinheritsched(a,PTHREAD_INHERIT_SCHED)) break;
            #ifdef PTHREAD_FPU_ENABLED
            if(pthread_setfpustate(a,PTHREAD_FPU_ENABLED)) break;
            #endif
            if(!pthread_create(tid,a,_timer_thread_worker,NULL))
                g->t.tv_sec=time(NULL)+3600;
        }while(0);
        pthread_attr_destroy(a);
        if(!g->t.tv_sec) pthread_cond_destroy(g->cond);
    } return g->t.tv_sec>0;
}

int _timer_init(void * const timer,void * const f,const void * const arg){
    _timer_cptr_t t=(_timer_cptr_t)timer;
    TIMER_GLOBAL_LINK(g);
    t->prev=t->next=(_timer_cptr_t)0;
    t->f=f;
    t->arg=arg;
    t->run=t->init=0;
    pthread_mutex_lock(g->mtx);
    if(_timer_thread_init()){
        _timer_link(t);
        t->init=1;
    }
    pthread_mutex_unlock(g->mtx);
    return t->init-1;
}

int _timer_start(void * const timer,const unsigned int delay,const unsigned int interval,void *f,const void * const arg){
    _timer_cptr_t t=(_timer_cptr_t)timer;
    if(t->init && (delay|interval) && (f || t->f)){
        TIMER_GLOBAL_LINK(g);
        const pthread_t tid=pthread_self();
        pthread_mutex_lock(g->mtx);
        if(arg) t->arg=arg;
        if(f) t->f=f;
        if(!t->run){
            t->run=1;
            _timer_link_beg(t);
        }
        t->tid=tid;
        t->i=interval;
        timespec_current(&t->t);
        timespec_change(&t->t,delay/1000,(delay%1000)*1000000);
        if(timespec_cmp(&t->t,&g->t)<0){
            g->t=t->t;
            pthread_cond_signal(g->cond);
        }
        pthread_mutex_unlock(g->mtx);
        return 0;
    } return -1;
}

void timer_stop(struct timer *timer){
    _timer_cptr_t t=(_timer_cptr_t)timer;
    if(t->init && t->run){
        TIMER_GLOBAL_LINK(g);
        pthread_mutex_lock(g->mtx);
        t->run=0;
        _timer_link_end(t);
        pthread_mutex_unlock(g->mtx);
    }
}


int timer_continue(struct timer *timer){
    _timer_cptr_t t=(_timer_cptr_t)timer;
    if(t->init && !t->run && t->f){
        TIMER_GLOBAL_LINK(g);
        pthread_mutex_lock(g->mtx);
        t->run=1;
        _timer_link_beg(t);
        if(timespec_cmp(&t->t,&g->t)<0){
            g->t=t->t;
            pthread_cond_signal(g->cond);
        }
        pthread_mutex_unlock(g->mtx);
        return 0;
    } return -1;
}

void timer_close(struct timer *timer){
    _timer_cptr_t t=(_timer_cptr_t)timer;
    if(t->init){
        TIMER_GLOBAL_LINK(g);
        _timer_cptr_t t=(_timer_cptr_t)timer;
        pthread_mutex_lock(g->mtx);
        _timer_unlink(t);
        if(!g->first) pthread_cond_signal(g->cond);
        pthread_mutex_unlock(g->mtx);
        t->init=0;
    }
}

#undef TIMER_GLOBAL_LINK

#endif
