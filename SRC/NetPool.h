/* * * * * * * * * * * * * * * * * */
/* MIT License                     */
/* Copyright (c) 2024 ANSI-Christ  */
/* * * * * * * * * * * * * * * * * */

#ifndef NETPOOL_H
#define NETPOOL_H

#include <stddef.h>

enum NET_PROTOCOL{
    NET_TCP = 1,
    NET_UDP
};

enum NET_EVENT{
    NET_DISCONNECT = 1,
    NET_CONNECT,        /* not for listeners */
    NET_CANWRITE,
    NET_CANREAD,
    NET_TIMEOUT,
    NET_ACCEPT,         /* only for listeners */
    NET_ERROR
};

enum NET_ENDIAN{
    NET_LTL = (1<<0),
    NET_PDP = (1<<16),
    NET_BIG = (1<<24)
};

#define NET_ANY4        "0.0.0.0"
#define NET_ANY6        "::"
#define NET_LOCAL4      "127.0.0.1"
#define NET_LOCAL6      "::1"



typedef struct NetPool NetPool;

typedef struct NetUnit NetUnit;

struct NetUnit{
    void(*handler)(NetUnit *unit,enum NET_EVENT event);
    union{
        void *ptr;
        const void *cptr;
    }data;
    unsigned int timeout;
    unsigned int pulse;   /* default: 20 */
};

typedef struct{
    union{
        unsigned char v4[4];
        unsigned short v6[8];
    }ip;
    unsigned short port;
    unsigned char ipv;  /* 4 or 6 */
}NetAddress;




char *NetAddressString(const NetAddress *address,char buffer[static 46]);

NetAddress *NetAddressDns(const char *host,unsigned short port,NetAddress *address);
NetAddress *NetAddressNumeric(const char *ip,unsigned short port,NetAddress *address);




int NetPoolEmit(NetPool *pool,int value);
int NetPoolDispatch(NetPool *pool,int *emit); /* return 0 on emit or [EINVAL,EINTR,...] */

void NetPoolDestroy(NetPool *pool);

NetPool *NetPoolCreate(void);
NetPool *NetPoolCreateEx(unsigned int base_count,unsigned int reserv_count,void*(*allocator)(size_t),void(*deallocator)(void*));

NetUnit *NetPoolUnit(NetPool *pool,enum NET_PROTOCOL protocol);




int NetUnitListen(NetUnit *unit,const NetAddress *address);
int NetUnitConnect(NetUnit *unit,const NetAddress *address);
int NetUnitRead(NetUnit *unit,void *buffer,unsigned int size,NetAddress *address);
int NetUnitWrite(NetUnit *unit,const void *data,unsigned int size,const NetAddress *address);

void NetUnitDisconnect(NetUnit *unit);
void NetUnitAutoRemove(NetUnit *unit);

short *NetUnitRDWR(const NetUnit *unit);
extern const short NET_RD;
extern const short NET_WR;

NetPool *NetUnitPool(const NetUnit *unit);

NetUnit *NetUnitNodeNext(const NetUnit *unit);
NetUnit *NetUnitNodeServer(const NetUnit *unit);

const NetAddress *NetUnitAddress(const NetUnit *unit);



const char *NetEventString(enum NET_EVENT event);


void NetViewNet(void *base_type_pointer);
void NetViewHost(void *base_type_pointer);

enum NET_ENDIAN NetEndian(void);




#define NetEndian() ((const union{unsigned char _; enum NET_ENDIAN e;}){1}).e
#define NetViewHost(_p_) NetViewNet(_p_)
#define NetViewNet(_p_) do{\
    switch(NetEndian()){\
        case NET_LTL: break;\
        case NET_BIG:\
            switch(sizeof(*(_p_))){\
                case 1:  break;\
                case 2:  {unsigned char * const _1_=(unsigned char *)(_p_),_2_; _NetViewSwap(1,0);} break;\
                case 4:  {unsigned char * const _1_=(unsigned char *)(_p_),_2_; _NetViewSwap(3,0),  _NetViewSwap(2,1);} break;\
                case 8:  {unsigned char * const _1_=(unsigned char *)(_p_),_2_; _NetViewSwap(7,0),  _NetViewSwap(6,1),  _NetViewSwap(5,2),  _NetViewSwap(4,3);} break;\
                case 12: {unsigned char * const _1_=(unsigned char *)(_p_),_2_; _NetViewSwap(11,0), _NetViewSwap(10,1), _NetViewSwap(9,2),  _NetViewSwap(8,3),  _NetViewSwap(7,4),  _NetViewSwap(6,5);} break;\
                case 16: {unsigned char * const _1_=(unsigned char *)(_p_),_2_; _NetViewSwap(15,0), _NetViewSwap(14,1), _NetViewSwap(13,2), _NetViewSwap(12,3), _NetViewSwap(11,4), _NetViewSwap(10,5), _NetViewSwap(9,6), _NetViewSwap(8,7);} break;\
            } break;\
        case NET_PDP:\
            switch(sizeof(*(_p_))){\
                case 1: case 2: break;\
                case 4: {unsigned short * const _1_=(unsigned short *)(_p_),_2_; _NetViewSwap(1,0);} break;\
                case 8: case 12: case 16: break;\
            } break;\
        default: break;  (_p_)[0]+=1;\
    }\
}while(0)
#define _NetViewSwap(_i_,_j_) _2_=_1_[_i_], _1_[_i_]=_1_[_j_], _1_[_j_]=_2_

