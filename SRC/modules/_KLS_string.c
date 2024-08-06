
char *_KLS_stringv(const char *format, va_list ap[2]){
    char *str=NULL;
    if(format){
        long len=vsnprintf(NULL,0,format,ap[0])+1; va_end(ap[0]);
        if(len>0 && (str=KLS_malloc(len)) ) vsnprintf(str,len,format,ap[1]);
    }
    va_end(ap[0]); va_end(ap[1]);
    return str;
}

char *KLS_string(char **variable,const char *format,...){
    if(variable){
        char *s=KLS_stringv(format);
        KLS_free(*variable);
        return *variable=s;
    } return KLS_stringv(format);
}

unsigned int KLS_stringWordCount(const char *string,const char *word){
    unsigned int cnt=0;
    if(string && word){
        const char *ptr=string;
        unsigned int len=strlen(word);
        while((ptr=strstr(ptr,word))){
            ptr+=len;
            ++cnt;
        }
    }
    return cnt;
}

char *KLS_stringReplace(const char *string,const char *from, const char *to){
    char *str=NULL;
    if(string && from){
        const KLS_size lenFrom=strlen(from), lenTo=to?strlen(to):0;
        KLS_size len=strlen(string)+1+KLS_stringWordCount(string,from)*(lenTo-lenFrom);
        if( (str=KLS_malloc(len)) ){
            const char *end;
            len=0;
            while( (end=strstr(string,from))){
                const unsigned int n=end-string;
                strncat(str+len,string,n); len+=n;
                strcat(str+len,to); len+=lenTo;
                string=end+lenFrom;
            }
            strcat(str+len,string);
        }
    }
    return str;
}

char *KLS_stringReadFile(const char *filePath){
    FILE *f=NULL;
    struct stat info;
    char *str=NULL;
    if(!filePath || stat(filePath, &info) || !(f=fopen(filePath,"rb")) || !(str=KLS_malloc(info.st_size+1))){
        KLS_freeFile(f);
        return NULL;
    }
    fread(str,info.st_size,1,f);
    KLS_freeFile(f);
    str[info.st_size]=0;
    return str;
}

const char *_KLS_stringSep(const char **string,unsigned int *len,const char * const * sep){
    const char *x=**string?*string:NULL, *p;
    unsigned int lenSep=0, lenStr;
    *len=strlen(*string);
    for(;*sep;++sep)
        if((p=strstr(*string,*sep)) && (lenStr=p-*string)<*len){
            *len=lenStr;
            lenSep=strlen(*sep);
        }
    *string+=*len+lenSep;
    return x;
}

typedef union{
    const char *c;
    char *v;
}_KLS_t_SOLVE_PTR;

typedef struct{
    _KLS_t_SOLVE_PTR op, left, right, beg, end;
}_KLS_t_SOLVE_MATCH;

typedef struct{
    char *str, *value;
    _KLS_t_SOLVE_PTR brecket;
    _KLS_t_SOLVE_MATCH op;
    char precision[8];
}_KLS_t_SOLVE;

typedef struct{
    const char *op;
    const unsigned char len, left, right;
}_KLS_t_SOLVE_OP;



