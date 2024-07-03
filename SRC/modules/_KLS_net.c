
#ifndef _NET_ERRNO
    #define _NET_ERRNO(...) __VA_ARGS__
    #define _NET_FUNC_DEF(_f_,...) _f_(__VA_ARGS__)
#else
    #define _NET_FUNC_DEF(_f_,...) ({ KLS_TYPEOF(_f_(__VA_ARGS__)) _1_=_f_(__VA_ARGS__); errno=_NET_ERRNO(); _1_; })
#endif

#define socket(...)   _NET_FUNC_DEF(socket,__VA_ARGS__)
#define connect(...)  _NET_FUNC_DEF(connect,__VA_ARGS__)
#define bind(...)     _NET_FUNC_DEF(bind,__VA_ARGS__)
#define listen(...)   _NET_FUNC_DEF(listen,__VA_ARGS__)
#define accept(...)   _NET_FUNC_DEF(accept,__VA_ARGS__)
#define recv(...)     _NET_FUNC_DEF(recv,__VA_ARGS__)
#define recvfrom(...) _NET_FUNC_DEF(recvfrom,__VA_ARGS__)
#define send(...)     _NET_FUNC_DEF(send,__VA_ARGS__)
#define sendto(...)   _NET_FUNC_DEF(sendto,__VA_ARGS__)
#define shutdown(...) _NET_FUNC_DEF(shutdown,__VA_ARGS__)
#define select(...)   _NET_FUNC_DEF(select,__VA_ARGS__)

#ifndef _NET_BAD_OP
    #define _NET_BAD_OP -1
#endif

#ifndef _NET_BAD_FD
    #define _NET_BAD_FD -1
#endif

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
    _NET_t_SOCKET fd;
    short events,revents;
};

int poll(struct pollfd *p,int cnt,int timeout){
    int i,s;
    struct timeval t={timeout/1000,1000*(timeout%1000)};
    _NET_t_SOCKET max=p->fd;
    fd_set set[3];
    FD_ZERO(set);FD_ZERO(set+1);FD_ZERO(set+2);
    for(i=0;i<cnt;++i){
        p[i].revents=0;
        if(p[i].events & POLLIN) FD_SET(p[i].fd,set);
        if(p[i].events & POLLOUT) FD_SET(p[i].fd,set+1);
        FD_SET(p[i].fd,set+2);
        if(p[i].fd>max) max=p[i].fd;
    }
    if((s=select(max+1,set,set+1,set+2,timeout<0?NULL:&t))>0)
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
    return s<-1 ? -1 : s;
}

#endif

#define poll(...)     _NET_FUNC_DEF(poll,__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////////

typedef struct{
    socklen_t l;
    union{
        struct sockaddr_storage st;
        struct sockaddr sa;
        struct sockaddr_in a4;
        struct sockaddr_in6 a6;
    }d;
}_NET_t_ADDR;

#define _NET_ADDR(_var_) _NET_t_ADDR _var_={sizeof(_var_.d)}

KLS_byte _NET_addrFromNet(const _NET_t_ADDR *in,NET_t_ADDRESS *out){
    out->created=0;
    if( in->d.st.ss_family==AF_INET || (in->d.st.ss_family==AF_UNSPEC && in->l==sizeof(in->d.a4)) ){
        memcpy(out->ip,&in->d.a4.sin_addr,sizeof(in->d.a4.sin_addr));
        out->port=ntohs(in->d.a4.sin_port);
        out->version=NET_IP4;
        out->created=1;
    }
    if(in->d.st.ss_family==AF_INET6 || (in->d.st.ss_family==AF_UNSPEC && in->l==sizeof(in->d.a6)) ){
        memcpy(out->ip,&in->d.a6.sin6_addr,sizeof(in->d.a6.sin6_addr));
        out->port=ntohs(in->d.a6.sin6_port);
        out->version=NET_IP6;
        out->created=1;
    }
    return out->created;
}

