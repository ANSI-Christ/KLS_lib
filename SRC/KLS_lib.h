/* * * * * * * * * * * * * * * * * */
/* MIT License                     */
/* Copyright (c) 2024 ANSI-Christ  */
/* * * * * * * * * * * * * * * * * */

#ifndef KLS_LIB_H
#define KLS_LIB_H

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// COMPILATION /////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// compile with:
//     -Wfloat-equal    ( warnings: float a=2.2,b=2.2; a==b; instead of fabs(a-b) )
//     -O               ( optimization [O,O2,O3] ) )
//     -D               ( [ _KLS_MALLOC_HEAP=xxxx, _KLS_MEMORY_DEBUG ] ; see also KLS_sysDefs.h )
// link with:
//     windows: -lm -lc -lpthread -lrt -lgdi32 -lws2_32
//     solaris: -lm -lc -lpthread -lrt -lX11 -lsocket
//     unix:    -lm -lc -lpthread -lrt -lX11
//     other:   -lm -lc -lpthread -lrt
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <setjmp.h>
#include <signal.h>

#include "macro.h"
#include "NetPool.h"
#include "class.h"
#include "trycatch.h"
#include "time_ext.h"
#include "pthread_ext.h"
#include "pile.h"


#include "KLS_0b.h"



#define KLS_NAN  (0.0/0.0)
#define KLS_INF  (1.0/0.0)

#define KLS_RADIAN  (M_PI/180.)
#define KLS_DEGREE  (180./M_PI)

#define KLS_ENDIAN_BIG     0
#define KLS_ENDIAN_LITTLE  1

#define KLS_COLOR_BLACK          KLS_RGB(0, 0, 0)
#define KLS_COLOR_WHITE          KLS_RGB(255, 255, 255)
#define KLS_COLOR_RED            KLS_RGB(255, 0, 0)
#define KLS_COLOR_GREEN          KLS_RGB(0, 255, 0)
#define KLS_COLOR_BLUE           KLS_RGB(0, 0, 255)
#define KLS_COLOR_YELLOW         KLS_RGB(255, 255, 0)
#define KLS_COLOR_ORANGE         KLS_RGB(255, 128, 0)
#define KLS_COLOR_GREY           KLS_RGB(128, 128, 128)
#define KLS_COLOR_DARK_GREY      KLS_RGB(88, 88, 88)
#define KLS_COLOR_LIGHT_GREY     KLS_RGB(195, 195, 195)




typedef void*          KLS_any;
typedef unsigned char  KLS_byte;
typedef ptrdiff_t      KLS_long;
typedef size_t         KLS_size;

typedef struct _GUI_t_DISPLAY GUI_t_DISPLAY;

typedef const struct{
    void *(*const new)(KLS_size size);
    void *(*const resize)(void *p,KLS_size size);
    void (*const del)(void *p);
}KLS_t_ALLOCATOR;


typedef struct{
    const char *symbols;
    const unsigned char symbolWidth;  // pixels
    const unsigned char symbolHeight; // pixels
}KLS_t_FONT_BITMAP;


typedef struct{
    const KLS_t_FONT_BITMAP *bitmap;
    unsigned char intervalSymbol;     // pixels default 2;
    unsigned char intervalRow;        // pixels default 2;
    unsigned short width;             // pixels default = bitmap->symbolWidth
    unsigned short height;            // pixels default = bitmap->symbolHeight
}KLS_t_FONT;


typedef struct{
    const char *url;
    const char *header;   //default "Connection: close\r\n"
    const char *protocol; //default "HTTP/1.0"
}KLS_t_URL;


typedef struct{
    const char *header;
    const char *data;
    KLS_size size;
}KLS_t_URL_DATA;




// GLOBAL VARIABLES INITIALIZATION SECTION

extern const KLS_t_FONT KLS_fontBase;
extern KLS_byte KLS_COLOR_BITS;


// TEMPLATES SECTION

#define KLS_MVN(_name_) M_JOIN(M_JOIN(_,M_LINE()),M_JOIN(__,_name_)) 

#define KLS_LMBD(_return_type_,...) ({ _return_type_ KLS_MVN(lambda) __VA_ARGS__ (void*)KLS_MVN(lambda); })

#define KLS_ONCE(...) {static char KLS_MVN(a)=1; if(KLS_MVN(a)){KLS_MVN(a)=0;{__VA_ARGS__}}}

#define KLS_$(_type_,...) ( (void*)((_type_[__VA_ARGS__]) _KLS_ALLOC

#define KLS_NONCODE(_type_)  ({ const union{KLS_size i; M_TYPEOF(_type_) t;} KLS_MVN(nc)={(((KLS_size)1)<<(sizeof(_type_)*8-1))}; KLS_MVN(nc).t; })

#define KLS_SIGNBIT(_value_) ({ const union{M_TYPEOF(_value_) t; KLS_size i;} KLS_MVN(sb)={(_value_)}; KLS_MVN(sb).i>>(sizeof(_value_)*8-1); })

#define KLS_SIGN(_value_) ((signed char)((KLS_IS_SIGNED(_value_) && KLS_SIGNBIT(_value_))?-1:1))

#define KLS_CAST(_type_) ( (_type_) _KLS_CAST

#define KLS_IS_ARRAY(_variable_) ((_variable_)==&(_variable_))

#define KLS_IS_CONST(_someExpression_) ( sizeof(int)==sizeof( *(1 ? ((void*)( (long)(_someExpression_)*0l )) : (int*)1) ) )

#define KLS_IS_FLOAT(_value_) ( ((const char)(*(M_TYPEOF(_value_)*)"\xa\xa\xa\xa\xa\xa\xa\xa\xa\xa\xa\xa\xa\xa\xa\xa")) == 0 )

#define KLS_IS_SIGNED(_value_) ( ((M_TYPEOF(_value_))-1) < 0 )

#define KLS_IS_POINTER(_value_) ( !KLS_IS_SIGNED(_value_) && sizeof(_value_)==sizeof(void*) )

#define KLS_IS_NONCODE(_value_) ({ const union{M_TYPEOF(_value_) t; KLS_size i;} KLS_MVN(nc)={(_value_)}; KLS_MVN(nc).i==(((KLS_size)1)<<(sizeof(_value_)*8-1)); })