#endif /* NETPOOL_H */






#ifdef NETPOOL_IMPL

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#ifdef __WIN32

#define FD_SETSIZE 1024
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#undef NOMINMAX

#ifndef EAGAIN
    #define EAGAIN EINPROGRESS
#endif

#ifndef EWOULDBLOCK
    #define EWOULDBLOCK EAGAIN
#endif

#ifndef WSAEAGAIN
    #define WSAEAGAIN EAGAIN
#endif


static int _NetConfig(const char on){
    if(on){
        WSADATA w;
        return WSAStartup(MAKEWORD(2,2),&w);
    } WSACleanup(); return 0;
}

static int _NetErrnoTranslate(const int e){
#define case(_name_) if(e==WSAE##_name_) return E##_name_
    case(NOTSOCK); case(INTR); case(TIMEDOUT); case(OPNOTSUPP); case(NETUNREACH); case(DESTADDRREQ); case(MSGSIZE);
    case(PROTOTYPE); case(ADDRINUSE); case(ADDRNOTAVAIL); case(NETDOWN); case(NETRESET); case(CONNABORTED); case(CONNRESET);
    case(NOBUFS); case(ISCONN); case(NOTCONN); case(CONNREFUSED); case(HOSTUNREACH); case(ALREADY); case(INPROGRESS); case(AGAIN); case(WOULDBLOCK);
    return e;
#undef case
}

#define _NET_LAST_ERROR() (errno=_NetErrnoTranslate(WSAGetLastError()))

#ifdef POLLIN
static int poll(struct pollfd * const p,const int c,const int t){
    return WSApoll(p,c,t);
}
#endif

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

    if (getaddrinfo(address, "0", &addr, &result)){
        WSASetLastError(EFAULT);
        goto fail;
    }

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

static int socketpair(int family,int type,int protocol,SOCKET sock[2]){
    const struct addrinfo cfg={.ai_family=AF_INET, .ai_socktype=type, .ai_protocol=protocol};
    struct addrinfo *info;
    const int err=getaddrinfo("127.0.0.1","0",&cfg,&info);
    if(err){WSASetLastError(EFAULT);return -1;}
    if(type==SOCK_DGRAM)
        family=socketpair_udp(info,sock);
    else family=-1;
    freeaddrinfo(info);
    return family;
}

#ifdef POLLIN
static int poll(struct pollfd * const p,const int c,const int t){
    const int count=WSApoll(p,c,t);
    if(count<0) _NET_LAST_ERROR();
    return count;
}
#endif

typedef SOCKET NetSocket;

static void NetSocketDestroy(NetSocket * const s){
    shutdown(*s,SD_BOTH); closesocket(*s); *s=INVALID_SOCKET;
}

static int NetSocketPair(NetSocket s[2]){
    if(socketpair(AF_INET,SOCK_DGRAM,0,s)){
        _NET_LAST_ERROR(); return -1;
    } return 0;
}

static void NetSocketUnblock(NetSocket s){
    const u_long opt=1;
    ioctlsocket(s,FIONBIO,&opt);
}

static int NetSocketError(NetSocket s){
    int e=0;
    unsigned int l=sizeof(e);
    if(getsockopt(s,SOL_SOCKET,SO_ERROR,&e,&l))
        return -1;
    return _NetErrnoTranslate(e);
}

#else /* end __WIN32 */

#include <poll.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

static int _NetConfig(const char on){return 0;(void)on;}

typedef int NetSocket;
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

#define _NET_LAST_ERROR() errno

static void NetSocketDestroy(NetSocket * const s){
    shutdown(*s,SHUT_RDWR); close(*s); *s=INVALID_SOCKET;
}

