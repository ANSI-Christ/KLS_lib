#define _KLS_GLOBVAR
#define TRY_CATCH_IMPL
#include "KLS_lib.h"
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>


#define _KLS_tm1sec 1000000000

static const char *_KLS_execName=NULL;
static KLS_byte _KLS_exec=1;
KLS_byte KLS_COLOR_BITS=32;

extern int close(int fd);
extern int nanosleep(const struct timespec *req, struct timespec *rem);
extern float roundf(float x);
extern double round(double x);
extern double log2(double x);
extern void _KLS_rgbGetInfo(int bits);


#include "./os_dep/KLS_osDep.h"

#include "./modules/_KLS_bitmap.c"
#include "./modules/_KLS_string.c"
#include "./modules/_KLS_array.c"
#include "./modules/_KLS_thread.c"
#include "./modules/_KLS_geometry.c"
#include "./modules/_KLS_fs.c"
#include "./modules/_KLS_heap.c"
#include "./modules/_KLS_vector.c"
#include "./modules/_KLS_list.c"
#include "./modules/_KLS_queue.c"
#include "./modules/_KLS_matrix.c"
#include "./modules/_KLS_canvas.c"
#include "./modules/_KLS_signal.c"
#include "./modules/_KLS_timer.c"
#include "./modules/_KLS_regex.c"
#include "./modules/_KLS_net.c"
#include "./modules/_KLS_gui.c"



void KLS_execKill(void)      { _KLS_exec=0; }
KLS_byte KLS_execLive(void)  { return _KLS_exec; }
const KLS_byte *KLS_exec(void){ return &_KLS_exec; }

void KLS_execNameSet(const char *name){ _KLS_execName=name; }


int KLS_backTrace(void *address[],int count){
    if( (count=backtrace(address,count)) )
        address[0]=(void*)1;
    return count;
}

const char *_KLS_addr2lineOpt(void){
    static const char *s=(void*)0x1;
    if(s==(void*)0x1){
        if(system("addr2line -h"))
            s=NULL;
        else{
            const char *opt[]={" ","-p","-a","-a -p"};
            s=opt[((!system("addr2line -a -h"))<<1) | (!system("addr2line -p -h"))];
        }
    }
    return s;
}

void KLS_addr2line(void * const *address,unsigned int count){
    if(KLS_execNameGet()){
        const char *opt=_KLS_addr2lineOpt();
        if(opt){
            char str[2048];
            unsigned int l=snprintf(str,sizeof(str)-1,"addr2line %s -f -e \"%s\"",opt,KLS_execNameGet());
            while(count--) l+=snprintf(str+l,sizeof(str)-l-1," %p",address[count]);
            system(str);
            return;
        }
    }
    while(count--) printf("%p: ?? at ??:0\n",address[count]);
}

const char *KLS_getOpt(int argc, char *argv[],const char *opt){
    if(opt && argc>1 && argv){
        int i,len=strlen(opt);
        const char *s=NULL;
        for(i=1;i<argc;++i){
            if(!argv[i]) continue;
            s=argv[i];
            if(!strncmp(s,"--",2))
                s+=2;
            else if(!strncmp(s,"-",1))
                s+=1;
            if(!s) continue;
            if(!strncmp(s,opt,len))
                s+=len;
            if(!s) continue;
            switch(*s){
                case '\0': /*s=((i+1)<argc?argv[i+1]:NULL);*/ return s;
                case ':':
                case '@':
                case '%':
                case '=':
                    if(*(++s)=='\0')
                        s=((i+1)<argc?argv[i+1]:NULL);
                    return s;
            }
        }
    }
    return NULL;
}

KLS_byte KLS_dataJoin(void **dst,KLS_size *dstSize,void **src,KLS_size *srcSize,KLS_byte frees){
    if(dst && src && dstSize && srcSize){
        if(*srcSize){
            if(*dstSize){
                void *tmp;
                if(!*dst || !*src || !(tmp=KLS_malloc(*dstSize + *srcSize)))
                    return 0;
                memcpy(tmp,*dst,*dstSize);
                memcpy(tmp+*dstSize,*src,*srcSize);
                if(frees & 1) KLS_free(*dst);
                if(frees & 2) KLS_freeData(*src);
                *dst=tmp;
                *dstSize+=*srcSize;
                if(frees & 2) *srcSize=0;
                return 1;
            }
            if(*dst && (frees & 1)) KLS_free(*dst);
            *dst=*src; *src=NULL;
            *dstSize=*srcSize; *srcSize=0;
            return 1;
        }
    } return 0;
}