#define KLS_IS_CORRECT(_value_) ( (_value_)==(_value_) && !KLS_IS_INF(_value_) )

#define KLS_IS_INF(_value_) (_value_==KLS_INF ? 1 : (_value_==-KLS_INF ? -1 : 0))

#define KLS_ARRAY_LEN(_array_) ((KLS_size)(sizeof(_array_)/sizeof(*(_array_))))

#define KLS_ARRAY_AT(_array_,...) ({ const KLS_size KLS_MVN(_sa)[]={__VA_ARGS__}; (_array_) + _KLS_ARRAY_AT(__VA_ARGS__)(KLS_MVN(_sa)); })

#define KLS_MAX(...) _KLS_EXTREMUM(>,__VA_ARGS__)

#define KLS_MIN(...) _KLS_EXTREMUM(<,__VA_ARGS__)

#define KLS_CMP(...) M_OVERLOAD(_KLS_CMP,__VA_ARGS__)(__VA_ARGS__)

#define KLS_AVERAGE(...) _KLS_AVERAGE(__VA_ARGS__)

#define KLS_ARGS_COUNT(...) (M_FOREACH(_KLS_ARGS_COUNT,-,__VA_ARGS__) 0)

#define KLS_ENDIAN() ({ const union{ short s; char c[sizeof(short)];}u={1}; u.c[0]; })

#define KLS_RUNTIME RUNTIME



// COMMON SECTION
int KLS_utos(size_t n,char *s);
int KLS_itos(ptrdiff_t n,char *s);

void KLS_execKill(void);
void KLS_pausef(double sec);
void KLS_freeFile(FILE *file);
void KLS_freeData(void *data);
void KLS_ptrDeleter(void *data);
void KLS_execNameSet(const char *name);
void KLS_variableRevers(void *var,KLS_size size);
void KLS_swap(void *var1, void *var2,KLS_size size);
void KLS_pause(int sec,int mlsec,int mksec,int nnsec);
void KLS_memmove(void *to,const void *from,KLS_size size);
void KLS_binaryPrint(const void *ptr,KLS_size size,FILE *f);
void KLS_addr2line(void * const *address,unsigned int count);
void KLS_bitSet(void *data,unsigned int index,KLS_byte value);
void KLS_revers(void *array,KLS_size arraySize,KLS_size elementSize);

void *KLS_free(void *data);
void *KLS_malloc(KLS_size size);
void *KLS_dublicate(const void *data,KLS_size len);

double KLS_round(double value,double step);
double KLS_mod(double value,double division);

KLS_byte KLS_execLive(void);
KLS_byte KLS_dataJoin(void **dst,KLS_size *dstSize,void **src,KLS_size *srcSize,KLS_byte frees); // frees:000000xy , where 'x' for free src, 'y' for free dst

const char *KLS_execNameGet(void);
const char *KLS_getOpt(int argc, char *argv[],const char *opt);

const KLS_byte *KLS_exec(void);

KLS_t_URL_DATA *KLS_urlRequest(const KLS_t_URL *url);

unsigned int KLS_crc32(unsigned int crc,const void *data,KLS_size size); //first call with crc=-1

unsigned char KLS_bitGet(void *data,unsigned int index);

#define KLS_freeData(_1_) (_1_)=KLS_free(_1_)
#define KLS_freeFile(_1_) ({ if((_1_) && (_1_)!=stdin && (_1_)!=stdout && (_1_)!=stderr) {fclose(_1_);(_1_)=NULL;} })






// SYSTEM SECTION
int KLS_sysCmd(char **output,const char *cmdFormat, ...);

KLS_byte KLS_sysInfoRam(KLS_size *left,KLS_size *all);
KLS_byte KLS_sysInfoHdd(const char *folder,KLS_size *left,KLS_size *all);

unsigned int KLS_sysInfoCores(void);




// ARRAY SECTION
void KLS_arrayForEach(KLS_any);
void KLS_arraySet(void *array,KLS_size arraySize,KLS_size elementSize,const void *element);
void KLS_arrayPrint(const void *array,KLS_size arraySize,KLS_size elementSize,void(*printer)(const void *element));
void KLS_arraySort(void *array,KLS_size arraySize,KLS_size elementSize,KLS_byte(*comparator)(const void *prev,const void *next,void *arg),void *arg);

void *KLS_arrayAt(void *array,KLS_size arraySize,KLS_size elementSize,KLS_size index);

KLS_size KLS_arrayExtremum(const void *array,KLS_size arraySize,KLS_size elementSize,KLS_byte(*comparator)(const void * elementArray, const void *extremum));
KLS_size KLS_arrayFind(const void *array,KLS_size arraySize,KLS_size elementSize,const void *element,KLS_byte(*comparator)(const void *elementArray, const void *element)); //comparator returns [0,...]: if(cmp) return index+cmp-1;

KLS_byte KLS_arrayRemove(void *array,KLS_size arraySize,KLS_size elementSize,KLS_size index);
KLS_byte KLS_arrayInsert(void *array,KLS_size arraySize,KLS_size elementSize,KLS_size index,const void *element);

#define KLS_arrayForEach(_1_) ({ _1_* _KLS_arrayForEach
#define _KLS_arrayForEach(_1_,_2_,_3_) _3_=(_1_), *_4_=_3_+(_2_); for(;_3_!=_4_;++_3_) __KLS_arrayForEach
#define __KLS_arrayForEach(...) {__VA_ARGS__}})





// FILE SYSTEM SECTION

typedef struct{
    struct _KLS_t_FS_RULES{
        KLS_byte r:2,w:2,e:2,error:2;
    } owner, groop, other;
}KLS_t_FS_RULE;

typedef struct{
    KLS_size size;
    time_t create, access, mod;
    KLS_byte type;
}KLS_t_FS_INFO;

#define KLS_FS_TYPE_ERROR   1
#define KLS_FS_TYPE_UNKNOWN 2
#define KLS_FS_TYPE_FOLDER  4
#define KLS_FS_TYPE_FILE    8
#define KLS_FS_TYPE_LINK    16
#define KLS_FS_TYPE_ALL     (KLS_FS_TYPE_ERROR | KLS_FS_TYPE_UNKNOWN | KLS_FS_TYPE_FOLDER | KLS_FS_TYPE_FILE | KLS_FS_TYPE_LINK)

