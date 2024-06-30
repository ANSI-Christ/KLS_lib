#ifndef KLS_SYSDEFS_H
#define KLS_SYSEFS_H

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                            MACROS DEFINED TABLE
//        The macroses below are avalible to in your code for checking
//  +---------------------------------------------------------------------------+
//  |              MACRO               |                 VALUES                 |
//  |---------------------------------------------------------------------------|
//  |  KLS_SYS_OS                      | KLS_SYS_OS_...                         |
//  |                                  | KLS_SYS_OS_UNKNOWN                     |
//  |---------------------------------------------------------------------------|
//  |  KLS_SYS_BITNESS                 | 0, 16, 32, 64, 128                     |
//  |---------------------------------------------------------------------------|
//  |  KLS_SYS_PLATFORM                | KLS_SYS_PLATFORM_...                   |
//  |                                  | KLS_SYS_PLATFORM_UNKNOWN               |
//  |---------------------------------------------------------------------------|
//  |  KLS_SYS_ENDIAN                  | KLS_SYS_ENDIAN_LITTLE                  |
//  |                                  | KLS_SYS_ENDIAN_BIG                     |
//  |                                  | KLS_SYS_ENDIAN_PDP                     |
//  |                                  | KLS_SYS_ENDIAN_UNKNOWN                 |
//  |---------------------------------------------------------------------------|
//  |                                  |                                        |
//  +---------------------------------------------------------------------------+
//
/////////////////////////////////////////////////////////////////////////////////////////////
//
//                            MACROS REDEFINED TABLE
//     The macroses below are avalible to define in makefile with "-D" option
//      * example -D_KLS_SYS_OS_SETUP=KLS_SYS_OS_WINDOWS
//  +---------------------------------------------------------------------------+
//  |              MACRO               |                 VALUES                 |
//  |---------------------------------------------------------------------------|
//  |  _KLS_SYS_OS_SETUP               | KLS_SYS_OS_...                         |
//  |                                  | KLS_SYS_OS_UNKNOWN                     |
//  |---------------------------------------------------------------------------|
//  |  _KLS_SYS_BITNESS_SETUP          | 0, 32, 64                              |
//  |---------------------------------------------------------------------------|
//  |  _KLS_SYS_PLATFORM_SETUP         | KLS_SYS_PLATFORM_...                   |
//  |                                  | KLS_SYS_PLATFORM_UNKNOWN               |
//  |---------------------------------------------------------------------------|
//  |  _KLS_SYS_ENDIAN_SETUP           | KLS_SYS_ENDIAN_LITTLE                  |
//  |                                  | KLS_SYS_ENDIAN_BIG                     |
//  |                                  | KLS_SYS_ENDIAN_PDP                     |
//  |                                  | KLS_SYS_ENDIAN_UNKNOWN                 |
//  |---------------------------------------------------------------------------|
//  |  _KLS_SYS_SHOW                   | anything                               |
//  |---------------------------------------------------------------------------|
//  |                                  |                                        |
//  +---------------------------------------------------------------------------+
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <limits.h>

/////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////   PRE DEFINES   /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
#define KLS_SYS_ENDIAN_UNKNOWN  0
#define KLS_SYS_ENDIAN_BIG      4321
#define KLS_SYS_ENDIAN_LITTLE   1234
#define KLS_SYS_ENDIAN_PDP      3412
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
#define KLS_SYS_OS_UNKNOWN    0
#define KLS_SYS_OS_WINDOWS    1
#define KLS_SYS_OS_UNIX       2
#define KLS_SYS_OS_SOLARIS    3
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
#define KLS_SYS_PLATFORM_UNKNOWN   0
#define KLS_SYS_PLATFORM_x86_64    1
#define KLS_SYS_PLATFORM_arm64     2
#define KLS_SYS_PLATFORM_i386      3
#define KLS_SYS_PLATFORM_mips      4
#define KLS_SYS_PLATFORM_ppc       5
#define KLS_SYS_PLATFORM_sparc     6
#define KLS_SYS_PLATFORM_alpha     7
#define KLS_SYS_PLATFORM_blackfin  8
#define KLS_SYS_PLATFORM_convex    9
#define KLS_SYS_PLATFORM_epiphany  10
#define KLS_SYS_PLATFORM_hppa      11
#define KLS_SYS_PLATFORM_ia64      12
#define KLS_SYS_PLATFORM_m68k      13
#define KLS_SYS_PLATFORM_pyramid   14
#define KLS_SYS_PLATFORM_rs6000    15
#define KLS_SYS_PLATFORM_superh    16
#define KLS_SYS_PLATFORM_systemz   17
#define KLS_SYS_PLATFORM_tms320    18
#define KLS_SYS_PLATFORM_tms470    19
/////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////  OS  ///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __WIN32
    #ifndef KLS_SYS_OS
        #define KLS_SYS_OS KLS_SYS_OS_WINDOWS
    #endif