double KLS_mod(double value,double division){
    return value-division*(KLS_long)(value/division);
}

double KLS_round(double value,double step){
    double m=fabs(KLS_mod(value,step));
    if(value>0) return value-(m>step/2 ? m-step : m);
    return value+(m>step/2 ? m-step : m);
}

void *KLS_dublicate(const void *data,KLS_size len){
    void *ptr=NULL;
    if( data && len && (ptr=KLS_malloc(len)) )
        memcpy(ptr,data,len);
    return ptr;
}

void KLS_pause(int sec,int mlsec,int mksec,int nnsec){
    struct timespec timer={sec,(KLS_size)mlsec*1000000 + (KLS_size)mksec*1000 + (KLS_size)nnsec};
    while(nanosleep(&timer,&timer) && KLS_execLive());
}

void KLS_pausef(double sec){
    struct timespec timer; timer.tv_sec=(int)sec;
    timer.tv_nsec=(sec-(double)timer.tv_sec)*_KLS_tm1sec;
    while(nanosleep(&timer,&timer) && KLS_execLive());
}

void KLS_bitSet(void *data,unsigned int index,KLS_byte value){
    KLS_byte *bt=((KLS_byte*)data)+(index>>3);
    index&=7; /* index % 8*/
    if(value){*bt|=1<<index; return;}
    *bt&=255-(1<<index);
}

unsigned char KLS_bitGet(void *data,unsigned int index){
    return !!(*( ((KLS_byte*)data)+(index>>3) ) & 1<<(index&7));
}

void KLS_memmove(void *to,const void *from,KLS_size size){
    union{const void *_;const unsigned char *c; const KLS_size *i;} f={from};
    union{void *_; unsigned char *c; KLS_size *i;} t={to};
    if(to<from){
        while(size & (sizeof(*f.i)-1)){ --size; *t.c=*f.c; ++t.c; ++f.c; }
        size/=sizeof(*f.i);
        while(size){ --size; *t.i=*f.i; ++t.i; ++f.i; };
        return;
    }
    t.c+=size; f.c+=size;
    while(size & (sizeof(*f.i)-1)){ --size; *--t.c=*--f.c; }
    size/=sizeof(*f.i);
    while(size){ --size; *--t.i=*--f.i; }
}

void KLS_swap(void *var1,void *var2,KLS_size size){
    union{void *_; unsigned char *c; KLS_size *i;} a={var1}, b={var2};
    unsigned char bytes=size & (sizeof(*a.i)-1);
    size/=sizeof(*a.i);
    while(size){
        *a.i^=*b.i;
        *b.i^=*a.i;
        *a.i^=*b.i;
        ++a.i; ++b.i; --size;
    }
    while(bytes){
        *a.c^=*b.c;
        *b.c^=*a.c;
        *a.c^=*b.c;
        ++a.c; ++b.c; --bytes;
    }
}

void KLS_revers(void *array,KLS_size arraySize,KLS_size elementSize){
    const KLS_size len=arraySize>>1;
    KLS_size i=0;
    for(;i<len;++i) KLS_swap(array+elementSize*i,array+elementSize*(arraySize-i-1),elementSize);
}

void KLS_variableRevers(void *var,KLS_size size){
    KLS_revers(var,size,1);
}

int KLS_utos(size_t n,char *s){
    int cnt=-1,f=0,b;
    char c;
    do{
        s[++cnt]=n%10+'0'; n/=10;
    }while(n>0);
    for(b=cnt;f<b;++f,--b){
        c=s[f];
        s[f]=s[b];
        s[b]=c;
    }
    return cnt+1;
}

int KLS_itos(ptrdiff_t n,char *s){
    if(n<0){
        *s='-'; return KLS_utos(-n,s+1)+1;
    } return KLS_utos(n,s);
}

void KLS_binaryPrint(const void *ptr,KLS_size size,FILE *f){
    KLS_byte i;
    unsigned char *tmp=KLS_malloc(size*8+size+1), *str=tmp;
    if(!tmp) return;
    if(!f) f=stdout;
    *tmp=0;
    while(size--){
        for(i=0;i<8;++i,++tmp)
            *(tmp)=(( (*(KLS_byte*)(ptr+size)) <<i) & KLS_0b10000000)?'1':'0';
        *(tmp++)=' ';
    }
    *tmp=0;
    fprintf(f,"%s\n",str);
    KLS_freeData(str);
}