static int NetSocketPair(NetSocket s[2]){
    return socketpair(AF_UNIX,SOCK_DGRAM,0,s);
}

static void NetSocketUnblock(NetSocket s){
    fcntl(s,F_SETFL, (O_NONBLOCK | fcntl(s,F_GETFL)) );
}

static int NetSocketError(NetSocket s){
    int e=0;
    unsigned int l=sizeof(e);
    if(getsockopt(s,SOL_SOCKET,SO_ERROR,&e,&l))
        return -1;
    return e;
}

#endif /* end else __WIN32*/

#ifndef MSG_NOSIGNAL
    #define MSG_NOSIGNAL 0
#endif

#ifndef POLLIN

#define POLLIN 1
#define POLLOUT 2
#define POLLERR 4
#define POLLNVAL 8
#define POLLHUP 16

struct pollfd{
    NetSocket fd;
    short events,revents;
};

static int poll(struct pollfd * const p,int cnt,const int timeout){
    int i,s;
    struct timeval t={timeout/1000,1000*(timeout%1000)};
    NetSocket max=p->fd;
    fd_set set[3];
    FD_ZERO(set);FD_ZERO(set+1);FD_ZERO(set+2);
    for(i=0;i<cnt;++i){
        p[i].revents=0;
        if(p[i].events & POLLIN) FD_SET(p[i].fd,set);
        if(p[i].events & POLLOUT) FD_SET(p[i].fd,set+1);
        FD_SET(p[i].fd,set+2);
        if(p[i].fd>max) max=p[i].fd;
    }
    if( (s=select(max+1,set,set+1,set+2,timeout<0?NULL:&t))!=SOCKET_ERROR && s>0)
        for(i=0;i<cnt;++i){
            if(FD_ISSET(p[i].fd,set)){
                char tmp;
                switch(recv(p[i].fd,&tmp,1,MSG_PEEK|MSG_NOSIGNAL)){
                    case 1: p[i].revents|=POLLIN; break;
                    default: p[i].revents|=POLLIN|POLLHUP; break;
                }
            }
            if(FD_ISSET(p[i].fd,set+1)) p[i].revents|=POLLOUT;
            if(FD_ISSET(p[i].fd,set+2)) p[i].revents|=POLLERR;
        }
    return s;
}

#endif

#ifndef AF_INET6
    #define AF_INET6 -1
    struct sockaddr_in6{
        __typeof__( ((struct sockaddr_in*)0)->sin_family ) sin6_family;
        unsigned short sin6_port;
        int sin6_addr[4];
    }
#endif

const short NET_WR=POLLOUT;
const short NET_RD=POLLIN|POLLHUP;

typedef union{
    int _;
    struct sockaddr sa;
    struct sockaddr_in a4;
    struct sockaddr_in6 a6;
}_NetAddress;

typedef union{
    struct sockaddr sa;
    struct sockaddr_in a4;
    struct sockaddr_in6 a6;
    struct sockaddr_storage st;
}_NetAddressStorage;


static int NetAddressFromNet(const _NetAddressStorage * const in,const unsigned int l,NetAddress * const out){
    if(in->st.ss_family==AF_INET || (in->st.ss_family==AF_UNSPEC && l==sizeof(in->a4)) ){
        memcpy(out,&in->a4.sin_addr,sizeof(in->a4.sin_addr));
        out->port=ntohs(in->a4.sin_port); out->ipv=4;
        return 0;
    }
    if(in->st.ss_family==AF_INET6 || (in->st.ss_family==AF_UNSPEC && l==sizeof(in->a6)) ){
        memcpy(out,&in->a6.sin6_addr,sizeof(in->a6.sin6_addr));
        out->port=ntohs(in->a6.sin6_port); out->ipv=6;
        return 0;
    }
    return -1;
}

static unsigned int NetAddressToNet(const NetAddress * const in,_NetAddress * const out){
    switch(in->ipv){
        case 4:
            memcpy(&out->a4.sin_addr,in,sizeof(out->a4.sin_addr));
            out->a4.sin_family=AF_INET; out->a4.sin_port=htons(in->port);
            return sizeof(out->a4);
        case 6:
            memcpy(&out->a6.sin6_addr,in,sizeof(out->a6.sin6_addr));
            out->a6.sin6_family=AF_INET6; out->a6.sin6_port=htons(in->port);
            return sizeof(out->a6);
    }
    return 0;
}

