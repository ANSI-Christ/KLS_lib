static struct{
    void(*f)(void *arg);
    void *arg;
    pthread_key_t key;
    char init;
}_KLS_tc={};


void KLS_tryCatchSetSignalInitializer(void(*f)(void *arg),void *arg){
    _KLS_tc.f=f; _KLS_tc.arg=arg;
}

void _KLS_tryCatchDeleter(_KLS_t_TRYCATCH *s){
    if(s){
        pthread_setspecific(_KLS_tc.key,NULL);
        if(s->e->data!=s->buffer) KLS_free(s->e->data);
        free(s);
    }
}

void _KLS_tryCatchInit(){
    _KLS_tc.init=!pthread_key_create(&_KLS_tc.key,(void*)_KLS_tryCatchDeleter);
}

void _KLS_tryCatchClose(){
    if(_KLS_tc.init){
        _KLS_tryCatchDeleter(pthread_getspecific(_KLS_tc.key));
        pthread_key_delete(_KLS_tc.key);
    }
}

_KLS_t_TRYCATCH *_KLS_tryCatch(){
    _KLS_t_TRYCATCH *s=NULL;
    if(_KLS_tc.init && !(s=pthread_getspecific(_KLS_tc.key)) && (s=malloc(sizeof(*s))) ){
        if(pthread_setspecific(_KLS_tc.key,s)){
            free(s); s=NULL;
        }else{
            memset(s,0,sizeof(*s));
            if(_KLS_tc.f) _KLS_tc.f(_KLS_tc.arg);
        }
    }
    return s;
}