int KLS_sysCmd(char **output,const char *cmdFormat,...){
    char *str=KLS_stringv(cmdFormat);
    if(output) KLS_freeData(*output);
    if(str){
        if(output){
            FILE *f=popen(str,"r");
            KLS_free(str);
            if(f){
                char buf[1024],chk=0;
                while(fgets(buf,sizeof(buf)-1,f)){
                    chk=1;
                    str=KLS_string(NULL,"%s%s",*output,buf);
                    KLS_freeData(*output);
                    if(!(*output=str))
                        break;
                }
                pclose(f);
                if(*output) return 0;
                if(chk) return -3;
                *output=KLS_string(NULL,"%s","\0");
                return !*output;
            }
            return -2;
        }else{
            int len=system(str);
            KLS_free(str);
            return len;
        }
    }
    return -1;
}

KLS_t_DATETIME KLS_dateTimeFrom(time_t time){
    KLS_t_DATETIME dt={0};
    struct tm *timeTm=localtime(&time);
    if(timeTm){
       dt.day=timeTm->tm_mday;         dt.hour=timeTm->tm_hour;
       dt.month=timeTm->tm_mon+1;      dt.minute=timeTm->tm_min;
       dt.year=timeTm->tm_year+1900;   dt.second=timeTm->tm_sec;
    } return dt;
}

KLS_t_DATETIME KLS_dateTimeSystem(void){
    time_t timeT=time(NULL);
    return KLS_dateTimeFrom(timeT);
}

void KLS_dateTimePrint(const KLS_t_DATETIME *dt,FILE *f){
    if(!f) f=stdout;
    fprintf(f,"%0*d.%0*d.%0*d / %0*d:%0*d:%0*d",  2,dt->day,2,dt->month,2,dt->year,  2,dt->hour,2,dt->minute,2,dt->second);
}

int KLS_timeToSec(int hour,int min,int sec){
    return sec + min*60 + hour*60*60;
}

void KLS_timeFromSec(int timeSec,int *hour,int *min,int *sec){
    if(hour)(*hour)=(int)floor(timeSec/3600.);
    if(min)(*min)=(int)floor((timeSec-((int)floor(timeSec/3600.))*3600)/60.);
    if(sec)(*sec)=(int)floor(timeSec-((int)floor(timeSec/3600.))*3600)%60;
}

void KLS_timespecNorm(struct timespec *tm){
    tm->tv_sec+=(tm->tv_nsec/_KLS_tm1sec);
    tm->tv_nsec%=_KLS_tm1sec;
}

void KLS_timespecAdd(struct timespec *tm,int sec,int nanosec){
    tm->tv_sec+=sec;
    tm->tv_nsec+=nanosec;
    KLS_timespecNorm(tm);
}

void KLS_timespecSub(struct timespec *tm,int sec,int nanosec){
    sec+=nanosec/_KLS_tm1sec;
    nanosec%=_KLS_tm1sec;
    KLS_timespecNorm(tm);
    if(tm->tv_sec >= sec){
        tm->tv_sec-=sec;
        if(tm->tv_nsec>nanosec){
            tm->tv_nsec-=nanosec;
        }else if(tm->tv_sec--){
            tm->tv_nsec+=_KLS_tm1sec-nanosec;
        }else tm->tv_sec=tm->tv_nsec=0;
    }else tm->tv_sec=tm->tv_nsec=0;
}



void *_KLS_malloc(KLS_size size){
    if(size){
        void *ptr=malloc(size);
        _KLS_MEMORY_PUSH(ptr)
        return ptr;
    } return NULL;
}

void _KLS_free(void *data){
    if(data) free(data);
    _KLS_MEMORY_POP(data)
}

void *KLS_malloc(KLS_size size){
    #ifdef _KLS_MALLOC_HEAP
    static pthread_mutex_t mtx[1]={PTHREAD_MUTEX_INITIALIZER};
    static KLS_HEAP(heap,_KLS_MALLOC_HEAP,mtx);
    return KLS_heapAlloc(heap,size);
    #else
    return _KLS_malloc(size);
    #endif
}

void *KLS_free(void *data){
    #ifdef _KLS_MALLOC_HEAP
    if(_KLS_MALLOC_HEAP){KLS_heapFree(data);return NULL;}
    #else
    _KLS_free(data); return NULL;
    #endif
}

