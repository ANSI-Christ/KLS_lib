#if(KLS_SYS_OS == KLS_SYS_OS_WINDOWS)
    #include "./os_dep/_KLS_libWin.h"
#elif(KLS_SYS_OS == KLS_SYS_OS_UNIX)
    #include "./os_dep/_KLS_libUnix.h"
#elif(KLS_SYS_OS == KLS_SYS_OS_SOLARIS)
    #include "./os_dep/_KLS_libSolaris.h"
#elif(KLS_SYS_OS == KLS_SYS_OS_OTHER)
    #include "./os_dep/_KLS_libOther.h"
#else
    #include KLS_SYS_OS_HEADER
#endif
