#ifndef _KLS_OS_DEP_INC
#define _KLS_OS_DEP_INC

#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <fcntl.h>


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

const char *KLS_execNameGet(){
    if(!_KLS_execName)
        KLS_execNameSet("\000");
    return _KLS_execName;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////  SYS INFO  //////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


KLS_byte KLS_sysInfoRam(KLS_size *left,KLS_size *all){return 0;}
KLS_byte KLS_sysInfoHdd(const char *folder,KLS_size *left,KLS_size *all){return 0;}
unsigned int KLS_sysInfoCores(){return 1;}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////  DRAW DISPLAY  //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

KLS_COLOR _KLS_rgbDetect(KLS_byte r,KLS_byte g,KLS_byte b){_KLS_rgbGetInfo(32);return KLS_RGB(r,g,b);}
KLS_byte _GUI_event(KLS_t_DISPLAY *d,GUI_t_EVENT *ev){return 0;}
KLS_t_DISPLAY KLS_displayNew(const char *title,unsigned int width,unsigned int height){KLS_t_DISPLAY res={{0}};return res;}
void KLS_displayDraw(KLS_t_DISPLAY *d){}
void KLS_displayFree(KLS_t_DISPLAY *d){}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////  SIGNAL  ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define backtrace(...) 0

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
