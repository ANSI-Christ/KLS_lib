
KLS_t_LIST KLS_listNew(KLS_size elementSize,void(*deleter)(void *element)){
    const KLS_t_LIST l={NULL,NULL,deleter,0,elementSize}; return l;
}

KLS_t_LIST KLS_listCopy(const KLS_t_LIST *list){
    KLS_t_LIST l={0};
    if(list && list->first){
        void **n=list->first;
        l=KLS_listNew(list->sizeElement,list->deleter);
        while(n){
            KLS_listPushBack(&l,n);
            n=KLS_listNext(n);
        }
    }
    return l;
}

void **_KLS_listNodeNew(const void *element,KLS_size elementSize){
    void **n=KLS_malloc((sizeof(void*)<<1)+elementSize);
    if(n){
        n+=2; KLS_listPrev(n)=KLS_listNext(n)=NULL;
        if(element) memcpy(n,element,elementSize);
        else memset(n,0,elementSize);
    }
    return n;
}

void *KLS_listAt(const KLS_t_LIST *list,KLS_size index){
    void **n=NULL;
    if(list && index<list->size){
        if(index<(list->size>>1)){
            n=list->first;
            while(index--) n=KLS_listNext(n);
        }else{
            n=list->last;
            index=list->size-index-1;
            while(index--) n=KLS_listPrev(n);
        }
    }
    return n;
}

KLS_size KLS_listIndexOf(const KLS_t_LIST *list,const void *element,KLS_byte(*comparator)(const void *elementList,const void *element),void **pointer){
    KLS_size i;
    if(list && element){
        void **n=list->first;
        if(comparator)
            for(i=0;n;++i){
                if(comparator(n,element)){
                    if(pointer) *pointer=n;
                    return i;
                }
                n=KLS_listNext(n);
            }
        else
            for(i=0;n;++i){
                if(!memcmp(n,element,list->sizeElement)){
                    if(pointer) *pointer=n;
                    return i;
                }
                n=KLS_listNext(n);
            }
    }
    return -1;
}

void* KLS_listPushBack(KLS_t_LIST *list,const void *element){
    if(list){
        void **n=_KLS_listNodeNew(element,list->sizeElement);
        if(n){
            if(list->last){
                KLS_listPrev(n)=list->last;
                KLS_listNext(list->last)=n;
                list->last=n;
            }else list->first=list->last=n;
            ++list->size;
            return n;
        }
    }
    return NULL;
}

void* KLS_listPushFront(KLS_t_LIST *list,const void *element){
    if(list){
        void **n=_KLS_listNodeNew(element,list->sizeElement);
        if(n){
            if(list->first){
                KLS_listNext(n)=list->first;
                KLS_listPrev(list->first)=n;
                list->first=n;
            }else list->first=list->last=n;
            ++list->size;
            return n;
        }
    }
    return NULL;
}

void* KLS_listInsert(KLS_t_LIST *list,const void *element,KLS_size index){
    if(list){
        if(index<list->size){
            if(index){
                void **n=_KLS_listNodeNew(element,list->sizeElement);
                if(n){
                    void **c=KLS_listAt(list,index);
                    ++list->size;
                    KLS_listNext(n)=c;
                    KLS_listPrev(n)=KLS_listPrev(c);
                    KLS_listNext(KLS_listPrev(n))=n;
                    KLS_listPrev(c)=n;
                    return n;
                } return NULL;
            } return KLS_listPushFront(list,element);
        } return KLS_listPushBack(list,element);
    } return NULL;
}

void* KLS_listPushBefore(KLS_t_LIST *list,const void *element,void *current){
    if(list){
        if(current && KLS_listPrev(current)){
            void **n=_KLS_listNodeNew(element,list->sizeElement);
            if(n){
                void **prev=KLS_listPrev(current);
                KLS_listNext(n)=current;
                KLS_listPrev(n)=prev;
                KLS_listPrev(current)=KLS_listNext(prev)=n;
                ++list->size;
                return n;
            } return NULL;
        } return KLS_listPushFront(list,element);
    } return NULL;
}