#endif
#ifdef __sun
    #ifndef KLS_SYS_OS
        #define KLS_SYS_OS KLS_SYS_OS_SOLARIS
    #endif
#endif
#ifdef __unix
    #ifndef KLS_SYS_OS
        #define KLS_SYS_OS KLS_SYS_OS_UNIX
    #endif
#endif
/////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////  PLATFORM  /////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
    #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_x86_64
    #define KLS_SYS_BITNESS     64
#elif defined(__aarch64__)
    #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_arm64
    #define KLS_SYS_BITNESS     64
#elif defined(i386) || defined(__i386) || defined(__i386__) || defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(__i386) || defined(__i386) || defined(__IA32__) || defined(_M_I86) || defined(_M_IX86) || defined(_M_IX86) || defined(__X86__) || defined(_X86_) || defined(__THW_INTEL__) || defined(__I86__) || defined(__I86__) || defined(__INTEL__) || defined(__386) || defined(_M_IX86) || defined(__I86__)
    #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_i386
#elif defined(__arm__) || defined(__thumb__) || defined(__TARGET_ARCH_ARM) || defined(__TARGET_ARCH_THUMB) || defined(_ARM) || defined(_M_ARM) || defined(_M_ARMT) || defined(__arm) || defined(_M_ARM) || defined(__ARM_ARCH_2__) || defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__) || defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T) || defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5E__) || defined(__ARM_ARCH_5T__) || defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_5TEJ__) || defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__) || defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
    #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_arm
#elif defined(__mips__) || defined(mips) || defined(_R3000) || defined(_R4000) || defined(_R5900) || defined(__mips) || defined(__mips) || defined(__MIPS__) || defined(_MIPS_ISA) || defined(__mips) || defined(_MIPS_ISA_MIPS1) || defined(_MIPS_ISA_MIPS1) || defined(_R3000) || defined(_MIPS_ISA_MIPS2) || defined(__MIPS_ISA2__) || defined(_R4000) || defined(_MIPS_ISA_MIPS3) || defined(__MIPS_ISA3__) || defined(_MIPS_ISA_MIPS4) || defined(__MIPS_ISA4__) || defined(_MIPS_ISA_MIPS4) || defined(__MIPS_ISA4__)
    #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_mips
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__ppc64__) || defined(__PPC__) || defined(__PPC64__) || defined(_ARCH_PPC) || defined(_ARCH_PPC64) || defined(_M_PPC) || defined(_M_PPC) || defined(_ARCH_PPC) || defined(_ARCH_PPC64) || defined(__PPCGECKO__) || defined(__PPCBROADWAY__) || defined(_XENON) || defined(__ppc) || defined(_M_PPC) || defined(_ARCH_440) || defined(_ARCH_450) || defined(__ppc601__) || defined(_ARCH_601) || defined(__ppc603__) || defined(_ARCH_603) || defined(__ppc604__) || defined(_ARCH_604)
    #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_ppc
#elif defined(__sparc) || defined(__sparc_v8__) || defined(__sparc_v9__) || defined(__sparcv8) || defined(__sparcv9) || defined(__sparcv8) || defined(__sparc_v8__) || defined(__sparcv9) || defined(__sparc_v9__)
    #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_sparc
#endif

