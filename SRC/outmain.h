#include "macro.h"

#ifdef OUTMAIN_SYNTAX
    #include OUTMAIN_SYNTAX
    #define _OUTMAIN_SYNTAX 0 /* by user */
#endif

#ifndef _OUTMAIN_SYNTAX
    /* detecting compiller */
    #define _OUTMAIN_SYNTAX 4
#endif



#if _OUTMAIN_SYNTAX == 1 /* GCC */
    #ifdef OUTMAIN_BEFORE
        __attribute__((__constructor__)) static void M_JOIN(_OM_,M_PEAK(OUTMAIN_BEFORE))(void){ M_REMAINED(OUTMAIN_BEFORE); }
    #endif 
    #ifdef OUTMAIN_AFTER
        __attribute__((__destructor__)) static void M_JOIN(_OM_,M_PEAK(OUTMAIN_AFTER))(void){ M_REMAINED(OUTMAIN_AFTER); }
    #endif 
#endif

#if _OUTMAIN_SYNTAX == 2 /* VS */

#endif

#if _OUTMAIN_SYNTAX == 3 /*  */
    #ifdef OUTMAIN_BEFORE
        static void M_JOIN(_OM_,M_PEAK(OUTMAIN_BEFORE))(void){ M_REMAINED(OUTMAIN_BEFORE); }
        #pragma startup M_JOIN(_OM_,M_PEAK(OUTMAIN_BEFORE))
    #endif 
    #ifdef OUTMAIN_AFTER
        static void M_JOIN(_OM_,M_PEAK(OUTMAIN_AFTER))(void){ M_REMAINED(OUTMAIN_AFTER); }
        #pragma end M_JOIN(_OM_,M_PEAK(OUTMAIN_AFTER))
    #endif 
#endif

#if _OUTMAIN_SYNTAX == 4 /*  */
    #ifdef OUTMAIN_BEFORE
        void M_JOIN(_OM_,M_PEAK(OUTMAIN_BEFORE))(void){
            __asm__ (".section .init \n call " M_STRING(M_JOIN(_OM_,M_PEAK(OUTMAIN_BEFORE))) " \n .section .text\n");
            M_REMAINED(OUTMAIN_BEFORE);
        }
    #endif
    #ifdef OUTMAIN_AFTER
        void M_JOIN(_OM_,M_PEAK(OUTMAIN_AFTER))(void){
            __asm__ (".section .fini \n call " M_STRING(M_JOIN(_OM_,M_PEAK(OUTMAIN_AFTER))) " \n .section .text\n");
            M_REMAINED(OUTMAIN_AFTER);
        }
    #endif 
#endif




#ifdef OUTMAIN_BEFORE
    #undef OUTMAIN_BEFORE
#endif

#ifdef OUTMAIN_AFTER
    #undef OUTMAIN_AFTER
#endif

