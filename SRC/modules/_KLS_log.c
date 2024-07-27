
typedef struct{
    pthread_t tid;
    struct{ FILE *f; int opt; }f[5];
}_KLS_t_LOG;

static struct{
    pthread_mutex_t mtx[1];
    pthread_key_t key;
    KLS_t_LIST list[1];
    _KLS_t_LOG constant[1];
    _KLS_t_LOG template[1];
}_KLS_log;



unsigned int _KLS_logFileLinks(FILE *f){
    unsigned int i,cnt=0;
    KLS_listForEach(_KLS_t_LOG)(_KLS_log.list,l)(
        for(i=0;i<KLS_ARRAY_LEN(l->f);++i)
            cnt+=(l->f[i].f==f) && (l->f[i].opt & KLS_LOG_CLOSE);
    );
    return cnt;
}

void _KLS_logRemover(_KLS_t_LOG *p){
    unsigned int i=0;
    for(;i<KLS_ARRAY_LEN(p->f);++i){
        if(!p->f[i].f || p->f[i].f == stdout || p->f[i].f == stderr || p->f[i].f == stdin)
            continue;
        if( (p->f[i].opt & KLS_LOG_CLOSE) && (_KLS_logFileLinks(p->f[i].f)==1) )
            KLS_freeFile(p->f[i].f);
    }
}

void _KLS_logDeleter(_KLS_t_LOG *p){
    pthread_setspecific(_KLS_log.key,NULL);
    pthread_mutex_lock(_KLS_log.mtx);
    KLS_listRemove(_KLS_log.list,p);
    pthread_mutex_unlock(_KLS_log.mtx);
}

char _KLS_logInit(){
    _KLS_log.constant->f->f=_KLS_log.template->f->f=stdout;
    if(pthread_mutex_init(_KLS_log.mtx,NULL)) return 0;
    if(pthread_key_create(&_KLS_log.key,(void*)_KLS_logDeleter)){
        pthread_mutex_destroy(_KLS_log.mtx);
        return 0;
    }
    *_KLS_log.list=KLS_listNew(sizeof(_KLS_t_LOG),(void*)_KLS_logRemover);
    return 1;
}

void _KLS_logClose(){
    if(!_KLS_log.list->deleter) return;
    pthread_key_delete(_KLS_log.key);
    pthread_mutex_destroy(_KLS_log.mtx);
    KLS_listClear(_KLS_log.list);
}

_KLS_t_LOG *_KLS_logFind(){
    _KLS_t_LOG *p=NULL;
    if(_KLS_log.list->deleter){
        if( !(p=pthread_getspecific(_KLS_log.key)) ){
            pthread_mutex_lock(_KLS_log.mtx);
            if( (p=KLS_listPushBack(_KLS_log.list,NULL)) ){
                if(pthread_setspecific(_KLS_log.key,p)){
                    KLS_listRemove(_KLS_log.list,p);
                    p=NULL;
                }else{
                    memcpy(p,_KLS_log.template,sizeof(*p));
                    p->tid=pthread_self();
                }
            }
            pthread_mutex_unlock(_KLS_log.mtx);
        }
    }
    return p ? p : _KLS_log.constant;
}

void _KLS_logFmt(const char *file, unsigned int line,const char *func,const char *format, ...){
#define _LOG_ADD(...) ofs+=snprintf(buf+ofs,sizeof(buf)-1-ofs,__VA_ARGS__)
    unsigned int i,ofs=0;
    char buf[128],*tmp;
    _KLS_t_LOG *l=_KLS_logFind(NULL);
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
                sprintf(tmp,"%s%s",buf,format);
                vfprintf(l->f[i].f,tmp,ap);
                free(tmp);
            }
            va_end(ap);
        }
    }
#undef _LOG_ADD
}



KLS_byte _KLS_logSet(_KLS_t_LOG *l,unsigned int index,FILE *file,int options){
    if(l!=_KLS_log.constant && index<KLS_ARRAY_LEN(l->f)){
        if( file!=l->f[index].f && (l->f[index].opt & KLS_LOG_CLOSE) && _KLS_logFileLinks(l->f[index].f)==1 )
            KLS_freeFile(l->f[index].f);
        l->f[index].f=file;
        l->f[index].opt=options;
        return 1;
    } return 0;
}

KLS_byte KLS_logSetCreation(unsigned int index,FILE *file,int options){
    return _KLS_logSet(_KLS_log.template,index,file,options);
}

KLS_byte KLS_logSet(unsigned int index,FILE *file,int options){
    return _KLS_logSet(_KLS_logFind(),index,file,options);
}

KLS_byte KLS_logGet(unsigned int index,FILE **file,int *options){
    if(index<KLS_ARRAY_LEN(_KLS_log.constant->f)){
        KLS_TYPEOF(_KLS_logFind()->f[0]) *f=_KLS_logFind()->f+index;
        if(file) *file=f->f;
        if(options) *options=f->opt;
        return 1;
    } return 0;
}
