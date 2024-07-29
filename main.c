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

void sigInterp(int s){
    KLS_signalSetHandler(s,KLS_SIGNAL_MODE_DEFAULT,sigInterp);
    KLS_execKill();
    printf("\n\ninterrp\n\n");
}

int mainNet(int argc,char **argv){
    int s;
    NET_t_MANAGER m=NET_new(10,0,0);
    NET_t_UNIT u;

    KLS_signalSetHandler(SIGINT,KLS_SIGNAL_MODE_UNBLOCK,sigInterp);

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
        s=NET_service(m,5);
        printf("service=%d\n",s);
        if(s<0) break;
    }

    NET_free(&m);
    return 0;
    (void)argc; (void)argv;
}


void main2(){
    int i,j;
    char e='@';

    KLS_t_MATRIX m=KLS_matrixNew(NULL,25,50,sizeof(e),0);
    memset(m.data,0,m.rows*m.columns);

    KLS_matrixPutString(&m,12,2,&e,0,0,"abc");

    KLS_matrixPrint(&m,KLS_LMBD(void,(char *p){
        if(p) printf("%c",*p);
        else printf("\n");
    }));

    KLS_matrixFree(&m);
}

void main42(){
    int w,h;
    char e='#';
    char align=KLS_ALIGN_H_MID | KLS_ALIGN_V_MID;
    const char *s="where\nare\nfucking\nwomans";
    KLS_t_MATRIX m=KLS_matrixNew(NULL,60,100,1,0);
    memset(m.data,'_',m.rows*m.columns);


    KLS_matrixPutString(&m,30,50,&e,0,align,s);

    KLS_matrixPrint(&m,KLS_LMBD(void,(char *p){
        if(p) printf("%c",*p);
        else printf("\n");
    }));


    KLS_matrixFree(&m);

    KLS_stringRect(s,0,&w,&h);
    printf("rect: [%d][%d]\n",w,h);

    printf("\n{\n%s\n}\n",KLS_stringPosition(s,0,align,3-40,7+14));

}

void sigInterp2(int s){
    KLS_signalSetHandler(s,KLS_SIGNAL_MODE_DEFAULT,sigInterp2);
    KLS_execKill();
    //printf("\n\ninterrp\n\n");
}

#define CLASS_BEGIN__CA \
    constructor(int a,float b)(\
        self->a=a; self->b=b;\
    ),\
    public(\
        int a;\
        float b;\
    ),\
    destructor()(\
        printf("%d %f\n",self->a,self->b);\
    )
CLASS_END(CA);
CLASS_COMPILE(CA)();


void pooltask(struct{int id; double b;} *i){
    KLS_LOG("(%d):%d %f\n",KLS_threadPoolSelfNum(),i->id,i->b);
    if(i->id<5){
        KLS_pausef(0.1);
        KLS_threadPoolTask(KLS_threadPoolSelf(),pooltask,i->id+1,0.001);
        KLS_pausef(0.1);
        return;
    }

    //KLS_pausef(2);
}

