#ifndef MULTITYPE_H
#define MULTITYPE_H

#include <stdio.h>
#include <string.h>

#include "macro.h"


typedef void* multitype;

void multitypeInit();

#define let(...)       M_IF(M_COUNT(M_REMAINED(__VA_ARGS__)))(_MT_LET,M_EXTRACT)(__VA_ARGS__)
#define return(...)    M_IF(M_COUNT(M_REMAINED(__VA_ARGS__)))(_MT_RETURN,return)(__VA_ARGS__)
#define multitype(...) M_IF(M_COUNT(__VA_ARGS__))( M_IF(M_COUNT(M_REMAINED(__VA_ARGS__)))(multitype,__VA_ARGS__),multitype)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _MT_TYPEOF(...) __typeof__(__VA_ARGS__)
#define _MT_UNPACK(_0_,_1_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( __VA_ARGS__=*(_MT_TYPEOF(__VA_ARGS__)*)(*_1_); ) ++_1_;
#define _MT_PACK(_1_,...) M_FOREACH(__MT_PACK,_1_,__VA_ARGS__)
#define __MT_PACK(_index_,_1_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( _1_->M_JOIN(_,_index_)=(__VA_ARGS__); *_1_->i++=&_1_->M_JOIN(_,_index_); )
#define _MT_STRUCT(...) struct{void *p[_MT_COUNT(__VA_ARGS__)]; M_FOREACH(__MT_STRUCT,-,__VA_ARGS__) void **i;}
#define __MT_STRUCT(_index_,_0_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( _MT_TYPEOF(__VA_ARGS__) M_JOIN(_,_index_); )
#define _MT_LET(...) for(;;({void **_1_=(&_1_)[_MT_diff==99]; _MT_VOLATILE(_1_); M_FOREACH(_MT_UNPACK,_1_,__VA_ARGS__) goto M_JOIN(_mark,M_LINE());}))if(!8){M_JOIN(_mark,M_LINE()): break;} else ({void *_1_; (&_1_);})[_MT_diff]
#define _MT_RETURN(...) { _MT_STRUCT(__VA_ARGS__) _1_[1]={{.i=(void*)_1_}}; _MT_PACK(_1_,__VA_ARGS__) _MT_VOLATILE(_1_); return _MT_Addr(_1_);} return NULL
#define _MT_COUNT(...) (M_FOREACH(__MT_COUNT,-,__VA_ARGS__) 0)
#define __MT_COUNT(_1_,_2_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( 1+ )
#define _MT_VOLATILE(...) if(_MT_diff==99)printf("ret %p\n",__VA_ARGS__)
extern signed char _MT_diff;
void *_MT_Addr(void *p);


#ifdef MULTITYPE_IMPL

signed char _MT_diff=100;

void multitypeInit(){
    if(_MT_diff==100){
        void *p1, *p2;
        for(;;({void **p; p1=&p; _MT_diff=(void**)p1-(void**)p2; return;}))
            *({void *p; p2=&p; &p;})=NULL;
    }
}

void *_MT_Addr(void *p){
    void *x=&_MT_diff;
    memcpy(&x,&p,sizeof(p));
    return x;
}

#endif

#endif // MULTITYPE_H
