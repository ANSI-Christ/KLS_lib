#include "macro.h"

#ifdef OUTMAIN_SYNTAX
    #define _OUTMAIN_SYNTAX 1
#endif

/********   detecting compiller   ********/
#if defined(__GNUC__) && !defined(_OUTMAIN_SYNTAX)
    #define _OUTMAIN_SYNTAX 2
#endif
#if defined(__clang__) && !defined(_OUTMAIN_SYNTAX)
    #define _OUTMAIN_SYNTAX 2
#endif
#if defined(__MINGW32__) && !defined(_OUTMAIN_SYNTAX)
    #define _OUTMAIN_SYNTAX 2
#endif
#if defined(_MSC_VER) && !defined(_OUTMAIN_SYNTAX)
    #define _OUTMAIN_SYNTAX 3
#endif
#if !defined(_OUTMAIN_SYNTAX)
    OUTMAIN: syntax isn't detected, define macro OUTMAIN_SYNTAX as path to header with syntax!
#endif

/*****************   USER   **************/
#if _OUTMAIN_SYNTAX == 1
    #include OUTMAIN_SYNTAX
#endif

/*****************   GCC   **************/
#if _OUTMAIN_SYNTAX == 2
    #ifdef OUTMAIN_BEFORE
        __attribute__((__constructor__)) static void(M_JOIN(_v,OUTMAIN_BEFORE))(void){OUTMAIN_BEFORE();}
    #endif 
    #ifdef OUTMAIN_AFTER
        __attribute__((__destructor__)) static void(M_JOIN(_v,OUTMAIN_AFTER))(void){OUTMAIN_AFTER();}
    #endif 
#endif

/*****************   VC   **************/
#if _OUTMAIN_SYNTAX == 3 
    #ifdef OUTMAIN_BEFORE
        #ifdef _WIN64
            #pragma section(".CRT$XCU",read)
            __declspec(allocate(".CRT$XCU"))
        #else
            #pragma data_seg(".CRT$XCU")
        #endif
            static void(*M_JOIN(_v,OUTMAIN_BEFORE))(void) = OUTMAIN_BEFORE;
        #pragma data_seg()
    #endif
    #ifdef OUTMAIN_AFTER
        #ifdef _WIN64
            #pragma section(".CRT$XPU",read)
            __declspec(allocate(".CRT$XPU"))
        #else
            #pragma data_seg(".CRT$XPU")
        #endif
            static void(*M_JOIN(_v,OUTMAIN_AFTER))(void) = OUTMAIN_AFTER;
        #pragma data_seg()
    #endif
#endif

/*****************   HZ   **************/
#if _OUTMAIN_SYNTAX == 4
    #ifdef OUTMAIN_BEFORE
        #pragma startup OUTMAIN_BEFORE
    #endif 
    #ifdef OUTMAIN_AFTER
        #pragma exit OUTMAIN_AFTER
    #endif 
#endif

/*****************   ASM   **************/
#if _OUTMAIN_SYNTAX == 5
    #ifdef OUTMAIN_BEFORE
        __asm__ (".section .init \n call " M_STRING(OUTMAIN_BEFORE) " \n .section .text\n");
    #endif
    #ifdef OUTMAIN_AFTER
        __asm__ (".section .fini \n call " M_STRING(OUTMAIN_AFTER) " \n .section .text\n");
    #endif 
#endif


#ifdef OUTMAIN_BEFORE
    #undef OUTMAIN_BEFORE
#endif

#ifdef OUTMAIN_AFTER
    #undef OUTMAIN_AFTER
#endif
