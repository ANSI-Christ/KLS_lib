
typedef struct{
    FILE *f;
    int opt;
}_KLS_t_LOG_FILE;

typedef struct{
    _KLS_t_LOG_FILE f[5];
}_KLS_t_LOG;

static struct{
    pthread_mutex_t mtx[1];
    pthread_key_t key;
    KLS_t_LIST list[1];
    _KLS_t_LOG constant[1];
    _KLS_t_LOG template[1];
}_KLS_log;


_KLS_t_LOG_FILE *_KLS_logFileInside(FILE *f,_KLS_t_LOG *c){
    unsigned int i;
    for(i=0;i<KLS_ARRAY_LEN(c->f);++i)
        if(f==c->f[i].f) return c->f+i;
    return NULL;
}

void _KLS_logFileFree(_KLS_t_LOG_FILE *f){
    if(f->f && f->f!=stdout && f->f!=stderr && f->f!=stdin){
        const int opt=f->opt & KLS_LOG_CLOSE;
        _KLS_t_LOG_FILE *i;
        unsigned int cnt=0, cls=0;
        if( (i=_KLS_logFileInside(f->f,_KLS_log.template)) ){
            ++cnt; i->opt|=opt;
            cls+=!!(i->opt & KLS_LOG_CLOSE);
        }
        KLS_listForEach(_KLS_t_LOG)(_KLS_log.list,l)(
            if( (i=_KLS_logFileInside(f->f,l)) ){
                ++cnt; i->opt|=opt;
                cls+=!!(i->opt & KLS_LOG_CLOSE);
            }
        );
        if(cnt==1 && cls){
            fclose(f->f);
            f->f=NULL;
        }
    }
}

void _KLS_logRemover(_KLS_t_LOG *p){
    unsigned int i=0;
    for(;i<KLS_ARRAY_LEN(p->f);++i)
        _KLS_logFileFree(p->f+i);
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
    _KLS_log.list->deleter=NULL;
    _KLS_logRemover(_KLS_log.template);
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
                }else memcpy(p,_KLS_log.template,sizeof(*p));
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
    pthread_t tid=pthread_self();
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
                _LOG_ADD("<%x>",*(unsigned int*)&tid);
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
    if(l!=_KLS_log.constant && index<KLS_ARRAY_LEN(l->f) && !_KLS_logFileInside(file,l)){\
        _KLS_t_LOG_FILE * const f=l->f+index;
        _KLS_logFileFree(f);
        f->f=file;
        f->opt=options;
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
        _KLS_t_LOG_FILE * const f=_KLS_logFind()->f+index;
        if(file) *file=f->f;
        if(options) *options=f->opt;
        return 1;
    } return 0;
}