void KLS_ptrDeleter(void *data){
    KLS_free(*(void**)data);
}


struct _KLS_t_URL_HELP{
    const KLS_t_URL *url;
    void *data;
    KLS_size dataSize;
};

void _KLS_urlHandler(NET_t_UNIT u, KLS_byte event){
    struct _KLS_t_URL_HELP *h=u->userData;
    switch(event){
        case NET_EVENT_CONNECT:{
            char req[1024],host[40];
            int reqSize=snprintf(req,sizeof(req),
                "GET %s %s\r\n"
                "Host: %s\r\n"  "%s\r\n",
                h->url->url, h->url->protocol?h->url->protocol:"HTTP/1.0",
                NET_addressToString(&u->address,host), h->url->header?h->url->header:"Connection: close\r\n"
            );
            u->timeout=5;
            u->pulse=10;
            NET_write(u,req,reqSize,NULL);
            break;
        }
        case NET_EVENT_RECEIVE:{
            void *d=NULL;
            KLS_size s=NET_read(u,&d,0,NULL);
            if(!KLS_dataJoin(&h->data,&h->dataSize,&d,&s,3)){
                KLS_freeData(d);
                KLS_freeData(h->data);
                NET_disconnect(u);
            }
            break;
        }
        case NET_EVENT_DISCONNECT:{
            char *ptr;
            h->url=NULL;
            NET_interrupt(u->manager);
            if( h->data && (ptr=strstr(h->data+sizeof(KLS_t_URL_DATA),"\r\n\r\n")) ){
                KLS_t_URL_DATA *ans=h->data;
                ptr[2]=ptr[3]=0;
                ans->header=(void*)(ans+1);
                ans->data=ptr+4;
                ans->size=h->dataSize-sizeof(KLS_t_URL_DATA)-(KLS_size)(ans->data-ans->header);
            }
            break;
        }
        case NET_EVENT_TIMEOUT:{
            NET_disconnect(u);
            break;
        }
    }
}

KLS_t_URL_DATA *KLS_urlRequest(const KLS_t_URL *url){
    KLS_t_URL_DATA *ret=NULL;
    if(url && url->url){
        struct _KLS_t_URL_HELP h={url};
        NET_t_MANAGER m=NET_new(1,0,0);
        NET_t_UNIT u=NET_unit(m,NET_TCP);
        if(NET_connect(u,NET_address(h.url->url,80),0)){
            u->handler=_KLS_urlHandler;
            u->timeout=30; u->userData=&h;
            while(NET_service(m)==EINTR && h.url);
            ret=h.data;
        }
        NET_free(&m);
    }
    return ret;
}


int _KLS_crc32Reflect(unsigned int val,int bits) {
    int result=0, bit=0;
    while(bit<bits){
        ++bit;
        if(val&1) result|=1<<(bits-bit);
        val>>=1;
    }
    return result;
}

void _KLS_crc32Init(unsigned int *tab){
    unsigned int crc;
    unsigned short byte=0;
    KLS_byte offset;
    for(;byte<256;++byte) {
        crc=_KLS_crc32Reflect(byte, 8)<<24;
        for(offset=0;offset<8;++offset) {
            if(crc&(1<<((sizeof(crc)*8)-1))){
                crc=(crc<<1)^0x04c11db7;
            }else crc<<=1;
        }
        tab[byte]=_KLS_crc32Reflect(crc,32);
    }
}

unsigned int KLS_crc32(unsigned int crc,const void *data,KLS_size size){
    unsigned int tab[256]={0};
    _KLS_crc32Init(tab);
    while(size){
        crc=(crc>>8)^tab[(crc&255)^*(KLS_byte*)data];
        ++data;--size;
    }
    return ~crc;
}


void _KLS_libClose(void){
    KLS_execKill();
    _NET_close();
    _KLS_threadClose();
    _KLS_timerClose();
    TryCatchClose();
    _KLS_MEMORY_SHOW()
    _KLS_MEMORY_MTX(0)
    puts("KLS: exit!\n");
}

void KLS_libInit(void){
    KLS_ONCE(
        _KLS_MEMORY_MTX(1)
        KLS_RGB(0,0,0);
        if(!_NET_init()) puts("KLS: can't init sockets!\n");
        if(!TryCatchInit()) puts("KLS: can't init try / catch\n");
        if(!_KLS_threadInit()) puts("KLS: can't init threads\n");
        atexit(_KLS_libClose);
    )
}

