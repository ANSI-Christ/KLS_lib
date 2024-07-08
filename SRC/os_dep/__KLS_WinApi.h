#include <windows.h>
#include <windowsx.h>

KLS_COLOR _KLS_rgbDetect(KLS_byte r,KLS_byte g,KLS_byte b){
    void *w=GetDC(0);
    if(w){
        _KLS_rgbGetInfo(GetDeviceCaps(w,BITSPIXEL));
        ReleaseDC(0,w);
    }else _KLS_rgbGetInfo(24);
    return KLS_RGB(r,g,b);
}

extern void _GUI_setMouse(GUI_t_DISPLAY *d,int x,int y);
extern int _GUI_setKeyboard(GUI_t_DISPLAY *d,int event,int symb,int code);

int _GUI_setKey(GUI_t_DISPLAY *d,MSG *e,int event){
#define GUI_KCASE(_f_,_t_) case VK_##_f_: _GUI_setKeyboard(d,event,GUI_KEY_##_t_,0); return event;
    switch(e->wParam){
        GUI_KCASE(END,END);
        GUI_KCASE(HOME,HOME);
        GUI_KCASE(LEFT,LEFT);
        GUI_KCASE(RIGHT,RIGHT);
        GUI_KCASE(UP,UP);
        GUI_KCASE(DOWN,DOWN);
        GUI_KCASE(RETURN,ENTER);
        GUI_KCASE(ESCAPE,ESC);
        GUI_KCASE(BACK,BCSP);
        GUI_KCASE(TAB,TAB);
        GUI_KCASE(DELETE,DEL);
        GUI_KCASE(SHIFT,SHIFT);
        GUI_KCASE(CONTROL,CTRL);
        GUI_KCASE(RBUTTON,RB);
        GUI_KCASE(LBUTTON,LB);
        GUI_KCASE(MBUTTON,WHEEL);
        GUI_KCASE(NONAME,ALT);
        GUI_KCASE(PRIOR,PAGEUP);
        GUI_KCASE(NEXT,PAGEDOWN);
        GUI_KCASE(INSERT,INS);
        default:{
            unsigned char k[2],s[256];
            if(GetKeyboardState(s) && ToAscii(e->wParam,e->lParam,s,k,0)==1)
                return _GUI_setKeyboard(d,event,k[0],e->wParam);
        }
    } printf("win key:%x\n",e->wParam); return GUI_EVENT_UPDATE;
#undef GUI_KCASE
}


int GUI_displayEvent(GUI_t_DISPLAY *d){
#define GUI_KCASE(_ev_) case WM_##_ev_: _GUI_KCASE
#define _GUI_KCASE(...) return ({__VA_ARGS__;})
    if(d->_.osDep[0].p){
        MSG e[1];
        while(GetMessage(e,d->_.osDep[0].p,0,0)>0){
            DispatchMessage(e);
            switch(e->message){
                GUI_KCASE(MOUSEMOVE)(
                    if( ({MSG m[1]; PeekMessage(m,d->_.osDep[0].p,0,0,PM_NOREMOVE) && m->message==e->message;}) )
                        continue;
                    _GUI_setMouse(d,GET_X_LPARAM(e->lParam),GET_Y_LPARAM(e->lParam));
                    GUI_EVENT_CURSOR
                );
                GUI_KCASE(SYNCPAINT)(
                    if( ({MSG m[1]; PeekMessage(m,d->_.osDep[0].p,0,0,PM_NOREMOVE) && m->message==e->message && m->lParam==e->lParam;}) )
                        continue;
                    e->lParam
                );
                GUI_KCASE(KEYDOWN)(_GUI_setKey(d,e,GUI_EVENT_PRESS));
                GUI_KCASE(KEYUP)(_GUI_setKey(d,e,GUI_EVENT_RELEASE));
                GUI_KCASE(LBUTTONDOWN)(e->wParam=VK_LBUTTON; _GUI_setKey(d,e,GUI_EVENT_PRESS));
                GUI_KCASE(LBUTTONUP)(e->wParam=VK_LBUTTON; _GUI_setKey(d,e,GUI_EVENT_RELEASE));
                GUI_KCASE(RBUTTONDOWN)(e->wParam=VK_RBUTTON; _GUI_setKey(d,e,GUI_EVENT_PRESS));
                GUI_KCASE(RBUTTONUP)(e->wParam=VK_RBUTTON; _GUI_setKey(d,e,GUI_EVENT_RELEASE));
                GUI_KCASE(MBUTTONDOWN)(e->wParam=VK_MBUTTON; _GUI_setKey(d,e,GUI_EVENT_PRESS));
                GUI_KCASE(MBUTTONUP)(e->wParam=VK_MBUTTON; _GUI_setKey(d,e,GUI_EVENT_RELEASE));
                GUI_KCASE(SYSKEYDOWN)(e->wParam=VK_NONAME; _GUI_setKey(d,e,GUI_EVENT_PRESS));
                GUI_KCASE(SYSKEYUP)(e->wParam=VK_NONAME; _GUI_setKey(d,e,GUI_EVENT_RELEASE));
                GUI_KCASE(MOUSEWHEEL)(d->input.wheel=GET_WHEEL_DELTA_WPARAM(e->wParam)>0?1:-1; GUI_EVENT_WHEEL);
            }
        }
    }
    return GUI_EVENT_DESTROY;
#undef GUI_KCASE
#undef _GUI_KCASE
}

