#include <poll.h>
#include <netdb.h>
#include <malloc.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <sys/statfs.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <execinfo.h>

#include "__KLS_X11.h"
#include "_KLS_posixTimer.h"

extern int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);

extern char *program_invocation_name;

const char *KLS_execNameGet(){
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

unsigned int KLS_sysInfoCores(){
    static unsigned int cores=0;
    KLS_ONCE(cores=get_nprocs();)
    return cores;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////  SOCKET  ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef KLS_TYPEOF(socket(0,0,0)) _NET_t_SOCKET;

_NET_t_SOCKET *_NET_sockOs(NET_t_SOCKET *s){return (void*)s->_osDep;}

void _NET_sockClose(NET_t_SOCKET *s){close(*_NET_sockOs(s));}

void NET_socketSetBlock(NET_t_SOCKET *s,KLS_byte block){
    block=!!block;
    if(s->created && s->blocked!=block){
        fcntl(*_NET_sockOs(s), F_SETFL, (!block ? O_NONBLOCK : 0) | (fcntl(*_NET_sockOs(s), F_GETFL) & ~O_NONBLOCK));
        s->blocked=block;
    }
}

unsigned int _NET_recvSize(NET_t_SOCKET *s){
    int len=0;
    ioctl(*_NET_sockOs(s),FIONREAD,&len);
    return len;
}

KLS_byte _NET_init(){return 1;}
void _NET_close(){}