static char NetAddressFind(const char * const host,NetAddress * const out,const int flags){
    const struct addrinfo in={.ai_flags=flags, .ai_family=AF_UNSPEC};
    struct addrinfo *i, *info=NULL;
    if(getaddrinfo(host,NULL,&in,&info)) return 0;
    for(i=info;i && NetAddressFromNet((_NetAddressStorage*)i->ai_addr,i->ai_addrlen,out);i=i->ai_next);
    freeaddrinfo(info);
    return out->ipv;
}

static const char *NetAddressDomain(const char *address,char *domain){
    const char *begin,*end;
    if((begin=strstr(address,"://"))) begin+=3; else begin=address;
    if(!(end=strstr(begin,"/"))) end=begin+strlen(begin);
    memcpy(domain,begin,end-begin);
    domain[end-begin]=0;
    return domain;
}

NetAddress *NetAddressDns(const char * const host,const unsigned short port,NetAddress * const address){
    char tmp[128];
    if(NetAddressFind(NetAddressDomain(host,tmp),address,0)){
        address->port=port;
        return address;
    }
    address->ipv=0;
    errno=EADDRNOTAVAIL;
    return NULL;
}

NetAddress *NetAddressNumeric(const char * const ip,const unsigned short port,NetAddress * const address){
    if(NetAddressFind(ip,address,AI_NUMERICHOST)){
        address->port=port;
        return address;
    }
    address->ipv=0;
    errno=EADDRNOTAVAIL;
    return NULL;
}

char *NetAddressString(const NetAddress * const address,char name[static 46]){
    switch(address->ipv){
        case 4:{
            const unsigned char * const p=address->ip.v4;
            sprintf(name,"%d.%d.%d.%d:%d",p[0],p[1],p[2],p[3],address->port);
        } return name;
        case 6:{
            const unsigned short * const p=address->ip.v6;
            sprintf(name,"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%d",ntohs(p[0]),ntohs(p[1]),ntohs(p[2]),ntohs(p[3]),ntohs(p[4]),ntohs(p[5]),ntohs(p[6]),ntohs(p[7]),address->port);
        } return name;
    }
    *name=0;
    return NULL;
}


static NetSocket NetSocketCreate(const unsigned char p,const unsigned char v){
    NetSocket s=socket((v==6?AF_INET6:AF_INET),(p==NET_TCP?SOCK_STREAM:SOCK_DGRAM),0);
    if(s!=INVALID_SOCKET){
        const int opt1=1;
        if(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&opt1,sizeof(opt1))){
            const char opt2=1; setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&opt2,sizeof(opt2));
        }
        NetSocketUnblock(s);
    } return s;
}

static NetSocket NetSocketAccept(NetSocket in,NetAddress * const a){
    _NetAddressStorage _a;
    unsigned int l=sizeof(_a);
    NetSocket s=accept(in,&_a.sa,&l);
    if(s!=INVALID_SOCKET){
        if(NetAddressFromNet(&_a,l,a)){
            NetSocketDestroy(&s); return INVALID_SOCKET;
        }
        NetSocketUnblock(s);
    } return s;
}

static int NetSocketListen(NetSocket s,const NetAddress * const a,const unsigned int peers,const unsigned char protocol){
    _NetAddress _a={0};
    const unsigned int l=NetAddressToNet(a,&_a);
    if(l){
        if(bind(s,&_a.sa,l)==SOCKET_ERROR) return _NET_LAST_ERROR();
        if(protocol==NET_TCP && listen(s,peers)==SOCKET_ERROR) return _NET_LAST_ERROR();
        return 0;
    } return -1;
}

static int NetSocketConnect(NetSocket s,const NetAddress * const a){
    _NetAddress _a={0};
    const unsigned int l=NetAddressToNet(a,&_a);
    if(l){
        if(connect(s,&_a.sa,l)==SOCKET_ERROR) return _NET_LAST_ERROR();
        return EISCONN;
    } return -1;
}

static void NetSocketKeepAlive(NetSocket s){
#ifdef SO_KEEPALIVE
    int x=1;
    setsockopt(s,SOL_SOCKET,SO_KEEPALIVE,&x,sizeof(x));
    #ifdef IPPROTO_TCP
    x=250*1000;
        #ifdef TCP_KEEPIDLE
    setsockopt(s,IPPROTO_TCP,TCP_KEEPIDLE,&x,sizeof(x));
        #endif
        #ifdef TCP_KEEPINTVL
    setsockopt(s,IPPROTO_TCP,TCP_KEEPINTVL,&x,sizeof(x));
        #endif
    #endif
#endif
}