int KLS_fsContentCount(const char *directory);    // -1 error, 0 empty, 1+ count

void KLS_fsInfoPrint(const KLS_t_FS_INFO *info,FILE *f);
void KLS_fsRulePrint(const KLS_t_FS_RULE *rules,FILE *f);

KLS_byte KLS_fsRemove(const char *name);
KLS_byte KLS_fsDirCreate(const char *directory);
KLS_byte KLS_fsContentForEach(const char *directory,void(*action)(const char *name,KLS_byte type,void *arg),void *arg);

KLS_byte KLS_fsRuleGet(const char *path,KLS_t_FS_RULE *rule);
KLS_byte KLS_fsInfoGet(const char *fileName,KLS_t_FS_INFO *info);




// STRING SECTION
int KLS_stringLen(const char *format,...);
#define KLS_stringLen(...) snprintf(NULL,0,__VA_ARGS__)

void KLS_stringRect(const char *string,const KLS_t_FONT *font,int *width,int *height);

char *KLS_stringReadFile(const char *filePath);
char *KLS_string(char **variable,const char *format, ...);
char *KLS_stringReplace(const char *string, const char *from, const char *to);

char *KLS_stringv(const char *format);
#define KLS_stringv(_1_) ({va_list _2_[2]; va_start(_2_[0],_1_); va_start(_2_[1],_1_); _KLS_stringv(_1_,_2_);})

double KLS_stringSolve(const char *string,...);

unsigned int KLS_stringWordCount(const char *string, const char *word);

const char *KLS_stringPosition(const char *string,const KLS_t_FONT *font,KLS_byte align,int x,int y);

const char *KLS_stringSep(const char * const * string,unsigned int *len,...);
const char *_KLS_stringSep(const char ** string,unsigned int *len,const char * const * separators);
#define KLS_stringSep(_1_,_2_,...) ({ const char *_3_[]={__VA_ARGS__,NULL}; _KLS_stringSep((_1_),_2_,_3_); })




// REGEX SECTION

/* supported syntax:
    .
    ^
    $
    *
    +
    ?
    {m}
    {m,}
    {m,n}
    *?
    +?
    ??
    {m}?
    {m,}?
    {m,n}?
    [ab-c\d(erw|bad)]
    [^ab-c\d(erw|bad)]
    (...)
    (?:...)
    (?ism-ism:...)
    (?ism-ism)
    \aAcCdDpPwWxXul
*/

typedef struct{ const char *begin, *end; }KLS_t_REGEX_CAP;

void KLS_regexPrint(const void *regex,FILE *f);

void *KLS_regexNew(const char *pattern,const char **err);

KLS_byte KLS_regexMatch(const void *regex,const char *begin,const char *end);

const char *KLS_regexFind(const void *regex,const char *begin,const char *end,KLS_t_REGEX_CAP *cap);





// MATRIX SECTION

#define KLS_ALIGN_H_LEFT    1
#define KLS_ALIGN_H_MID     2
#define KLS_ALIGN_H_RIGHT   4
#define KLS_ALIGN_V_BTM     8
#define KLS_ALIGN_V_MID     16
#define KLS_ALIGN_V_TOP     32

#define KLS_MATRIX_LOOP_COLUMN   1
#define KLS_MATRIX_LOOP_ROW      2
#define KLS_MATRIX_SUBUNLIM_H    4
#define KLS_MATRIX_SUBUNLIM_V    8

typedef struct{
    void *data;
    void (*deleter)(void *element);
    unsigned int rows, columns, elSize;
    struct{ unsigned int rr,rc,bh,bw,br,bc; }_;
    int subRow, subColumn;
    KLS_byte options, _free;
}KLS_t_MATRIX;

void KLS_matrixFree(KLS_t_MATRIX *matrix);
void KLS_matrixPrint(const KLS_t_MATRIX *matrix,void(*printer)(const void *element));
void KLS_matrixPutElement(KLS_t_MATRIX *matrix,int row,int column,const void *element);
void KLS_matrixPutPoint(KLS_t_MATRIX *matrix,int row,int column,const void *element,unsigned int width);
void KLS_matrixFillArea(KLS_t_MATRIX *matrix,int row,int column,const void *element,const void *elementOfBorder);
void KLS_matrixPutLine(KLS_t_MATRIX *matrix,int row1,int column1,int row2,int column2,const void *element,unsigned int width);
void KLS_matrixPutRound(KLS_t_MATRIX *matrix,int row,int column,unsigned int radius,const void *element,unsigned int width,const void *fill);
void KLS_matrixPutRect(KLS_t_MATRIX *matrix,int row_1,int column_1,int row_2,int column_2,const void *element,unsigned int width,const void *fill);
void KLS_matrixPutMatrix(KLS_t_MATRIX *matrix,int row,int column,const KLS_t_MATRIX *element,void(*putter)(void *elementDst,const void *elementSrc));
void KLS_matrixPutString(KLS_t_MATRIX *matrix,int row,int column,const void *element,const KLS_t_FONT *font,KLS_byte align,const char *string);
void KLS_matrixPutStringf(KLS_t_MATRIX *matrix,int row,int column,const void *element,const KLS_t_FONT *font,KLS_byte align,const char *format,...);
void KLS_matrixPutEllipse(KLS_t_MATRIX *matrix,int row,int column,KLS_long rowCount,KLS_long columnCount,const void *element,unsigned int width,const void *fill);
void KLS_matrixPutArcRound(KLS_t_MATRIX *matrix,int row,int column,unsigned int radius,float angle_rad,float angleRot_rad,const void *element,unsigned int width);
void KLS_matrixPutArcEllipse(KLS_t_MATRIX *matrix,int row,int column,KLS_long rowCount,KLS_long columnCount,float angle_rad,float angleRot_rad,const void *element,unsigned int width);

void *KLS_matrixAt(const KLS_t_MATRIX *matrix,int row,int column);

KLS_byte KLS_matrixTransform(const KLS_t_MATRIX *from,KLS_t_MATRIX *to,KLS_byte(*transformer)(const void *elFrom,void *elTo,void *arg),void *arg);