KLS_byte _NET_addrToNet(const NET_t_ADDRESS *in,_NET_t_ADDR *out){
    out->l=0;
    if(in->created){
        if(in->version==NET_IP4){
            memcpy(&out->d.a4.sin_addr,in->ip,sizeof(out->d.a4.sin_addr));
            out->d.a4.sin_family=AF_INET;
            out->d.a4.sin_port=htons(in->port);
            out->l=sizeof(out->d.a4);
        }
        if(in->version==NET_IP6){
            memcpy(&out->d.a6.sin6_addr,in->ip,sizeof(out->d.a6.sin6_addr));
            out->d.a6.sin6_family=AF_INET6;
            out->d.a6.sin6_port=htons(in->port);
            out->l=sizeof(out->d.a6);
        }
    }
    return out->l>0;
}

void _NET_addrFind(const char *host,NET_t_ADDRESS *addr){
    struct addrinfo in={.ai_family=AF_UNSPEC, .ai_flags=0}, *info=NULL,*iter=NULL;
    if(!getaddrinfo(host,NULL,&in,&info)){
        iter=info;
        do{
            if(iter->ai_family==AF_INET || iter->ai_family==AF_INET6 || (iter->ai_family==AF_UNSPEC && (iter->ai_addrlen==sizeof(struct sockaddr_in) || iter->ai_addrlen==sizeof(struct sockaddr_in6))) ){
                _NET_ADDR(a);
                a.l=iter->ai_addrlen;
                memcpy(&a.d,iter->ai_addr,a.l);
                _NET_addrFromNet(&a,addr);
                break;
            }
        }while((iter=iter->ai_next));
        freeaddrinfo(info);
    }
}

const char *_NET_address(const char *address,char *domain){
    const char *begin,*end;
    if((begin=strstr(address,"://"))) begin+=3; else begin=address;
    if(!(end=strstr(begin,"/"))) end=begin+strlen(begin);
    memcpy(domain,begin,end-begin);
    domain[end-begin]=0;
    return domain;
}

NET_t_ADDRESS NET_address(const char *host,unsigned short port){
    char tmp[128];
    NET_t_ADDRESS a;
    _NET_addrFind(_NET_address(host,tmp),&a);
    a.port=port;
    return a;
}

