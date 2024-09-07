#ifndef _KLS_OS_DEP_INC
#define _KLS_OS_DEP_INC

#define NOMINMAX
#include <winsock2.h>
#include <windows.h>
#include <imagehlp.h>
#include <ws2tcpip.h>

#include "__KLS_WinApi.h"


KLS_byte KLS_fsDirCreate(const char *directory){
    return directory && !mkdir(directory);
}

const char *KLS_execNameGet(void){
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

unsigned int KLS_sysInfoCores(void){
    SYSTEM_INFO sys; GetSystemInfo(&sys);
    return sys.dwNumberOfProcessors>1 ? sys.dwNumberOfProcessors : 1;
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


#define pthread_kill(_1_,_2_,_sig_) static void _pthread_kill_##_sig_(void){raise(_sig_);}
M_FOREACH(pthread_kill,-,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40)
#undef pthread_kill

static void *_pthread_killFunc(int sig){
#define pthread_kill(_1_,_2_,_sig_) case _sig_:return _pthread_kill_##_sig_;
    switch(sig){
        M_FOREACH(pthread_kill,-,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40)
    } return NULL;
#undef pthread_kill
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
    void *f=_pthread_killFunc(sig);
    if(f){
        CONTEXT c={.ContextFlags=CONTEXT_CONTROL};
        void **x=(void*)_CtxCtrlReg(c), *p=pthread_gethandle(t);
        SuspendThread(p);
        GetThreadContext(p,&c);
        *x=f;
        SetThreadContext(p,&c);
        c.ContextFlags=CONTEXT_ALL;
        GetThreadContext(p,&c);
        ResumeThread(p);
        return 0;
    } return -1;
}

#define pthread_kill pthread_kill_win

#ifndef _KLS_THREAD_SIGNAL_CONTINUE
    #define _KLS_THREAD_SIGNAL_CONTINUE  SIGBREAK
#endif

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

KLS_byte _NET_init(void){WSADATA w; return !WSAStartup(0x0202,&w);}
void _NET_close(void){WSACleanup();}



static int socketpair_tcp(struct addrinfo* addr_info, SOCKET sock[2]){
    SOCKET listener, client, server;
    int opt = 1;

    listener = server = client = INVALID_SOCKET;
    listener = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol); //Create server socket and bind monitoring, etc.
    if (INVALID_SOCKET == listener)
        goto fail;

    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,(const char*)&opt, sizeof(opt));

    if(SOCKET_ERROR == bind(listener, addr_info->ai_addr, addr_info->ai_addrlen))
        goto fail;

    if (SOCKET_ERROR == getsockname(listener, addr_info->ai_addr, (int*)&addr_info->ai_addrlen))
        goto fail;

    if(SOCKET_ERROR == listen(listener, 5))
        goto fail;

    client = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol); //Create a client socket and connect to the server

    if (INVALID_SOCKET == client)
        goto fail;

    if (SOCKET_ERROR == connect(client,addr_info->ai_addr,addr_info->ai_addrlen))
        goto fail;

    server = accept(listener, 0, 0);

    if (INVALID_SOCKET == server)
        goto fail;

    closesocket(listener);

    sock[0] = client;
    sock[1] = server;

    return 0;
fail:
    if(INVALID_SOCKET!=listener)
        closesocket(listener);
    if (INVALID_SOCKET!=client)
        closesocket(client);
    return -1;
}

static int socketpair_udp(struct addrinfo* addr_info,SOCKET sock[2]){
    SOCKET client, server;
    struct addrinfo addr, *result = NULL;
    const char* address;
    int opt = 1;

    server = client = INVALID_SOCKET;

    server = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);  
    if (INVALID_SOCKET == server)
        goto fail;

    setsockopt(server, SOL_SOCKET,SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    if(SOCKET_ERROR == bind(server, addr_info->ai_addr, addr_info->ai_addrlen))
        goto fail;

    if (SOCKET_ERROR == getsockname(server, addr_info->ai_addr, (int*)&addr_info->ai_addrlen))
        goto fail;

    client = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol); 
    if (INVALID_SOCKET == client)
        goto fail;

    memset(&addr,0,sizeof(addr));
    addr.ai_family = addr_info->ai_family;
    addr.ai_socktype = addr_info->ai_socktype;
    addr.ai_protocol = addr_info->ai_protocol;

    if (AF_INET6==addr.ai_family)
        address = "0:0:0:0:0:0:0:1";
    else
        address = "127.0.0.1";

    if (getaddrinfo(address, "0", &addr, &result))
        goto fail;

    setsockopt(client,SOL_SOCKET,SO_REUSEADDR,(const char*)&opt, sizeof(opt));
    if(SOCKET_ERROR == bind(client, result->ai_addr, result->ai_addrlen))
        goto fail;

    if (SOCKET_ERROR == getsockname(client, result->ai_addr, (int*)&result->ai_addrlen))
        goto fail;

    if (SOCKET_ERROR == connect(server, result->ai_addr, result->ai_addrlen))
        goto fail;

    if (SOCKET_ERROR == connect(client, addr_info->ai_addr, addr_info->ai_addrlen))
        goto fail;

    freeaddrinfo(result);
    sock[0] = client;
    sock[1] = server;
    return 0;

fail:
    if (INVALID_SOCKET!=client)
        closesocket(client);
    if (INVALID_SOCKET!=server)
        closesocket(server);
    if (result)
        freeaddrinfo(result);
    return -1;
}

int socketpair(int family,int type,int protocol,SOCKET sock[2]){
    const struct addrinfo cfg={.ai_family=AF_INET, .ai_socktype=type, .ai_protocol=protocol};
    struct addrinfo *info;
    if(getaddrinfo("127.0.0.1","0",&cfg,&info))
        return -1;
    if(type==SOCK_STREAM)
        family=socketpair_tcp(info,sock);
    else if(type==SOCK_DGRAM)
        family=socketpair_udp(info,sock);
    else family=-1;
    freeaddrinfo(info);
    return family;
}

#endif /* _KLS_OS_DEP_INC */
