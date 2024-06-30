#include <windows.h>
#include <imagehlp.h>
#include <ws2tcpip.h>
#include <winsock2.h>

#include "__KLS_WinApi.h"

KLS_byte KLS_fsDirCreate(const char *directory){
    return directory && !mkdir(directory);
}

const char *KLS_execNameGet(){
    if(!_KLS_execName){
        char *l=GetCommandLine(), *a=l;
        KLS_execNameSet(l);
        if( (l=strchr(l,' ')) ) *l=0;
        while( (a=strchr(a,'\\')) ) *a='/';
    }
    return _KLS_execName;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////  SYS INFO  //////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

KLS_byte KLS_sysInfoRam(KLS_size *left,KLS_size *all){
    MEMORYSTATUSEX mem={.dwLength = sizeof(MEMORYSTATUSEX)};
    if(GlobalMemoryStatusEx(&mem)){
        if(all) *all=mem.ullTotalPhys;
        if(left) *left=mem.ullAvailPhys;
        return 1;
    } return 0;
}

KLS_byte KLS_sysInfoHdd(const char *folder,KLS_size *left,KLS_size *all){
    ULARGE_INTEGER f,t;
    if(GetDiskFreeSpaceEx(folder, NULL, &t, &f)){
        if(all) *all=t.QuadPart;
        if(left) *left=f.QuadPart;
        return 1;
    } return 0;
}

unsigned int KLS_sysInfoCores(){
    static unsigned int cores=0;
    KLS_ONCE( SYSTEM_INFO sys; GetSystemInfo(&sys); cores=sys.dwNumberOfProcessors; )
    return cores;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////  SIGNAL  ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define sigemptyset(...) (void)0
#define sigaddset(...) (void)0
#define sigismember(...) 0
#ifndef pthread_sigmask
    #define pthread_sigmask(...) (void)0
#endif

#define backtrace(_buffer_,_size_) CaptureStackBackTrace(0,_size_,_buffer_,NULL)

typedef char sigset_t;


#define _KLS_PTHREAD_KILL(_sig_) static void _pthread_kill_##_sig_(){raise(_sig_);}
M_FOREACH(_KLS_PTHREAD_KILL,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40)
#undef _KLS_PTHREAD_KILL

static void *_pthread_killFunc(int sig){
#define _KLS_PTHREAD_KILL(_sig_) case _sig_:return _pthread_kill_##_sig_;
    if(KLS_signalGetString(sig));
        switch(sig){
            M_FOREACH(_KLS_PTHREAD_KILL,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40)
        }
    return NULL;
#undef _KLS_PTHREAD_KILL
}

#define _KLS_PTHREAD_KILL pthread_kill_win

static void *_CtxCtrlReg(CONTEXT *c){
    static unsigned int offset=-1;
    if(offset==-1){
        const uintptr_t * const end=(void*)(c+1), f=(uintptr_t)GetThreadContext;
        uintptr_t diff, min=0, *p=(void*)c;
        GetThreadContext(GetCurrentThread(),c);
        for(c->ContextFlags=0,min=~min;p!=end;++p)
            if((diff=(*p>f) ? (*p-f) : (f-*p))<min){
                min=diff; offset=(char*)p-(char*)c;
            }
        c->ContextFlags=CONTEXT_CONTROL;
    } return (char*)c+offset;
}

static int pthread_kill_win(pthread_t t,int sig){
    void *p=pthread_gethandle(t);
    void *f=_pthread_killFunc(sig);
    if(p && f){
        CONTEXT c={.ContextFlags=CONTEXT_CONTROL};
        void **x=_CtxCtrlReg(&c);
        SuspendThread(p);
        GetThreadContext(p,&c);
        *x=f;
        SetThreadContext(p,&c);
        ResumeThread(p);
        return 0;
    }
    return -1;
}
/*
int pthread_kill_win(pthread_t t,int sig){
    void *p=pthread_gethandle(t);
    void *f=_pthread_killFunc(sig);
    if(p && f){
        #ifndef _CMDREG
            #ifdef _ALPHA_
                #define _CMDREG Fir
            #endif
        #endif
        #ifndef _CMDREG
            #ifdef _ARM_
                #define _CMDREG Pc
            #endif
        #endif
        #ifndef _CMDREG
            #if(KLS_SYS_BITNESS==64)
                #define _CMDREG Rip
            #else
                #define _CMDREG Eip
            #endif
        #endif
        #ifdef _CMDREG
            CONTEXT c={.ContextFlags=CONTEXT_CONTROL};
            SuspendThread(p);
            GetThreadContext(p,&c);
            c._CMDREG=(uintptr_t)f;
            SetThreadContext(p,&c);
            ResumeThread(p);
            return 0;
        #endif
    }
    return -1;
}
*/


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////  SOCKET  ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef KLS_TYPEOF(socket(0,0,0)) _NET_t_SOCKET;

#define _NET_BAD_OP  SOCKET_ERROR
#define _NET_BAD_FD  INVALID_SOCKET

#define _NET_NOPULSE

#define _NET_ERR2POSIX(_1_) M_FOREACH_A(__NET_ERR2POSIX,_1_,NOTSOCK,INTR,TIMEDOUT,OPNOTSUPP,NETUNREACH,DESTADDRREQ,MSGSIZE,PROTOTYPE,ADDRINUSE,ADDRNOTAVAIL,NETDOWN,NETRESET,CONNABORTED,CONNRESET,NOBUFS,ISCONN,NOTCONN,CONNREFUSED,HOSTUNREACH,ALREADY,INPROGRESS,AGAIN,WOULDBLOCK)
#define __NET_ERR2POSIX(_1_,_2_) if(_1_==M_JOIN(WSAE,_2_)) return M_JOIN(E,_2_);

#define _NET_FUNC_DEF(_f_,...) ({KLS_TYPEOF(_f_(__VA_ARGS__)) _1_=_f_(__VA_ARGS__);   errno=WSAGetLastError(); _1_;})

#define SHUT_RD    SD_RECEIVE
#define SHUT_WR    SD_SEND
#define SHUT_RDWR  SD_BOTH

#ifndef EAGAIN
    #define EAGAIN EINPROGRESS
#endif

#ifndef EWOULDBLOCK
    #define EWOULDBLOCK EAGAIN
#endif

#ifndef WSAEAGAIN
    #define WSAEAGAIN EAGAIN
#endif


#ifdef POLLIOUT
    #define poll(...) _NET_FUNC_DEF(WSApoll,__VA_ARGS__)
#else
    #define _NET_POLL_BY_SELECT
#endif

_NET_t_SOCKET *_NET_sockOs(NET_t_SOCKET *s){return (void*)s->_osDep;}

void _NET_sockClose(NET_t_SOCKET *s){closesocket(*_NET_sockOs(s));}

void NET_socketSetBlock(NET_t_SOCKET *s,KLS_byte block){
    block=!!block;
    if(s->created && s->blocked!=block){
        u_long nb=!block;
        ioctlsocket(*_NET_sockOs(s),FIONBIO,&nb);
        s->blocked=block;
    }
}

unsigned int _NET_recvSize(NET_t_SOCKET *s){
    unsigned long len=0;
    ioctlsocket(*_NET_sockOs(s),FIONREAD,&len);
    return len;
}

KLS_byte _NET_init(){WSADATA w; return !WSAStartup(0x0202,&w);}
void _NET_close(){WSACleanup();}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////  TIMER  ////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct _SYS_t_TIMER{
    HANDLE t;
    void (*f)(SYS_t_TIMER timer,void *arg);
    void *arg;
    char run;
};

void _SYS_timerCall(SYS_t_TIMER t,char go){
    if(t->run) t->f(t,t->arg);
    if(t->run & 1) t->run=0;
}

#define _SYS_TIMER_MAXT (~(unsigned int)0)

SYS_t_TIMER SYS_timerCreate(void(*callback)(SYS_t_TIMER timer,void *arg),void *arg){
    SYS_t_TIMER t=KLS_malloc(sizeof(*t));
    if(t){
        memset(t,0,sizeof(*t));
        t->arg=arg;
        t->f=callback;
        if(!CreateTimerQueueTimer(&t->t,NULL,(void*)_SYS_timerCall,t,_SYS_TIMER_MAXT,_SYS_TIMER_MAXT,0))
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
            if( (timer->run=2-!msInterval) & 1 )
                msInterval=_SYS_TIMER_MAXT;
            return ChangeTimerQueueTimer(NULL,timer->t,msDelay,msInterval);
        }
    } return 0;
}

void SYS_timerStop(SYS_t_TIMER timer){
    if(timer){
        timer->run=0;
        ChangeTimerQueueTimer(NULL,timer->t,_SYS_TIMER_MAXT,_SYS_TIMER_MAXT);
    }
}

void SYS_timerDestroy(SYS_t_TIMER *timer){
    if(timer && *timer){
        DeleteTimerQueueTimer(0,timer[0]->t,0);
        KLS_freeData(*timer);
    }
}
