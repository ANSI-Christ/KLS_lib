#include "X11/Xlib.h"
#include "X11/keysym.h"
#include "X11/Xutil.h"

KLS_COLOR _KLS_rgbDetect(KLS_byte r,KLS_byte g,KLS_byte b){
    Display *d=XOpenDisplay(NULL);
    if(d){
        _KLS_rgbGetInfo(DefaultDepth(d, DefaultScreen(d)));
        XCloseDisplay(d);
    }else _KLS_rgbGetInfo(24);
    return KLS_RGB(r,g,b);
}

extern void _GUI_setMouse(GUI_t_DISPLAY *d,int x,int y);
extern int _GUI_setKeyboard(GUI_t_DISPLAY *d,int event,int symb,int code);

int _GUI_setBut(GUI_t_DISPLAY *d,XButtonEvent *e,int event){
    _GUI_setMouse(d,e->x,e->y);
    switch(e->button){
        case Button1: _GUI_setKeyboard(d,event,GUI_KEY_LB,0); return event;
        case Button2: _GUI_setKeyboard(d,event,GUI_KEY_WHEEL,0); return event;
        case Button3: _GUI_setKeyboard(d,event,GUI_KEY_RB,0); return event;
        case Button4: d->input.wheel=1; return GUI_EVENT_WHEEL;
        case Button5: d->input.wheel=-1; return GUI_EVENT_WHEEL;
    } return 0;
}

int _GUI_setKey(GUI_t_DISPLAY *d,XKeyEvent *e,int event){
#define GUI_KCASE(_f_,_t_) case XK_##_f_: _GUI_setKeyboard(d,event,GUI_KEY_##_t_,0); return event;
    KeySym k; unsigned char s[2]={0,0};
    XLookupString(e,(void*)s,sizeof(s),&k,0);
    _GUI_setMouse(d,e->x,e->y);
    switch(k){
        GUI_KCASE(End,END);
        GUI_KCASE(Home,HOME);
        GUI_KCASE(Insert,INS);
        GUI_KCASE(Page_Up,PAGEUP);
        GUI_KCASE(Page_Down,PAGEDOWN);
        GUI_KCASE(Left,LEFT);
        GUI_KCASE(Right,RIGHT);
        GUI_KCASE(Up,UP);
        GUI_KCASE(Down,DOWN);
        GUI_KCASE(Return,ENTER);
        GUI_KCASE(Escape,ESC);
        GUI_KCASE(BackSpace,BCSP);
        GUI_KCASE(Tab,TAB);
        GUI_KCASE(Delete,DEL);
        GUI_KCASE(Shift_L,SHIFT);
        GUI_KCASE(Shift_R,SHIFT);
        GUI_KCASE(Control_L,CTRL);
        GUI_KCASE(Control_R,CTRL);
        GUI_KCASE(Alt_L,ALT);
        GUI_KCASE(Alt_R,ALT);
        default: if(k<256) return _GUI_setKeyboard(d,event,s[0],k);
    }
    return GUI_EVENT_UPDATE;
#undef GUI_KCASE
}

int _GUI_nextEvent(GUI_t_DISPLAY *d,XEvent *e){
    while(1)
        if(XPending(d->_.osDep[0].p)>0){
            XNextEvent(d->_.osDep[0].p,e);
            return e->type;
        }else{
            fd_set set[1];
            int fd[2]={XConnectionNumber(d->_.osDep[0].p), d->_.osDep[4].i};
            FD_ZERO(set);
            FD_SET(fd[0],set);
            if(d->_.osDep[4].i!=-1) FD_SET(fd[1],set);
            if(select(fd[fd[0]<fd[1]]+1,set,NULL,NULL,NULL)>0 && FD_ISSET(fd[1],set)){
                read(fd[1],&e->type,sizeof(int));
                return GenericEvent;
            }
        }
    return -1;
}

int GUI_displayEvent(GUI_t_DISPLAY *d){
    #define GUI_KCASE(_ev_) case _ev_: _GUI_KCASE
    #define _GUI_KCASE(...) return ({__VA_ARGS__;})
    if(d->_.osDep[0].p){
        XEvent e[1];
        while(1)
            switch(_GUI_nextEvent(d,e)){
                GUI_KCASE(MotionNotify)(
                    if(XPending(d->_.osDep[0].p)>0 && ({XEvent p[1]; XPeekEvent(d->_.osDep[0].p,p); p->type==e->type;}) )
                        continue;
                    _GUI_setMouse(d,e->xmotion.x,e->xmotion.y);
                    GUI_EVENT_CURSOR
                );
                GUI_KCASE(KeyPress)(_GUI_setKey(d,(void*)e,GUI_EVENT_PRESS));
                GUI_KCASE(KeyRelease)(_GUI_setKey(d,(void*)e,GUI_EVENT_RELEASE));
                GUI_KCASE(ButtonPress)(_GUI_setBut(d,(void*)e,GUI_EVENT_PRESS));
                GUI_KCASE(ButtonRelease)(_GUI_setBut(d,(void*)e,GUI_EVENT_RELEASE));
                GUI_KCASE(ClientMessage)(GUI_EVENT_DESTROY);
                GUI_KCASE(GenericEvent)(e->type);
            }
    } return GUI_EVENT_DESTROY;
}