#define _ADD_OPS(...) KLS_$(_KLS_t_SOLVE_OP)(__VA_ARGS__,_ADD_OP(0,NULL,0))
#define _ADD_OP(left,op,right) { op,op ? sizeof(op)-1 : 0, left, right }
static const _KLS_t_SOLVE_OP *_KLS_solveOps[]={
    _ADD_OPS( _ADD_OP(0,"!",1),     _ADD_OP(0,"~",1) ),
    _ADD_OPS( _ADD_OP(1,"!",0) ),
    _ADD_OPS( _ADD_OP(0,"asin",1),  _ADD_OP(0,"acos",1), _ADD_OP(0,"atan",1),
              _ADD_OP(0,"sin",1),   _ADD_OP(0,"cos",1),  _ADD_OP(0,"tan",1),
              _ADD_OP(0,"log10",1), _ADD_OP(0,"log2",1), _ADD_OP(0,"log",1)
            ),
    _ADD_OPS( _ADD_OP(1,"@",1) ), /*pow(l,r)*/
    _ADD_OPS( _ADD_OP(1,"*",1),  _ADD_OP(1,"/",1),  _ADD_OP(1,"%",1) ),
    _ADD_OPS( _ADD_OP(1,"+",1),  _ADD_OP(1,"-",1) ),
    _ADD_OPS( _ADD_OP(1,"<<",1), _ADD_OP(1,">>",1) ),
    _ADD_OPS( _ADD_OP(1,"<=",1), _ADD_OP(1,">=",1), _ADD_OP(1,"<",1), _ADD_OP(1,">",1) ),
    _ADD_OPS( _ADD_OP(1,"==",1), _ADD_OP(1,"!=",1) ),
    _ADD_OPS( _ADD_OP(1,"&",1) ),
    _ADD_OPS( _ADD_OP(1,"^",1) ),
    _ADD_OPS( _ADD_OP(1,"|",1) ),
    _ADD_OPS( _ADD_OP(1,"&&",1) ),
    _ADD_OPS( _ADD_OP(1,"||",1) ),
};
#undef _ADD_OPS
#undef _ADD_OP

KLS_size _KLS_solveFac(KLS_size x){
    KLS_size i,res=1;
    for(i=2;i<=x;++i) res*=i;
    return res;
}

int _KLS_solveOpValue(const char *frmt,const char *l,const char *op, const char *r, char *tmp){
    switch(*op){
        case '+': return sprintf(tmp,frmt,strtod(l,0) + strtod(r,0));
        case '-': return sprintf(tmp,frmt,strtod(l,0) - strtod(r,0));
        case '*': return sprintf(tmp,frmt,strtod(l,0) * strtod(r,0));
        case '/': return sprintf(tmp,frmt,strtod(l,0) / strtod(r,0));
        case '@': return sprintf(tmp,frmt,pow(strtod(l,0),strtod(r,0)));
        case '%': return sprintf(tmp,frmt,KLS_mod(strtod(l,0),strtod(r,0)));
        case '~':{
            double vr=strtod(r,0);
            if(vr<=INT_MAX) return sprintf(tmp,"%d",~(int)vr);
            return -2;
        }
        case '^':{
            double vl=strtod(l,0),vr=strtod(r,0);
            if(vl<=INT_MAX && vr<=INT_MAX) return sprintf(tmp,"%d",(int)vl ^ (int)vr);
            return -2;
        }
        case '!':
            switch((!!l) | ((!!r)<<1)){
                case 1:{
                    double v=strtod(l,0);
                    if(v<0) return sprintf(tmp,"-%.0f" ,1.e-6+_KLS_solveFac(-v));
                    else return sprintf(tmp,"%.0f",1.e-6+_KLS_solveFac(v));
                }
                case 2: tmp[0]='0'+!strtod(r,0); tmp[1]=0; return 1; // logical negative
                default: return -2;
            }
    }
    if(!strncmp(op,"log10",5)) return sprintf(tmp,frmt,log10(strtod(r,0)*M_PI/180.));
    if(!strncmp(op,"log2",4)) return sprintf(tmp,frmt,log2(strtod(r,0)*M_PI/180.));
    if(!strncmp(op,"asin",4)) return sprintf(tmp,frmt,asin(strtod(r,0)*M_PI/180.));
    if(!strncmp(op,"acos",4)) return sprintf(tmp,frmt,acos(strtod(r,0)*M_PI/180.));
    if(!strncmp(op,"atan",4)) return sprintf(tmp,frmt,atan(strtod(r,0)*M_PI/180.));
    if(!strncmp(op,"log",3)) return sprintf(tmp,frmt,log(strtod(r,0)*M_PI/180.));
    if(!strncmp(op,"sin",3)) return sprintf(tmp,frmt,sin(strtod(r,0)*M_PI/180.));
    if(!strncmp(op,"cos",3)) return sprintf(tmp,frmt,cos(strtod(r,0)*M_PI/180.));
    if(!strncmp(op,"tan",3)) return sprintf(tmp,frmt,tan(strtod(r,0)*M_PI/180.));
    if(!strncmp(op,"||",2)) {tmp[0]='0'+(strtod(l,0) || strtod(r,0)); tmp[1]=0; return 1;}
    if(!strncmp(op,"&&",2)) {tmp[0]='0'+(strtod(l,0) && strtod(r,0)); tmp[1]=0; return 1;}
    if(!strncmp(op,">=",2)) {tmp[0]='0'+(strtod(l,0) >= strtod(r,0)); tmp[1]=0; return 1;}
    if(!strncmp(op,"<=",2)) {tmp[0]='0'+(strtod(l,0) <= strtod(r,0)); tmp[1]=0; return 1;}
    if(!strncmp(op,"==",2)) {tmp[0]='0'+(strtod(l,0) == strtod(r,0)); tmp[1]=0; return 1;}
    if(!strncmp(op,"!=",2)) {tmp[0]='0'+(strtod(l,0) != strtod(r,0)); tmp[1]=0; return 1;}
    if(!strncmp(op,"<<",2)) {
        double vl=strtod(l,0), vr=strtod(r,0);
        if(vl<=INT_MAX && vr<=INT_MAX) return sprintf(tmp,"%d",((int)vl) << ((int)vr));
        return -2;
    }
    if(!strncmp(op,">>",2)) {
        double vl=strtod(l,0), vr=strtod(r,0);
        if(vl<=INT_MAX && vr<=INT_MAX) return sprintf(tmp,"%d",((int)vl) << ((int)vr));
        return -2;
    }

    if(*op=='>') {tmp[0]='0'+(strtod(l,0) > strtod(r,0)); tmp[1]=0; return 1;}
    if(*op=='<') {tmp[0]='0'+(strtod(l,0) < strtod(r,0)); tmp[1]=0; return 1;}
    if(*op=='&') {
        double vl=strtod(l,0),vr=strtod(r,0);
        if(vl<=INT_MAX && vr<=INT_MAX) return sprintf(tmp,"%d",(int)vl & (int)vr);
        return -2;
    }
    if(*op=='|') {
        double vl=strtod(l,0),vr=strtod(r,0);
        if(vl<=INT_MAX && vr<=INT_MAX) return sprintf(tmp,"%d",(int)vl | (int)vr);
        return -2;
    }
    return -1;
}