int main(){
    int i;
    if(1){
        int j;
        KLS_logSetCreation(1,fopen("./log.txt","w"),KLS_LOG_TIME|KLS_LOG_CLOSE);
        KLS_t_THREAD_POOL pool=KLS_threadPoolCreate(4,0);
        for(j=0;j<10;++j) KLS_threadPoolTask(pool,pooltask,0,1111.);
        while(!KLS_threadPoolWaitTime(pool,100))
            printf("nwait...\n");
        printf("\n\n\n\nwait all\n\n\n");
        KLS_threadPoolDestroy(&pool);
    }

    KLS_t_TIMER timers[]={KLS_timerCreate(0,0),KLS_timerCreate(0,0)};
    CLASS GUI *gui=GUI_widgetNew(GUI)(640,480);
    CLASS GUI_BUTTON *b1=GUI_widgetNew(GUI_BUTTON)(gui,"b1");
    CLASS GUI_SLIDER *s1=GUI_widgetNew(GUI_SLIDER_H)(gui,"s1",0,3,1);
    CLASS GUI_SLIDER *s2=GUI_widgetNew(GUI_SLIDER_V)(gui,"s2",-99,0,1.5);
    CLASS GUI_CANVAS *cnv=GUI_widgetNew(GUI_CANVAS)(gui,"plot",300,200);
    CLASS GUI_LABEL *dateTime=GUI_widgetNew(GUI_LABEL)(gui,"dateTime","                                             ");
    CLASS GUI_TEXTBOX *t=GUI_widgetNew(GUI_TEXTBOX)(gui,"textbox",200,200);
    CLASS GUI_PROGRESS *ind=GUI_widgetNew(GUI_PROGRESS)(gui,"progress");

    gui->setFps(gui,30);
    gui->widthMax=gui->width<<2;
    gui->heightMax=gui->height<<2;


    KLS_signalSetHandler(SIGINT,KLS_SIGNAL_MODE_UNBLOCK,SIG_IGN);

    ind->x=100;
    ind->y=10;
    ind->width=300;

    KLS_timerStart(timers[0],100,100,KLS_LMBD(void,(CLASS GUI_PROGRESS *self,int *ms){
        self->value+=0.5;
        if(self->value>100){
            *ms=0;
            self->value=0;
        }
    }),ind);

    KLS_timerStart(timers[1],0,1000,KLS_LMBD(void,(CLASS GUI_LABEL *l){
        KLS_t_DATETIME dt=KLS_dateTimeSystem();
        sprintf(l->text,"%0*d.%0*d.%0*d\n%0*d.%0*d.%0*d",2,dt.day,2,dt.month,4,dt.year,2,dt.hour,2,dt.minute,2,dt.second);
        KLS_stringRect(l->text,NULL,&l->width,&l->height);
    }),dateTime);

    dateTime->x=dateTime->y=5;
    dateTime->detachable=1;

    cnv->x=300;
    cnv->y=100;
    cnv->onInput=KLS_LMBD(void,(CLASS GUI_CANVAS *c,int e,GUI_t_INPUT *i){
        if((e & GUI_EVENT_PRESS) && (short)i->key==GUI_KEY_LB){
            KLS_COLOR color=KLS_COLOR_BLUE;
            KLS_canvasPoint(&c->canvas,c->p.x,c->p.y,&color,3);
        }
        if((e & GUI_EVENT_CURSOR) && (short)i->keys==GUI_KEY_LB){
            KLS_COLOR color=KLS_COLOR_BLUE;
            KLS_canvasPoint(&c->canvas,c->p.x,c->p.y,&color,3);
        }
    });

    b1->x=50;
    b1->y=50;
    b1->pressable=1;
    b1->onInput=KLS_LMBD(void,(CLASS GUI_BUTTON *b,int e,GUI_t_INPUT *i){
        /*if((e & GUI_EVENT_PRESS) && (short)i->key==GUI_KEY_LB){
            if(b->isPressed) GUI_widgetBlockOn(b);
            else{GUI_widgetBlockOn(b->gui); GUI_widgetDelete(&b);}
        }*/
        if((e & GUI_EVENT_PRESS) && (short)i->key==GUI_KEY_LB){
            KLS_COLOR color=KLS_COLOR_WHITE;
            KLS_canvasClear(&cnv->canvas,&color);
        }
    });
    b1->core.draw=KLS_LMBD(void,(CLASS GUI_BUTTON *b){
        KLS_t_CANVAS dst=GUI_widgetAsCanvas(b);
        KLS_t_CANVAS *src=&cnv->canvas;
        KLS_matrixTransform(&src->m,&dst.m,NULL,NULL);
    });

    s1->x=50;
    s1->y=80;
    s1->onChange=KLS_LMBD(void,(CLASS GUI_SLIDER *s){
        printf("s1 value:%f\n",s->value);
    });

    s2->detachable=1;
    s2->x=50;
    s2->y=100;
    s2->onChange=KLS_LMBD(void,(CLASS GUI_SLIDER *s){
        printf("s2 value:%f\n",s->value);
    });


    t->x=10;
    t->y=300;
    t->font.width=10;
    t->font.height=24;
    t->font.intervalRow=t->font.intervalSymbol=4;
    t->colorBackground=KLS_COLOR_LIGHT_GREY;
    t->onInput=KLS_LMBD(void,(CLASS GUI_TEXTBOX *t,int e,GUI_t_INPUT *i){
        if( (e & GUI_EVENT_PRESS) && i->key==(GUI_KEY_ENTER|GUI_KEY_CTRL))
        printf("text:%s\n",t->text);
    });

    while(KLS_execLive()){
        int s;
        s=gui->service(gui);
        printf("event=%d\n",s);
        if(!s) break;
        //printf("event=%d\n",s);
    }
    for(i=0;i<KLS_ARRAY_LEN(timers);++i)
        KLS_timerDestroy(timers+i);
    GUI_widgetDelete(&gui);
}