KLS_t_MATRIX KLS_matrixGetMatrix(const KLS_t_MATRIX *matrix,int row,int column,unsigned int rows,unsigned int columns,KLS_byte options);
KLS_t_MATRIX KLS_matrixNew(void *matrix,unsigned int countRow,unsigned int countColumn,unsigned int elementSize,void(*deleter)(void *element));





// VECTOR SECTION

typedef struct{
    void *data;
    void (*deleter)(void *element);
    KLS_size (*policy)(KLS_size size);
    KLS_size size,sizeReal,sizeElement;
}KLS_t_VECTOR;

void KLS_vectorFree(KLS_t_VECTOR *vector);
void KLS_vectorClear(KLS_t_VECTOR *vector);
void KLS_vectorRemove(KLS_t_VECTOR *vector,KLS_size index);
void KLS_vectorRemoveFast(KLS_t_VECTOR *vector,KLS_size index);
void KLS_vectorSetPolicy(KLS_t_VECTOR *vector,KLS_size(*policy)(KLS_size size));
void KLS_vectorPrint(const KLS_t_VECTOR *vector,void(*printer)(const void *element));
void KLS_vectorSort(KLS_t_VECTOR *vector,KLS_byte(*comparator)(const void *prev,const void *next,void *arg),void *arg);
void *KLS_vectorAt(const KLS_t_VECTOR *vector,KLS_size index);

KLS_size KLS_vectorIndexOf(const KLS_t_VECTOR *vector,const void *element,KLS_byte(*comparator)(const void *elementVector,const void *element));

KLS_byte KLS_vectorPushBack(KLS_t_VECTOR *vector,const void *element);
KLS_byte KLS_vectorInsert(KLS_t_VECTOR *vector,KLS_size index,const void *element);

KLS_t_VECTOR KLS_vectorCopy(const KLS_t_VECTOR *vector);
KLS_t_VECTOR KLS_vectorNew(KLS_size size,KLS_size elementSize,void(*deleter)(void *element));



// LIST SECTION

typedef struct{
    void *first, *last;
    void (*deleter)(void *element);
    KLS_size size, sizeElement;
}KLS_t_LIST;

void KLS_listForEach(KLS_any);
void KLS_listClear(KLS_t_LIST *list);
void KLS_listRemove(KLS_t_LIST *list,void *element);
void KLS_listRemoveAt(KLS_t_LIST *list,KLS_size index);
void KLS_listMoveAfter(KLS_t_LIST *list,void *element,void *current);
void KLS_listMoveBefore(KLS_t_LIST *list,void *element,void *current);
void KLS_listPrint(const KLS_t_LIST *list,void(*printer)(const void *element));
void KLS_listSort(KLS_t_LIST *list, KLS_byte(*comparator)(const void *prev,const void *next,void *arg),void *arg);

void *KLS_listNext(const void *element);
void *KLS_listPrev(const void *element);
void *KLS_listAt(const KLS_t_LIST *list,KLS_size index);

void* KLS_listPushBack(KLS_t_LIST *list,const void *element);
void* KLS_listPushFront(KLS_t_LIST *list,const void *element);
void* KLS_listInsert(KLS_t_LIST *list,const void *element,KLS_size index);
void* KLS_listPushAfter(KLS_t_LIST *list,const void *element,void *current);
void* KLS_listPushBefore(KLS_t_LIST *list,const void *element,void *current);

KLS_size KLS_listIndexOf(const KLS_t_LIST *list,const void *element,KLS_byte(*comparator)(const void *elementList,const void *element),void **pointer);

KLS_t_LIST KLS_listCopy(const KLS_t_LIST *list);
KLS_t_LIST KLS_listMerge(KLS_t_LIST *list_1, KLS_t_LIST *list_2);
KLS_t_LIST KLS_listNew(KLS_size elementSize,void(*deleter)(void *element));

#define KLS_listNext(_1_) ((void**)(_1_))[-1]
#define KLS_listPrev(_1_) ((void**)(_1_))[-2]
#define KLS_listForEach(_1_) ({ _1_* _KLS_listForEach
#define _KLS_listForEach(_1_,_2_) _2_=(void*)(((KLS_t_LIST*)(_1_))->first); for(;_2_;_2_=KLS_listNext(_2_)) __KLS_listForEach
#define __KLS_listForEach(...) {__VA_ARGS__}})




// QUEUE SECTION
enum{ KLS_FIFO=0, KLS_LIFO };

typedef struct{
    void (*deleter)(void *element);
    void *_[2];
    KLS_size size;
    unsigned int sizeElement;
    KLS_byte _opt;
}KLS_t_QUEUE;

KLS_t_QUEUE KLS_queueNew(KLS_byte order,KLS_size elementSize,void(*deleter)(void *element));

void KLS_queueClear(KLS_t_QUEUE *queue);
void KLS_queueSort(KLS_t_QUEUE *queue,KLS_byte(*comparator)(const void *prev, const void *next,void *arg),void *arg);

KLS_byte KLS_queuePop(KLS_t_QUEUE *queue,void *var);
KLS_byte KLS_queuePush(KLS_t_QUEUE *queue,const void *element);




// GEOMETRY COMMON SECTION

typedef struct{
    double x,y;
}KLS_t_POINT;

double KLS_angleTo360(double angle);
double KLS_angleTo360_rad(double angle_rad);
double KLS_lendir_x(double len,double angle);
double KLS_lendir_y(double len,double angle);
double KLS_lendir_x_rad(double len,double angle_rad);
double KLS_lendir_y_rad(double len,double angle_rad);
double KLS_angle2Range(double angle,double angleRange);
double KLS_p2dir(double x1,double y1,double x2,double y2);
double KLS_p2dist(double x1,double y1,double x2,double y2);
double KLS_p2dir_rad(double x1,double y1,double x2,double y2);
double KLS_angle2Range_rad(double angle_rad,double angleRange_rad);
double KLS_interpol(double x1,double y1,double x2,double y2,double y3);
double KLS_point2Line(double x,double y,double x1,double y1,double x2,double y2);

