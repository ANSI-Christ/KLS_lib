
#ifdef _KLS_OS_DEP_HEADER
    #include _KLS_OS_DEP_HEADER
#endif

#ifdef __WIN32
    #include "./os_dep/_KLS_libWin.h"
#endif

#ifdef __sun
    #include "./os_dep/_KLS_libSolaris.h"
#endif

#if defined(__APPLE__) || defined(__MACH__)
    #include "./os_dep/_KLS_libMacOs.h"
#endif

#ifdef __unix__
    #include "./os_dep/_KLS_libUnix.h"
#endif

#if 1 /* default */
    #include "./os_dep/_KLS_libOther.h"
#endif
