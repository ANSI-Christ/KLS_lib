
void *KLS_arrayAt(void *array,KLS_size arraySize,KLS_size elementSize,KLS_size index){
    return (array && arraySize!=-1 && index<arraySize) ? array+index*elementSize : NULL;
}

KLS_byte KLS_arrayInsert(void *array,KLS_size arraySize,KLS_size elementSize,KLS_size index,const void *element){
    if(array && arraySize!=-1 && index<arraySize){
        memmove(array+elementSize*(index+1),array+elementSize*index,elementSize*(arraySize-index-1));
        if(element) memcpy(array+elementSize*index,element,elementSize);
        else memset(array+elementSize*index,0,elementSize);
        return 1;
    } return 0;
}

KLS_size KLS_arrayFind(const void *array,KLS_size arraySize,KLS_size elementSize,const void *element,KLS_byte(*comparator)(const void *elementArray, const void *element)){
    if(array && element && arraySize!=-1){
        KLS_size i;
        if(comparator){
            KLS_byte cmp;
            for(i=0;i<arraySize;++i)
                if( (cmp=comparator(array+i*elementSize,element)) )
                    return i+cmp-1;
        }else{
            for(i=0;i<arraySize;++i)
                if( !memcmp(array+i*elementSize,element,elementSize) )
                    return i;
        }
    }
    return -1;
}

KLS_size KLS_arrayExtremum(const void *array,KLS_size arraySize,KLS_size elementSize,KLS_byte(*comparator)(const void *elementArray, const void *extremum)){
    if(arraySize && arraySize!=-1 && comparator && array){
        KLS_size i,m=0;
        for(i=1;i<arraySize;++i)
            if(comparator(array+m*elementSize,array+i*elementSize))
                m=i;
        return m;
    }
    return -1;
}

KLS_byte KLS_arrayRemove(void *array,KLS_size arraySize,KLS_size elementSize,KLS_size index){
    if(array && arraySize!=-1 && index<arraySize){
        if(index!=--arraySize) memmove(array+elementSize*index,array+elementSize*(index+1),elementSize*(arraySize-index));
        return 1;
    } return 0;
}


KLS_byte KLS_arrayRemoveFast(void *array,KLS_size arraySize,KLS_size elementSize,KLS_size index){
    if(array && arraySize!=-1 && index<arraySize){
        if(index!=--arraySize) memcpy(array+elementSize*index,array+elementSize*arraySize,elementSize); //KLS_swap
        return 1;
    } return 0;
}

void KLS_arraySet(void *array,KLS_size arraySize,KLS_size elementSize,const void *element){
    if(element && array && arraySize!=-1)
        while(arraySize)
            memcpy(array+elementSize*(--arraySize),element,elementSize);
}

void KLS_arraySort(void *array,KLS_size arraySize,KLS_size elementSize,KLS_byte(*comparator)(const void *prev,const void *next,void *arg),void *arg){
    if(arraySize>1 && arraySize!=-1 && comparator && array){
        KLS_size i,j;
        void *a,*b;
        for(i=0;i<arraySize;++i)
            for(b=array+i*elementSize,j=i+1;j<arraySize;++j)
                if(comparator((a=array+j*elementSize),b,arg))
                    KLS_swap(a,b,elementSize);
    }
}

void KLS_arrayPrint(const void *array,KLS_size arraySize,KLS_size elementSize,void(*printer)(const void *element)){
    if(array && printer && arraySize!=-1)
        while(arraySize){
            printer(array);
            array+=elementSize;
            --arraySize;
        }
}