#ifndef KLS_SYS_OS // Vintage or obscure architectures
    #if defined(__alpha__) || defined(__alpha) || defined(_M_ALPHA) || defined(__alpha_ev4__) || defined(__alpha_ev5__) || defined(__alpha_ev6__)
        #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_alpha
    #endif
    #if defined(__bfin) || defined(__BFIN__)
        #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_blackfin
    #endif
    #if defined(__convex__) || defined(__convex_c1__) || defined(__convex_c2__) || defined(__convex_c32__) || defined(__convex_c34__) || defined(__convex_c38__)
        #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_convex
    #endif
    #if defined(__epiphany__)
        #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_epiphany
    #endif
    #if defined(__hppa__) || defined(__HPPA__) || defined(__hppa) || defined(_PA_RISC1_0) || defined(_PA_RISC1_1) || defined(__HPPA11__) || defined(__PA7100__) || defined(_PA_RISC2_0) || defined(__RISC2_0__) || defined(__HPPA20__) || defined(__PA8000__)
        #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_hppa
    #endif
    #if defined(__ia64__) || defined(_IA64) || defined(__IA64__) || defined(__ia64) || defined(_M_IA64) || defined(_M_IA64) || defined(_M_IA64) || defined(__itanium__) || defined(_M_IA64)
        #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_ia64
        #define KLS_SYS_BITNESS     64
    #endif
    #if defined(__m68k__) || defined(M68000) || defined(__MC68K__) || defined(__mc68000__) || defined(__MC68000__) || defined(__mc68010__) || defined(__mc68020__) || defined(__MC68020__) || defined(__mc68030__) || defined(__MC68030__) || defined(__mc68040__) || defined(__mc68060__) 
        #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_m68k
    #endif
    #if defined(pyr)
        #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_pyramid
    #endif
    #if defined(__THW_RS6000) || defined(_IBMR2) || defined(_POWER) || defined(_ARCH_PWR) || defined(_ARCH_PWR2) || defined(_ARCH_PWR3) || defined(_ARCH_PWR4) || defined(__sparc__)
        #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_rs6000
    #endif
    #if defined(__sh__) || defined(__sh1__) || defined(__sh2__) || defined(__sh3__) || defined(__SH3__) || defined(__SH4__) || defined(__SH5__)
        #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_superh
    #endif
    #if defined(__370__) || defined(__THW_370__) || defined(__s390__) || defined(__s390x__) || defined(__zarch__) || defined(__SYSC_ZARCH__)
        #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_systemz
    #endif
    #if defined(_TMS320C2XX) || defined(__TMS320C2000__) || defined(_TMS320C5X) || defined(__TMS320C55X__) || defined(_TMS320C6X) || defined(__TMS320C6X__) || defined(_TMS320C28X) || defined(_TMS320C5XX) || defined(__TMS320C55X__) || defined(_TMS320C6200) || defined(_TMS320C6400) || defined(_TMS320C6400_PLUS) || defined(_TMS320C6600) || defined(_TMS320C6700) || defined(_TMS320C6700_PLUS) || defined(_TMS320C6740)
        #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_tms320
    #endif
    #if defined(__TMS470__)
        #define KLS_SYS_PLATFORM    KLS_SYS_PLATFORM_tms470
    #endif
#endif // ifndef KLS_SYS_OS //Vintage or obscure architectures
/////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////  BITNESS  //////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef KLS_SYS_BITNESS
    #if defined(_LP64) || defined(__LP64) || defined(__LP64__)  || defined(_WIN64) || defined(__WIN64) || defined(__WIN64__) || defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) || defined(_ARCH_PPC64) || defined(_ARCH_PPC64)
        #define KLS_SYS_BITNESS 64
    #else
        #ifdef ULONG_MAX
            #if(ULONG_MAX==UINT_MAX)
                #define KLS_SYS_BITNESS 32
            #else
                #define KLS_SYS_BITNESS 64
            #endif
        #endif
    #endif
#endif
/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////  DEFAULT  ////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef KLS_SYS_OS
    #define KLS_SYS_OS KLS_SYS_OS_UNKNOWN
#endif
#ifndef KLS_SYS_BITNESS
    #define KLS_SYS_BITNESS 0
#endif
#ifndef KLS_SYS_PLATFORM
    #define KLS_SYS_PLATFORM KLS_SYS_PLATFORM_UNKNOWN