const char *NetEventString(const enum NET_EVENT event){
    switch(event){
        #define _CASE(_e_) case NET_##_e_: return #_e_
        _CASE(DISCONNECT); _CASE(CONNECT); _CASE(CANWRITE); _CASE(CANREAD); _CASE(TIMEOUT); _CASE(ACCEPT); _CASE(ERROR);
        #undef _CASE
        default: return "UNKNOWN";
    }
}


enum NET_SOCK_STATE{
    NET_DISCONNECTED = 0,
    NET_CONNECTING,
    NET_CONNECTED,
    NET_LISTENING,
    NET_ACCEPTING
};

typedef struct NetNode NetNode;

struct NetNode{
    NetUnit u[1];
    NetNode *prev, *next, *server;
    NetPool *pool;
    unsigned int id;
    unsigned char sock_state, protocol, del;
    NetAddress address[1];
    time_t timeout, pulse;
    NetSocket sock, ctrl;
};

typedef struct{
    NetNode *first, *last;
}NetList;

struct NetPool{
    void*(*allocator)(size_t);
    void(*deallocator)(void*);
    NetList reserv[1], units[1];
    NetNode **node;
    struct pollfd *sock;
    unsigned int size, real, reserv_count, reserv_max;
    NetSocket emit[2];
};


static void NetListLink(NetList * const l,NetNode * const n){
    if( (n->prev=l->last) ) l->last->next=n;
    else l->first=n;
    l->last=n;
    n->next=NULL;
}

static void NetListUnlink(NetList * const l,NetNode * const n){
    if(n->prev) n->prev->next=n->next;
    else l->first=n->next;
    if(n->next) n->next->prev=n->prev;
    else l->last=n->prev;
}

static void NetListLinkAfter(NetList * const l,NetNode * const n, NetNode * const c){
    if(n!=c){
        NetListUnlink(l,n);
        if( (n->next=c->next) ) c->next->prev=n;
        else l->last=n;
        n->prev=c;
        c->next=n;
    }
}

static int NetPollAdd(NetPool * const p,NetNode * const n,const short flags){
    if(!p->node) return -1;
    if(p->size==p->real){
        const unsigned int count=p->real+(p->real<2000 ? p->real : 1000);
        void * const x[2]={p->allocator(sizeof(*p->node)*count), p->allocator(sizeof(*p->sock)*count)};
        if(!x[0] || !x[1]){
            p->deallocator(x[0]);
            p->deallocator(x[1]);
            return -1;
        }
        memcpy(x[0],p->node,sizeof(*p->node)*p->size);
        memcpy(x[1],p->sock,sizeof(*p->sock)*p->size);
        p->real=count;
    }
    n->id=p->size++;
    p->node[n->id]=n;
    p->sock[n->id].revents=0;
    p->sock[n->id].fd=n->sock;
    p->sock[n->id].events=flags;
    return 0;
}

static void NetPollRem(NetPool * const p,const unsigned int i){
    if(!p->node) return;
    p->node[i]->id=0;
    if(i!=--p->size){
        p->node[i]=p->node[p->size];
        p->sock[i]=p->sock[p->size];
        p->node[i]->id=i;
    }
}

static void _NetUdpBoth(NetNode * const n,struct pollfd * const p,const time_t t){
    if(p->revents & (POLLERR | POLLNVAL)){
        errno=NetSocketError(n->sock);
        n->u->handler(n->u,NET_ERROR);
        return;
    }
    if(n->sock_state==NET_CONNECTING){
        p->events&=~POLLOUT;
        n->sock_state=NET_CONNECTED;
        n->timeout=n->pulse=t;
        n->u->handler(n->u,NET_CONNECT);
        return;
    }
    if(p->revents & POLLOUT){
        p->events^=POLLOUT;
        n->timeout=t;
        n->u->handler(n->u,NET_CANWRITE);
        return;
    }
    if(p->revents & POLLIN){
        n->timeout=n->pulse=t;
        n->u->handler(n->u,NET_CANREAD);
        return;
    }
}

