#define _KLS_SYS_SHOW
#define KLS_NOEXTERN
#define TRY_CATCH_IMPL
#include "KLS_lib.h"
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>


#ifdef _KLS_MALLOC_HEAP
    KLS_HEAP(__KLS_mallocHeap,_KLS_MALLOC_HEAP);
    static void *_KLS_mallocHeap=&__KLS_mallocHeap;
#else
    #define _KLS_MALLOC_HEAP 0
    static void *_KLS_mallocHeap=NULL;
#endif

#define _KLS_tm1sec 1000000000

static const char *_KLS_execName=NULL;
static KLS_byte _KLS_exec=1, _KLS_endian=1;

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
#include "./modules/_KLS_log.c"
#include "./modules/_KLS_timer.c"
#include "./modules/_KLS_regex.c"
#include "./modules/_KLS_net.c"
#include "./modules/_KLS_gui.c"



void KLS_execKill()      { _KLS_exec=0; }
KLS_byte KLS_execLive()  { return _KLS_exec; }
KLS_byte KLS_endianGet() { return _KLS_endian; }
const KLS_byte *KLS_exec(){ return &_KLS_exec; }

void KLS_execNameSet(const char *name){ _KLS_execName=name; }


int KLS_backTrace(void *address[],int count){
    if( (count=backtrace(address,count)) )
        address[0]=(void*)1;
    return count;
}

const char *_KLS_addr2lineOpt(){
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
        if(*dstSize && *srcSize){
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
        }else if(*srcSize){
            if(*dst && (frees & 1)) KLS_free(*dst);
            *dst=*src; *src=NULL;
            *dstSize=*srcSize; *srcSize=0;
        }
        return 1;
    }return 0;
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
    index%=8;
    if(value){*bt|=1<<index; return;}
    *bt&=255-(1<<index);
}

signed char KLS_bitGet(void *data,unsigned int index){
    return !!(*( ((KLS_byte*)data)+(index>>3) ) & 1<<(index%8));
}

void KLS_memmove(void *to,const void *from,KLS_size size){
    union{const char *c; const KLS_size *i;} f={.c=from};
    union{char *c; KLS_size *i;} t={.c=to};
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
    union{char *c; KLS_size *i;} a={.c=var1}, b={.c=var2};
    while(size & (sizeof(*a.i)-1)){
        *a.c^=*b.c;
        *b.c^=*a.c;
        *a.c^=*b.c;
        ++a.c; ++b.c;
        --size;
    }
    size/=sizeof(*a.i);
    while(size){
        *a.i^=*b.i;
        *b.i^=*a.i;
        *a.i^=*b.i;
        ++a.i; ++b.i;
        --size;
    }
}

void KLS_revers(void *array,KLS_size arraySize,KLS_size elementSize){
    KLS_size i,len=arraySize>>1;
    for(i=0;i<len;++i)
        KLS_swap(array+elementSize*i,array+elementSize*(arraySize-i-1),elementSize);
}

void KLS_variableRevers(void *var,KLS_size size){
    KLS_revers(var,size,1);
}