#endif
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////  ENDIAN  //////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __BYTE_ORDER
    #define _KLS_BYTE_ORDER         __BYTE_ORDER
#else
    #ifdef __BYTE_ORDER__
        #define _KLS_BYTE_ORDER         __BYTE_ORDER__
    #else
        #define _KLS_BYTE_ORDER         0
    #endif
#endif

#if defined(__BIG_ENDIAN) || defined(__LITTLE_ENDIAN) || defined(__PDP_ENDIAN)
    #define _KLS_BYTE_ORDER_BIG     __BIG_ENDIAN
    #define _KLS_BYTE_ORDER_LITTLE  __LITTLE_ENDIAN
    #define _KLS_BYTE_ORDER_PDP     __PDP_ENDIAN
#else
    #if defined(__BIG_ENDIAN__) || defined(__LITTLE_ENDIAN__) || defined(__PDP_ENDIAN__)
        #define _KLS_BYTE_ORDER_BIG     __BIG_ENDIAN__
        #define _KLS_BYTE_ORDER_LITTLE  __LITTLE_ENDIAN__
        #define _KLS_BYTE_ORDER_PDP     __PDP_ENDIAN__
    #else
        #if defined(__ORDER_BIG_ENDIAN) || defined(__ORDER_LITTLE_ENDIAN) || defined(__ORDER_PDP_ENDIAN)
            #define _KLS_BYTE_ORDER_BIG     __ORDER_BIG_ENDIAN
            #define _KLS_BYTE_ORDER_LITTLE  __ORDER_LITTLE_ENDIAN
            #define _KLS_BYTE_ORDER_PDP     __ORDER_PDP_ENDIAN
        #else
            #if defined(__ORDER_BIG_ENDIAN__) || defined(__ORDER_LITTLE_ENDIAN__) || defined(__ORDER_PDP_ENDIAN__)
                #define _KLS_BYTE_ORDER_BIG     __ORDER_BIG_ENDIAN__
                #define _KLS_BYTE_ORDER_LITTLE  __ORDER_LITTLE_ENDIAN__
                #define _KLS_BYTE_ORDER_PDP     __ORDER_PDP_ENDIAN__
            #else
                #define _KLS_BYTE_ORDER_BIG     4321
                #define _KLS_BYTE_ORDER_LITTLE  1234
                #define _KLS_BYTE_ORDER_PDP     3412
            #endif
        #endif
    #endif
#endif

#if(_KLS_BYTE_ORDER==_KLS_BYTE_ORDER_LITTLE)
    #define KLS_SYS_ENDIAN KLS_SYS_ENDIAN_LITTLE
#elif(_KLS_BYTE_ORDER==_KLS_BYTE_ORDER_BIG)
    #define KLS_SYS_ENDIAN KLS_SYS_ENDIAN_BIG
#elif(_KLS_BYTE_ORDER==_KLS_BYTE_ORDER_PDP)
    #define KLS_SYS_ENDIAN KLS_SYS_ENDIAN_PDP
#else
    #define KLS_SYS_ENDIAN KLS_SYS_ENDIAN_UNKNOWN
#endif

#undef _KLS_BYTE_ORDER
#undef _KLS_BYTE_ORDER_BIG
#undef _KLS_BYTE_ORDER_LITTLE
#undef _KLS_BYTE_ORDER_PDP
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////  HAND SETUP  //////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _KLS_SYS_OS_SETUP
    #undef KLS_SYS_OS
    #define KLS_SYS_OS _KLS_SYS_OS_SETUP
#endif
#ifdef _KLS_SYS_BITNESS_SETUP
    #undef KLS_SYS_BITNESS
    #define KLS_SYS_BITNESS _KLS_SYS_BITNESS_SETUP
#endif
#ifdef _KLS_SYS_PLATFORM_SETUP
    #undef KLS_SYS_PLATFORM
    #define KLS_SYS_PLATFORM _KLS_SYS_PLATFORM_SETUP
#endif
#ifdef _KLS_SYS_ENDIAN_SETUP
    #undef KLS_SYS_ENDIAN
    #define KLS_SYS_ENDIAN _KLS_SYS_ENDIAN_SETUP
#endif
/////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// _KLS_SYS_SHOW /////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _KLS_SYS_SHOW
        #warning _____________________________________