static void  _NetTcpServer(NetNode * const n,struct pollfd * const p,const time_t t){
    if(p->revents & (POLLERR|POLLNVAL)){
        errno=NetSocketError(n->sock);
        n->u->handler(n->u,NET_ERROR);
        return;
    }
    if(p->revents & (POLLIN|POLLHUP)){
        NetAddress a[1];
        NetSocket s=NetSocketAccept(n->sock,a);
        NetNode *x;
        n->timeout=t;
        if(s!=INVALID_SOCKET){
            while( (x=(NetNode*)NetPoolUnit(n->pool,NET_TCP)) ){
                x->sock=s;
                if(!NetPollAdd(n->pool,x,POLLIN|POLLHUP)){
                    NetUnitAutoRemove(x->u);
                    x->server=n;
                    x->address[0]=a[0];
                    x->timeout=x->pulse=t;
                    x->sock_state=NET_ACCEPTING;
                    n->u->handler(x->u,NET_ACCEPT);
                    if(x->sock_state==NET_ACCEPTING){
                        NetSocketKeepAlive(s);
                        NetListLinkAfter(n->pool->units,x,n);
                        x->sock_state=NET_CONNECTED;
                        x->u->handler(x->u,NET_CONNECT);
                    }
                    if( (s=NetSocketAccept(n->sock,a))!=INVALID_SOCKET )
                        continue;
                    return;
                }
                NetListUnlink(n->pool->units,x);
                NetListLink(n->pool->reserv,x);
                ++n->pool->reserv_count;
                break;
            } NetSocketDestroy(&s);
        } n->u->handler(n->u,NET_ERROR);
    }
}

static void _NetTcpClient(NetNode * const n,struct pollfd * const p,const time_t t){
    if(p->revents & (POLLERR|POLLNVAL)){
        errno=NetSocketError(n->sock);
        n->u->handler(n->u,NET_ERROR);
        return;
    }
    if(p->revents & POLLHUP){
        if(n->sock_state==NET_CONNECTED){
            NetUnitDisconnect(n->u);
            return;
        }
        errno=EHOSTUNREACH;
        n->u->handler(n->u,NET_ERROR);
        return;
    }
    if(n->sock_state==NET_CONNECTING){
        if( (errno=NetSocketError(n->sock)) || NetSocketConnect(n->sock,n->address)!=EISCONN){
            n->u->handler(n->u,NET_ERROR);
            return;
        }
        NetSocketKeepAlive(n->sock);
        n->timeout=n->pulse=t;
        n->sock_state=NET_CONNECTED;
        n->u->handler(n->u,NET_CONNECT);
        return;
    }
    if(p->revents & POLLIN){
        char skip[1];
        switch(recv(n->sock,skip,1,MSG_NOSIGNAL|MSG_PEEK)){
            case 0:
                NetUnitDisconnect(n->u);
                return;
            case 1:
                if(n->server) NetListLinkAfter(n->pool->units,n,n->server);
                n->timeout=n->pulse=t;
                n->u->handler(n->u,NET_CANREAD);
                return;
            default:
                _NET_LAST_ERROR();
                n->u->handler(n->u,NET_ERROR);
                return;
        }
    }
    if(p->revents & POLLOUT){
        n->timeout=t;
        p->events^=POLLOUT;
        n->u->handler(n->u,NET_CANWRITE);
        return;
    }
}

static void NetChecks(NetPool * const p){
    NetNode *n=p->units->first;
    while(n){
        const time_t t=time(NULL);
        NetNode * const next=n->next;
        switch(n->sock_state){
            case NET_CONNECTING:
                if(n->u->timeout && t>n->timeout+n->u->timeout){
                    errno=ETIMEDOUT;
                    n->u->handler(n->u,NET_ERROR);
                }break;
#ifndef __WIN32
            case NET_CONNECTED:
                if(n->ctrl!=INVALID_SOCKET){
                    switch(NetSocketError(n->ctrl)){
                        case 0:
                            switch( NetSocketConnect(n->ctrl,n->address) ){
                                case EISCONN: case ECONNREFUSED: n->pulse=t; NetSocketDestroy(&n->ctrl); break;
                                default: if(t>=n->pulse) NetUnitDisconnect(n->u); break;
                            } break;
                        case ECONNREFUSED: n->pulse=t; NetSocketDestroy(&n->ctrl); break;
                        default: NetUnitDisconnect(n->u); break;
                    }
                }else if(n->u->pulse && t>n->pulse+n->u->pulse && (n->ctrl=NetSocketCreate(NET_TCP,n->address->ipv))!=INVALID_SOCKET ){
                    /* may be connect to same address, but different port: 7 or else */
                    NetSocketConnect(n->ctrl,n->address);
                    n->pulse=t+20;
                }
#endif
            default:
                if(n->u->timeout){
                    if(t>n->timeout+n->u->timeout){
                        n->timeout=t;
                        n->u->handler(n->u,NET_TIMEOUT);
                    }
                }else if(!n->id && n->del){
                    NetListUnlink(p->units,n);
                    NetListLink(p->reserv,n);
                    ++p->reserv_count;
                }
        }
        n=next;
    }
}