void _GUI_displayPost(GUI_t_DISPLAY *d,int value){
    if(d->_.osDep[0].p) PostMessage(d->_.osDep[0].p,WM_SYNCPAINT,0,value);
}

void _displayClassFree();
const void *_displayClassReg(){
    KLS_ONCE(
        WNDCLASS wc;
        memset(&wc,0,sizeof(wc));
        wc.lpfnWndProc=DefWindowProc;
        wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
        wc.lpszClassName=_displayClassReg();
        if(RegisterClass(&wc)) atexit(_displayClassFree);
    ) return L"_KLS_displayClass";
}
void _displayClassFree(){UnregisterClass(_displayClassReg(),0);}


GUI_t_DISPLAY GUI_displayNew(const char *title,int x,int y,int width,int height){
    GUI_t_DISPLAY res={};
    HWND w=CreateWindow(_displayClassReg(),L"\0\0",WS_VISIBLE|WS_CAPTION|WS_BORDER|WS_MINIMIZEBOX|WS_SYSMENU,x,y,50,50,NULL,NULL,NULL,NULL);
    if(w){
        UpdateWindow(w);
        res._.osDep[0].p=w; res._.osDep[1].p=GetDC(w);
        GUI_displaySetTitle(&res,title);
        GUI_displaySetSize(&res,width,height);
    }
    return res;
}

void GUI_displaySetPos(GUI_t_DISPLAY *d,int x,int y){
    if(d && d->_.osDep[0].p && (x!=d->x || y!=d->y) && SetWindowPos(d->_.osDep[0].p,0,x,y,0,0,SWP_NOSIZE)){
        d->x=x; d->y=y;
    }
}

void GUI_displaySetSize(GUI_t_DISPLAY *d,int width,int height){
    if(d && d->_.osDep[0].p && (width!=d->width || height!=d->height) && SetWindowPos(d->_.osDep[0].p,0,0,0,width+6,height+28,SWP_NOMOVE)){
        d->width=width;
        d->height=height;
        KLS_freeData(d->_.osDep[2].p);
        if( (d->_.osDep[2].p=KLS_malloc(sizeof(BITMAPINFO)+width*height*sizeof(KLS_COLOR))) ){
            *(BITMAPINFO*)d->_.osDep[2].p=(BITMAPINFO){{sizeof(BITMAPINFOHEADER), width, 1,1, KLS_COLOR_BITS, BI_RGB, width*sizeof(KLS_COLOR)}};
            d->m=KLS_matrixNew(d->_.osDep[2].p+sizeof(BITMAPINFO),height,width,sizeof(KLS_COLOR),NULL);
            return;
        }
        GUI_displayFree(d);
    }
}

void GUI_displaySetTitle(GUI_t_DISPLAY *d,const char *title){
    if(d && title!=d->title && d->_.osDep[0].p)
        SetWindowTextA(d->_.osDep[0].p,(d->title=title?title:"\0"));
}

void GUI_displayDraw(GUI_t_DISPLAY *d){
    if(d && d->_.osDep[0].p){
        unsigned int r=d->m.rows;
        while(r--) SetDIBitsToDevice(d->_.osDep[1].p,0,r,d->m.columns,1,0,0,0,1,KLS_matrixAt(&d->m,r,0),d->_.osDep[2].p,DIB_RGB_COLORS);
    }
}

void GUI_displayFree(GUI_t_DISPLAY *d){
    if(d->_.osDep[0].p){
        MSG msg;
        ReleaseDC(d->_.osDep[0].p, d->_.osDep[1].p);
        DestroyWindow(d->_.osDep[0].p);
    }
    KLS_free(d->_.osDep[2].p);
    KLS_matrixFree(&d->m);
    memset(d,0,sizeof(*d));
}