char _KLS_solveOpCalc(_KLS_t_SOLVE * const s){
    const int lenTo=_KLS_solveOpValue(s->precision,s->op.left.c,s->op.op.c,s->op.right.c,s->value);
    if(lenTo>0){
        const int lenFrom=s->op.end.c-s->op.beg.c, lenDiff=lenTo-lenFrom, lenRest=strlen(s->op.end.c)+1;
        memmove(s->op.end.v+lenDiff,s->op.end.c,lenRest);
        memcpy(s->op.beg.v,s->value,lenTo);
        return 1;
    } return 0;
}

char _KLS_solveBrecket(_KLS_t_SOLVE * const s){
    s->brecket.c=NULL;
    const char *str=s->str;
    while(1){
        switch(*str){
            case 0: return 0;
            case '(': s->brecket.c=str; break;
            case ')': return !!s->brecket.c;
        }
        ++str;
    }
    return 0;
}

const char *_KLS_solveOpLeft(const char *s){
    char c, flag=0;
    while( (c=*(--s)) && ((c>='0' && c<='9') || c=='.') ) flag|=1;
    if(!flag) return NULL;
    if(c!='-') ++s;
    return s;
}

const char *_KLS_solveOpEnd(const char *s){
    char c=*(s++), flag=0;
    if(c>='0' && c<='9') flag|=1;
    else if(c!='-') return NULL;
    while( (c=*(s++)) && ((c>='0' && c<='9') || c=='.') ) flag|=1;
    if(flag) return s-1;
    return NULL;
}

