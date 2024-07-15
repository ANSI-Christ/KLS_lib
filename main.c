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
        u->userData=0x1;
    }
    NET_detach(u);

    if((u=NET_unit(m,NET_TCP)) && NET_connect(u,NET_address(NET_HOST_LOCAL4,12345),3)){
        u->timeout=5;
        u->handler=serverTcp;
        u->userData=0x2;
    }
    NET_detach(u);

    while(KLS_execLive()){
        s=NET_service(m,5);
        printf("service=%d\n",s);
        if(s<0) break;
    }

    NET_free(&m);
    return 0;
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


void main1(){
    int w,h;
    char e='#';
    char align=KLS_ALIGN_H_MID | KLS_ALIGN_V_TOP;
    const char *s="where\nare\nfucking\nwomans";
    KLS_t_MATRIX m=KLS_matrixNew(NULL,60,80,1,0);
    memset(m.data,'_',m.rows*m.columns);


    KLS_matrixPutString(&m,0,0,&e,0,align,s);

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

void main(){
    int i;


    KLS_t_TIMER timers[]={KLS_timerCreate(0,0),KLS_timerCreate(0,0)};
    CLASS GUI *gui=GUI_widgetNew(GUI)(640,480);
    CLASS GUI_BUTTON *b1=GUI_widgetNew(GUI_BUTTON)(gui,"b1");
    CLASS GUI_SLIDER *s1=GUI_widgetNew(GUI_SLIDER_H)(gui,"s1",0,3,1);
    CLASS GUI_SLIDER *s2=GUI_widgetNew(GUI_SLIDER_V)(gui,"s2",-99,0,1.5);
    CLASS GUI_CANVAS *cnv=GUI_widgetNew(GUI_CANVAS)(gui,"plot",300,200);
    CLASS GUI_LABEL *dateTime=GUI_widgetNew(GUI_LABEL)(gui,"dateTime",NULL);

    gui->setFps(gui,30);
    gui->widthMax=gui->width<<2;
    gui->heightMax=gui->height<<2;

    KLS_signalSetHandler(SIGINT,KLS_SIGNAL_MODE_UNBLOCK,SIG_IGN);

    KLS_timerStart(timers[1],0,1000,KLS_LMBD(void,(CLASS GUI_LABEL *l){
        KLS_t_DATETIME dt=KLS_dateTimeSystem();
        KLS_string(&l->text,"%0*d.%0*d.%0*d\n%0*d.%0*d.%0*d",2,dt.day,2,dt.month,4,dt.year,2,dt.hour,2,dt.minute,2,dt.second);
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


    if(1){
        CLASS _GUI_TEXTBOX *t=GUI_widgetNew(GUI_TEXTBOX)(gui,"textbox",200,200);
        t->x=10;
        t->y=300;
        t->font.width=8;
        t->font.height=16;
        t->font.intervalRow=t->font.intervalSymbol=3;
        t->colorBackground=KLS_COLOR_LIGHT_GREY;
        t->onInput=KLS_LMBD(void,(CLASS GUI_TEXTBOX *t,int e,GUI_t_INPUT *i){
            if( (e & GUI_EVENT_PRESS) && i->key==(GUI_KEY_ENTER|GUI_KEY_CTRL))
                printf("text:%s\n",t->text);
        });
    }

    while(KLS_execLive()){
        int s;
        s=gui->service(gui);

        if(!s) break;
        //printf("event=%d\n",s);
    }

    for(i=0;i<KLS_ARRAY_LEN(timers);++i)
        KLS_timerDestroy(timers+i);
    GUI_widgetDelete(&gui);
}


#define CLASS_BEGIN__A \
    constructor(float x,float y)(\
    (void)x;(void)y;\
    )
CLASS_END(A);
CLASS_COMPILE(A)(

);

#define CLASS_BEGIN__B \
    extends(A),\
    constructor(float x,float y,float z)(\
        super(self,x,y);\
        (void)z;\
    )
CLASS_END(B);
CLASS_COMPILE(B)(

);

#define CLASS_BEGIN__C \
    extends(A),\
    constructor()(\
        super(self,4,5);\
    )
CLASS_END(C);
CLASS_COMPILE(C)(

);

multitype mftx(int x){
    float a=x*1.3;
    int b=x<<1;
    char c='0'+x;
    return (a,b,c);
}


void _task1(KLS_THREAD_ARGS(int x,int y,int z) i){
    float a;
    int b;
    char c;
    printf("%d %d %d\n",i->x,i->y,i->z);
    printf("%d\n",KLS_MIN(3,0,1,2,0,3));
    let(,,c)=mftx(2);
    printf("%f %d %c\n\n",a,b,c);
}

int mainT()
{
    {
        KLS_t_THREAD t=KLS_threadCreate(1);

        KLS_threadTask(t,_task1,2,3,4);

        KLS_threadDestroyLater(&t);
    }
    {
        CLASS A c;
        A()->constructor(&c,1,2);
        printf("dtor %p\n",c.destructor);
    }

     {
        CLASS A c;
        B()->constructor(&c,1,2,3);
        printf("dtor %p\n",c.destructor);
    }

    {
        CLASS A c;
        C()->constructor(&c);
        printf("dtor %p\n",c.destructor);
    }
    #define _M1(_1_,_2_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))(printf("bla\n");)
    M_FOREACH(_M1,-,)
    //printf("%f, %f, %d\n",c.x,c.y,c.z);

    return 0;
}

double GxG[2048]={1.3,6.6};


void mainTt(){
    int i;

    printf("rt 1 : %f\n",KLS_RUNTIME(
        for(i=0;i<100000;++i) memmove(GxG,GxG+KLS_ARRAY_LEN(GxG)/2,sizeof(GxG)/2);
    ));
    printf("rt 2 : %f\n",KLS_RUNTIME(
        for(i=0;i<100000;++i) KLS_memmove(GxG,GxG+KLS_ARRAY_LEN(GxG)/2,sizeof(GxG)/2);
    ));
    printf("rt 3 : %f\n",KLS_RUNTIME(
        //for(i=0;i<100000;++i) KLS_memmove2(GxG,GxG+KLS_ARRAY_LEN(GxG)/2,sizeof(GxG)/2);
    ));

    printf("\n\n%f\n\n",GxG[1024]);

    printf("rt 1 : %f\n",KLS_RUNTIME(
        for(i=0;i<100000;++i) memmove(GxG+KLS_ARRAY_LEN(GxG)/2,GxG,sizeof(GxG)/2);
    ));
    printf("rt 2 : %f\n",KLS_RUNTIME(
        for(i=0;i<100000;++i) KLS_memmove(GxG+KLS_ARRAY_LEN(GxG)/2,GxG,sizeof(GxG)/2);
    ));
    printf("rt 3 : %f\n",KLS_RUNTIME(
        //for(i=0;i<100000;++i) KLS_memmove2(GxG+KLS_ARRAY_LEN(GxG)/2,GxG,sizeof(GxG)/2);
    ));

    printf("\n\n%f\n\n",GxG[0]);
}