const char *NET_addressToString(NET_t_ADDRESS *address,char name[40]){
    if(address->created){
        if(address->version==NET_IP6){
            const unsigned short *p=(const void*)address->ip;
            sprintf(name,"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
            return name;
        }
        if(address->version==NET_IP4){
            const unsigned char *p=(const void*)address->ip;
            sprintf(name,"%d.%d.%d.%d",p[0],p[1],p[2],p[3]);
            return name;
        }
    }
    *name=0;
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////

void *NET_lowLevelSocket(NET_t_SOCKET *socket){
    return socket->_osDep;
}

int _NET_protocol(KLS_byte p){
    switch(p){
        case NET_TCP: return SOCK_STREAM;
        case NET_UDP: return SOCK_DGRAM;
    } return -1;
}

int _NET_version(KLS_byte v){
    switch(v){
        case NET_IPU: return AF_UNSPEC;
        case NET_IP4: return AF_INET;
        case NET_IP6: return AF_INET6;
    } return -1;
}

NET_t_SOCKET NET_socketNew(int protocol,int version){
    NET_t_SOCKET s={.protocol=protocol,.version=version};
    if( (*_NET_sockOs(&s)=socket(_NET_version(version),_NET_protocol(protocol),0))!=_NET_BAD_FD){
        s.status=NET_SOCK_DISCONNECTED;
        s.blocked=s.created=protocol=1;
        if(!NET_socketOptionSet(&s,SOL_SOCKET,SO_REUSEADDR,&protocol,sizeof(protocol)))
            NET_socketOptionSet(&s,SOL_SOCKET,SO_REUSEADDR,&protocol,1);
    }
    return s;
}

void NET_socketFree(NET_t_SOCKET *socket){
    if(socket->created){
        shutdown(*_NET_sockOs(socket),SHUT_RDWR);
        _NET_sockClose(socket);
        socket->created=socket->status=0;
    }
}

signed char NET_socketConnect(NET_t_SOCKET *socket,const NET_t_ADDRESS *address){
    if(socket->created){
        _NET_ADDR(a);
        if(_NET_addrToNet(address,&a)){
            int e;
            if(connect(*_NET_sockOs(socket),&a.d.sa,a.l)!=_NET_BAD_OP || (e=errno)==EISCONN){
                socket->status=NET_SOCK_CONNECTED; return 1;
            }
            if(e==EALREADY || e==EINPROGRESS || e==EAGAIN || e==EWOULDBLOCK){
                socket->status=NET_SOCK_CONNECTING; return 0;
            }
        }
    }
    return -1;
}

KLS_byte NET_socketBind(NET_t_SOCKET *socket,const NET_t_ADDRESS *address){
    if(socket->created){
        _NET_ADDR(a);
        if(_NET_addrToNet(address,&a))
            return bind(*_NET_sockOs(socket),&a.d.sa,a.l)!=_NET_BAD_OP;
    }
    return 0;
}

KLS_byte NET_socketListen(NET_t_SOCKET *socket,unsigned int peers){
    if(socket->created && (socket->protocol!=NET_TCP || listen(*_NET_sockOs(socket),peers)!=_NET_BAD_OP)){
        socket->status=NET_SOCK_LISTENING;
        return 1;
    }
    return 0;
}

NET_t_SOCKET NET_socketAccept(NET_t_SOCKET *socket,NET_t_ADDRESS *address){
    NET_t_SOCKET ret={};
    if(socket->status==NET_SOCK_LISTENING){
        if(address){
            _NET_ADDR(a);
            if( (ret.created=( (*_NET_sockOs(&ret)=accept(*_NET_sockOs(socket),&a.d.sa,&a.l)) != _NET_BAD_FD )) ){
                if( !(ret.created= _NET_addrFromNet(&a,address)) )
                    _NET_sockClose(&ret);
            }
        }else ret.created=( (*_NET_sockOs(&ret)=accept(*_NET_sockOs(socket),NULL,NULL)) != _NET_BAD_FD);
        if( (ret.blocked=ret.created) ){
            ret.status=NET_SOCK_CONNECTED;
            ret.version=socket->version;
            ret.protocol=socket->protocol;
        }
    }
    return ret;
}

unsigned int NET_socketReceive(NET_t_SOCKET *socket,void *data,unsigned int size,NET_t_ADDRESS *from){
    unsigned int ret=0;
    if(socket->created && data){
        if(from){
            _NET_ADDR(a);
            ret=recvfrom(*_NET_sockOs(socket),data,size,MSG_NOSIGNAL,&a.d.sa,&a.l);
            if(ret!=-1) _NET_addrFromNet(&a,from);
        }else ret=recvfrom(*_NET_sockOs(socket),data,size,MSG_NOSIGNAL,NULL,NULL);
    }
    return ret!=-1?ret:0;
}

unsigned int NET_socketSend(NET_t_SOCKET *socket,const void *data,unsigned int size,const NET_t_ADDRESS *to){
    unsigned int ret=0;
    if(socket->created && data){
        if(to){
            _NET_ADDR(a);
            if(_NET_addrToNet(to,&a))
                ret=sendto(*_NET_sockOs(socket),data,size,MSG_NOSIGNAL,&a.d.sa,a.l);
        }else ret=send(*_NET_sockOs(socket),data,size,MSG_NOSIGNAL);
    }
    return ret!=-1?ret:0;
}

KLS_byte NET_socketOptionSet(NET_t_SOCKET *s,int level,int option,const void *data,unsigned int dataSize){
    return !setsockopt(*_NET_sockOs(s),level,option,data,dataSize);
}

KLS_byte NET_socketOptionGet(NET_t_SOCKET *s,int level,int option,void *data,unsigned int dataSize){
    int l[3]={0,dataSize};
    return !getsockopt(*_NET_sockOs(s),level,option,data,(void*)(l+1));
}

int NET_socketError(NET_t_SOCKET *s){
    int e=~0;
    NET_socketOptionGet(s,SOL_SOCKET,SO_ERROR,&e,sizeof(e));
    return _NET_ERRNO(e);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NET_KEEPALIVE_DEFAULT
    #define NET_KEEPALIVE_DEFAULT 250
#endif

#define _NET_POLL_ER  (POLLERR | POLLNVAL)
#define _NET_POLL_RD  POLLIN
#define _NET_POLL_WR  POLLOUT
#define _NET_POLL_CS  POLLHUP

const KLS_byte NET_R=_NET_POLL_RD;
const KLS_byte NET_W=_NET_POLL_WR;

typedef struct pollfd _NET_t_POLL;

struct _NET_t_MANAGER{
    KLS_t_VECTOR service, poll, delays;
    KLS_t_LIST units;
    sem_t sem[1];
    struct _NET_t_UNIT help;
};

void _NET_trashPrint(NET_t_MANAGER m,const char *str){
    printf("%s:",str);
    KLS_listPrint(&m->units,KLS_LMBD(void,(NET_t_UNIT u){printf("%d",(u->_.flags<<1)|u->socket.created);}));
    printf("\n");
}

void _NET_trashBlock(NET_t_UNIT u){
    KLS_listMoveAfter(&u->manager->units,u,u->manager->units.last);
}

void _NET_trashAllow(NET_t_UNIT u){
    if((u->_.flags&3)!=1 || u->socket.created) return;
    KLS_listMoveBefore(&u->manager->units,u,u->manager->units.first);
}

NET_t_UNIT _NET_trashAdd(NET_t_UNIT u){
    if((u->_.flags&3)==1 && !u->socket.created){
        NET_t_UNIT next=KLS_listNext(u);
        if(u->manager->help.pulse<u->manager->help._.pulse){
            u->_.flags|=2;
            ((NET_t_UNIT*)(u->manager+1))[u->manager->help.pulse++]=u;
        }else KLS_listRemove(&u->manager->units,u);
        return next;
    } return NULL;
}

NET_t_UNIT _NET_trashLast(NET_t_MANAGER m){
    m->help._.timeout=time(NULL)+m->help.timeout;
    return m->help.pulse ? ((NET_t_UNIT*)(m+1))[--m->help.pulse] : NULL;
}

KLS_byte _NET_serviceAdd(NET_t_UNIT u,time_t t,short flags){
    if(KLS_vectorPushBack(&u->manager->poll,u)){
        if(KLS_vectorPushBack(&u->manager->service,&u)){
            _NET_t_POLL *p=(_NET_t_POLL*)u->manager->poll.data+(u->_.id=u->manager->poll.size-1);
            p->fd=*_NET_sockOs(&u->socket);
            p->events=flags;
            u->_.timeout=t;
            return 1;
        } KLS_vectorRemoveFast(&u->manager->poll,u->manager->poll.size-1);
    } return 0;
}

void _NET_vectorDisconnect(KLS_t_VECTOR *v,KLS_size i){
    KLS_vectorRemoveFast(v,i);
    if((v=KLS_vectorAt(v,i))) (*(NET_t_UNIT*)v)->_.id=i;
}

void _NET_trashing(NET_t_MANAGER m,time_t t){
    NET_t_UNIT u=(void*)m->units.first;
    while( (u=_NET_trashAdd(u)) );
    if(m->help.timeout && t>m->help._.timeout){
        NET_t_UNIT *trash=(void*)(m+1);
        while(m->help.pulse)
            KLS_listRemove(&m->units,trash[--m->help.pulse]);
        m->help._.timeout=t+m->help.timeout;
    }
}

void _NET_timeouts(NET_t_MANAGER m,time_t t){
    NET_t_UNIT u;
    KLS_size i;
    KLS_byte r;
    do{
        for(r=0,i=1;i<m->service.size;++i){
            u=*(void**)KLS_vectorAt(&m->service,i);
            if(u->socket.status==NET_SOCK_CONNECTING){
                if(u->timeout && t>u->_.timeout+u->timeout){
                    NET_disconnect(u); --i; r|=1;
                } continue;
            }
#ifndef _NET_NOPULSE
            if(u->socket.status==NET_SOCK_CONNECTED){
                if(u->_.cntr.created)
                    switch(NET_socketError(&u->_.cntr)){
                        case 0: NET_socketConnect(&u->_.cntr,&u->address);
                            switch(errno){
                                case EISCONN: case ECONNREFUSED: break;
                                default: if(t<u->_.pulse) break; NET_disconnect(u); --i; r|=1; continue;
                            }
                        case ECONNREFUSED: u->_.pulse=t; NET_socketFree(&u->_.cntr); break;
                        default: NET_disconnect(u); --i; r|=1; continue;
                    }
                else if(u->pulse && t>u->_.pulse+u->pulse && (u->_.cntr=NET_socketNew(NET_TCP,u->address.version)).created ){
                    u->_.pulse=t+20;
                    NET_socketSetBlock(&u->_.cntr,0);
                    NET_socketConnect(&u->_.cntr,&u->address);
                }
            }
#endif
            if(u->timeout && t>u->_.timeout+u->timeout){
                r|=1; u->_.timeout=t;
                u->handler(u,NET_EVENT_TIMEOUT);
            }
        }
    }while(r);
}

void _NET_delays(KLS_t_VECTOR *v,time_t t){
    KLS_size i=0;
    while(i<v->size){
        NET_t_UNIT u=*(NET_t_UNIT*)KLS_vectorAt(v,i);
        if(t<u->_.timeout){++i; continue;}
        if(NET_type(u)==NET_LISTENER){
            if(_NET_serviceAdd(u,t,_NET_POLL_RD|_NET_POLL_CS))
                _NET_vectorDisconnect(v,i);
            else u->handler(u,NET_EVENT_ERROR);
        }else{
            if(NET_socketConnect(&u->socket,&u->address)>-1){
                u->socket.status=NET_SOCK_CONNECTING;
                if(_NET_serviceAdd(u,t,_NET_POLL_WR|_NET_POLL_RD|_NET_POLL_CS))
                    _NET_vectorDisconnect(v,i);
                else u->handler(u,NET_EVENT_ERROR);
            }else NET_disconnect(u);
        }
    }
}

void _NET_keepAlive(NET_t_UNIT u){
    #ifdef SO_KEEPALIVE
    if(u->_.ka){
        NET_t_SOCKET *s=&u->socket;
        int x[1]={1};
        if(NET_socketOptionSet(s,SOL_SOCKET,SO_KEEPALIVE,x,sizeof(x))){
        #ifdef IPPROTO_TCP
            *x=1000*(int)u->_.ka;
            #ifdef TCP_KEEPIDLE
            NET_socketOptionSet(s,IPPROTO_TCP,TCP_KEEPIDLE,x,sizeof(x));
            #endif
            #ifdef TCP_KEEPINTVL
            NET_socketOptionSet(s,IPPROTO_TCP,TCP_KEEPINTVL,x,sizeof(x));
            #endif
        #endif
        }
    }
    #endif
}

void  _NET_udp(NET_t_UNIT u,_NET_t_POLL *p,time_t t){
    if(p->revents & _NET_POLL_ER){
        errno=NET_socketError(&u->socket);
        u->handler(u,NET_EVENT_ERROR);
        return;
    }
    if(u->socket.status==NET_SOCK_CONNECTING){
        u->socket.status=NET_SOCK_CONNECTED;
        u->_.timeout=u->_.pulse=t;
        p->events&=~_NET_POLL_WR;
        u->handler(u,NET_EVENT_CONNECT);
        return;
    }
    if(p->revents & _NET_POLL_WR){
        u->_.timeout=t;
        p->events^=_NET_POLL_WR;
        u->handler(u,NET_EVENT_MAYSEND);
        return;
    }
    if(p->revents & _NET_POLL_RD){
        u->_.timeout=u->_.pulse=t;
        u->handler(u,NET_EVENT_RECEIVE);
        return;
    }
}

void  _NET_tcpSrv(NET_t_UNIT u,_NET_t_POLL *p,time_t t){
    if(p->revents & (_NET_POLL_RD|_NET_POLL_CS)){
        NET_t_ADDRESS a;
        NET_t_SOCKET s=NET_socketAccept(&u->socket,&a);
        u->_.timeout=t;
        if(s.created){
            NET_t_UNIT c=NET_unit(u->manager,NET_TCP);
            if(c){
                KLS_swap(&c->socket,&s,sizeof(s));
                NET_detach(c);
                if(_NET_serviceAdd(c,t,_NET_POLL_RD|_NET_POLL_CS)){
                    KLS_size cid=c->_.id;
                    NET_socketSetBlock(&c->socket,0);
                    c->address=a; c->parent=u;
                    c->_.timeout=c->_.pulse=t;
                    c->_.tp=NET_INCOMING;
                    u->handler(c,NET_EVENT_ACCEPT);
                    if(*(NET_t_UNIT*)KLS_vectorAt(&u->manager->service,cid)==c){
                        _NET_keepAlive(c);
                        KLS_listMoveAfter(&u->manager->units,c,u);
                        c->handler(c,NET_EVENT_CONNECT);
                    } return;
                }
                NET_socketFree(&c->socket);
                _NET_trashAdd(c);
            }
            NET_socketFree(&s);
        }
        u->handler(u,NET_EVENT_ERROR);
    }
}

void _NET_tcpCli(NET_t_UNIT u,_NET_t_POLL *p,time_t t){
    do{
        if(p->revents & (_NET_POLL_CS|_NET_POLL_ER))
            break;
        if(u->socket.status==NET_SOCK_CONNECTING){
            if(!NET_socketError(&u->socket))
                switch(NET_socketConnect(&u->socket,&u->address)){
                    case 1:
                        _NET_keepAlive(u);
                        u->_.timeout=u->_.pulse=t;
                        p->events&=~_NET_POLL_WR;
                        u->handler(u,NET_EVENT_CONNECT);
                    case 0: return;
                }
            break;
        }
        if(p->revents & _NET_POLL_RD){
            if(recv(p->fd,&p,1,MSG_NOSIGNAL|MSG_PEEK)){
                u->_.timeout=u->_.pulse=t;
                u->handler(u,NET_EVENT_RECEIVE);
                return;
            }
            break;
        }
        if(p->revents & _NET_POLL_WR){
            u->_.timeout=t;
            p->events^=_NET_POLL_WR;
            u->handler(u,NET_EVENT_MAYSEND);
            return;
        }
    }while(0);
    NET_disconnect(u);
}

signed char _NET_proc(KLS_t_VECTOR *p,KLS_t_VECTOR *s,KLS_size i,time_t *t){
    void(*f[2][2])(NET_t_UNIT,_NET_t_POLL*,time_t)={{_NET_tcpCli,_NET_tcpSrv},{_NET_udp,_NET_udp}};
    NET_t_UNIT u;
    while(1){
        i=poll(p->data,p->size,i);
        *t=time(NULL);
        switch(i){
            case -1: return errno==EINTR ? 1 : -1;
            case 0: return 0;
        }
        for(i=0;i<p->size;++i)
            if(((_NET_t_POLL*)p->data)[i].revents){
                u=((NET_t_UNIT*)s->data)[i];
                f[u->socket.protocol][NET_type(u)==NET_LISTENER](u,((_NET_t_POLL*)p->data)+i,*t);
            }
        if(((_NET_t_POLL*)p->data)->revents)
            return 2;
        i=0;
    }
}

int NET_service(NET_t_MANAGER manager,unsigned short timeout){
    if(manager){
        if(sem_trywait(manager->sem)){
            signed char ret=0;
            time_t t=time(NULL), tto=t+timeout;
            do{
                _NET_delays(&manager->delays,t);
                ret=_NET_proc(&manager->poll,&manager->service,timeout?1000:0,&t);
                _NET_timeouts(manager,t);
                _NET_trashing(manager,t);
            }while(!ret && (timeout==-1 || t<tto));
            return ret;
        }
        return 2;
    }
    return -2;
}

void NET_interrupt(NET_t_MANAGER manager){
    if(manager){
         NET_write(&manager->help,manager,1,NULL);
         sem_post(manager->sem);
    }
}

KLS_size _NET_vectorPolicy(KLS_size size){
    return size+(size<1000?(size>>1)+1:size/10);
}

void _NET_helpHandler(NET_t_UNIT u,KLS_byte e){
    switch(e){
        case NET_EVENT_RECEIVE: case NET_EVENT_ERROR:
            NET_read(u,NULL,-1,NULL); sem_trywait(u->manager->sem);
    }
}

NET_t_MANAGER NET_new(unsigned int baseSize,unsigned short trashSize,unsigned short trashTimeout){
    NET_t_MANAGER m=KLS_malloc(sizeof(*m)+sizeof(NET_t_UNIT)*trashSize);
    if(m){
        memset(m,0,sizeof(*m));
        if(!(baseSize++)) baseSize=10;
        do{
            if(sem_init(m->sem,0,0)) break;
            if(!(m->delays=KLS_vectorNew(10,sizeof(NET_t_UNIT),NULL)).data) break;
            if(!(m->poll=KLS_vectorNew(baseSize,sizeof(_NET_t_POLL),NULL)).data) break;
            if(!(m->service=KLS_vectorNew(baseSize,sizeof(NET_t_UNIT),NULL)).data) break;
            KLS_vectorSetPolicy(&m->poll,_NET_vectorPolicy);
            KLS_vectorSetPolicy(&m->delays,_NET_vectorPolicy);
            KLS_vectorSetPolicy(&m->service,_NET_vectorPolicy);
            m->units=KLS_listNew(sizeof(struct _NET_t_UNIT),(void*)NET_disconnect);

            m->help._.pulse=trashSize;
            m->help.manager=m;
            while(trashSize--){
                NET_t_UNIT u=KLS_listPushFront(&m->units,&m->help);
                if(u){NET_detach(u);_NET_trashAdd(u);}
            }
            m->help.timeout=trashTimeout;
            m->help.handler=_NET_helpHandler;
            m->help.address=NET_address("127.0.0.1",7);
            m->help.socket=NET_socketNew(NET_UDP,NET_IP4);
            NET_socketSetBlock(&m->help.socket,0);
            NET_socketConnect(&m->help.socket,&m->help.address);
            _NET_serviceAdd(&m->help,0,_NET_POLL_RD|_NET_POLL_CS);
            return m;
        }while(0);
        NET_free(&m);
    }
    return m;
}

void NET_free(NET_t_MANAGER *manager){
    NET_t_MANAGER m=*manager;
    if(m){
        KLS_vectorFree(&m->delays);
        KLS_vectorFree(&m->service);
        KLS_vectorFree(&m->poll);
        KLS_listClear(&m->units);
        sem_destroy(m->sem);
        NET_socketFree(&m->help.socket);
        KLS_freeData(*manager);
    }
}

NET_t_UNIT NET_unit(NET_t_MANAGER manager,KLS_byte protocol){
    NET_t_UNIT u=NULL;
    if( manager && ( (u=_NET_trashLast(manager)) || (u=KLS_listPushBack(&manager->units,manager)) ) ){
        memset(u,0,sizeof(*u));
        u->_.ka=NET_KEEPALIVE_DEFAULT; // default keepalive time(sec) to keep router's NAT table
        u->manager=manager;
        u->socket.protocol=protocol;
    }
    return u;
}

KLS_byte NET_listen(NET_t_UNIT u,NET_t_ADDRESS address,unsigned int queueSize){
    if(u && !u->socket.created && (u->socket=NET_socketNew(u->socket.protocol,address.version)).created){
        NET_socketSetBlock(&u->socket,0);
        if(NET_socketBind(&u->socket,&address) && NET_socketListen(&u->socket,queueSize) && KLS_vectorPushBack(&u->manager->delays,&u)){
            _NET_trashBlock(u);
            u->_.id=u->manager->delays.size-1;
            u->parent=NULL; u->address=address;
            u->_.tp=NET_LISTENER;
            u->_.timeout=time(NULL);
            return 1;
        }
        NET_socketFree(&u->socket);
    }
    return 0;
}

KLS_byte NET_connect(NET_t_UNIT u,NET_t_ADDRESS address,unsigned short delay){
    if(u && !u->socket.created && address.created && (u->socket=NET_socketNew(u->socket.protocol,address.version)).created){
        NET_socketSetBlock(&u->socket,0);
        if(KLS_vectorPushBack(&u->manager->delays,&u)){
            _NET_trashBlock(u);
            u->_.id=u->manager->delays.size-1;
            u->parent=NULL; u->address=address;
            u->_.tp=NET_OUTGOING;
            u->_.timeout=time(NULL)+delay;
            return 1;
        }
        NET_socketFree(&u->socket);
    }
    return 0;
}

void NET_disconnect(NET_t_UNIT u){
    if(u && u->socket.created){
        NET_t_UNIT c;
        NET_t_MANAGER m=u->manager;
        NET_socketFree(&u->socket);
        if( (c=KLS_vectorAt(&m->service,u->_.id)) && (*(NET_t_UNIT*)c)==u){
            c=KLS_listNext(u);
            NET_socketFree(&u->_.cntr);
            KLS_vectorRemoveFast(&m->poll,u->_.id);
            _NET_vectorDisconnect(&m->service,u->_.id);
            _NET_trashAllow(u);
            u->handler(u,NET_EVENT_DISCONNECT);
            while(c && c->parent==u){
                c=KLS_listNext((m=(void*)c));
                NET_disconnect((void*)m);
            }
            return;
        }
        if( (c=KLS_vectorAt(&m->delays,u->_.id)) && (*(NET_t_UNIT*)c)==u){
            _NET_vectorDisconnect(&m->delays,u->_.id);
            _NET_trashAllow(u);
        }
        u->handler(u,NET_EVENT_DISCONNECT);
    }
}

KLS_byte NET_write(const NET_t_UNIT u,const void *data,unsigned int size,const NET_t_ADDRESS *address){
    if(u){
        int window,c;
        while(size){
            window=size>1024*32?1024*32:size;
            c=NET_socketSend(&u->socket,data,window,address);
            if(c<1) return 0;
            if(c<window) window=window-c;
            data+=window; size-=window;
        }
    }
    return !size;
}

unsigned int NET_read(const NET_t_UNIT u,void *data,unsigned int size,NET_t_ADDRESS *address){
    if(u)
        switch(((!data)<<1) | (!size)){
            case 0:{
                int s=NET_socketReceive(&u->socket,data,size,address);
                return s>0?s:0;
            } break;
            case 1:{
                void **d=data;
                if( (*d=KLS_malloc( (size=_NET_recvSize(&u->socket)) )) )
                    return NET_read(u,*d,size,address);
            } break;
            case 2: while( (size=NET_socketReceive(&u->socket,&size,sizeof(size),address))==sizeof(size) ); break;
            case 3: return _NET_recvSize(&u->socket);
        }
    return 0;
}

short *NET_rwControl(NET_t_UNIT u){
    if(u){
        NET_t_UNIT *c=KLS_vectorAt(&u->manager->service,u->_.id);
        if( c && *c==u ) return &((_NET_t_POLL*)KLS_vectorAt(&u->manager->poll,u->_.id))->events;
    } return NULL;
}

KLS_byte NET_type(NET_t_UNIT u){
    return u ? u->_.tp : 0;
}

void NET_detach(NET_t_UNIT u){
    if(u){ u->_.flags|=1; _NET_trashAllow(u); }
}

#undef socket
#undef connect
#undef bind
#undef listen
#undef accept
#undef recv
#undef recvfrom
#undef send
#undef sendto
#undef shutdown
#undef select
#undef poll
