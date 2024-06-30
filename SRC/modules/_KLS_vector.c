
KLS_size _KLS_vectorPolicyDefault(KLS_size size){
    return size+(size>>1)+1;
}

KLS_t_VECTOR KLS_vectorNew(KLS_size size,KLS_size elementSize,void(*deleter)(void *element)){
    KLS_t_VECTOR v={NULL,deleter,_KLS_vectorPolicyDefault,0,0,elementSize};
    if( size && (v.data=KLS_malloc((size+1)*elementSize)) )
        v.sizeReal=size;
    return v;
}

KLS_t_VECTOR KLS_vectorCopy(const KLS_t_VECTOR *vector){
    KLS_t_VECTOR v={NULL};
    if(vector){
        v=KLS_vectorNew(vector->size,vector->sizeElement,vector->deleter);
        KLS_vectorSetPolicy(&v,vector->policy);
        if(v.data){
            memcpy(v.data,vector->data,vector->size*vector->sizeElement);
            v.size=vector->size;
        }
    }
    return v;
}

void KLS_vectorSetPolicy(KLS_t_VECTOR *vector,KLS_size(*policy)(KLS_size size)){
    if(vector && !(vector->policy=policy) )
        vector->policy=_KLS_vectorPolicyDefault;
}

void KLS_vectorFree(KLS_t_VECTOR *vector){
    if(vector){
        KLS_vectorClear(vector);
        KLS_freeData(vector->data);
        vector->sizeReal=0;
    }
}

void KLS_vectorClear(KLS_t_VECTOR *vector){
    if(vector && vector->data){
        if(vector->deleter)
            while(vector->size)
                vector->deleter(vector->data+vector->sizeElement*(--vector->size));
        vector->size=0;
    }
}

void *KLS_vectorAt(const KLS_t_VECTOR *vector,KLS_size index){
    if(vector && vector->data && index<vector->size)
        return (char*)vector->data+index*vector->sizeElement;
    return NULL;
}

KLS_size KLS_vectorIndexOf(const KLS_t_VECTOR *vector,const void *element,KLS_byte(*comparator)(const void *elementVector,const void *element)){
    return KLS_arrayFind(vector->data,vector->size,vector->sizeElement,element,comparator);
}

KLS_byte KLS_vectorResize(KLS_t_VECTOR *vector,KLS_size size){
    if(vector && vector->data){
        if(size){
            KLS_t_VECTOR buff=KLS_vectorNew(size,vector->sizeElement,vector->deleter);
            if(buff.data){
                KLS_vectorSetPolicy(&buff,vector->policy);
                buff.size=KLS_MIN(vector->size,size);
                if(buff.size) memcpy(buff.data,vector->data,buff.size*buff.sizeElement);
                if(vector->deleter)
                    while(vector->size>buff.size)
                        KLS_vectorRemove(vector,vector->size-1);
                KLS_freeData(vector->data);
                *vector=buff;
            }
        }else KLS_vectorFree(vector);
        return 1;
    } return 0;
}

KLS_byte KLS_vectorPushBack(KLS_t_VECTOR *vector,const void *element){
    if(vector && vector->data){
        if(vector->size>=vector->sizeReal)
            if(!KLS_vectorResize(vector,vector->policy(vector->size)))
                return 0;
        KLS_arrayInsert(vector->data,vector->size+1,vector->sizeElement,vector->size,element);
        ++vector->size;
        return 1;
    } return 0;
}

KLS_byte KLS_vectorInsert(KLS_t_VECTOR *vector,KLS_size index,const void *element){
    if(vector && vector->data){
        if(index>=vector->size) return KLS_vectorPushBack(vector,element);
        if(vector->size>=vector->sizeReal)
            if(!KLS_vectorResize(vector,vector->policy(vector->size)))
                return 0;
        KLS_arrayInsert(vector->data,++vector->size,vector->sizeElement,index,element);
        return 1;
    } return 0;
}

void KLS_vectorRemove(KLS_t_VECTOR *vector,KLS_size index){
    if(vector && vector->data && index<vector->size){
        memcpy(vector->data+vector->sizeElement*vector->size,vector->data+vector->sizeElement*index,vector->sizeElement);
        if(index!=--vector->size) memmove(vector->data+vector->sizeElement*index,vector->data+vector->sizeElement*(index+1),vector->sizeElement*(vector->size-index));
        if(vector->deleter) vector->deleter(vector->data+(vector->size+1)*vector->sizeElement);
    }
}

void KLS_vectorRemoveFast(KLS_t_VECTOR *vector,KLS_size index){
    if(vector && vector->data && index<vector->size){
        memcpy(vector->data+vector->sizeElement*vector->size,vector->data+vector->sizeElement*index,vector->sizeElement);
        if(index!=--vector->size) memcpy(vector->data+vector->sizeElement*index,vector->data+vector->sizeElement*vector->size,vector->sizeElement);
        if(vector->deleter) vector->deleter(vector->data+(vector->size+1)*vector->sizeElement);
    }
}

void KLS_vectorPrint(const KLS_t_VECTOR *vector,void(*printer)(const void *element)){
    if(vector && printer){
        KLS_size i=0;
        while(i<vector->size) printer(KLS_vectorAt(vector,i++));
    }
}

void KLS_vectorSort(KLS_t_VECTOR *vector,KLS_byte(*comparator)(const void *prev,const void *next,void *arg),void *arg){
    if(vector) KLS_arraySort(vector->data,vector->size,vector->sizeElement,comparator,arg);
}