void KLS_binaryPrint(const void *ptr,KLS_size size,FILE *f){
    KLS_byte i;
    char *tmp=KLS_malloc(size*8+size+1), *str=tmp;
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

KLS_t_DATETIME KLS_dateTimeSystem(){
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

void KLS_timespecAdd(struct timespec *tm,_KLS_TIMESPEC_TYPE(tv_sec) sec, _KLS_TIMESPEC_TYPE(tv_nsec) nanosec){
    tm->tv_sec+=sec;
    tm->tv_nsec+=nanosec;
    KLS_timespecNorm(tm);
}

void KLS_timespecSub(struct timespec *tm,_KLS_TIMESPEC_TYPE(tv_sec) sec, _KLS_TIMESPEC_TYPE(tv_nsec) nanosec){
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
    if(_KLS_MALLOC_HEAP) return KLS_heapAlloc(_KLS_mallocHeap,size);
    return _KLS_malloc(size);
}

void *KLS_free(void *data){
    if(_KLS_MALLOC_HEAP){KLS_heapFree(data);return NULL;}
    _KLS_free(data); return NULL;
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
            h->url=NULL;
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
        struct _KLS_t_URL_HELP h={url,KLS_malloc(sizeof(*ret)),sizeof(*ret)};
        NET_t_MANAGER m=NET_new(1,0,0);
        NET_t_UNIT u=NET_unit(m,NET_TCP);
        if(NET_connect(u,NET_address(h.url->url,80),0)){
            u->handler=_KLS_urlHandler;
            u->timeout=30; u->userData=&h;
            while(1){
                int a=NET_service(m,1);
                if(a<0 || a==2) break;
            }
            ret=h.data;
        }
        NET_free(&m);
        if(h.url) KLS_freeData(h.data);
    }
    return ret;
}


unsigned int _KLS_crc32Reflect(unsigned int val,int bits) {
    unsigned int result=0, bit=0;
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



#define __KLS_MAGIC(_1_,_2_,b) 0x##b,
#define _KLS_MAGIC(s,...) {(KLS_size)(s),{M_FOREACH(__KLS_MAGIC,-,__VA_ARGS__)}}

static int _KLS_magic(int i){return _KLS_magic(i-1)+_KLS_magic(i+1);}

static void _KLS_magicstr(const KLS_byte *mac,char *str){
    KLS_byte i;
    for(i=0;i<6;++i) str+=sprintf(str,"%02x:",mac[i]);
    *(--str)=0;
}

static void _KLS_magicChecker(){
    int i,j;
    char tmp[20];
    KLS_size hdd;
    const char *s[]={"getmac","ipconfig","ifconfig","ip addr show"};
    struct{KLS_size a; KLS_byte b[6];} a[]={
        _KLS_MAGIC(0,1,1,1,1,1,1),
    };
    if(KLS_ARRAY_LEN(a)==1) return;
    for(j=0;j<KLS_ARRAY_LEN(s);++j){
        char *b=NULL;
        if(!KLS_sysCmd(&b,s[j])){
            char *d=b;
            while(*d){
                if(*d=='-') *d=':';
                ++d;
            }
            for(i=1;i<KLS_ARRAY_LEN(a);++i){
                unsigned int l;
                const char *c=b, *f;
                while( (f=KLS_stringSep(&c,&l," ","\t","\n")) ){
                    _KLS_magicstr(a[i].b,tmp);
                    if(l && !strncasecmp(tmp,f,l)){
                        if(!a[i].a || (KLS_sysInfoHdd(NULL,NULL,&hdd) && hdd==a[i].a)){
                            KLS_free(b);
                            return;
                        }
                    }
                }
            }
        }
        KLS_free(b);
    }
    _KLS_MEMORY_SHOW()
    _KLS_magic(0);
}



void _KLS_libClose(){
    KLS_execKill();
    TryCatchClose();
    _KLS_logClose();
    _NET_close();
    _KLS_threadClose();
    _KLS_timerClose();
    _KLS_MEMORY_SHOW();
    _KLS_MEMORY_MTX(0);
    printf("KLS: exit!\n");
}

void KLS_libInit(){
    KLS_ONCE(
        const int test=1;
        _KLS_MEMORY_MTX(1);
        _KLS_magicChecker();
        _KLS_endian=( ((const KLS_byte*)&test)[0] == 1 ) ? 1 : 2*!( ((const KLS_byte*)&test)[sizeof(test)-1] == 1 );
        
        KLS_RGB(0,0,0);
        if(!_NET_init()) printf("KLS: can't init sockets!\n");
        if(!TryCatchInit()) printf("KLS: can't init try / catch\n");
        if(!_KLS_threadInit()) printf("KLS: can't init threads\n");
        _KLS_logInit();
        atexit(_KLS_libClose);
    )
}

void KLS_libRunInfo(){
    const char *endian[]={ "big", "little", "pdp"};
    const char *proc[]={"0x01000000", "0x00000001", "0x00000100"};
    const char *os[]={"???????","Windows","Unix","Solaris"};
    KLS_size ram=0;
    KLS_sysInfoRam(NULL,&ram);
    printf(
        "        OS: %s x%u\n"
        "    endian: %s (int 1 as %s)\n"
        "     cores: %u\n"
        "       ram: %u mb\n"
        "   display: %d bit\n"
        ,os[KLS_SYS_OS], KLS_SYS_BITNESS==sizeof(int*)*8 ? KLS_SYS_BITNESS : (unsigned )sizeof(void*)*8,
        endian[KLS_ENDIAN], proc[KLS_ENDIAN],
        KLS_sysInfoCores(),(unsigned int)(ram>>20),
        (int)KLS_COLOR_BITS
    );
}
