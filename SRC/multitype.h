#ifndef MULTITYPE_H
#define MULTITYPE_H

#include "macro.h"
#include <pthread.h>

typedef unsigned char multitype;

#define let(...)       M_IF(M_COUNT(M_REMAINED(__VA_ARGS__)))(_MT_LET,M_EXTRACT)(__VA_ARGS__)
#define return(...)    M_IF(M_COUNT(M_REMAINED(__VA_ARGS__)))(_MT_RETURN,return)(__VA_ARGS__)
#define multitype(...) M_IF(M_COUNT(__VA_ARGS__))( M_IF(M_COUNT(M_REMAINED(__VA_ARGS__)))(multitype,__VA_ARGS__),multitype)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _MT_TYPEOF(...) __typeof__(__VA_ARGS__)
#define _MT_UNPACK(_0_,_1_,_val_) if(*_1_>(void*)1){*(_MT_TYPEOF(_val_)*)(*_1_)=(_val_);++_1_;}else if(*_1_==(void*)1){++_1_;}
#define _MT_PACK(_index_,_arg_,_var_) M_IF(M_IS_ARG(_var_))(&_var_,(void*)1),
#define _MT_RETURN(...) { void **_1_=(void**)pthread_getspecific(_MT_key);if(_1_){++_1_;M_FOREACH(_MT_UNPACK,_1_,__VA_ARGS__)}}return 0;
#define _MT_LET(...) \
    for(pthread_once(&_MT_once,_MT_init), ({ void *_1_[M_COUNT(__VA_ARGS__)+2]={pthread_getspecific(_MT_key),M_FOREACH(_MT_PACK,-,__VA_ARGS__)}; pthread_setspecific(_MT_key,_1_); }); ; ({goto M_JOIN(_mark,M_LINE());}) )\
        if(0){M_JOIN(_mark,M_LINE()): pthread_setspecific(_MT_key,*(void**)pthread_getspecific(_MT_key)); break;}else*({char _2_;&_2_;})

#endif /* MULTITYPE_H */


#ifdef MULTITYPE_IMPL

extern int atexit(void(*)(void));
pthread_key_t _MT_key;
pthread_once_t _MT_once=PTHREAD_ONCE_INIT;
static void _MT_exit(void){ pthread_key_delete(_MT_key); }
void _MT_init(void){ if(!pthread_key_create(&_MT_key,NULL)) atexit(_MT_exit); }

#else

void _MT_init(void);
extern pthread_key_t _MT_key;
extern pthread_once_t _MT_once;

#endif
