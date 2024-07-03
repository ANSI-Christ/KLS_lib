#ifndef KLS_TERMINAL_H
#define KLS_TERMINAL_H

#define KLS_TRM(...) M_FOREACH(_KLS_TRM,-,__VA_ARGS__) __KLS_TRM
#define _KLS_TRM(_1_,_2_,...) M_WHEN(M_IS_ARG(__VA_ARGS__))( KLS_TRM_ ## __VA_ARGS__ )
#define __KLS_TRM(...) __VA_ARGS__ KLS_TRM_RST

#define KLS_TRM_ESC	"\033"

#define KLS_TRM_RST	KLS_TRM_ESC "[0m"
#define KLS_TRM_BOLD	KLS_TRM_ESC "[1m"
#define KLS_TRM_KURS	KLS_TRM_ESC "[4m"
#define KLS_TRM_EPIC	KLS_TRM_ESC "[5m"

#define KLS_TRM_bWHITE	KLS_TRM_ESC "[7m"
#define KLS_TRM_bRED	KLS_TRM_ESC "[41m"
#define KLS_TRM_bYELLOW	KLS_TRM_ESC "[42m"
#define KLS_TRM_bGREEN	KLS_TRM_ESC "[43m"
#define KLS_TRM_bBLUE	KLS_TRM_ESC "[44m"

#define KLS_TRM_fWHITE	KLS_TRM_ESC "[0m"
#define KLS_TRM_fRED	KLS_TRM_ESC "[31m"
#define KLS_TRM_fYELLOW	KLS_TRM_ESC "[32m"
#define KLS_TRM_fGREEN	KLS_TRM_ESC "[33m"
#define KLS_TRM_fBLUE	KLS_TRM_ESC "[34m"

#endif //KLS_TERMINAL_H
