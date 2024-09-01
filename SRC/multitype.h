#ifndef MULTITYPE_H
#define MULTITYPE_H

#include "macro.h"
#include <stddef.h>

typedef size_t multitype;

#define let(...)       M_IF(M_COUNT(M_REMAINED(__VA_ARGS__)))(_MT_LET,M_EXTRACT)(__VA_ARGS__)
#define return(...)    M_IF(M_COUNT(M_REMAINED(__VA_ARGS__)))(_MT_RETURN,return)(__VA_ARGS__)
#define multitype(...) M_IF(M_COUNT(__VA_ARGS__))( M_IF(M_COUNT(M_REMAINED(__VA_ARGS__)))(multitype,__VA_ARGS__),multitype)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _MT_TYPEOF(...) __typeof__(__VA_ARGS__)
#define _MT_UNPACK(_0_,_1_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( __VA_ARGS__=*(_MT_TYPEOF(__VA_ARGS__)*)(*_1_), ) ++_1_,
#define _MT_PACK(_1_,...) {M_FOREACH(__MT_PACK,_1_,__VA_ARGS__)}
#define __MT_PACK(_index_,_1_,...) &_1_.M_JOIN(_,_index_),
#define _MT_STRUCT(...) struct{void **i; void *p[_MT_COUNT(__VA_ARGS__)]; M_FOREACH(__MT_STRUCT,-,__VA_ARGS__)}
#define __MT_STRUCT(_index_,_0_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( _MT_TYPEOF(__VA_ARGS__) M_JOIN(_,_index_); )
#define _MT_COUNT(...) (M_FOREACH(__MT_COUNT,-,__VA_ARGS__) 0)
#define __MT_COUNT(_1_,_2_,...) 1+
#define _MT_LET(...) for( void **_1_=(void*)0; !_1_ ; M_FOREACH(_MT_UNPACK,_1_,__VA_ARGS__) (void)0 ) *((size_t*)&_1_)
#define _MT_RETURN(...) { _MT_STRUCT(__VA_ARGS__) _1_={ (void*)_1_.p, _MT_PACK(_1_,__VA_ARGS__), __VA_ARGS__ }; return _MT_address(_1_.p); } (void)0

size_t _MT_address(void *p);

#endif // MULTITYPE_H