KLS_byte KLS_pointInRay(double x,double y,double x1,double y1,double x2,double y2);
KLS_byte KLS_pointInLine(double x,double y,double x1,double y1,double x2,double y2);
KLS_byte KLS_pointInSlice(double x,double y,double x1,double y1,double x2,double y2);
KLS_byte KLS_pointMaskSlice(double x,double y,double x1,double y1,double x2,double y2);
KLS_byte KLS_angleInSector(double angle,double angleSector,double angleSectorRot);
KLS_byte KLS_angleInSector_rad(double angle_rad,double angleSector_rad,double angleSectorRot_rad);

KLS_t_POINT KLS_point(double x,double y);



//GEOMETRY CROSS SECTION

void KLS_pointsPrint(const KLS_t_POINT *p,unsigned int count,FILE *f);

signed char KLS_crossRoundRound(double r1,double x1,double y1,double r2,double x2,double y2,KLS_t_POINT p[static 2]);
signed char KLS_crossRoundLine(double r,double x0,double y0,double x1,double y1,double x2,double y2,KLS_t_POINT p[static 2]);
signed char KLS_crossRoundSlice(double r,double x0,double y0,double x1,double y1,double x2,double y2,KLS_t_POINT p[static 2]);
signed char KLS_crossLineLine(double x11,double y11,double x12,double y12,double x21,double y21,double x22,double y22,KLS_t_POINT p[static 1]);
signed char KLS_crossSliceLine(double x11,double y11,double x12,double y12,double x21,double y21,double x22,double y22,KLS_t_POINT p[static 1]);
signed char KLS_crossSliceSlice(double x11,double y11,double x12,double y12,double x21,double y21,double x22,double y22,KLS_t_POINT p[static 1]);
signed char KLS_crossArcLine(double r, double x0, double y0,double angle_rad,double angleRot_rad, double x1, double y1, double x2, double y2,KLS_t_POINT p[static 2]);
signed char KLS_crossArcSlice(double r, double x0, double y0,double angle_rad,double angleRot_rad, double x1, double y1, double x2, double y2,KLS_t_POINT p[static 2]);



//GEOMETRY POLYGON SECTION
double KLS_polySquad(const KLS_t_POINT *poly,int len);

KLS_byte KLS_polyIsConcave(const KLS_t_POINT *poly,int len);
KLS_byte KLS_polyIsIntersected(const KLS_t_POINT *poly,int len);
KLS_byte KLS_pointInPoly(double x,double y,const KLS_t_POINT *poly,int count); // 0=no, 1=yes, 2=on border

KLS_t_POINT KLS_polyCentroid(const KLS_t_POINT *poly,int len);
KLS_t_POINT KLS_polyLabel(const KLS_t_POINT *poly,int len,double precision);

KLS_t_VECTOR KLS_polyBounce(const KLS_t_POINT *poly,int len,double delta); // <KLS_t_POINT>
KLS_t_VECTOR KLS_polyBounce_v2(const KLS_t_POINT *poly,int len,double delta); // <KLS_t_POINT>
KLS_t_VECTOR KLS_polyStretch(const KLS_t_POINT *poly,int len,double delta); // <KLS_t_POINT>

KLS_t_VECTOR KLS_polyFromMatrixAll(const KLS_t_MATRIX *matrix,const void *element,KLS_byte chainMode,KLS_byte(*comparator)(const void *matrixElement,const void *element)); //<KLS_t_VECTOR <KLS_t_POINT> >  chainDirection may be 4 or 8
KLS_t_VECTOR KLS_polyFromMatrix(const KLS_t_MATRIX *matrix,int row,int column,const void *element,KLS_byte chainMode,KLS_byte(*comparator)(const void *matrixElement,const void *element)); // <KLS_t_POINT>  chainDirection may be 4 or 8




// DRAW SECTION

typedef int KLS_COLOR;

typedef struct{
    KLS_t_MATRIX m;
    double left,up,right,down;
    double _dv,_dh;
}KLS_t_CANVAS;

typedef struct{
    unsigned short division; // default 2
    KLS_byte gridOn;         // default off
    const char *format;      // default "%.0f"
}KLS_t_AXIS;

#define KLS_RGB(_r_,_g_,_b_) _KLS_rgb(_r_,_g_,_b_)

void KLS_canvasFree(KLS_t_CANVAS *canvas);
void KLS_canvasClear(KLS_t_CANVAS *canvas,const void *color);
void KLS_canvasBMP(const KLS_t_CANVAS *canvas,const char *fileName);
void KLS_canvas2Display(KLS_t_CANVAS *canvas,GUI_t_DISPLAY *display);
void KLS_canvasBMPf(const KLS_t_CANVAS *canvas,const char *format,...);
void KLS_canvasArea(KLS_t_CANVAS *canvas,double left,double up,double right,double down);
void KLS_canvasPoint(KLS_t_CANVAS *canvas,double x,double y,const void *color,unsigned int width);
void KLS_canvasLine(KLS_t_CANVAS *canvas,double x1,double y1,double x2,double y2,const void *color,unsigned int width);
void KLS_canvasRound(KLS_t_CANVAS *canvas,double x,double y,double r,const void *color,unsigned int width,const void *colorFill);
void KLS_canvasArc(KLS_t_CANVAS *canvas,double x,double y,double r,float angle, float angleRot,const void *color,unsigned int width);
void KLS_canvasRect(KLS_t_CANVAS *canvas,double x1,double y1,double x2,double y2,const void *color,unsigned int width,const void *colorFill);
void KLS_canvasText(KLS_t_CANVAS *canvas,double x,double y,const KLS_t_FONT *font,KLS_byte align,const void *color,const char *text);
void KLS_canvasTextf(KLS_t_CANVAS *canvas,double x,double y,const KLS_t_FONT *font,KLS_byte align,const void *color,const char *format,...);
void KLS_canvasEllipse(KLS_t_CANVAS *canvas,double x,double y,double xWidth,double yHeight,const void *color,unsigned int width,const void *colorFill);
void KLS_canvasArcEllipse(KLS_t_CANVAS *canvas,double x,double y,double xWidth,double yHeight,float angle, float angleRot,const void *color,unsigned int width);

