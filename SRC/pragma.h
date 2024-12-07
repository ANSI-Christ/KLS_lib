#include "macro.h"

#ifdef PRAGMA_SYNTAX
    #define _PRAGMA_SYNTAX 1
#endif

/********   detecting compiller   ********/
#if defined(__GNUC__) && !defined(_PRAGMA_SYNTAX)
    #define _PRAGMA_SYNTAX 2
#endif
#if defined(__clang__) && !defined(_PRAGMA_SYNTAX)
    #define _PRAGMA_SYNTAX 2
#endif
#if defined(__MINGW32__) && !defined(_PRAGMA_SYNTAX)
    #define _PRAGMA_SYNTAX 2
#endif
#if defined(_MSC_VER) && !defined(_PRAGMA_SYNTAX)
    #define _PRAGMA_SYNTAX 3
#endif
#if !defined(_PRAGMA_SYNTAX)
    pragma.h: syntax undefined, define macro PRAGMA_SYNTAX as path to header with syntax!
#endif

/*****************   USER   **************/
#if _PRAGMA_SYNTAX == 1
    #include PRAGMA_SYNTAX
#endif

/*****************   GCC   **************/
#if _PRAGMA_SYNTAX == 2
    #ifdef PRAGMA_STARTUP
        __attribute__((__constructor__)) static void M_JOIN(_f_PRAGMA_STARTUP,PRAGMA_STARTUP) (void) {PRAGMA_STARTUP();}
    #endif
    #ifdef PRAGMA_ATEXIT
        __attribute__((__destructor__)) static void M_JOIN(_f_PRAGMA_ATEXIT,PRAGMA_ATEXIT) (void) {PRAGMA_ATEXIT();}
    #endif
    #ifdef _PRAGMA_PACK
        __attribute__((packed,aligned(PRAGMA_PACK)));
    #endif
#endif

/*****************   VC   **************/
#if _PRAGMA_SYNTAX == 3 
    #ifdef PRAGMA_STARTUP
        #ifdef _WIN64
            #pragma section(".CRT$XCU",read)
            __declspec(allocate(".CRT$XCU"))
        #else
            #pragma data_seg(".CRT$XCU")
        #endif
            static void (*M_JOIN(_f_PRAGMA_STARTUP,PRAGMA_STARTUP)) (void) = PRAGMA_STARTUP;
        #pragma data_seg()
    #endif
    #ifdef PRAGMA_ATEXIT
        #ifdef _WIN64
            #pragma section(".CRT$XPU",read)
            __declspec(allocate(".CRT$XPU"))
        #else
            #pragma data_seg(".CRT$XPU")
        #endif
            static void (*M_JOIN(_f_PRAGMA_ATEXIT,PRAGMA_ATEXIT)) (void) = PRAGMA_ATEXIT;
        #pragma data_seg()
    #endif
    #ifdef PRAGMA_PACK
        #pragma pack(push,PRAGMA_PACK)
    #endif
    #ifdef _PRAGMA_PACK
        ;#pragma pop()
    #endif
#endif

/*****************   HZ   **************/
#if _PRAGMA_SYNTAX == 4
    #ifdef PRAGMA_STARTUP
        #pragma startup PRAGMA_STARTUP
    #endif
    #ifdef PRAGMA_ATEXIT
        #pragma exit PRAGMA_ATEXIT
    #endif
    #ifdef PRAGMA_PACK
        #pragma pack(push,PRAGMA_PACK)
    #endif
    #ifdef _PRAGMA_PACK
        ;#pragma pop()
    #endif
#endif

/*****************   ASM   **************/
#if _PRAGMA_SYNTAX == 5
    #ifdef PRAGMA_STARTUP
        __asm__ (".section .init \n call " M_STRING(PRAGMA_STARTUP) " \n .section .text\n");
    #endif
    #ifdef PRAGMA_ATEXIT
        __asm__ (".section .fini \n call " M_STRING(PRAGMA_ATEXIT) " \n .section .text\n");
    #endif
    #ifdef PRAGMA_PACK
        #pragma pack(push,PRAGMA_PACK)
    #endif
    #ifdef _PRAGMA_PACK
        ;#pragma pop()
    #endif
#endif




#ifdef PRAGMA_STARTUP
    typedef struct{char _;} M_JOIN(_t_PRAGMA_ONCE_STARTUP,PRAGMA_STARTUP);
    #undef PRAGMA_STARTUP
#endif

#ifdef PRAGMA_ATEXIT
    typedef struct{char _;} M_JOIN(_t_PRAGMA_ONCE_ATEXIT,PRAGMA_ATEXIT);
    #undef PRAGMA_ATEXIT
#endif

#ifdef _PRAGMA_PACK
    #undef _PRAGMA_PACK
    #undef PRAGMA_PACK
#endif

#ifdef PRAGMA_PACK
    #ifndef _PRAGMA_PACK
        #define _PRAGMA_PACK
    #endif
#endif


