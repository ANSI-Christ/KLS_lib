#include "KLS_lib.h"

void incomingTcp(NET_t_UNIT u,KLS_byte e){
    switch(e){
        case NET_EVENT_ERROR: printf("incoming error\n"); break;
        case NET_EVENT_TIMEOUT: NET_disconnect(u); break;
        case NET_EVENT_CONNECT: printf("client new\n"); break;
        case NET_EVENT_DISCONNECT: printf("client del\n"); break;
    }
}

void serverTcp(NET_t_UNIT u,KLS_byte e){
    switch(e){
        case NET_EVENT_ACCEPT:
            u->timeout=3;
            u->handler=incomingTcp;
            break;
        case NET_EVENT_ERROR: printf("error %p\n",u->userData); break;
        case NET_EVENT_TIMEOUT: printf("timeout %p\n",u->userData); break;
        case NET_EVENT_CONNECT: printf("connect %p\n",u->userData); break;
        case NET_EVENT_DISCONNECT: printf("disconnect %p\n",u->userData); break;
    }
}

static void **gmanager;
void sigInterp(int s){
    KLS_signalSetHandler(s,sigInterp);
    KLS_execKill();
    NET_interrupt(*gmanager);
    printf("\n\ninterrp\n\n");
}

int main(int argc,char **argv){
    NET_t_MANAGER m=NET_new(10,0,0);
    NET_t_UNIT u;
    gmanager=&m;

    KLS_signalSetMode(SIGINT,KLS_SIGNAL_UNBLOCK);
    KLS_signalSetHandler(SIGINT,sigInterp);

    if((u=NET_unit(m,NET_TCP)) && NET_listen(u,NET_address(NET_HOST_ANY4,12345),4)){
        u->timeout=5;
        u->handler=serverTcp;
        u->userData=(void*)0x1;
    }
    NET_detach(u);

    if((u=NET_unit(m,NET_TCP)) && NET_connect(u,NET_address(NET_HOST_LOCAL4,12345),3)){
        u->timeout=5;
        u->handler=serverTcp;
        u->userData=(void*)0x2;
    }
    NET_detach(u);

    while(KLS_execLive()){
        printf("service=%d\n",NET_service(m));
    }

    NET_free(&m);

    return 0;
    (void)argc; (void)argv;
}