KLS_t_CANVAS KLS_canvasNew(void *buffer,unsigned int width,unsigned int height);
KLS_t_CANVAS KLS_canvasNewExt(void *buffer,unsigned int width,unsigned int height,unsigned char colorSize,double left,double up,double right,double down);
KLS_t_CANVAS KLS_canvasPlot(KLS_t_CANVAS *canvas,const KLS_t_AXIS *axisX,const KLS_t_AXIS *axisY,const void *colorAxis,const void *colorBackground);
KLS_t_CANVAS KLS_canvasSub(const KLS_t_CANVAS *canvas,double left,double up,double right,double down);
KLS_t_CANVAS KLS_canvasSubExt(const KLS_t_CANVAS *canvas,int pixelX, int pixelY,unsigned int pixWidth,unsigned int pixHeight,double left,double up,double right,double down);

void *KLS_canvasAtPix(const KLS_t_CANVAS *canvas,int pixelX,int pixelY,KLS_t_POINT *point);





// DISPLAY SECTION

enum GUI_KEYS{
    GUI_KEY_SPACE=' ',
    GUI_KEY_ESC=256,
    GUI_KEY_HOME,
    GUI_KEY_END,
    GUI_KEY_PAGEUP,
    GUI_KEY_PAGEDOWN,
    GUI_KEY_INS,
    GUI_KEY_DEL,
    GUI_KEY_TAB,
    GUI_KEY_ENTER,
    GUI_KEY_BCSP,
    GUI_KEY_LEFT,
    GUI_KEY_RIGHT,
    GUI_KEY_UP,
    GUI_KEY_DOWN,
    GUI_KEY_LB,
    GUI_KEY_RB,
    GUI_KEY_WHEEL,
    GUI_KEY_CTRL=1<<16,
    GUI_KEY_ALT=1<<17,
    GUI_KEY_SHIFT=1<<18,
};

enum GUI_EVENTS{
    GUI_EVENT_DESTROY=0,
    GUI_EVENT_INTERRUPT=1<<0,
    GUI_EVENT_UPDATE=1<<1,
    GUI_EVENT_PRESS=1<<2,
    GUI_EVENT_RELEASE=1<<3,
    GUI_EVENT_CURSOR=1<<4,
    GUI_EVENT_WHEEL=1<<5,
};

typedef struct{
    struct{int x,y,dx,dy;} mouse;
    int key,keys;
    signed char wheel;
}GUI_t_INPUT;

struct _GUI_t_DISPLAY{
    struct{
        union{void *p; unsigned long l; int i;} osDep[6];
    }_;
    const char *title;
    int x,y,width,height;
    KLS_t_MATRIX m;
    GUI_t_INPUT input;
};

int GUI_displayEvent(GUI_t_DISPLAY *display);
void GUI_displayFree(GUI_t_DISPLAY *display);
void GUI_displayDraw(GUI_t_DISPLAY *display);
void GUI_displayUpdate(GUI_t_DISPLAY *display);
void GUI_displayInterrupt(GUI_t_DISPLAY *display);
void GUI_displaySetPos(GUI_t_DISPLAY *display,int x,int y);
void GUI_displaySetTitle(GUI_t_DISPLAY *display,const char *title);
void GUI_displaySetSize(GUI_t_DISPLAY *display,int width,int height);

GUI_t_DISPLAY GUI_displayNew(const char *title,int x,int y,int width,int height);




// WIDGETS SECTION

#define CLASS_BEGIN__GUI_WIDGET \
    constructor(void *parent,const char *id)(\
        self->visible=self->able=1;\
    ),\
    public(\
        struct{\
            void (*draw)(void *self);\
            void (*update)(void *self);\
            void (*select)(void *self);\
            void (*insert)(void *other);\
            void (*input)(void *self,int event,GUI_t_INPUT *input);\
        }core;\
        const char * const id;\
        CLASS GUI * const gui;\
        CLASS GUI_WIDGET * const parent;\
        void (*onInput)(void *self,int event,GUI_t_INPUT *input);\
        void *userData;\
        int x,y;\
        int width,height;\
        KLS_byte able:1;\
        KLS_byte visible:1;\
        KLS_byte movable:1;\
        KLS_byte resizable:1;\
        KLS_byte detachable:1;\
    ),\
    private(\
        CLASS GUI_WIDGET *child,*prev,*next,*last;\
        KLS_t_MATRIX m;\
    )
CLASS_END(GUI_WIDGET);


#define CLASS_BEGIN__GUI_LABEL\
    extends(GUI_WIDGET),\
    constructor(void *parent,const char *id,const char *text)(\
        super(self,parent,id);\
        self->font=KLS_fontBase;\
    ),\
    public(\
        char *text;\
        KLS_t_FONT font;\
        KLS_COLOR color;\
        KLS_byte align;\
    )
CLASS_END(GUI_LABEL);


#define CLASS_BEGIN__GUI_BUTTON \
    extends(GUI_WIDGET),\
    constructor(void *parent,const char *id)(\
        super(self,parent,id);\
        self->width=80;\
        self->height=20;\
        self->font=KLS_fontBase;\
        self->colorBorder=KLS_COLOR_BLACK;\
        self->colorOff=KLS_COLOR_GREY;\
        self->colorOn=KLS_COLOR_DARK_GREY;\
        self->colorText=KLS_COLOR_BLACK;\
    ),\
    public(\
        char *text;\
        KLS_t_FONT font;\
        KLS_COLOR colorOn;\
        KLS_COLOR colorOff;\
        KLS_COLOR colorText;\
        KLS_COLOR colorBorder;\
        KLS_byte pressable:1;\
        KLS_byte isPressed:1;\
    )
CLASS_END(GUI_BUTTON);


#define CLASS_BEGIN__GUI_PROGRESS\
    extends(GUI_WIDGET),\
    constructor(void *parent,const char *id)(\
        super(self,parent,id);\
        self->width=100;\
        self->height=20;\
        self->format="%.1f";\
        self->colorBorder=KLS_COLOR_BLACK;\
        self->colorBackground=KLS_COLOR_GREY;\
        self->colorProgress=KLS_COLOR_YELLOW;\
    ),\
    public(\
        const char *format;\
        float value;\
        KLS_COLOR colorBorder;\
        KLS_COLOR colorBackground;\
        KLS_COLOR colorProgress;\
    )
CLASS_END(GUI_PROGRESS);


