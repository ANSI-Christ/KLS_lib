
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
    char *m;
}_KLS_u_SOLVE_PTR;

typedef struct{
    _KLS_u_SOLVE_PTR op, beg, end;
    int left, right, res;
}_KLS_t_SOLVE_MATCH;

typedef struct{
    double *val;
    char str[1];
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

double _KLS_solveFac(KLS_size x){
    KLS_size i=2,res=1;
    for(++x;i<x;++i) res*=i;
    return res;
}

char _KLS_solveOpCalc(const double * const l,const char *op,const double * const r,double * const x){
    switch(*op){
        case '+': *x = *l + *r; return 0;
        case '-': *x = *l - *r; return 0;
        case '*': *x = *l * *r; return 0;
        case '/': *x = *l / *r; return 0;
        case '@': *x = pow(*l,*r); return 0;
        case '%': *x = KLS_mod(*l,*r); return 0;
        case '~':{
            if(*r>INT_MAX) return -1;
            *x = ~(int)*r; return 0;
        }
        case '^':{
            if(*l>INT_MAX || *r>INT_MAX) return -1;
            *x = (int)*l ^ (int)*r; return 0;
        }
        case '!':
            switch((!!l) | ((!!r)<<1)){
                case 1:{
                    if(*l<0) *x = -_KLS_solveFac(-*l);
                    else *x = _KLS_solveFac(*l);
                    return 0;
                }
                case 2: *x = !*r; return 0;
                default: return -1;
            }
    }
    if(!strncmp(op,"log10",5)) { *x = log10(*r); return 0; }
    if(!strncmp(op,"log2",4)) { *x = log2(*r); return 0; }
    if(!strncmp(op,"asin",4)) { *x = asin(*r); return 0; }
    if(!strncmp(op,"acos",4)) { *x = acos(*r); return 0; }
    if(!strncmp(op,"atan",4)) { *x = atan(*r); return 0; }
    if(!strncmp(op,"log",3)) { *x = log(*r); return 0; }
    if(!strncmp(op,"sin",3)) { *x = sin(*r); return 0; }
    if(!strncmp(op,"cos",3)) { *x = cos(*r); return 0; }
    if(!strncmp(op,"tan",3)) { *x = tan(*r); return 0; }
    if(!strncmp(op,"||",2)) { *x = *l || *r; return 0; }
    if(!strncmp(op,"&&",2)) { *x = *l && *r; return 0; }
    if(!strncmp(op,">=",2)) { *x = *l >= *r; return 0; }
    if(!strncmp(op,"<=",2)) { *x = *l <= *r; return 0; }
    if(!strncmp(op,"==",2)) { *x = *l == *r; return 0; }
    if(!strncmp(op,"!=",2)) { *x = *l != *r; return 0; }
    if(!strncmp(op,"<<",2)) {
        if(*l>INT_MAX || *r>INT_MAX) return -1;
        *x = (int)*l << (int)*r; return 0;
    }
    if(!strncmp(op,">>",2)) {
        if(*l>INT_MAX || *r>INT_MAX) return -1;
        *x = (int)*l >> (int)*r; return 0;
    }
    if(*op=='>') { *x = *l > *r; return 0;}
    if(*op=='<') { *x = *l < *r; return 0;}
    if(*op=='&') {
        if(*l>INT_MAX || *r>INT_MAX) return -1;
        *x = (int)*l & (int)*r; return 0;
    }
    if(*op=='|') {
        if(*l>INT_MAX || *r>INT_MAX) return -1;
        *x = (int)*l | (int)*r; return 0;
    }
    return -1;
}

void _KLS_solveRemove(char *beg, char *end){
    for(;*end;++beg,++end) *beg=*end;
    *beg=0;
}

const char *_KLS_solveOpLeft(const char *s,int *id){
    if(*--s==']')
        for(--s;*s!='(';--s)
            if(*s=='['){
                *id=-atoi(s+1);
                return s;
            }
    return NULL;
}

const char *_KLS_solveOpRight(const char *s,int *id){
    if(*s=='['){
        char *end;
        *id=-strtod(s+1,&end);
        return end+1;
    }
    return NULL;
}

void _KLS_solveUnbrecket(char *s){
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

const char *_KLS_solveBrecket(const char *s){
    const char *b=NULL;
    for(;*s;++s)
        switch(*s){
            case '(': b=s; break;
            case ')': return b;
        }
    return NULL;
}

KLS_byte _KLS_solveOpFind(const char *s,const _KLS_t_SOLVE_OP * const ops,_KLS_t_SOLVE_MATCH *m){
    const _KLS_t_SOLVE_OP *op;
    for(;*s!=')';++s)
        for(op=ops;op->len;++op)
            if(!strncmp(s,op->op,op->len)){
                if(op->right){
                    if( !(m->end.c=_KLS_solveOpRight(s+op->len,&m->right)) )
                        break;
                    m->res=m->right;
                }else{
                    m->right=1;
                    m->end.c=s+op->len;
                }
                if(op->left){
                    if( !_KLS_solveOpLeft(s,&m->left) )
                        break;
                    m->res=m->left;
                }else m->left=1;
                m->beg.c=s;
                m->op.c=s;
                return 1;
            }
    return 0;
}

KLS_byte _KLS_solveTry(_KLS_t_SOLVE * const solve){
    _KLS_t_SOLVE_MATCH match;
    _KLS_u_SOLVE_PTR brecket;
    double val;
    unsigned int i;
    while( (brecket.c=_KLS_solveBrecket(solve->str)) ){
        for(i=0;i<KLS_ARRAY_LEN(_KLS_solveOps);++i)
            while(_KLS_solveOpFind(brecket.c,_KLS_solveOps[i],&match)){
                if(_KLS_solveOpCalc((match.left>0 ? NULL : solve->val+match.left),match.op.c,(match.right>0 ? NULL : solve->val+match.right),&val))
                    return 0;
                solve->val[match.res]=val;
                _KLS_solveRemove(match.beg.m,match.end.m);
            }
        _KLS_solveUnbrecket(brecket.m);
    }
    return 1;
}

char _KLS_solveInit(_KLS_t_SOLVE *solve,const char *s){
#define _KLS_SOLVE_PUSH(_symb_) *str=_symb_, ++str
    char *str=solve->str;
    signed int vals=0;
    for(_KLS_SOLVE_PUSH('(');*s;++s)
        switch(*s){
            case ' ': case '\n': case '\t': case '\r':
                continue;
            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
            case '8': case '9': case '+': case '-':
            {
                char *end;
                double x=strtod(s,&end);
                if(end==s){
                    _KLS_SOLVE_PUSH(*s);
                    continue;
                }
                s=end-1;
                solve->val[vals]=x;
                if(str[-1]==']'){
                    _KLS_SOLVE_PUSH('+');
                }
                _KLS_SOLVE_PUSH('[');
                str+=sprintf(str,"%d",-vals);
                _KLS_SOLVE_PUSH(']');
                --vals; continue;
            }
            default:
                if(strchr("[]{}\'\";:|\\",*s)) return 0;
                _KLS_SOLVE_PUSH(*s); continue;
        }
    _KLS_SOLVE_PUSH(')');
    _KLS_SOLVE_PUSH(0);
    return 1;
#undef _KLS_SOLVE_PUSH
}

_KLS_t_SOLVE *_KLS_solveNew(const char *s){
    const unsigned int vals=s?strlen(s):0;
    if(vals){
        unsigned int size=vals*2+8;
        size+=sizeof(double)-(size%sizeof(double)) + (vals-1)*sizeof(double);
        _KLS_t_SOLVE *p=KLS_malloc(KLS_OFFSET(*p,str)+size+sizeof(double));
        if(p) p->val=(void*)(p->str+size);
        return p;
    } return NULL;
}

double KLS_stringSolve(const char *str){
    _KLS_t_SOLVE *solve=_KLS_solveNew(str);
    double answer=KLS_NAN;
    if(solve){
        if(_KLS_solveInit(solve,str) && _KLS_solveTry(solve)){
            int i;
            const char *end=_KLS_solveOpRight(solve->str,&i);
            if(end && !*end) answer=solve->val[i];
        }
        KLS_free(solve);
    }
    return answer;
}
