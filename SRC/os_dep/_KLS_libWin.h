#define NOMINMAX
#include <winsock2.h>
#include <windows.h>
#include <imagehlp.h>
#include <ws2tcpip.h>

#include "__KLS_WinApi.h"


KLS_byte KLS_fsDirCreate(const char *directory){
    return directory && !mkdir(directory);
}

const char *KLS_execNameGet(){
    if(!_KLS_execName){
        char *l=GetCommandLineA(), *a=l;
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
    if(GetDiskFreeSpaceExA(folder, NULL, &t, &f)){
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


#define _KLS_PTHREAD_KILL(_1_,_2_,_sig_) static void _pthread_kill_##_sig_(){raise(_sig_);}
M_FOREACH(_KLS_PTHREAD_KILL,-,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40)
#undef _KLS_PTHREAD_KILL

static void *_pthread_killFunc(int sig){
#define _KLS_PTHREAD_KILL(_1_,_2_,_sig_) case _sig_:return _pthread_kill_##_sig_;
    switch(sig){
        M_FOREACH(_KLS_PTHREAD_KILL,-,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40)
    } return NULL;
#undef _KLS_PTHREAD_KILL
}

#if defined(_M_IX86) || (defined(_X86_) && !defined(__amd64__))
    #define _CtxCtrlReg(_1_)  &(_1_.Eip)
#endif

#if defined (_M_IA64) || defined(_IA64)
    #define _CtxCtrlReg(_1_)  &(_1_.StIIP)
#endif

#if defined(_MIPS_) || defined(MIPS)
    #define _CtxCtrlReg(_1_)  &(_1_.Fir)
#endif

#if defined(_ALPHA_)
    #define _CtxCtrlReg(_1_)  &(_1_.Fir)
#endif

#ifdef _PPC_ 
    #define _CtxCtrlReg(_1_)  &(_1_.Iar)
#endif

#if defined(_AMD64_) || defined(__amd64__)
    #define _CtxCtrlReg(_1_)  &(_1_.Rip)
#endif

#if defined(_ARM_) || defined(ARM) || defined(_M_ARM) || defined(_M_ARM64)
    #define _CtxCtrlReg(_1_)  &(_1_.Pc)
#endif

#ifndef _CtxCtrlReg
static void *_CtxCtrlRegf(CONTEXT *c){
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
#define _CtxCtrlReg(_1_) _CtxCtrlRegf(&_1_)
#endif

static int pthread_kill_win(pthread_t t,int sig){
    void *p=pthread_gethandle(t);
    void *f=_pthread_killFunc(sig);
    if(p && f){
        CONTEXT c={.ContextFlags=CONTEXT_CONTROL};
        void **x=(void*)_CtxCtrlReg(c);
        SuspendThread(p);
        GetThreadContext(p,&c);
        *x=f;
        SetThreadContext(p,&c);
        ResumeThread(p);
        return 0;
    }
    return -1;
}

#define _KLS_PTHREAD_KILL pthread_kill_win

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////  SOCKET  ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef KLS_TYPEOF(socket(0,0,0)) _NET_t_SOCKET;

#define _NET_NOPULSE

#define __NET_ERR2POSIX(_0_,_1_,_2_) if(_1_==M_JOIN(WSAE,_2_)) return M_JOIN(E,_2_);

#define _NET_ERRNO(...)   _NET_errnoTranslate( M_IF(M_IS_ARG(__VA_ARGS__)) ((__VA_ARGS__),WSAGetLastError()) )

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

#define closesocket closesocket

#ifdef POLLIN
    int poll(void *p,int c,int t){return WSApoll(p,c,t);}
#endif


int _NET_errnoTranslate(int e){
    M_FOREACH(__NET_ERR2POSIX,e,NOTSOCK,INTR,TIMEDOUT,OPNOTSUPP,NETUNREACH,DESTADDRREQ,MSGSIZE,PROTOTYPE,ADDRINUSE,ADDRNOTAVAIL,NETDOWN,NETRESET,CONNABORTED,CONNRESET,NOBUFS,ISCONN,NOTCONN,CONNREFUSED,HOSTUNREACH,ALREADY,INPROGRESS,AGAIN,WOULDBLOCK)
    return e;
}

void NET_socketSetBlock(NET_t_SOCKET *s,KLS_byte block){
    block=!!block;
    if(s->created && s->blocked!=block){
        u_long nb=!block;
        ioctlsocket(*(_NET_t_SOCKET*)s->_osDep,FIONBIO,&nb);
        s->blocked=block;
    }
}

unsigned int _NET_recvSize(NET_t_SOCKET *s){
    unsigned long len=0;
    ioctlsocket(*(_NET_t_SOCKET*)s->_osDep,FIONREAD,&len);
    return len;
}

KLS_byte _NET_init(){WSADATA w; return !WSAStartup(0x0202,&w);}
void _NET_close(){WSACleanup();}