#define CLASS_BEGIN__GUI_CANVAS \
    extends(GUI_WIDGET),\
    constructor(void *parent,const char *id,unsigned int width,unsigned int height)(\
        super(self,parent,id);\
        self->width=width;\
        self->height=height;\
    ),\
    public(\
        KLS_t_CANVAS canvas;\
        KLS_t_POINT p;\
    )
CLASS_END(GUI_CANVAS);


#define CLASS_BEGIN__GUI_SLIDER   abstract,\
    extends(GUI_WIDGET),\
    constructor(void *parent,const char *id,double min,double max,double step)(\
        super(self,parent,id);\
        self->min=self->value=min;\
        self->max=max;\
        self->step=step;\
        self->colorBorder=KLS_COLOR_BLACK;\
        self->colorBackground=KLS_COLOR_GREY;\
    ),\
    public(\
        void (*onChange)(void *self);\
        double min;\
        double max;\
        double step;\
        double value;\
        KLS_COLOR colorBorder;\
        KLS_COLOR colorBackground;\
    ),\
    private(\
        CLASS GUI_WIDGET *p;\
        double v;\
    )
CLASS_END(GUI_SLIDER);


#define CLASS_BEGIN__GUI_SLIDER_V \
    extends(GUI_SLIDER),\
    constructor(void *parent, const char *id,double min,double max,double step)(\
        super(self,parent,id,min,max,step);\
        self->width=15;\
        self->height=100;\
    )
CLASS_END(GUI_SLIDER_V);


#define CLASS_BEGIN__GUI_SLIDER_H \
    extends(GUI_SLIDER),\
    constructor(void *parent, const char *id,double min,double max,double step)(\
        super(self,parent,id,min,max,step);\
        self->width=100;\
        self->height=15;\
    )
CLASS_END(GUI_SLIDER_H);


#define CLASS_BEGIN__GUI_BOX  abstract,\
    extends(GUI_WIDGET),\
    constructor(void *parent, const char *id,unsigned int width,unsigned int height)(\
        super(self,parent,id);\
        self->width=self->widthMax=width;\
        self->height=self->heightMax=height;\
    ),\
    public(\
        CLASS GUI_WIDGET *box;\
        CLASS GUI_SLIDER *sliderV, *sliderH;\
        int widthMax, heightMax;\
    )
CLASS_END(GUI_BOX);


#define CLASS_BEGIN__GUI_TEXTBOX \
    extends(GUI_BOX),\
    constructor(void *parent, const char *id,unsigned int width,unsigned int height)(\
        super(self,parent,id,width,height);\
        self->editable=1;\
        self->font=KLS_fontBase;\
        self->colorText=self->colorBorder=KLS_COLOR_BLACK;\
        self->colorBackground=KLS_COLOR_WHITE;\
    ),\
    public(\
        char *text;\
        KLS_t_FONT font;\
        KLS_COLOR colorText;\
        KLS_COLOR colorBorder;\
        KLS_COLOR colorBackground;\
        KLS_byte editable:1;\
    ),\
    private(\
        struct{int i,x,y,w,h;}pos;\
    )
CLASS_END(GUI_TEXTBOX);


#define CLASS_BEGIN__GUI \
    extends(GUI_BOX),\
    constructor(unsigned int width,unsigned int height)(\
        super(self,self,NULL,width,height);\
        self->color=KLS_COLOR_GREY;\
    ),\
    public(\
        const char *title;\
        int (* const service)(void *self);\
        void (* const update)(void *self);\
        void (* const interrupt)(void *self);\
        void (* const setFps)(void *self,KLS_byte fps);\
        KLS_COLOR color;\
    ),\
    private(\
        void *focus, *select, *block, *trash;\
        struct timer timer;\
        GUI_t_DISPLAY display;\
        unsigned int flags,tout;\
        int wmv[2];\
    )
CLASS_END(GUI);

void GUI_coreDefault(void);

void GUI_widgetDelete(CLASS GUI_WIDGET **widget);

void GUI_widgetDrawText(void *widget,int x,int y,const char *text,const void *color);
void GUI_widgetDrawRect(void *widget,int marginH,int marginV,const void *color,const void *fill);
void GUI_widgetDrawRectExt(void *widget,int x,int y,int w,int h,const void *color,const void *fill);
void GUI_widgetDrawTextExt(void *widget,int x,int y,const KLS_t_FONT *font,KLS_byte align,const char *text,const void *color);

KLS_byte GUI_widgetInFocus(void *widget);
KLS_byte GUI_widgetIsMoved(void *widget);    /* returns (x<<0) | (y<<1) */
KLS_byte GUI_widgetIsResized(void *widget);  /* returns (width<<0) | (height<<1) */
KLS_byte GUI_widgetIsSelected(void *widget);

void *GUI_widgetSelect(void *widget);
void *GUI_widgetBlockOn(void *widget); /* return prev (this function help create widgets that blocking parents, for example message box or warning box) */
void *GUI_widgetFind(void *widget,const char *id);
void *GUI_widgetInsert(void *widget,void *parent);

KLS_t_CANVAS GUI_widgetAsCanvas(void *widget);

CLASS GUI_WIDGET *(*GUI_widgetNew(KLS_any))(void *self,...);
#define GUI_widgetNew(_name_) ({ void *KLS_MVN(_wgt_)=KLS_malloc(sizeof(CLASS _name_)); if(!_name_()->constructor(KLS_MVN(_wgt_) __GUI_widgetNew
#define __GUI_widgetNew(...) M_WHEN(M_IS_ARG(M_PEAK(__VA_ARGS__)))(,__VA_ARGS__) )) KLS_freeData(KLS_MVN(_wgt_)); KLS_MVN(_wgt_); })





// HIDDEN MACRO SECTION

#ifdef _KLS_GLOBVAR
    #undef _KLS_GLOBVAR
    #define _KLS_GLOBVAR(var, ...) var __VA_ARGS__
