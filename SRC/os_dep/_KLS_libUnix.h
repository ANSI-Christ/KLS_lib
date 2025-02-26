#ifndef _KLS_OS_DEP_INC
#define _KLS_OS_DEP_INC

#include <poll.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <sys/statfs.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <execinfo.h>

#include "__KLS_X11.h"

extern int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);

extern char *program_invocation_name;

const char *KLS_execNameGet(void){
    if(!_KLS_execName)
        KLS_execNameSet(program_invocation_name);
    return _KLS_execName;
}

KLS_byte KLS_fsDirCreate(const char *directory){
    return directory && !mkdir(directory, 0
        #ifdef S_IRWXU
            |  S_IRWXU
        #endif
        #ifdef S_IRWXG
            |  S_IRWXG
        #endif
        #ifdef S_IRWXO
            |  S_IRWXO
        #endif
    );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////  SYS INFO  //////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

KLS_byte KLS_sysInfoRam(KLS_size *left,KLS_size *all){
    struct sysinfo mem;
    if(!sysinfo(&mem)){
        if(all) *all=mem.totalram;
        if(left) *left=mem.freeram;
        return 1;
    } return 0;
}

KLS_byte KLS_sysInfoHdd(const char *folder,KLS_size *left,KLS_size *all){
    struct statfs sfs;
    if(!statfs(folder?folder:"./", &sfs)){
        if(all) *all=(KLS_size)sfs.f_frsize*sfs.f_blocks;
        if(left) *left=(KLS_size)sfs.f_frsize*sfs.f_bavail;
        return 1;
    } return 0;
}

unsigned int KLS_sysInfoCores(void){
    const int n=get_nprocs();
    return n>1 ? n : 1;
}

#endif /* _KLS_OS_DEP_INC */