void* KLS_listPushAfter(KLS_t_LIST *list,const void *element,void *current){
    if(list){
        if(current && KLS_listNext(current)){
            void **n=_KLS_listNodeNew(element,list->sizeElement);
            if(n){
                void **next=KLS_listNext(current);
                KLS_listPrev(n)=current;
                KLS_listNext(n)=next;
                KLS_listPrev(next)=KLS_listNext(current)=n;
                ++list->size;
                return n;
            } return NULL;
        } return KLS_listPushBack(list,element);
    } return NULL;
}

void _KLS_listUnlink(KLS_t_LIST *list,void *element){
    void *n;
    if( (n=KLS_listPrev(element)) ) KLS_listNext(n)=KLS_listNext(element);
    else list->first=KLS_listNext(element);
    if( (n=KLS_listNext(element)) ) KLS_listPrev(n)=KLS_listPrev(element);
    else list->last=KLS_listPrev(element);
    KLS_listNext(element)=KLS_listPrev(element)=NULL;
}

void KLS_listRemove(KLS_t_LIST *list,void *element){
    if(list && element){
        void **n=element;
        _KLS_listUnlink(list,n);
        --list->size;
        if(list->deleter) list->deleter(n);
        KLS_free(n-2);
    }
}

void KLS_listMoveAfter(KLS_t_LIST *list,void *element,void *current){
    if(list && element!=current){
        void *n;
        _KLS_listUnlink(list,element);
        if( (n=KLS_listNext(current)) ){
            KLS_listPrev(n)=element;
            KLS_listNext(element)=n;
        }else list->last=element;
        KLS_listPrev(element)=current;
        KLS_listNext(current)=element;
    }
}

void KLS_listMoveBefore(KLS_t_LIST *list,void *element,void *current){
    if(list && element!=current){
        void *n;
        _KLS_listUnlink(list,element);
        if( (n=KLS_listPrev(current)) ){
            KLS_listNext(n)=element;
            KLS_listPrev(element)=n;
        }else list->first=element;
        KLS_listNext(element)=current;
        KLS_listPrev(current)=element;
    }
}

void KLS_listRemoveAt(KLS_t_LIST *list,KLS_size index){
    KLS_listRemove(list,KLS_listAt(list,index));
}

void KLS_listClear(KLS_t_LIST *list){
    if(list){
        void **n=list->first, **next;
        if(list->deleter)
            while(n){
                list->deleter(n);
                next=KLS_listNext(n); KLS_free(n-2); n=next;
            }
        else
            while(n){
                next=KLS_listNext(n); KLS_free(n-2); n=next;
            }
        list->size=0;
        list->first=list->last=NULL;
    }
}

void KLS_listSort(KLS_t_LIST *list, KLS_byte(*comparator)(const void *prev,const void *next,void *arg),void *arg){
    if(list && list->size && comparator){
        void **i=list->first, **j;
        for(;i;i=KLS_listNext(i))
            for(j=KLS_listNext(i);j;j=KLS_listNext(j))
                if(comparator(j,i,arg))
                    KLS_swap(i,j,list->sizeElement);
    }
}

KLS_t_LIST KLS_listMerge(KLS_t_LIST *list_1, KLS_t_LIST *list_2){
    KLS_t_LIST l={0};
    if(list_1 && list_2){
        if(list_1==list_2) return *list_1;
        if(!list_1->size) KLS_swap(&list_1,&list_2,sizeof(list_1));
        if(list_1->last){
            if( (KLS_listNext(list_1->last)=list_2->first) ){
                KLS_listPrev(list_2->first)=list_1->last;
                list_1->last=list_2->last;
            }
        }else{
            list_1->first=list_2->first;
            list_1->last=list_2->last;
        }
        list_1->size+=list_2->size;
        l=*list_1;
        memset(list_1,0,sizeof(*list_1));
        memset(list_2,0,sizeof(*list_2));
    }
    return l;
}

void KLS_listPrint(const KLS_t_LIST *list,void(*printer)(const void *element)){
    if(list && printer){
        void **n=list->first;
        while(n){ printer(n); n=KLS_listNext(n); }
    }
}