#else
    #define _KLS_GLOBVAR(var, ...) extern var
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _KLS_MEMORY_DEBUG
    _KLS_GLOBVAR(KLS_size _KLS_memAlloc,=0);
    _KLS_GLOBVAR(KLS_size _KLS_memFree,=0);
    _KLS_GLOBVAR(pthread_mutex_t _KLS_memMtx[2],={PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER});
    #define _KLS_MEMORY_PUSH(_1_) if(_1_){ pthread_mutex_lock(_KLS_memMtx); ++_KLS_memAlloc; pthread_mutex_unlock(_KLS_memMtx); }
    #define _KLS_MEMORY_POP(_1_)  if(_1_){ pthread_mutex_lock(_KLS_memMtx+1); ++_KLS_memFree;  pthread_mutex_unlock(_KLS_memMtx+1); }
    #define _KLS_MEMORY_SHOW()    { printf("KLS: allocs(%zu) frees(%zu)\n",_KLS_memAlloc,_KLS_memFree); }
    #warning _KLS_MEMORY_DEBUG is defined
#else
    #define _KLS_MEMORY_PUSH(...)
    #define _KLS_MEMORY_POP(...)
    #define _KLS_MEMORY_SHOW()
#endif
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
#define _KLS_MAIN0()
#define _KLS_MAIN1(...) argc
#define _KLS_MAIN2(...) argc,argv
#define _KLS_MAIN3(...) argc,argv,env
#define main(...)\
    __KLS_main(); M_TYPEOF(__KLS_main()) _KLS_main(__VA_ARGS__); \
    M_TYPEOF(__KLS_main()) main(int argc,char *argv[],char *env[]){KLS_execNameSet(argv[0]); return _KLS_main(M_OVERLOAD(_KLS_MAIN,__VA_ARGS__)()); (void)argc; (void)argv; (void)env;}\
    M_TYPEOF(__KLS_main()) _KLS_main(__VA_ARGS__)
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
#define _KLS_CAST(_pointer_) ({ const union{const void * const _; void *p;} M_JOIN(_cast,M_LINE())={(const void * const)(_pointer_)}; M_JOIN(_cast,M_LINE()).p; }) )
#define _KLS_ARGS_COUNT(_1_,_2_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( 1+ )
#define _KLS_ALLOC(...) {__VA_ARGS__}) )
#define _KLS_CMPCAST(_t_,_a_) ((unsigned _t_*)&(_a_))
#define __KLS_CMP(_t_,_1_,_2_,...) ( sizeof(_1_)==sizeof(_t_) ? *_KLS_CMPCAST(_t_,_1_) ^ *_KLS_CMPCAST(_t_,_2_) : (__VA_ARGS__) )
#define _KLS_CMP(_1_,_2_) (sizeof(_1_)==sizeof(_2_) ? __KLS_CMP(char,_1_,_2_, __KLS_CMP(short,_1_,_2_, __KLS_CMP(int,_1_,_2_, _KLS_CMPCAST(int,_1_)[0] ^ _KLS_CMPCAST(int,_2_)[0] || _KLS_CMPCAST(int,_1_)[1] ^ _KLS_CMPCAST(int,_2_)[1] ) ) ) : 1 )
#define _KLS_CMP2(_a_,_b_) ({ M_TYPEOF(_a_) _1_=(_a_); M_TYPEOF(_b_) _2_=(_b_); _KLS_CMP(_1_,_2_); })
#define _KLS_CMP3(_a_,_b_,_d_) (fabs((_a_)-(_b_))>(_d_))
#define __KLS_AVERAGE(_i_,_s_,_arg_) M_WHEN(M_IS_ARG(_arg_))( (_s_)+=(_arg_); )
#define ____KLS_EXTREMUM(_op_,_1_,_c_) {M_TYPEOF(_c_) _3_=(_c_); if(_3_ _op_ _1_.x) memcpy(&_1_,&_3_,sizeof(_3_));}
#define ___KLS_EXTREMUM(...) ____KLS_EXTREMUM(__VA_ARGS__)
#define __KLS_EXTREMUM(_i_,_a_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( ___KLS_EXTREMUM(M_EXTRACT _a_, __VA_ARGS__) )
#define _KLS_EXTREMUM(_op_,_value_,...) ({\
    struct{M_TYPEOF(_value_) x;}_1_={(_value_)};\
    M_FOREACH(__KLS_EXTREMUM,(_op_,_1_),__VA_ARGS__) _1_.x;\
})
#define _KLS_AVERAGE(_value_,...) ({\
    double _1_=(_value_); M_FOREACH(__KLS_AVERAGE,_1_,__VA_ARGS__)\
    _1_/(KLS_ARGS_COUNT(__VA_ARGS__)+1);\
})
char *_KLS_stringv(const char *format, va_list ap[2]);
KLS_COLOR _KLS_rgbDetect(KLS_byte,KLS_byte,KLS_byte);
_KLS_GLOBVAR(KLS_COLOR(*_KLS_rgb)(KLS_byte,KLS_byte,KLS_byte),=_KLS_rgbDetect);
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
#define _KLS_ARRAY_AT(...) M_OVERLOAD(_KLS_ARRAY_AT_,__VA_ARGS__)(__VA_ARGS__)
#define _KLS_ARRAY_AT_2(_i_)  (_i_[0])
#define _KLS_ARRAY_AT_4(_i_)  (_KLS_ARRAY_AT_2(_i_)*_i_[3]+_i_[2])
#define _KLS_ARRAY_AT_6(_i_)  (_KLS_ARRAY_AT_4(_i_)*_i_[5]+_i_[4])
#define _KLS_ARRAY_AT_8(_i_)  (_KLS_ARRAY_AT_6(_i_)*_i_[7]+_i_[6])
#define _KLS_ARRAY_AT_10(_i_) (_KLS_ARRAY_AT_8(_i_)*_i_[9]+_i_[8])
#define _KLS_ARRAY_AT_12(_i_) (_KLS_ARRAY_AT_10(_i_)*_i_[11]+_i_[10])
#define _KLS_ARRAY_AT_14(_i_) (_KLS_ARRAY_AT_12(_i_)*_i_[13]+_i_[12])
#define _KLS_ARRAY_AT_16(_i_) (_KLS_ARRAY_AT_14(_i_)*_i_[15]+_i_[14])
#define _KLS_ARRAY_AT_18(_i_) (_KLS_ARRAY_AT_16(_i_)*_i_[17]+_i_[16])
#define _KLS_ARRAY_AT_20(_i_) (_KLS_ARRAY_AT_18(_i_)*_i_[19]+_i_[18])
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


#undef _KLS_GLOBVAR

#endif // KLS_LIB_H
