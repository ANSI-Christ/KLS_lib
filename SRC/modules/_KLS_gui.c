
void _GUI_setMouse(GUI_t_DISPLAY *d,int x,int y){
    if(x<d->m._.rc){
        d->input.mouse.dx=x-d->input.mouse.x;
        d->input.mouse.x=x;
    }else d->input.mouse.dx=0;
    if(y<d->m._.rr){
        d->input.mouse.dy=y-d->input.mouse.y;
        d->input.mouse.y=y;
    }else d->input.mouse.dy=0;
}


int _GUI_setKeyboard(GUI_t_DISPLAY *d,int event,int symb,int code){
    d->input.key=d->input.keys;
    if(!code) code=symb;
    if((d->input.key & GUI_KEY_CTRL)) symb=code;
    if(symb & 0xffff) d->input.key&=~0xffff;
    d->input.key|=symb;
    if(event & GUI_EVENT_PRESS){
        d->input.keys=d->input.key;
    }else{
        if(symb & 0xffff) symb=0xffff;
        d->input.keys&=~symb;
    }
    return event;
}

void GUI_displayUpdate(GUI_t_DISPLAY *d){
    _GUI_displayPost(d,GUI_EVENT_UPDATE);
}

void GUI_displayInterrupt(GUI_t_DISPLAY *d){
    _GUI_displayPost(d,GUI_EVENT_INTERRUPT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void _GUI_widgetLink(CLASS GUI_WIDGET *w,CLASS GUI_WIDGET *p){
    if(p){
        if(p->last){
            p->last->next=w;
            w->prev=p->last;
            p->last=w;
        }else p->child=p->last=w;
        *(void**)KLS_UNCONST(&w->parent)=p;
        p->core.insert(w);
    }else if( (p=w->parent) ){
        if(w->prev) w->prev->next=w->next;
        else p->child=w->next;
        if(w->next) w->next->prev=w->prev;
        else p->last=w->prev;
        w->prev=w->next=NULL;
        *(void**)KLS_UNCONST(&w->parent)=NULL;
    }
}

void _GUI_widgetReorder(CLASS GUI_WIDGET *w){
    if(w!=(void*)w->gui){
        if(w->next){
            if(w->prev) w->prev->next=w->next;
            else w->parent->child=w->next;
            w->next->prev=w->prev;
            w->next=NULL;
            w->parent->last->next=w;
            w->prev=w->parent->last;
            w->parent->last=w;
        }
        _GUI_widgetReorder(w->parent);
    }
}

void _GUI_widgetFind(CLASS GUI_WIDGET *w,CLASS GUI_WIDGET *id[2]){
    if(w->id && !strcmp(w->id,(const char*)id[0])){
        id[1]=w; return;
    }
    w=w->child;
    while(w){
        _GUI_widgetFind(w,id);
        if(id[1]) return;
        w=w->next;
    }
}

KLS_byte _GUI_widgetIn(CLASS GUI_WIDGET *w,int x,int y){
    return KLS_matrixAt(&w->m,y-w->m.subRow,x-w->m.subColumn)!=NULL;
}

void *_GUI_widgetByXY(CLASS GUI_WIDGET *w,int x,int y){
    CLASS GUI_WIDGET *p=NULL;
    if(w && _GUI_widgetIn(w,x,y)){
_mark:
        p=w; w=w->last;
        while(w){
            if(w->able && _GUI_widgetIn(w,x,y))
                goto _mark;
            w=w->prev;
        }
    }
    return p;
}

void _GUI_widgetUpdate(CLASS GUI_WIDGET *w){
    w->core.update(w);
    if(w==w->gui) w->m=w->gui->display.m;
    else w->m=KLS_matrixGetMatrix(&w->parent->m,w->y,w->x,w->height,w->width,w->m.options);
    w=w->last;
    while(w){
        _GUI_widgetUpdate(w);
        w=w->prev;
    }
}

void _GUI_widgetDraw(CLASS GUI_WIDGET *w){
    if(w->visible){
        w->core.draw(w);
        w=w->child;
        while(w){
            _GUI_widgetDraw(w);
            w=w->next;
        }
    }
}

char _GUI_widgetBase(CLASS GUI_WIDGET *w,int e,int key){
    if(w){
        CLASS GUI *gui=w->gui;
        if((e & GUI_EVENT_CURSOR) && (gui->flags & 16)){
            KLS_TYPEOF(gui->display.input.mouse) const * const m=&gui->display.input.mouse;
            key=0;
            if(w->resizable){
                gui->flags|=32;
                key=gui->flags&15;
                if(gui->flags & 1){w->width+=m->dx;}
                if(gui->flags & 2){w->width-=m->dx; w->x+=m->dx;}
                if(gui->flags & 4){w->height+=m->dy;}
                if(gui->flags & 8){w->height-=m->dy; w->y+=m->dy;}
            }
            if((w->movable|w->detachable) && !key){
                gui->flags|=32;
                w->x=m->x-w->parent->m.subColumn-gui->wmv[0];
                w->y=m->y-w->parent->m.subRow-gui->wmv[1];
                if(w->detachable) w->m.options|=KLS_MATRIX_SUBUNLIM_H|KLS_MATRIX_SUBUNLIM_V;
            }
        }
        if((e & GUI_EVENT_PRESS) && (short)key==GUI_KEY_LB){
            int x=gui->wmv[0]=gui->display.input.mouse.x-w->m.subColumn, y=gui->wmv[1]=gui->display.input.mouse.y-w->m.subRow;
            gui->flags |= 16 | ((x<4)<<1) | ((x>w->width-4)) | ((y<4)<<3) | ((y>w->height-4)<<2);
        }
        if((e & GUI_EVENT_RELEASE) && (short)key==GUI_KEY_LB && ((gui->flags&=~31) & 32)){
            gui->flags&=~32;
            if(w->detachable && gui->block!=w){
                CLASS GUI_WIDGET *from=w->parent, *to;
                w->m.options&=~(KLS_MATRIX_SUBUNLIM_H|KLS_MATRIX_SUBUNLIM_V);
                _GUI_widgetLink(w,NULL);
                to=_GUI_widgetByXY(gui->block,gui->display.input.mouse.x,gui->display.input.mouse.y);
                _GUI_widgetLink(w,to?to:from);
                w->x=w->m.subColumn-w->parent->m.subColumn;
                w->y=w->m.subRow-w->parent->m.subRow;
            }
        }
        return gui->flags & 32;
    }
    return -1;
}

void _GUI_inputService(CLASS GUI *gui,int event,int key){
    if(event){
        if(!(gui->flags & 32)){ // current widget isn't moving or resizing
            if( (event & GUI_EVENT_CURSOR)
            && (gui->focus=_GUI_widgetByXY(gui->block,gui->display.input.mouse.x,gui->display.input.mouse.y))!=gui->select )
                return;
            if( ((event & (GUI_EVENT_RELEASE|GUI_EVENT_PRESS)) && ((short)key==GUI_KEY_LB || (short)key==GUI_KEY_RB || (short)key==GUI_KEY_WHEEL))
            || (event & GUI_EVENT_WHEEL) )
                gui->select=GUI_widgetSelect(gui->focus);
        }
        if(!_GUI_widgetBase(gui->select,event,key))
            while(gui->select){
                CLASS GUI_WIDGET *w=gui->select;
                GUI_t_INPUT i=gui->display.input;
                i.mouse.x-=w->m.subColumn;
                i.mouse.y-=w->m.subRow;
                w->core.input(w,event,&i);
                if(w->onInput) w->onInput(w,event,&i);
                if(w==gui->select) break;
            }
    }
}

void _GUI_widgetGetFocus(CLASS GUI *gui){
    if(gui->flags & 32) return; // current widget moving or resizing
    gui->focus=_GUI_widgetByXY(gui->block,gui->display.input.mouse.x,gui->display.input.mouse.y);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void GUI_widgetDrawRectExt(void *widget,int x,int y,int w,int h,const void *color,const void *fill){
    KLS_t_MATRIX *m=&((CLASS GUI_WIDGET*)widget)->m;
    int i,j;
    w+=x; h+=y;
    if(color){
        for(i=x,j=h-1;i<w;++i){
            KLS_matrixPutElement(m,y,i,color);
            KLS_matrixPutElement(m,j,i,color);
        }
        for(i=y,j=w-1;i<h;++i){
            KLS_matrixPutElement(m,i,x,color);
            KLS_matrixPutElement(m,i,j,color);
        }
    }
    if(fill){
        if(color){--w;--h;++x;++y;}
        for(i=x;i<w;++i)
            for(j=y;j<h;++j)
                KLS_matrixPutElement(m,j,i,fill);
    }
}

void GUI_widgetDrawText(void *widget,int x,int y,const char *text,const void *color){
    KLS_matrixPutString(&((CLASS GUI_WIDGET*)widget)->m,y,x,color,NULL,0,text);
}

void GUI_widgetDrawTextExt(void *widget,int x,int y,const KLS_t_FONT *font,KLS_byte align,const char *text,const void *color){
    KLS_matrixPutString(&((CLASS GUI_WIDGET*)widget)->m,y,x,color,font,align,text);
}

void GUI_widgetDrawRect(void *widget,int marginH,int marginV,const void *color,const void *fill){
    GUI_widgetDrawRectExt(widget,marginH,marginV,((CLASS GUI_WIDGET*)widget)->m.columns-(marginH<<1),((CLASS GUI_WIDGET*)widget)->m.rows-(marginV<<1),color,fill);
}

KLS_t_CANVAS GUI_widgetAsCanvas(void *widget){
    KLS_t_CANVAS c={.m=((CLASS GUI_WIDGET*)widget)->m};
    KLS_canvasArea(&c,0,0,c.m.columns,c.m.rows);
    return c;
}

void GUI_widgetDelete(CLASS GUI_WIDGET **widget){
    if(widget){
        CLASS GUI_WIDGET * const w=*widget;
        if(w){
            if(w->gui){
                if(GUI_widgetIsSelected(w))
                    w->gui->select=NULL;
                if(w->gui->flags & (1<<31)){
                    if(w!=(void*)w->gui){
                        if(w->gui->trash){
                            GUI_widgetInsert(w,w->gui->trash);
                            return;
                        }
                        w->core.insert=(void*)GUI_coreDefault;
                        _GUI_widgetLink((w->gui->trash=w),NULL);
                        return;
                    }
                    _GUI_displayPost(&w->gui->display,GUI_EVENT_DESTROY);
                    return;
                }
            }
            w->destructor(w);
            KLS_freeData(*widget);
        }
    }
}

void *GUI_widgetInsert(void *widget,void *parent){
    if(widget && parent && widget!=parent){
        _GUI_widgetLink(widget,NULL);
        _GUI_widgetLink(widget,parent);
        return ((CLASS GUI_WIDGET*)widget)->parent;
    } return NULL;
}

void *GUI_widgetFind(void *widget,const char *id){
    if(widget){
        if(id){
            CLASS GUI_WIDGET *ret[2]={KLS_UNCONST(id)};
            _GUI_widgetFind(widget,ret); return ret[1];
        } return (void*)((CLASS GUI_WIDGET*)widget)->gui;
    } return NULL;
}

void *GUI_widgetSelect(void *widget){
    if(widget){
        CLASS GUI *gui=((CLASS GUI_WIDGET*)widget)->gui;
        gui->select=widget;
        ((CLASS GUI_WIDGET*)widget)->core.select(widget);
        _GUI_widgetReorder(gui->select);
        return gui->select;
    }
    return NULL;
}

void *GUI_widgetBlockOn(void *widget){
    if(widget) KLS_swap(&widget,&((CLASS GUI_WIDGET*)widget)->gui->block,sizeof(widget));
    return widget;
}

KLS_byte GUI_widgetInFocus(void *widget){
    return ((CLASS GUI_WIDGET*)widget)->gui->focus==widget;
}

KLS_byte GUI_widgetIsSelected(void *widget){
    return ((CLASS GUI_WIDGET*)widget)->gui->select==widget;
}

KLS_byte GUI_widgetIsMoved(void *widget){
    if(widget==((CLASS GUI_WIDGET*)widget)->gui) return 0;
    return (((CLASS GUI_WIDGET*)widget)->x+((CLASS GUI_WIDGET*)widget)->parent->m.subColumn!=((CLASS GUI_WIDGET*)widget)->m.subColumn)
        | ((((CLASS GUI_WIDGET*)widget)->y+((CLASS GUI_WIDGET*)widget)->parent->m.subRow!=((CLASS GUI_WIDGET*)widget)->m.subRow)<<1);
}

KLS_byte GUI_widgetIsResized(void *widget){
    return (((CLASS GUI_WIDGET*)widget)->width!=((CLASS GUI_WIDGET*)widget)->m.columns)
        | ((((CLASS GUI_WIDGET*)widget)->height!=((CLASS GUI_WIDGET*)widget)->m.rows)<<1);
}

void GUI_coreDefault(){}

//////////////////////////////////////////////////////////////////////////////////////////////////

void _GUI_insertUp(CLASS GUI_WIDGET *w){
    GUI_widgetInsert(w,w->parent->parent);
}


CLASS_COMPILE(GUI_WIDGET)(
    constructor(parent,id)(
        self->core.draw=(void*)GUI_coreDefault;
        self->core.input=(void*)GUI_coreDefault;
        self->core.select=(void*)GUI_coreDefault;
        self->core.insert=(void*)GUI_coreDefault;
        self->core.update=(void*)GUI_coreDefault;
        *(void**)KLS_UNCONST(&self->id)=KLS_UNCONST(id);
        if(parent==self){
            *(void**)KLS_UNCONST(&self->gui)=(void*)self;
            parent=NULL;
        }
        if(parent){
            *(void**)KLS_UNCONST(&self->gui)=((CLASS GUI_WIDGET*)parent)->gui;
            _GUI_widgetLink(self,parent);
        }
    ),
    destructor()(
        CLASS GUI_WIDGET *w;
        _GUI_widgetLink(self,NULL);
        while( (w=self->child) ){
            w->destructor(w);
            KLS_free(w);
        }
    )
)

void _GUI_setFps(CLASS GUI *self,KLS_byte fps){
    if(fps){
        if(!self->timer) self->timer=KLS_timerCreate(self->update,self);
        self->tout=1000/fps;
    }else KLS_timerDestroy(&self->timer);
}

int _GUI_objService(CLASS GUI *gui){
    int e;
    KLS_timerStart(gui->timer,gui->tout,0,NULL,NULL);
    e=GUI_displayEvent(&gui->display);
    KLS_timerStop(gui->timer);
    if(e & (GUI_EVENT_PRESS|GUI_EVENT_RELEASE|GUI_EVENT_CURSOR|GUI_EVENT_WHEEL|GUI_EVENT_UPDATE)){
        gui->flags|=(1<<31); // service flag on
        _GUI_inputService(gui,e & ~GUI_EVENT_UPDATE,gui->display.input.key);
        _GUI_widgetUpdate((void*)gui);
        gui->flags&=~(1<<31); // service flag off
        GUI_widgetDelete(&gui->trash);
        _GUI_widgetGetFocus(gui);
        _GUI_widgetDraw((void*)gui);
        GUI_displayDraw(&gui->display);
    }
    return e;
}

void _GUI_objInterrupt(CLASS GUI *gui){
    GUI_displayInterrupt(&gui->display);
}

void _GUI_objFps(CLASS GUI *self){
    GUI_displayUpdate(&self->display);
}

void _GUI_objDraw(CLASS GUI *self){
    GUI_widgetDrawRect(self,0,0,NULL,&self->color);
}

void _GUI_objUpdate(CLASS GUI *self){
    GUI_BOX()->core.update(self);
    GUI_displaySetTitle(&self->display,self->title);
    GUI_displaySetSize(&self->display,self->width,self->height);
}

CLASS_COMPILE(GUI)(
    constructor()(
        *(void**)KLS_UNCONST(&self->service)=_GUI_objService;
        *(void**)KLS_UNCONST(&self->update)=_GUI_objFps;
        *(void**)KLS_UNCONST(&self->interrupt)=_GUI_objInterrupt;
        *(void**)KLS_UNCONST(&self->setFps)=_GUI_setFps;
        self->core.draw=(void*)_GUI_objDraw;
        self->core.update=(void*)_GUI_objUpdate;
        self->focus=self->select=self->block=(void*)self;
        self->display=GUI_displayNew((self->title=self->id),self->x,self->y,self->width,self->height);
        self->update(&self->display);
        ((CLASS GUI_WIDGET*)self)->m=self->display.m;
    ),
    destructor()(
        KLS_timerDestroy(&self->timer);
        GUI_displayFree(&self->display);
    )
)



void _GUI_lblDraw(CLASS GUI_LABEL *self){
    GUI_widgetDrawTextExt(self,1,1,&self->font,self->align,self->text,&self->color);
}

CLASS_COMPILE(GUI_LABEL)(
    constructor()(
        KLS_string(&self->text,text);
        self->core.draw=(void*)_GUI_lblDraw;
        self->core.insert=(void*)_GUI_insertUp;
    ),
    destructor()(
        KLS_string(&self->text,NULL);
    )
)


void _GUI_btnDraw(CLASS GUI_BUTTON *self){
    GUI_widgetDrawRect(self,0,0,&self->colorBorder,self->isPressed?&self->colorOn:&self->colorOff);
    GUI_widgetDrawTextExt(self,self->width>>1,self->height>>1,&self->font,KLS_ALIGN_H_MID|KLS_ALIGN_V_MID,self->text,&self->colorText);
    if(GUI_widgetInFocus(self)) GUI_widgetDrawRect(self,1,1,&self->colorBorder,NULL);
}

void _GUI_btnInput(CLASS GUI_BUTTON *self,int e,GUI_t_INPUT *i){
    short key=i->key;
    if( (e & GUI_EVENT_PRESS) && key==GUI_KEY_LB && self->pressable )
        self->isPressed=!self->isPressed;
    if(e & GUI_EVENT_WHEEL)
        GUI_widgetSelect(self->parent);
}

CLASS_COMPILE(GUI_BUTTON)(
    constructor()(
        KLS_string(&self->text,self->id);
        self->core.draw=(void*)_GUI_btnDraw;
        self->core.input=(void*)_GUI_btnInput;
    ),
    destructor()(
        KLS_string(&self->text,NULL);
    )
)




void _GUI_sliderValue(CLASS GUI_SLIDER *self){
    if(self->value>=self->max) self->value=self->max;
    else if(self->value<=self->min) self->value=self->min;
    else self->value=KLS_round(self->value,self->step);
}

void _GUI_sliderPosDraw(CLASS GUI_WIDGET *self){
    CLASS GUI_SLIDER *parent=(void*)self->parent;
    if(parent->able && (GUI_widgetInFocus(self) || GUI_widgetInFocus(parent)))
        GUI_widgetDrawRect(self,0,0,&parent->colorBorder,&parent->colorBackground);
    else GUI_widgetDrawRect(self,0,0,NULL,&parent->colorBorder);
}

void _GUI_sliderPosInput(CLASS GUI_WIDGET *self,int e,GUI_t_INPUT *i){
    if( (e & GUI_EVENT_PRESS) && (short)i->key==GUI_KEY_LB )
        return;
    GUI_widgetSelect(self->parent);
}

void _GUI_sliderDraw(CLASS GUI_SLIDER *self){
    GUI_widgetDrawRect(self,0,0,&self->colorBorder,&self->colorBackground);
}

CLASS_COMPILE(GUI_SLIDER)(
    constructor()(
        self->v=KLS_INF;
        self->p=GUI_widgetNew(GUI_WIDGET)(self,NULL);
        self->p->movable=1;
        self->p->core.draw=(void*)_GUI_sliderPosDraw;
        self->p->core.input=(void*)_GUI_sliderPosInput;
        self->p->core.insert=(void*)_GUI_insertUp;
        self->core.draw=(void*)_GUI_sliderDraw;
        self->core.insert=(void*)_GUI_insertUp;
    )
)




void _GUI_sliderInputH(CLASS GUI_SLIDER *self,int e,GUI_t_INPUT *i){
    if(e & GUI_EVENT_PRESS){
        short key=i->key;
        if(key==GUI_KEY_RIGHT)self->value+=self->step;
        if(key==GUI_KEY_LEFT)self->value-=self->step;
        if(key==GUI_KEY_LB){
            if(i->mouse.x<self->p->x) self->value-=self->step;
            if(i->mouse.x>self->p->x+self->p->width) self->value+=self->step;
        }
    }
    if(e & GUI_EVENT_WHEEL){
        self->value+=self->step*i->wheel;
    }
}

void _GUI_sliderUpdateH(CLASS GUI_SLIDER *self){
    CLASS GUI_WIDGET *p=self->p;
    unsigned int pix=fabs((self->max-self->min)/self->step);
    p->y=0;
    p->height=self->height;
    if(p->x<0) p->x=0;
    if((p->width=self->width/++pix)<5) p->width=5;
    if(p->x>self->width-p->width) p->x=self->width-p->width;
    if(GUI_widgetIsMoved(p)) self->value=KLS_interpol(self->min,0,self->max,self->width-p->width,p->x);
    _GUI_sliderValue(self);
    if(KLS_CMP(self->value,self->v,self->step/10.)){
        p->x=KLS_interpol(0,self->min,self->width-p->width,self->max,(self->v=self->value));
        if(self->onChange) self->onChange(self);
    }

}

CLASS_COMPILE(GUI_SLIDER_H)(
    constructor()(
        CLASS GUI_SLIDER *s=(void*)self;
        s->core.update=(void*)_GUI_sliderUpdateH;
        s->core.input=(void*)_GUI_sliderInputH;
        _GUI_sliderUpdateH(s);
    )
)

void _GUI_sliderInputV(CLASS GUI_SLIDER *self,int e,GUI_t_INPUT *i){
    if(e & GUI_EVENT_PRESS){
        short key=i->key;
        if(key==GUI_KEY_UP)self->value+=self->step;
        if(key==GUI_KEY_DOWN)self->value-=self->step;
        if(key==GUI_KEY_LB){
            if(i->mouse.y>self->p->y) self->value-=self->step;
            if(i->mouse.y<self->p->y+self->p->height) self->value+=self->step;
        }
    }
    if(e & GUI_EVENT_WHEEL){
        self->value+=self->step*i->wheel;
    }
}

void _GUI_sliderUpdateV(CLASS GUI_SLIDER *self){
    CLASS GUI_WIDGET *p=self->p;
    unsigned int pix=fabs((self->max-self->min)/self->step);
    p->x=0;
    p->width=self->width;
    if(p->y<0) p->y=0;
    if((p->height=self->height/++pix)<5) p->height=5;
    if(p->y>self->height-p->height) p->y=self->height-p->height;
    if(GUI_widgetIsMoved(p)) self->value=KLS_interpol(self->max,0,self->min,self->height-p->height,p->y);
    _GUI_sliderValue(self);
    if(KLS_CMP(self->value,self->v,self->step/10.)){
        p->y=KLS_interpol(0,self->max,self->height-p->height,self->min,(self->v=self->value));
        if(self->onChange) self->onChange(self);
    }
}

CLASS_COMPILE(GUI_SLIDER_V)(
    constructor()(
        CLASS GUI_SLIDER *s=(void*)self;
        s->core.update=(void*)_GUI_sliderUpdateV;
        s->core.input=(void*)_GUI_sliderInputV;
        _GUI_sliderUpdateV(s);
    )
)





void _GUI_boxSliderInputH(CLASS GUI_SLIDER *self,int e,GUI_t_INPUT *i){
    GUI_SLIDER_H()->core.input(self,e,i);
    if(e & GUI_EVENT_PRESS){
        short key=i->key;
        if(key==GUI_KEY_RIGHT || key==GUI_KEY_LEFT) return;
    }
    if(e & GUI_EVENT_WHEEL) return;
    GUI_widgetSelect(self->parent);
}

void _GUI_boxSliderInputV(CLASS GUI_SLIDER *self,int e,GUI_t_INPUT *i){
    GUI_SLIDER_V()->core.input(self,e,i);
    if(e & GUI_EVENT_PRESS){
        short key=i->key;
        if(key==GUI_KEY_UP || key==GUI_KEY_DOWN) return;
        if(key==GUI_KEY_PAGEUP){
            self->value+=((CLASS GUI_BOX*)self->parent)->box->height;
            return;
        }
        if(key==GUI_KEY_PAGEDOWN){
            self->value-=((CLASS GUI_BOX*)self->parent)->box->height;
            return;
        }
    }
    if(e & GUI_EVENT_WHEEL) return;
    GUI_widgetSelect(self->parent);
}

void _GUI_boxInput(CLASS GUI_BOX *self,int e,GUI_t_INPUT *i){
    if(e & GUI_EVENT_WHEEL){
        if(self->sliderV->visible) GUI_widgetSelect(self->sliderV);
        else if(self->sliderH->visible) GUI_widgetSelect(self->sliderH);
    }
    if(e & GUI_EVENT_PRESS){
        short key=i->key;
        if(self->sliderH->visible && (key==GUI_KEY_LEFT || key==GUI_KEY_RIGHT))
            GUI_widgetSelect(self->sliderH);
        if(self->sliderV->visible && (key==GUI_KEY_UP || key==GUI_KEY_DOWN || key==GUI_KEY_PAGEUP || key==GUI_KEY_PAGEDOWN))
            GUI_widgetSelect(self->sliderV);
    }
}

void _GUI_boxUpdate(CLASS GUI_BOX *self){
    CLASS GUI_SLIDER *h=self->sliderH, *v=self->sliderV;
    CLASS GUI_WIDGET *b=self->box;

    if(self->widthMax<self->width) self->widthMax=self->width;
    if(self->heightMax<self->height) self->heightMax=self->height;

    h->max=self->widthMax-self->width;
    h->able=h->visible=!!(unsigned int)fabs((h->max-h->min)/h->step);
    h->y=self->height-h->height-1;
    h->x=v->width+2;
    h->width=self->width-(h->x<<1);

    v->min=self->height-self->heightMax;
    v->able=v->visible=!!(unsigned int)fabs((v->max-v->min)/v->step);
    v->x=self->width-v->width-1;
    v->y=h->height+2;
    v->height=self->height-(v->y<<1);

    if(v->visible) b->width=v->x-b->x;
    else b->width=self->width-(b->x<<1);
    if(h->visible) b->height=h->y-b->y;
    else b->height=self->height-(b->y<<1);
}

void _GUI_boxImplUpdate(CLASS GUI_WIDGET *self){
    CLASS GUI_BOX *box=(void*)self->parent->parent;
    self->x=-box->sliderH->value;
    self->y=box->sliderV->value;
}

void _GUI_boxImplSelect(CLASS GUI_WIDGET *self){
    GUI_widgetSelect(self->parent);
}

void _GUI_boxInsert(CLASS GUI_WIDGET *other){
    GUI_widgetInsert(other,((CLASS GUI_BOX*)(other->parent))->box);
}

void _GUI_boxImplInsert(CLASS GUI_WIDGET *other){
    GUI_widgetInsert(other,other->parent->userData);
}

CLASS_COMPILE(GUI_BOX)(
    constructor()(
        self->sliderV=GUI_widgetNew(GUI_SLIDER_V)(self,NULL,0,0,5);
        self->sliderH=GUI_widgetNew(GUI_SLIDER_H)(self,NULL,0,0,5);
        self->box=GUI_widgetNew(GUI_WIDGET)(self,NULL);
        self->box->x=self->box->y=1;
        {
            CLASS GUI_WIDGET *w=self->box->userData=GUI_widgetNew(GUI_WIDGET)(self->box,NULL);
            w->width=w->height=(1u<<(sizeof(w->height)*8-1))-1;
            w->core.select=(void*)_GUI_boxImplSelect;
            w->core.update=(void*)_GUI_boxImplUpdate;
        }
        self->core.insert=(void*)_GUI_boxInsert;
        self->core.input=(void*)_GUI_boxInput;
        self->core.update=(void*)_GUI_boxUpdate;

        self->box->x=self->box->y=1;
        self->box->core.select=(void*)_GUI_boxImplSelect;
        self->box->core.insert=(void*)_GUI_boxImplInsert;

        self->sliderV->width=self->sliderH->height=10;

        self->sliderV->core.input=(void*)_GUI_boxSliderInputV;
        self->sliderH->core.input=(void*)_GUI_boxSliderInputH;
    )
)




void _GUI_textboxImplDraw(CLASS GUI_WIDGET *self){
    CLASS GUI_TEXTBOX *t=(void*)(self->parent->parent);
    if(t->able) GUI_widgetDrawRectExt(self,t->pos.x,t->pos.y,t->pos.w,t->pos.h,&t->colorText,NULL);
    GUI_widgetDrawTextExt(self,1,1,&t->font,0,t->text,&t->colorText);
}

void _GUI_textboxDraw(CLASS GUI_TEXTBOX *self){
    GUI_widgetDrawRect(self,0,0,&self->colorBorder,&self->colorBackground);
}

void _GUI_tbSetPos(CLASS GUI_TEXTBOX *self,const char *select,char rect){
    const KLS_t_FONT * const f=&self->font;
    self->pos.h=f->height+f->intervalRow;
    self->pos.w=f->width+f->intervalSymbol;
    if(self->text){
        if(select){
            const char *s=self->text;
            self->pos.y=self->pos.x=0;
            if(select<s)select=s;
            while(*s && s!=select){
                if(*s++=='\n'){
                    ++self->pos.y; self->pos.x=0; continue;
                } ++self->pos.x;
            }
            self->pos.i=s-self->text;
            self->pos.y*=self->pos.h;
            self->pos.x*=self->pos.w;
        }
        if(rect){ // FIX ME dergaetsya
            KLS_stringRect(self->text,f,&self->widthMax,&self->heightMax);
            self->widthMax+=self->pos.w+f->intervalSymbol;
            self->heightMax+=self->pos.h+f->intervalRow;
            if(self->heightMax>self->height) self->widthMax+=self->pos.w;
            if(self->widthMax>self->width) self->heightMax+=self->pos.h;
        }
        if(self->widthMax>self->width){
            if(self->pos.x<self->sliderH->value) self->sliderH->value=self->pos.x;
            if(self->pos.x+self->pos.w>self->sliderH->value+self->box->width) self->sliderH->value=self->pos.x+self->pos.w-self->box->width;
        }
        if(self->heightMax>self->height){
            if(self->pos.y<-self->sliderV->value) self->sliderV->value=-self->pos.y;
            if(self->pos.y+self->pos.h>self->box->height-self->sliderV->value) self->sliderV->value=-(self->pos.y+self->pos.h-self->box->height);
        }
    }
}

void _GUI_textboxInput(CLASS GUI_TEXTBOX *self,int e,GUI_t_INPUT *i){
    if(e & GUI_EVENT_WHEEL){
        if(self->sliderV->visible) GUI_widgetSelect(self->sliderV);
        else if(self->sliderH->visible) GUI_widgetSelect(self->sliderH);
        else GUI_widgetSelect(self->parent);
    }
    if(e & GUI_EVENT_PRESS){
        short key=i->key;
        if(key==GUI_KEY_LB || key==GUI_KEY_RB || key==GUI_KEY_WHEEL){
            _GUI_tbSetPos(self,KLS_stringPosition(self->text,&self->font,0,i->mouse.x+self->sliderH->value,i->mouse.y-self->sliderV->value),0);
        }else{
            switch(key){
                case GUI_KEY_LEFT:     _GUI_tbSetPos(self,self->text+self->pos.i-1,0); return;
                case GUI_KEY_RIGHT:    _GUI_tbSetPos(self,self->text+self->pos.i+1,0); return;
                case GUI_KEY_HOME:     _GUI_tbSetPos(self,self->text+self->pos.i-self->pos.x/self->pos.w,0); return;
                case GUI_KEY_END:      _GUI_tbSetPos(self,KLS_stringPosition(self->text+self->pos.i,&self->font,0,self->widthMax,0),0); return;
                case GUI_KEY_PAGEUP:   _GUI_tbSetPos(self,KLS_stringPosition(self->text,&self->font,0,self->pos.x,self->pos.y-self->box->width),0); return;
                case GUI_KEY_PAGEDOWN: _GUI_tbSetPos(self,KLS_stringPosition(self->text,&self->font,0,self->pos.x,self->pos.y+self->box->width),0); return;
                case GUI_KEY_UP:  if(self->pos.y) _GUI_tbSetPos(self,KLS_stringPosition(self->text,&self->font,0,self->pos.x,self->pos.y-self->pos.h),0); return;
                case GUI_KEY_DOWN:{
                    const char *s=KLS_stringPosition(self->text+self->pos.i,&self->font,0,self->pos.x,self->pos.h);
                    const char *c=self->text+self->pos.i;
                    while(c!=s && *c!='\n') ++c;
                    if(c!=s) _GUI_tbSetPos(self,s,0);
                } return;
            }
            if(self->editable){
                switch(key){
                    case 0: return;
                    case GUI_KEY_ENTER:
                        if((i->key & GUI_KEY_CTRL)) return;
                        key='\n';
                        break;
                    case GUI_KEY_TAB:
                        KLS_string(&self->text,"%.*s    %s",self->pos.i,self->text,self->text+self->pos.i);
                        _GUI_tbSetPos(self,self->text+self->pos.i+4,1);
                        return;
                    case GUI_KEY_BCSP:{
                        unsigned int l=strlen(self->text);
                        l-=KLS_arrayRemove(self->text,l,1,self->pos.i-1);
                        self->text[l]=0; _GUI_tbSetPos(self,self->text+self->pos.i-1,1);
                        return;
                    }
                    case GUI_KEY_DEL:{
                        unsigned int l=strlen(self->text);
                        l-=KLS_arrayRemove(self->text,l,1,self->pos.i);
                        self->text[l]=0; _GUI_tbSetPos(self,NULL,1);
                        return;
                    }
                }
                if(key<256 && strchr("`=\\qwertyuiop[]asdfghjkl;\'zxcvbnm,/!@#$%^&*()_|QWERTYUIOP{}ASDFGHJKL:\"ZXCVBNM<>?\n .-+1234567890",(char)key)){
                    KLS_string(&self->text,"%.*s%c%s",self->pos.i,self->text,(char)key,self->text+self->pos.i);
                    _GUI_tbSetPos(self,self->text+self->pos.i+1,1);
                }
            }
        }
    }
}

CLASS_COMPILE(GUI_TEXTBOX)(
    constructor()(
        self->editable=1;
        self->core.insert=(void*)_GUI_insertUp;
        self->core.input=(void*)_GUI_textboxInput;
        self->core.draw=(void*)_GUI_textboxDraw;
        KLS_string(&self->text,"%s","\000");
        _GUI_tbSetPos(self,self->text,1);
        {
            CLASS GUI_WIDGET *w=((CLASS GUI_BOX*)self)->box->userData;
            w->core.draw=(void*)_GUI_textboxImplDraw;
        }
    ),
    destructor()(
        KLS_string(&self->text,NULL);
    )
)



void _GUI_canvasUpdate(CLASS GUI_CANVAS *self){
    if(GUI_widgetIsResized(self)){
        KLS_t_CANVAS cnv=KLS_canvasNewExt(NULL,self->width,self->height,self->canvas.left,self->canvas.m.elSize,self->canvas.up,self->canvas.right,self->canvas.down);
        KLS_matrixTransform(&self->canvas.m,&cnv.m,NULL,NULL);
        KLS_canvasFree(&self->canvas); self->canvas=cnv;
    }
}

void _GUI_canvasDraw(CLASS GUI_CANVAS *self){
    const unsigned int w=self->width,h=self->height;
    const KLS_COLOR *p=self->canvas.m.data;
    const KLS_t_MATRIX * const m=&((CLASS GUI_WIDGET*)self)->m;
    KLS_COLOR *d;
    unsigned int x,y;
    for(y=0;y<h;++y)
        for(x=0;x<w;++x,++p)
            if( (d=KLS_matrixAt(m,y,x)) )
                *d=*p;
}

void _GUI_canvasInput(CLASS GUI_CANVAS *self,int e,GUI_t_INPUT *i){
    KLS_canvasAtPix(&self->canvas,i->mouse.x-self->canvas.m.subColumn,i->mouse.y-self->canvas.m.subRow,&self->p);
}

CLASS_COMPILE(GUI_CANVAS)(
    constructor()(
        self->core.update=(void*)_GUI_canvasUpdate;
        self->core.input=(void*)_GUI_canvasInput;
        self->core.draw=(void*)_GUI_canvasDraw;
        self->core.insert=(void*)_GUI_insertUp;
        self->canvas=KLS_canvasNew(NULL,self->width,self->height);
        ((CLASS GUI_WIDGET*)self)->m=self->canvas.m;
        {
            KLS_COLOR color=KLS_COLOR_WHITE;
            KLS_canvasClear(&self->canvas,&color);
        }
    ),
    destructor()(
        self->canvas.m._free=1;
        KLS_canvasFree(&self->canvas);
    )
)

