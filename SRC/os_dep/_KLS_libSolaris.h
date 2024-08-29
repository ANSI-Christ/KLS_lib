#ifndef _KLS_OS_DEP_INC
#define _KLS_OS_DEP_INC

#include <poll.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/filio.h>
#include <netinet/tcp.h>
#include <execinfo.h>

#include "__KLS_X11.h"

extern int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);

const char *KLS_execNameGet(){
    if(!_KLS_execName)
        KLS_execNameSet("\000");
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
    KLS_size ps=sysconf(_SC_PAGE_SIZE);
    if(all) *all=ps*sysconf(_SC_PHYS_PAGES);
    if(left) *left=ps*sysconf(_SC_AVPHYS_PAGES);
    return ps>0;
}

KLS_byte KLS_sysInfoHdd(const char *folder,KLS_size *left,KLS_size *all){
    struct statvfs sfs;
    if(!statvfs(folder?folder:"./", &sfs)){
        if(all) *all=(KLS_size)sfs.f_frsize*sfs.f_blocks;
        if(left) *left=(KLS_size)sfs.f_frsize*sfs.f_bavail;
        return 1;
    } return 0;
}

unsigned int KLS_sysInfoCores(){
    const int n=sysconf(_SC_NPROCESSORS_CONF);
    return n>1 ? n : 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////  SOCKET  ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef KLS_TYPEOF(socket(0,0,0)) _NET_t_SOCKET;

void NET_socketSetBlock(NET_t_SOCKET *s,KLS_byte block){
    block=!!block;
    if(s->created && s->blocked!=block){
        fcntl(*(_NET_t_SOCKET*)s->_osDep, F_SETFL, (!block ? O_NONBLOCK : 0) | (fcntl(*(_NET_t_SOCKET*)s->_osDep, F_GETFL) & ~O_NONBLOCK));
        s->blocked=block;
    }
}

unsigned int _NET_recvSize(NET_t_SOCKET *s){
    int len=0;
    ioctl(*(_NET_t_SOCKET*)s->_osDep,FIONREAD,&len);
    return len;
}

KLS_byte _NET_init(){return 1;}
void _NET_close(){}

#endif /* _KLS_OS_DEP_INC */