KLS_byte _KLS_solveOpFind(_KLS_t_SOLVE * const s,const _KLS_t_SOLVE_OP * const op){
    const _KLS_t_SOLVE_OP *o=op;
    const char *str=s->brecket.c+1;
    for(;*str!=')';++str)
        for(o=op;o->len;++o)
            if(!strncmp(str,o->op,o->len)){
                if(o->left){
                    if( !(s->op.left.c=_KLS_solveOpLeft(str)) )
                        break;
                    s->op.beg.c=s->op.left.c;
                }else{
                    s->op.left.c=NULL;
                    s->op.beg.c=str;
                }
                if(o->right){
                    s->op.right.c=str+o->len;
                    if( !(s->op.end.c=_KLS_solveOpEnd(s->op.right.c)) )
                        break;
                }else{
                    s->op.right.c=NULL;
                    s->op.end.c=str+o->len;
                }
                s->op.op.c=o->op;
                return 1;
            }
    return 0;
}

void _KLS_solveUnbrecket(_KLS_t_SOLVE * const solve){
    char *s=solve->brecket.v;
    const char *fwd=s+1;
    while(*fwd!=')'){
        *s=*fwd; ++s; ++fwd;
    }
    ++fwd;
    while(*fwd){
        *s=*fwd; ++s; ++fwd;
    }
    *s=0;
}

KLS_byte _KLS_solveTry(_KLS_t_SOLVE * const s){
    unsigned int i;
    while(_KLS_solveBrecket(s)){
        for(i=0;i<KLS_ARRAY_LEN(_KLS_solveOps);++i)
            while(_KLS_solveOpFind(s,_KLS_solveOps[i]))
                if(!_KLS_solveOpCalc(s))
                    return 0;
        _KLS_solveUnbrecket(s);
    }
    return 1;
}

int _KLS_solveAddLen(const char *str,const char *precision){
    int ofs=0,size=0,n;
    float tmp;
    char val[64];
    do{
        n=0;
        sscanf(str+ofs,"%*[^0-9.]%n",&n);
        ofs+=n;
        if(sscanf(str+ofs,"%f%n",&tmp,&n)!=1)
            break;
        ofs+=n;
        size+=sprintf(val,precision,tmp)-n;
    }while(1);
    return size;
}

void _KLS_solveRemoveSymbols(char *s,const char *symbols){
    const char *fwd,*c;
    while(*s){
        for(c=symbols;*c;++c)
            if(*s==*c){
                fwd=s+1; goto _mark;
            }
        ++s;
    }
    return;
_mark:
    while(*fwd){
        for(c=symbols;*c;++c)
            if(*fwd==*c){
                ++fwd; goto _mark;
            }
        *s=*fwd;
        ++s;
        ++fwd;
    }
    *s=0;
}

KLS_byte _KLS_solveMake(const char *str,_KLS_t_SOLVE *s,int precision){
    sprintf(s->precision,"%%.%df",precision<0?6:precision);
    if(str){
        const unsigned int len=(strlen(str)+_KLS_solveAddLen(str,s->precision)+100);
        if( (s->str=KLS_malloc(len+1)) ){
            s->value=s->str+len-65;
            sprintf(s->str,"(%s)",str);
            _KLS_solveRemoveSymbols(s->str,"\t\n\r ");
            return 1;
        }
    }
    return 0;
}

KLS_byte _KLS_solveFindVal(const char *str){
    int n=-2; return sscanf(str,"%*f%n",&n)==0 && n==(int)strlen(str);
}

double KLS_stringSolve(const char *str,int floatPrecision){
    _KLS_t_SOLVE s={NULL};
    double answer=KLS_NAN;
    if(_KLS_solveMake(str,&s,floatPrecision)){
        if(_KLS_solveTry(&s) && _KLS_solveFindVal(s.str))
            answer=strtod(s.str,0);
        KLS_freeData(s.str);
    }
    return answer;
}