////////////////////////////////////////////////////////////////////
    #ifdef _KLS_SYS_OS_SETUP
        #warning ________OS:_________redefined by _KLS_SYS_OS_SETUP!
    #endif
    #if (KLS_SYS_OS==KLS_SYS_OS_WINDOWS)
        #warning ________OS:_________WINDOWS__________
    #elif (KLS_SYS_OS==KLS_SYS_OS_UNIX)
        #warning ________OS:_________UNIX_____________
    #elif (KLS_SYS_OS==KLS_SYS_OS_SOLARIS)
        #warning ________OS:_________SOLARIS__________
    #elif (KLS_SYS_OS==KLS_SYS_OS_UNKNOWN)
        #warning ________OS:_________unknown__________
    #endif
////////////////////////////////////////////////////////////////////
    #ifdef _KLS_SYS_BITNESS_SETUP
        #warning ________BITNESS:____redefined by _KLS_SYS_BITNESS_SETUP!
    #endif
    #if (KLS_SYS_BITNESS==128)
        #warning ________BITNESS:____x128_____________
    #elif (KLS_SYS_BITNESS==64)
        #warning ________BITNESS:____x64______________
    #elif (KLS_SYS_BITNESS==32)
        #warning ________BITNESS:____x32______________
    #elif (KLS_SYS_BITNESS==16)
        #warning ________BITNESS:____x16______________
    #elif (KLS_SYS_BITNESS==8)
        #warning ________BITNESS:____x8______________
    #else
        #warning ________BITNESS:____unknown__________
    #endif
////////////////////////////////////////////////////////////////////
    #ifdef _KLS_SYS_PLATFORM_SETUP
        #warning ________PLATFORM:___redefined by _KLS_SYS_PLATFORM_SETUP!
    #endif
    #if (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_x86_64)
        #warning ________PLATFORM:___x86_64___________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_arm64)
        #warning ________PLATFORM:___arm64____________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_i386)
        #warning ________PLATFORM:___i386_____________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_mips)
        #warning ________PLATFORM:___mips_____________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_ppc)
        #warning ________PLATFORM:___PowerPC__________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_sparc)
        #warning ________PLATFORM:___sparc____________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_alpha)
        #warning ________PLATFORM:___alpha____________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_blackfin)
        #warning ________PLATFORM:___blackfin_________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_convex)
        #warning ________PLATFORM:___convex___________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_epiphany)
        #warning ________PLATFORM:___epiphany_________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_hppa)
        #warning ________PLATFORM:___HP/PA____________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_ia64)
        #warning ________PLATFORM:___ia64_____________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_m68k)
        #warning ________PLATFORM:___M68K_____________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_pyramid)
        #warning ________PLATFORM:___pyramid__________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_rs6000)
        #warning ________PLATFORM:___RS6000___________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_superh)
        #warning ________PLATFORM:___SuperH____________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_systemz)
        #warning ________PLATFORM:___SystemZ____________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_tms320)
        #warning ________PLATFORM:___TMS320____________
    #elif (KLS_SYS_PLATFORM==KLS_SYS_PLATFORM_tms470)
        #warning ________PLATFORM:___tms470____________
    #else
        #warning ________PLATFORM:___unknown__________
    #endif
////////////////////////////////////////////////////////////////////
    #ifdef _KLS_SYS_ENDIAN_SETUP
        #warning ________ENDIAN:_____redefined by _KLS_SYS_ENDIAN_SETUP!
    #endif
    #if(KLS_SYS_ENDIAN == KLS_SYS_ENDIAN_LITTLE)
        #warning ________ENDIAN:_____LITTLE___________
    #elif(KLS_SYS_ENDIAN == KLS_SYS_ENDIAN_BIG)
        #warning ________ENDIAN:_____BIG______________
    #elif(KLS_SYS_ENDIAN == KLS_SYS_ENDIAN_PDP)
        #warning ________ENDIAN:_____PDP______________
    #else
        #warning ________ENDIAN:_____using runtime____
    #endif
////////////////////////////////////////////////////////////////////
        #warning _____________________________________
#endif // _KLS_SYS_SHOW
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


#endif //KLS_SYSDEFS_H