static void NetDispatch(NetPool * const p,int c){
    void(* const f[2][2])(NetNode*,struct pollfd*,const time_t)={{_NetTcpClient,_NetTcpServer},{_NetUdpBoth,_NetUdpBoth}};
    const time_t t=time(NULL);
    unsigned int i=p->size;
    while(c && --i)
        if(p->sock[i].revents){
            NetNode * const n=p->node[i];
            f[n->protocol==NET_UDP][n->sock_state==NET_LISTENING](n,p->sock+i,t);
            --c; if(i>p->size) i=p->size;
        }
}

static void NetReservCut(NetPool * const p,const unsigned int max){
    while(p->reserv_count>max){
        NetNode * const n=p->reserv->first;
        NetListUnlink(p->reserv,n);
        p->deallocator(n);
        --p->reserv_count;
    }
}

int NetPoolDispatch(NetPool * const pool,int *emit){
    int tmp[1];
    char work=1;
    if(!emit) emit=tmp;
    do{
        int count=poll(pool->sock,pool->size,1000);
        if(count==SOCKET_ERROR) return errno;
        if(pool->sock->revents){
            recv(pool->emit[0],emit,sizeof(*emit),MSG_NOSIGNAL);
            --count; work=0;
        }
        NetDispatch(pool,count);
        NetChecks(pool);
        NetReservCut(pool,pool->reserv_max);
    }while(work);
    return 0;
}

NetPool *NetPoolCreate(void){
    return NetPoolCreateEx(7,10,(void*(*)(size_t))0,(void(*)(void*))0);
}

NetPool *NetPoolCreateEx(unsigned int base_count,unsigned int reserv_count,void*(*allocator)(size_t),void(*deallocator)(void*)){
    NetPool *p=NULL;
    if(_NetConfig(1)){errno=ENETDOWN; return NULL;}
    if(!allocator) allocator=malloc;
    if(!deallocator) deallocator=free;
    if(base_count<10) base_count=10;
    do{
        if( !(p=(NetPool*)allocator(sizeof(*p))) ){
            _NetConfig(0);
            return NULL;
        }
        memset(p,0,sizeof(*p));
        p->emit[0]=p->emit[1]=INVALID_SOCKET;
        if(NetSocketPair(p->emit)) break;
        ++base_count;
        if( !(p->node=(NetNode**)allocator(sizeof(*p->node)*base_count)) ) break;
        if( !(p->sock=(struct pollfd*)allocator(sizeof(*p->sock)*base_count)) ) break;
        p->allocator=allocator;
        p->deallocator=deallocator;
        p->real=base_count;
        p->reserv_max=reserv_count;
        p->size=1;
        p->sock->fd=p->emit[0];
        p->sock->events=POLLIN;
        return p;
    }while(0);
    if(p->node) deallocator(p->node);
    if(p->sock) deallocator(p->sock);
    if(p->emit[0]!=INVALID_SOCKET){
        NetSocketDestroy(p->emit);
        NetSocketDestroy(p->emit+1);
    }
    _NetConfig(0);
    return NULL;
}

void NetPoolDestroy(NetPool * const pool){
    NetSocketDestroy(pool->emit);
    NetSocketDestroy(pool->emit+1);
    pool->deallocator(pool->node); pool->node=NULL;
    pool->deallocator(pool->sock); pool->sock=NULL;
    while(pool->units->first){
        NetNode * const n=pool->units->first;
        NetUnitDisconnect(n->u);
        NetListUnlink(pool->units,n);
        pool->deallocator(n);
    }
    NetReservCut(pool,0);
    pool->deallocator(pool);
    _NetConfig(0);
}

int NetPoolEmit(NetPool * const pool,const int value){
    return send(pool->emit[1],&value,sizeof(value),MSG_NOSIGNAL)==SOCKET_ERROR;
}

NetUnit *NetPoolUnit(NetPool * const pool,const enum NET_PROTOCOL protocol){
    if(pool->node){
        NetNode *n=pool->reserv->first;
        if(n){
            NetListUnlink(pool->reserv,n);
            --pool->reserv_count;
        }else n=(NetNode*)pool->allocator(sizeof(*n));
        if(n){
            memset(n,0,sizeof(*n));
            n->u->pulse=20;
            n->pool=pool;
            n->protocol=protocol;
            n->sock=n->ctrl=INVALID_SOCKET;
            n->sock_state=NET_DISCONNECTED;
            time(&n->timeout);
            NetListLink(pool->units,n);
            return (NetUnit*)n;
        }
        pool->deallocator(n);
    }
    errno=EINVAL;
    return NULL;
}

