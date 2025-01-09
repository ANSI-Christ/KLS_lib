#ifndef _KLS_OS_DEP_INC
#define _KLS_OS_DEP_INC

#define NOMINMAX
#include <winsock2.h>
#include <windows.h>
#include <imagehlp.h>
#include <ws2tcpip.h>
#include "__KLS_WinApi.h"
#undef NOMINMAX

KLS_byte KLS_fsDirCreate(const char *directory){
    return directory && !mkdir(directory);
}

const char *KLS_execNameGet(void){
    if(!_KLS_execName){
        char *l=GetCommandLineA(), *a=l;
        KLS_execNameSet(l);
        if( (l=strchr(l,' ')) ) *l=0;
        while( (a=strchr(a,'\\')) ) *a='/';
    }
    return _KLS_execName;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////  SYS INFO  //////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

KLS_byte KLS_sysInfoRam(KLS_size *left,KLS_size *all){
    MEMORYSTATUSEX mem={.dwLength = sizeof(MEMORYSTATUSEX)};
    if(GlobalMemoryStatusEx(&mem)){
        if(all) *all=mem.ullTotalPhys;
        if(left) *left=mem.ullAvailPhys;
        return 1;
    } return 0;
}

KLS_byte KLS_sysInfoHdd(const char *folder,KLS_size *left,KLS_size *all){
    ULARGE_INTEGER f,t;
    if(GetDiskFreeSpaceExA(folder, NULL, &t, &f)){
        if(all) *all=t.QuadPart;
        if(left) *left=f.QuadPart;
        return 1;
    } return 0;
}

unsigned int KLS_sysInfoCores(void){
    SYSTEM_INFO sys; GetSystemInfo(&sys);
    return sys.dwNumberOfProcessors>1 ? sys.dwNumberOfProcessors : 1;
}

#endif /* _KLS_OS_DEP_INC */
