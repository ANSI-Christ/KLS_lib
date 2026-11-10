/* * * * * * * * * * * * * * * * * */
/* MIT License                     */
/* Copyright (c) 2024 ANSI-Christ  */
/* * * * * * * * * * * * * * * * * */

#ifndef PILE_H
#define PILE_H

#include <stddef.h>

void pile_release(void *data);
void *pile_request(void *pile,size_t size);

int pile_init(void *area,size_t size,void(*fini)(void *area));
int pile_increase(void *pile,void *area,size_t size,void(*fini)(void *area));

void pile_close(void *pile);
void pile_info(const void *pile);


#define PILE(_name_,_size_) \
    struct{\
        struct _pile_header h;\
        struct _pile_node n;\
        unsigned char pile[(_size_) & ~(sizeof(void*)-1)];\
        struct _pile_able able;\
    }_name_[1]={{\
        { _name_, (void(*)(void*))0, (struct _pile_able*)((char*)(_name_+1)-sizeof(struct _pile_able)), NULL },\
        { (struct _pile_header*)_name_, NULL, (struct _pile_node**)((char*)(_name_+1)-sizeof(struct _pile_able)), (_size_) & ~(sizeof(void*)-1) },\
        { 0 },\
        { (struct _pile_node*)(((struct _pile_header*)_name_)+1), (struct _pile_node**)((char*)(_name_+1)-sizeof(struct _pile_able)) }\
    }}


struct _pile_header{
    void *area;
    void(*deallocator)(void*);
    struct _pile_able *able;
    struct _pile_header *next;
};

struct _pile_able{
    struct _pile_node *first, **last;
};

struct _pile_node{
    struct _pile_header *header;
    struct _pile_node *prev, **able;
    size_t size;
};

#endif /* PILE_H */



#ifdef PILE_IMPL

#include <stdio.h>

#define _pile_size(_n_) (_n_->size & ~(sizeof(void*)-1))
#define _pile_next(_n_) ((struct _pile_node*)(_n_->size & 1 ? ((char*)((_n_)+1))+_n_->size-1 : NULL))

static int _pile_is_init(const void * const h){
    return h && ((const struct _pile_header*)h)->area==h;
}

int pile_init(void * const area,size_t size,void(* const fini)(void *area)){
    struct _pile_default{ struct _pile_header h; struct _pile_node n; struct _pile_able a;};
    if(!area || ((size_t)area) & (sizeof(void*)-1))
        return -1;
    if( (size&=~(sizeof(void*)-1))>sizeof(struct _pile_default) ){
        struct _pile_header * const h=(struct _pile_header*)area;
        struct _pile_node * const n=(struct _pile_node*)(h+1);
        n->header=(struct _pile_header*)(h->area=area);
        h->next=NULL;
        h->deallocator=fini;
        h->able=(struct _pile_able*)((char*)area+size-sizeof(struct _pile_able));
        h->able->first=n;
        h->able->last=n->able=&h->able->first;
        n->prev=NULL;
        n->size=size-sizeof(struct _pile_default);
        return 0;
    } return -2;
}

int pile_increase(void * const pile,void * const area,const size_t size,void(* const fini)(void *area)){
    int err=-3;
    if(_pile_is_init(pile) && !(err=pile_init(area,size,fini)) ){
        struct _pile_header * const h=(struct _pile_header*)pile;
        struct _pile_header * const next=(struct _pile_header*)area;
        next->next=h->next;
        h->next=next;
        return 0;
    } return err;
}

static void *_pile_occupy(struct _pile_node ** const a,const size_t size){
    struct _pile_node * const n=*a, *next=_pile_next(n);
    struct _pile_header * const h=n->header;
    if(h->able->first->size<sizeof(void*))
        return NULL;
    h->able->first->size-=sizeof(void*);
    if(next){
        if(n->size>(size+sizeof(*n)+1)){
            struct _pile_node * const e=(struct _pile_node*)((char*)(n+1)+size);
            e->size=(n->size-(sizeof(*n)+size));
            next->prev=e;
            next=e;
            n->size=size|1;
            (*a=e)->able=a;
        }else{
            (*a=*h->able->last)->able=a;
            ++h->able->last;
            h->able->first->size+=sizeof(void*);
        }
    }else{
        if((size+sizeof(*n))>n->size){
            h->able->first->size+=sizeof(void*);
            return NULL;
        }
        next=(struct _pile_node*)((char*)(n+1)+size);
        next->size=n->size-(sizeof(*n)+size);
        n->size=size|1;
        (*a=next)->able=a;
    }
    n->able=NULL;
    next->header=h;
    next->prev=n;
    return n+1;
}

static struct _pile_node **_pile_find(const struct _pile_header *h,const size_t size){
    struct _pile_node **n=NULL;
    do{
        const struct _pile_able * const a=h->able;
        const void * const first=&a->first;
        struct _pile_node **i=a->last;
        do{
            const size_t sz=_pile_size(i[0]);
            if(sz>=size && (!n || sz<_pile_size(n[0]))){
                n=i;
                if(sz-size<=sizeof(void*))
                    return n;
            }
        }while((void*)(i++)!=first);
    }while( (h=h->next) );
    return n;
}

void *pile_request(void * const pile,size_t size){
    if(_pile_is_init(pile)){
        struct _pile_node **n;
        struct _pile_header * const h=(struct _pile_header*)pile;
        size = (size + sizeof(void *) - 1) & ~(sizeof(void *) - 1);
        if( (n=_pile_find(h,size)) )
            return _pile_occupy(n,size);
    } return NULL;
}

static void _pile_free(struct _pile_node *n){
    struct _pile_able * const a=n->header->able;
    struct _pile_node * const prev=n->prev, * const next=_pile_next(n);
    unsigned char i=0;
    if(prev && prev->able){
        a->first->size+=sizeof(void*);
        prev->size+=n->size+sizeof(*n)-1;
        if(next){
            next->prev=prev;
            n=prev;
        }
        i|=1;
    }
    if(next && next->able){
        struct _pile_node * const after=_pile_next(next);
        a->first->size+=sizeof(void*);
        n->size+=next->size+sizeof(*n)-1;
        if(after) after->prev=n;
        i|=2;
    }
    switch(i){
        case 0: *(n->able=--a->last)=n; return;
        case 2: *(n->able=next->able)=n; return;
        case 3: n=*a->last; ++a->last; *(n->able=next->able)=n; return;
    }
}

void pile_release(void * const data){
    if(data){
        struct _pile_node * const n=((struct _pile_node*)data)-1;
        if(n->able) return;
        _pile_free(n);
    }
}

void pile_close(void * const pile){
    if(_pile_is_init(pile)){
        struct _pile_header *h=(struct _pile_header*)pile, *next;
        do{
            next=h->next;
            if(h->deallocator) h->deallocator(h);
            else h->area=NULL;
        }while( (h=next) );
    }
}

void pile_info(const void * const pile){
    if(_pile_is_init(pile)){
        const struct _pile_header * h=(const struct _pile_header*)pile, *next;
        do{
            const struct _pile_node *n=(const struct _pile_node *)(h+1);
            printf("pile: %p -> %p\n",h,(next=h->next));
            do{
                const struct _pile_node * const i=_pile_next(n);
                const size_t sz=_pile_size(n);
                printf("  %c [%zu / %zu]: %p<-(%p)->%p {%p}\n",n->able?' ':'*',sz,sz+sizeof(*n),n->prev,n,i,n->able);
                n=i;
            }while(n);
        }while( (h=next) );
    }
}

#undef _pile_size
#undef _pile_next

#endif /* PILE_IMPL */
