
typedef struct{
    const KLS_byte constant;
    pthread_t tid;
    struct{ FILE *f; int opt; }f[5];
}_KLS_t_LOG;

static KLS_byte _KLS_logStatus=0;
static pthread_key_t _KLS_logKey;
static _KLS_t_LOG _KLS_logVar[2]={{1},{0}};
static KLS_t_LIST _KLS_logList={0};
static pthread_mutex_t _KLS_logMtx=PTHREAD_MUTEX_INITIALIZER;

unsigned int _KLS_logFileLinks(FILE *f){
    unsigned int i,cnt=0;
    KLS_listForEach(_KLS_t_LOG)(&_KLS_logList,l)(
        for(i=0;i<KLS_ARRAY_LEN(l->f);++i)
            cnt+=(l->f[i].f==f) && (l->f[i].opt & KLS_LOG_CLOSE);
    );
    return cnt;
}

void _KLS_logDeleter(_KLS_t_LOG *p){
    int i;
    void *id=NULL;
    if(!p || p->constant) return;
    pthread_mutex_lock(&_KLS_logMtx);
    for(i=0;i<KLS_ARRAY_LEN(p->f);++i){
        if(!p->f[i].f || p->f[i].f == stdout || p->f[i].f == stderr || p->f[i].f == stdin)
            continue;
        if( (p->f[i].opt & KLS_LOG_CLOSE) && (_KLS_logFileLinks(p->f[i].f)==1) )
            KLS_freeFile(p->f[i].f);
    }
    KLS_listIndexOf(&_KLS_logList,&p,NULL,&id);
    KLS_listRemove(&_KLS_logList,id);
    pthread_mutex_unlock(&_KLS_logMtx);
    KLS_freeData(p);
}

void _KLS_logClose(){
    void **last=KLS_listAt(&_KLS_logList,0);
    if(last) _KLS_logDeleter(*last);
    KLS_listClear(&_KLS_logList);
    if(_KLS_logStatus) pthread_key_delete(_KLS_logKey);
}

void _KLS_logInit(){
    _KLS_logVar[0].f[0].f=stdout;
    _KLS_logVar[1].f[0].f=stdout;
    _KLS_logList=KLS_listNew(sizeof(_KLS_t_LOG*),NULL);
    _KLS_logStatus=!pthread_key_create(&_KLS_logKey,(void*)_KLS_logDeleter);
}

_KLS_t_LOG *_KLS_logFind(){
    _KLS_t_LOG *p=NULL;
    if(!_KLS_logStatus) return _KLS_logVar;
    if(!(p=pthread_getspecific(_KLS_logKey)) ){
        if( (p=KLS_malloc(sizeof(*p))) ){
            memcpy(p,_KLS_logVar+1,sizeof(*p));
            p->tid=pthread_self();
            pthread_setspecific(_KLS_logKey,p);
            pthread_mutex_lock(&_KLS_logMtx);
            KLS_listPushBack(&_KLS_logList,&p);
            pthread_mutex_unlock(&_KLS_logMtx);
        }else p=_KLS_logVar;
    }
    return p;
}

KLS_byte _KLS_logFinderTid(_KLS_t_LOG **l,pthread_t *t){
    return pthread_equal(*t,(*l)->tid)!=0;
}

_KLS_t_LOG *_KLS_logFindTid(void *tid){
    _KLS_t_LOG **l=NULL;
    KLS_listIndexOf(&_KLS_logList,tid,(void*)_KLS_logFinderTid,(void*)&l);
    return *l;
}

