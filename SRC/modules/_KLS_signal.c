
// LOOK AT ./os_dep

#ifndef _KLS_PTHREAD_KILL
    #define _KLS_PTHREAD_KILL pthread_kill
#endif

void *KLS_signalSetHandler(int sigNum,int mode,void(*handler)(int sigNum)){
    if(mode!=KLS_SIGNAL_MODE_DEFAULT){
        sigset_t s[1];
        sigemptyset(s);
        sigaddset(s,sigNum);
        mode=(mode==KLS_SIGNAL_MODE_BLOCK ? SIG_BLOCK : SIG_UNBLOCK);
        pthread_sigmask(mode,s,NULL);
    }
    if(handler) return signal(sigNum,(void*)handler);
    return NULL;
}

KLS_byte KLS_signalSend(pthread_t tid,int sigNum){
    return !_KLS_PTHREAD_KILL(tid,sigNum);
}

int KLS_signalGetMode(int sigNum){
    sigset_t s[1];
    sigemptyset(s);
    pthread_sigmask(0,NULL,s);
    return sigismember(s,sigNum)?KLS_SIGNAL_MODE_UNBLOCK:KLS_SIGNAL_MODE_BLOCK;
}


const char *KLS_signalGetString(int sigNum){
    #define _KLS_SIGSTR(_1_) case _1_: return #_1_
    switch(sigNum){
        #ifdef SIGHUP
        _KLS_SIGSTR(SIGHUP);
        #endif
        #ifdef SIGINT
        _KLS_SIGSTR(SIGINT);
        #endif
        #ifdef SIGQUIT
        _KLS_SIGSTR(SIGQUIT);
        #endif
        #ifdef SIGILL
        _KLS_SIGSTR(SIGILL);
        #endif
        #ifdef SIGTRAP
        _KLS_SIGSTR(SIGTRAP);
        #endif
        #ifdef SIGABRT
        _KLS_SIGSTR(SIGABRT);
        #endif
        #ifdef SIGEMT
        _KLS_SIGSTR(SIGEMT);
        #endif
        #ifdef SIGFPE
        _KLS_SIGSTR(SIGFPE);
        #endif
        #ifdef SIGKILL
        _KLS_SIGSTR(SIGKILL);
        #endif
        #ifdef SIGBUS
        _KLS_SIGSTR(SIGBUS);
        #endif
        #ifdef SIGSEGV
        _KLS_SIGSTR(SIGSEGV);
        #endif
        #ifdef SIGSYS
        _KLS_SIGSTR(SIGSYS);
        #endif
        #ifdef SIGPIPE
        _KLS_SIGSTR(SIGPIPE);
        #endif
        #ifdef SIGALRM
        _KLS_SIGSTR(SIGALRM);
        #endif
        #ifdef SIGTERM
        _KLS_SIGSTR(SIGTERM);
        #endif
        #ifdef SIGUSR1
        _KLS_SIGSTR(SIGUSR1);
        #endif
        #ifdef SIGUSR2
        _KLS_SIGSTR(SIGUSR2);
        #endif
        #ifdef SIGCHLD
        _KLS_SIGSTR(SIGCHLD);
        #endif
        #ifdef SIGPWR
        _KLS_SIGSTR(SIGPWR);
        #endif
        #ifdef SIGWINCH
        _KLS_SIGSTR(SIGWINCH);
        #endif
        #ifdef SIGURG
        _KLS_SIGSTR(SIGURG);
        #endif
        #ifdef SIGPOLL
        _KLS_SIGSTR(SIGPOLL);
        #endif
        #ifdef SIGSTOP
        _KLS_SIGSTR(SIGSTOP);
        #endif
        #ifdef SIGTSTP
        _KLS_SIGSTR(SIGTSTP);
        #endif
        #ifdef SIGCONT
        _KLS_SIGSTR(SIGCONT);
        #endif
        #ifdef SIGTTIN
        _KLS_SIGSTR(SIGTTIN);
        #endif
        #ifdef SIGTTOU
        _KLS_SIGSTR(SIGTTOU);
        #endif
        #ifdef SIGVTALRM
        _KLS_SIGSTR(SIGVTALRM);
        #endif
        #ifdef SIGPROF
        _KLS_SIGSTR(SIGPROF);
        #endif
        #ifdef SIGXCPU
        _KLS_SIGSTR(SIGXCPU);
        #endif
        #ifdef SIGXFSZ
        _KLS_SIGSTR(SIGXFSZ);
        #endif
        #ifdef SIGWAITING
        _KLS_SIGSTR(SIGWAITING);
        #endif
        #ifdef SIGLWP
        _KLS_SIGSTR(SIGLWP);
        #endif
        #ifdef SIGFREEZE
        _KLS_SIGSTR(SIGFREEZE);
        #endif
        #ifdef SIGTHAW
        _KLS_SIGSTR(SIGTHAW);
        #endif
        #ifdef SIGCANCEL
        _KLS_SIGSTR(SIGCANCEL);
        #endif
        #ifdef SIGLOST
        _KLS_SIGSTR(SIGLOST);
        #endif
        #ifdef SIGXRES
        _KLS_SIGSTR(SIGXRES);
        #endif
        #ifdef SIGJVM1
        _KLS_SIGSTR(SIGJVM1);
        #endif
        #ifdef SIGJVM2
        _KLS_SIGSTR(SIGJVM2);
        #endif
    }
    return "???";
}

