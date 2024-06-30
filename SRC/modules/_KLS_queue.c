
KLS_t_QUEUE KLS_queueNew(KLS_byte order,KLS_size elementSize,void(*deleter)(void *element)){
    return (KLS_t_QUEUE){deleter,{NULL},0,elementSize,order};
}

void KLS_queueClear(KLS_t_QUEUE *queue){
    if(queue){
        void **curr=queue->_[0],**tmp;
        if(queue->deleter)
            while(curr){
                queue->deleter(curr+1);
                tmp=*curr; KLS_free(curr); curr=tmp;
            }
        else
            while(curr){
                tmp=*curr; KLS_free(curr); curr=tmp;
            }
        queue->size=0;
        queue->_[0]=queue->_[1]=NULL;
    }
}

void *_KLS_queueNewNode(const void *element,KLS_size elementSize){
    void **n=KLS_malloc(sizeof(void*)+elementSize);
    if(n){*n=NULL; memcpy(n+1,element,elementSize);}
    return n;
}

KLS_byte _KLS_queuePushFifo(KLS_t_QUEUE *queue,void **node){
    if(node){
        if(queue->size) *(void**)(queue->_[1])=node;
        else queue->_[0]=node;
        queue->_[1]=node;
        ++queue->size;
        return 1;
    } return 0;
}

KLS_byte _KLS_queuePushLifo(KLS_t_QUEUE *queue,void **node){
    if(node){
        if(queue->size) *node=queue->_[0];
        else queue->_[1]=node;
        queue->_[0]=node;
        ++queue->size;
        return 1;
    } return 0;
}

KLS_byte KLS_queuePush(KLS_t_QUEUE *queue,const void *element){
    return queue && element && (queue->_opt?_KLS_queuePushLifo:_KLS_queuePushFifo)(queue,_KLS_queueNewNode(element,queue->sizeElement));
}

KLS_byte KLS_queuePop(KLS_t_QUEUE *queue,void *var){
    if(queue && queue->size){
        void **data=queue->_[0];
        if( !(queue->_[0]=*data) ) queue->_[1]=NULL;
        --queue->size;
        if(var) memcpy(var,data+1,queue->sizeElement);
        else if(queue->deleter) queue->deleter(data+1);
        KLS_free(data);
        return 1;
    } return 0;
}

void KLS_queueSort(KLS_t_QUEUE *queue,KLS_byte(*comparator)(const void *prev, const void *next,void *arg),void *arg){
    if(queue && queue->size && comparator){
        void **i=queue->_[0], **j=NULL, *a, *b;
        for(;i;i=*i)
            for(b=i+1,j=*i;j;j=*j)
                if(comparator((a=j+1),b,arg))
                    KLS_swap(a,b,queue->sizeElement);
    }
}