void _KLS_log(const char *file, unsigned int line,const char *func,const char *format, ...){
#define _LOG_ADD(...) ofs+=snprintf(buf+ofs,sizeof(buf)-1-ofs,__VA_ARGS__)
    unsigned int i,ofs=0;
    char buf[128],*tmp;
    _KLS_t_LOG *l=_KLS_logFind();
    KLS_t_DATETIME dt;
    for(i=0;i<KLS_ARRAY_LEN(l->f);++i){
        if(l->f[i].f){
            va_list ap;
            va_start(ap,format);

            if( !(l->f[i].opt & (KLS_LOG_FILE|KLS_LOG_LINE|KLS_LOG_FUNC|KLS_LOG_DATE|KLS_LOG_TIME|KLS_LOG_TID)) ){
                vfprintf(l->f[i].f,format,ap);
                va_end(ap);
                continue;
            }

            ofs=*buf=0;

            if( l->f[i].opt & (KLS_LOG_FILE|KLS_LOG_LINE|KLS_LOG_FUNC|KLS_LOG_DATE|KLS_LOG_TIME|KLS_LOG_TID) )
                _LOG_ADD("\n");

            ////////////////// FILE and LINE //////////////
            if(l->f[i].opt & (KLS_LOG_FILE|KLS_LOG_LINE))
                _LOG_ADD("[");
            if(l->f[i].opt & KLS_LOG_FILE){
                _LOG_ADD(file);
                if(l->f[i].opt & KLS_LOG_LINE)
                    _LOG_ADD(":");
            }
            if(l->f[i].opt & KLS_LOG_LINE)
                _LOG_ADD("%u",line);
            if(l->f[i].opt & (KLS_LOG_FILE|KLS_LOG_LINE))
                _LOG_ADD("]");

            //////////////////// FUNC //////////////////////
            if(l->f[i].opt & KLS_LOG_FUNC)
                _LOG_ADD("{%s()}",func);

            ///////////////////// TID //////////////////////
            if(l->f[i].opt & KLS_LOG_TID){
                void *tidPtr=&l->tid;
                _LOG_ADD("<%x>",*(unsigned int*)tidPtr);
            }

            ////////////////// DATE TIME ///////////////////
            if(l->f[i].opt & (KLS_LOG_DATE|KLS_LOG_TIME)){
                dt=KLS_dateTimeSystem();
                _LOG_ADD("(");
            }
            if(l->f[i].opt & KLS_LOG_DATE){
                _LOG_ADD("%02d.%02d.%02d",dt.day,dt.month,dt.year);
                if(l->f[i].opt & KLS_LOG_TIME)
                _LOG_ADD("$");
            }
            if(l->f[i].opt & KLS_LOG_TIME)
                _LOG_ADD("%02d:%02d:%02d",dt.hour,dt.minute,dt.second);
            if(l->f[i].opt & (KLS_LOG_DATE|KLS_LOG_TIME))
                _LOG_ADD(")");

            ///////////////////// TEXT //////////////////////
            _LOG_ADD("\n\t");
            if( (tmp=malloc(strlen(buf)+strlen(format)+1)) ){
                *tmp=0; strcat(tmp,buf); strcat(tmp,format);
                vfprintf(l->f[i].f,tmp,ap);
                free(tmp);
            }
            va_end(ap);
        }
    }
#undef _LOG_ADD
}

KLS_byte _KLS_logSet(_KLS_t_LOG *l,unsigned int index, FILE *file,int options){
    if(!l || index>=KLS_ARRAY_LEN(l->f) || l->constant) return 0;
    if( file!=l->f[index].f && (l->f[index].opt & KLS_LOG_CLOSE) && _KLS_logFileLinks(l->f[index].f)==1 )
        KLS_freeFile(l->f[index].f);
    l->f[index].f=file;
    l->f[index].opt=options;
    return 1;
}

KLS_byte KLS_logSetCreation(unsigned int index, FILE *file,int options){
    return _KLS_logSet(_KLS_logVar+1,index,file,options);
}

KLS_byte KLS_logSet(unsigned int index, FILE *file,int options){
    return _KLS_logSet(_KLS_logFind(),index,file,options);
}

KLS_byte KLS_logSetTid(pthread_t tid,unsigned int index, FILE *file,int options){
    KLS_byte res;
    pthread_mutex_lock(&_KLS_logMtx);
    res=_KLS_logSet(_KLS_logFindTid(&tid),index,file,options);
    pthread_mutex_unlock(&_KLS_logMtx);
    return res;
}

int KLS_logGetOptions(unsigned int index){
    _KLS_t_LOG *l;
    if(index>=KLS_ARRAY_LEN(_KLS_logVar->f) || !(l=_KLS_logFind()))
        return 0;
    return l->f[index].opt;
}

FILE *KLS_logGetFile(unsigned int index){
    _KLS_t_LOG *l;
    if(index>=KLS_ARRAY_LEN(_KLS_logVar->f) || !(l=_KLS_logFind()))
        return NULL;
    return l->f[index].f;
}