int NetUnitListen(NetUnit * const unit,const NetAddress * const address){
    NetNode * const n=(NetNode*)unit;
    if(n->sock!=INVALID_SOCKET){errno=EBUSY; return -1;}
    if( (n->sock=NetSocketCreate(n->protocol,address->ipv))!=INVALID_SOCKET ){
        n->server=NULL;
        if(NetSocketListen(n->sock,address,5,n->protocol) || NetPollAdd(n->pool,n,POLLIN|POLLHUP)){
            NetSocketDestroy(&n->sock);
            return -1;
        }
        n->sock_state=NET_LISTENING;
        n->address[0]=address[0];
        time(&n->timeout);
        return 0;
    } return -1;
}

int NetUnitConnect(NetUnit * const unit,const NetAddress * const address){
    NetNode * const n=(NetNode*)unit;
    if(n->sock!=INVALID_SOCKET){errno=EBUSY; return -1;}
    if( (n->sock=NetSocketCreate(n->protocol,address->ipv))!=INVALID_SOCKET ){
        n->server=NULL;
        if(NetPollAdd(n->pool,n,POLLIN|POLLOUT|POLLHUP)){
            NetSocketDestroy(&n->sock);
            return -1;
        }
        NetSocketConnect(n->sock,address);
        n->sock_state=NET_CONNECTING;
        n->address[0]=address[0];
        time(&n->timeout);
        return 0;
    } return -1;
}

int NetUnitWrite(NetUnit * const unit,const void * const data,const unsigned int size,const NetAddress * const address){
    NetNode * const n=(NetNode*)unit;
    ssize_t bytes;
    if(address){
        _NetAddress _a={0};
        const unsigned int l=NetAddressToNet(address,&_a);
        if(!l) return -2;
        bytes=sendto(n->sock,data,size,MSG_NOSIGNAL,&_a.sa,l);
    }else bytes=send(n->sock,data,size,MSG_NOSIGNAL);
    if(bytes<0) _NET_LAST_ERROR();
    return bytes;
}

int NetUnitRead(NetUnit * const unit,void * const buffer,const unsigned int size,NetAddress * const address){
    NetNode * const n=(NetNode*)unit;
    ssize_t bytes;
    if(address){
        _NetAddressStorage _a;
        unsigned int l=sizeof(_a);
        bytes=recvfrom(n->sock,buffer,size,MSG_NOSIGNAL,&_a.sa,&l);
        NetAddressFromNet(&_a,l,address);
    }else bytes=recv(n->sock,buffer,size,MSG_NOSIGNAL);
    if(bytes<0) _NET_LAST_ERROR();
    return bytes;
}

void NetUnitDisconnect(NetUnit * const unit){
    NetNode * const n=(NetNode*)unit;
    unit->timeout=0;
    if(n->sock!=INVALID_SOCKET){
        NetList * const l=n->pool->units;
        NetNode *x=n->next;
        NetSocketDestroy(&n->sock);
        NetSocketDestroy(&n->ctrl);
        NetPollRem(n->pool,n->id);
        NetListLinkAfter(l,n,l->last);
        switch(n->sock_state){
            case NET_CONNECTED:
            case NET_LISTENING:
                n->sock_state=NET_DISCONNECTED;
                n->u->handler(n->u,NET_DISCONNECT);
                break;
            default: n->sock_state=NET_DISCONNECTED;
        }
        while(x && x->server==n){
            NetNode * const d=x;
            x=x->next;
            NetUnitDisconnect(d->u);
        }
    }
}

void NetUnitAutoRemove(NetUnit * const unit){
    ((NetNode*)unit)->del=1;
}

NetUnit *NetUnitNodeServer(const NetUnit * const unit){
    return (NetUnit*)(((const NetNode*)unit)->server);
}

NetUnit *NetUnitNodeNext(const NetUnit * const unit){
    return (NetUnit*)(((const NetNode*)unit)->next);
}

short *NetUnitRDWR(const NetUnit * const unit){
    const NetNode * const n=(const NetNode*)unit;
    if(n->id) return &n->pool->sock[n->id].events;
    return NULL;
}

NetPool *NetUnitPool(const NetUnit * const unit){
    return ((const NetNode*)unit)->pool;
}

const NetAddress *NetUnitAddress(const NetUnit * const unit){
    return ((const NetNode*)unit)->address;
}

#undef _NET_LAST_ERROR

#endif
