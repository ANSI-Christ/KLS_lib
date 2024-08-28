

void *KLS_heapInit(void *heap,KLS_size size){
#define _KLS_HEAP_STRUCT struct{_KLS_t_HEAP_HEADER h; _KLS_t_HEAP_NODE n; _KLS_t_HEAP_FREES f;}
    _KLS_t_HEAP_HEADER * const h=heap;
    if(h && size>sizeof(_KLS_HEAP_STRUCT)){
        _KLS_t_HEAP_NODE * const n=(void*)(h+1);
        if(pthread_mutex_init(&h->mtx,NULL))
            return NULL;
        h->p=n->h=heap;
        h->frees=heap+size-sizeof(_KLS_t_HEAP_FREES);
        h->frees->first=n;
        h->frees->last=n->free=&h->frees->first;
        n->next=n->prev=NULL;
        n->size=size-sizeof(_KLS_HEAP_STRUCT);
        return heap;
    }
    return NULL;
#undef _KLS_HEAP_STRUCT
}

void *_KLS_heapIsInit(_KLS_t_HEAP_HEADER * const h){
    return (h && h->p==h) ? h+1 : NULL;
}

void *_KLS_heapNewNode(_KLS_t_HEAP_HEADER * const h,_KLS_t_HEAP_NODE **f,const KLS_size size){
    _KLS_t_HEAP_NODE * const n=*f, *next=n->next;
    if(h->frees->first->size<sizeof(void*))
        return NULL;
    h->frees->first->size-=sizeof(void*);
    if(next){
        if(n->size>(size+sizeof(*n))){
            _KLS_t_HEAP_NODE * const e=(void*)(n+1)+size;
            e->size=n->size-(sizeof(*n)+size);
            e->next=next;
            next->prev=e;
            next=e;
            n->size=size;
            (*f=e)->free=f;
        }else{
            (*f=*h->frees->last)->free=f;
            ++h->frees->last;
            h->frees->first->size+=sizeof(void*);
        }
    }else{
        if((size+sizeof(*n))>(n->size)){
            h->frees->first->size+=sizeof(void*);
            return NULL;
        }
        next=(void*)(n+1)+size;
        next->next=NULL;
        next->size=n->size-(sizeof(*n)+size);
        n->size=size;
        (*f=next)->free=f;
    }
    n->free=NULL;
    n->next=next;
    next->prev=n;
    next->h=h;
    return n+1;
}

void *_KLS_heapFindFree(_KLS_t_HEAP_FREES *f,const KLS_size size){
    _KLS_t_HEAP_NODE **n=&f->first, **i=f->last;
    for(;i!=&f->first;++i)
        if(i[0]->size>=size && i[0]->size<n[0]->size && (n=i)[0]->size-size<=sizeof(void*))
            return n;
    return n[0]->size>=size ? n : NULL;
}

void *KLS_heapAlloc(void *heap,KLS_size size){
    void *p=NULL;
    if(size<(UINT64_MAX/2)){
        _KLS_t_HEAP_HEADER * const h=heap;
        if(_KLS_heapIsInit(h)){
            const unsigned char align=size%sizeof(void*);
            if(align) size+=sizeof(void*)-align;
            pthread_mutex_lock(&h->mtx);
            if((p=_KLS_heapFindFree(h->frees,size)))
                p=_KLS_heapNewNode(heap,p,size);
            pthread_mutex_unlock(&h->mtx);
        }
    }
    return p;
}

void _KLS_heapRemove(_KLS_t_HEAP_NODE *n){
    _KLS_t_HEAP_FREES * const f=n->h->frees;
    _KLS_t_HEAP_NODE * const prev=n->prev, * const next=n->next;
    unsigned char i=0;
    if(prev && prev->free){
        f->first->size+=sizeof(void*);
        prev->size+=n->size+sizeof(*n);
        if(next){
            prev->next=next;
            next->prev=prev;
            n=prev;
        }
        i|=1;
    }
    if(next && next->free){
        f->first->size+=sizeof(void*);
        n->size+=next->size+sizeof(*n);
        if(next->next){
            n->next=next->next;
            next->next->prev=n;
        }else{
            next->prev=n;
            n->next=NULL;
        }
        i|=2;
    }
    switch(i){
        case 0: *(n->free=--f->last)=n; return;
        case 2: *(n->free=next->free)=n; return;
        case 3: n=*f->last; ++f->last; *(n->free=next->free)=n; return;
    }
}

void KLS_heapFree(void *data){
    if(data){
        _KLS_t_HEAP_NODE *n=data;
        if(!(--n)->free){
            void *mtx=&n->h->mtx;
            pthread_mutex_lock(mtx);
            _KLS_heapRemove(n);
            pthread_mutex_unlock(mtx);
        }
    }
}

void KLS_heapClose(void *heap){
    _KLS_t_HEAP_HEADER *h=heap;
    if(_KLS_heapIsInit(h)){
        pthread_mutex_destroy(&h->mtx);
        h->p=NULL;
    }
}

void KLS_heapInfo(void *heap){
    _KLS_t_HEAP_NODE *n=_KLS_heapIsInit(heap);
    if(n){
        _KLS_t_HEAP_HEADER *h=heap;
        _KLS_t_HEAP_NODE **i=h->frees->last;
        pthread_mutex_lock(&h->mtx);
        printf("\nHEAP: %p\n",h);
        while(n){
            printf("  %c [%zu / %zu]: %p<-(%p)->%p {%p}\n",n->free?'f':'a',n->size,n->size+sizeof(*n),n->prev,n,n->next,n->free);
            n=n->next;
        }
        printf("free array:\n");
        do{
            printf("  {%p}: (%p)\n",i,*i);
        }while(i++!=&h->frees->first);
        pthread_mutex_unlock(&h->mtx);
    }
}

KLS_byte KLS_mallocSetHeap(void *heap){
    if(_KLS_heapIsInit(heap)){
        _KLS_mallocHeap=heap;
        return 1;
    } return 0;
}
