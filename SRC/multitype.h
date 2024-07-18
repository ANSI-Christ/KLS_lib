#ifndef MULTITYPE_H
#define MULTITYPE_H

#include <pthread.h>
#include <stdlib.h>

#include "macro.h"

typedef char multitype;

unsigned char multitypeInit();
void multitypeClose();

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
#define _MT_COUNT(...) (M_FOREACH(__MT_COUNT,-,__VA_ARGS__) 0)
#define __MT_COUNT(_1_,_2_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( 1+ )
#define _MT_LET(...) for(;;({goto M_JOIN(_mark,M_LINE());}))if(!8){M_JOIN(_mark,M_LINE()): void **_1_=pthread_getspecific(_MT_key); M_FOREACH(_MT_UNPACK,_1_,__VA_ARGS__) break;} else *({char _1_; (&_1_);})
#define _MT_RETURN(...) _MT_deleter(pthread_getspecific(_MT_key)); { _MT_STRUCT(__VA_ARGS__) *_1_=malloc(sizeof(*_1_)); _1_->i=(void*)_1_;_MT_PACK(_1_,__VA_ARGS__) pthread_setspecific(_MT_key,_1_); } return 0;
extern pthread_key_t _MT_key;

#ifdef MULTITYPE_IMPL

pthread_key_t _MT_key;
static void _MT_deleter(void *p){ if(p){free(p); pthread_setspecific(_MT_key,NULL);} }
unsigned char multitypeInit(){ return !pthread_key_create(&_MT_key,_MT_deleter); }
void multitypeClose(){ _MT_deleter(pthread_getspecific(_MT_key)); pthread_key_delete(_MT_key); }

#endif // MULTITYPE_IMPL

#endif // MULTITYPE_H