void _GUI_displayPost(GUI_t_DISPLAY *d,int value){
    if(d->_.osDep[0].p && d->_.osDep[4].i!=-1)
        write((&d->_.osDep[4].i)[1],&value,sizeof(value));
}

#if 1
    #define _KLS_pipeInit(_fd_) pipe(_fd_)
#else
static int _KLS_pipeInit(int fd[2]){
    struct _KLS_pipeStr{long fd[2];} s={-1,-1};
    struct _KLS_pipeStr(*f)(int fd[2])=(void*)pipe;
    fd[0]=fd[1]=-1; s=f(fd);
    if(s.fd[0]==-1) return -1;
    if(fd[0]==-1){
        fd[0]=s.fd[0];
        fd[1]=s.fd[1];
    }
    return fd[0]==-1;
}
#endif

GUI_t_DISPLAY GUI_displayNew(const char *title,int x,int y,int width,int height){
    GUI_t_DISPLAY res={};
    Display *d=XOpenDisplay(NULL);
    if(d){
        int s=DefaultScreen(d);
        Window w=XCreateSimpleWindow(d,RootWindow(d,s),x,y,50,50,1,BlackPixel(d,s),WhitePixel(d,s));
        Atom a=XInternAtom(d,"WM_DELETE_WINDOW",0);
        XSetWMProtocols(d,w,&a,1);
        XSelectInput(d,w,ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
        res._.osDep[0].p=d; res._.osDep[1].l=w;
        res._.osDep[2].p=DefaultGC(d,s);
        GUI_displaySetTitle(&res,title);
        GUI_displaySetSize(&res,width,height);
        if(_KLS_pipeInit(&res._.osDep[4].i))
            res._.osDep[4].i=-1;
    }
    return res;
}

void GUI_displaySetPos(GUI_t_DISPLAY *d,int x,int y){
    if(d && d->_.osDep[0].p && (x!=d->x || y!=d->y))
        XMoveWindow(d->_.osDep[0].p,d->_.osDep[1].l,(d->x=x),(d->y=y));
}

void _GUI_destroyImage(GUI_t_DISPLAY *d){
    if(d->_.osDep[3].p) XDestroyImage(((XImage*)d->_.osDep[3].p));
}

void GUI_displaySetSize(GUI_t_DISPLAY *d,int width,int height){
    if(d && d->_.osDep[0].p && (width!=d->width || height!=d->height)){
        XSizeHints *h=XAllocSizeHints();
        h->flags=PMinSize|PMaxSize;
        h->min_width=h->max_width=d->width=width;
        h->min_height=h->max_height=d->height=height;
        XSetNormalHints(d->_.osDep[0].p,d->_.osDep[1].l,h); XFree(h);
        XResizeWindow(d->_.osDep[0].p,d->_.osDep[1].l,width,height);
        XMapWindow(d->_.osDep[0].p,d->_.osDep[1].l);
        if(d->_.osDep[3].p) XDestroyImage(((XImage*)d->_.osDep[3].p));
        if( (h=(void*)malloc(width*height*sizeof(KLS_COLOR))) ){
            if( (d->_.osDep[3].p=XCreateImage(d->_.osDep[0].p,DefaultVisual(d->_.osDep[0].p,DefaultScreen(d->_.osDep[0].p)),KLS_COLOR_BITS,ZPixmap,0,(void*)h,width,height,XBitmapPad(d->_.osDep[0].p),0)) ){
                d->m=KLS_matrixNew((void*)h,height,width,sizeof(KLS_COLOR),NULL);
                return;
            }
            free(h);
        }
        GUI_displayFree(d);
    }
}

void GUI_displaySetTitle(GUI_t_DISPLAY *d,const char *title){
    if(d && title!=d->title && d->_.osDep[0].p)
        XStoreName(d->_.osDep[0].p,d->_.osDep[1].l,(d->title=title?title:"\000"));
}

void GUI_displayDraw(GUI_t_DISPLAY *d){
    if(d && d->_.osDep[0].p)
        XPutImage(d->_.osDep[0].p,d->_.osDep[1].l,d->_.osDep[2].p,d->_.osDep[3].p,0,0,0,0,d->m.columns,d->m.rows);
}

void GUI_displayFree(GUI_t_DISPLAY *d){
    if(d->_.osDep[0].p){
        XDestroyWindow(d->_.osDep[0].p,d->_.osDep[1].l);
        XCloseDisplay(d->_.osDep[0].p);
    }
    if(d->_.osDep[4].i!=-1){close((&d->_.osDep[4].i)[1]); close(d->_.osDep[4].i);}
    if(d->_.osDep[3].p) XDestroyImage(((XImage*)d->_.osDep[3].p));
    memset(d,0,sizeof(*d));
}
