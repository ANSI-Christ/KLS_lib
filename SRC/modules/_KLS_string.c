
char *_KLS_stringv(const char *format, va_list ap[2]){
    char *str=NULL;
    long len=vsnprintf(NULL,0,format,ap[0])+1; va_end(ap[0]);
    if(len>0 && (str=KLS_malloc(len)) ) vsnprintf(str,len,format,ap[1]);
    va_end(ap[0]); va_end(ap[1]);
    return str;
}

char *KLS_string(const char *format, ...){
    return KLS_stringv(format);
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
        KLS_size lenFrom=strlen(from), lenTo=to?strlen(to):0;
        KLS_size len=strlen(string)+1+KLS_stringWordCount(string,from)*(lenTo-lenFrom);
        if( (str=KLS_malloc(len)) ){
            const char *end;
            *str=0;
            while( (end=strstr(string,from))){
                strncat(str,string,KLS_PTRSUB(end,string));
                strncat(str,to,lenTo);
                string=end+lenFrom;
            }
            strcat(str,string);
        }
    }
    return str;
}

char *KLS_stringJoinList(const char **stringList,int stringListLen,const char *separator){
    int i,len=1;
    char *str=NULL;
    if(!stringList) return NULL;
    for(i=0;i<stringListLen;++i)
        if(stringList[i])
            len+=strlen(stringList[i]);
    if(separator)
        len+=strlen(separator)*(stringListLen-1);
    if( len>0 && (str=KLS_malloc(len)) ){
        memset(str,0,len);
        for(i=0;i<stringListLen;++i){
            if(stringList[i])
                strcat(str,stringList[i]);
            if(separator && i<(stringListLen-1))
                strcat(str,separator);
        }
    }
    return str;
}

KLS_t_VECTOR KLS_stringSplit(const char *string, const char *separator){
    KLS_t_VECTOR v={NULL};
    if(string && separator){
        char *tmp;
        const char *end;
        KLS_size lenSep=strlen(separator),cnt=KLS_stringWordCount(string,separator)+2;
        if( !(v=KLS_vectorNew(cnt,sizeof(char*),KLS_ptrDeleter)).data )
            return v;
        while( (end=strstr(string,separator)) ){
            if( !(tmp=KLS_string("%.*s",(int)(KLS_PTRSUB(end,string)),string)) ){
                KLS_vectorFree(&v);
                return v;
            }
            KLS_vectorPushBack(&v,&tmp);
            string=end+lenSep;
        }
        if( (tmp=KLS_string("%s",string)) )
            KLS_vectorPushBack(&v,&tmp);
        else KLS_vectorFree(&v);
    }
    return v;
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



typedef struct{
    char *str, *breckets, *unBreckets;
    char *tmp, *tmp2, *tmp3;
    char precision[8];
}KLS_t_SOLVE;

typedef struct{
    const char *op;
    const KLS_byte opLen:4;
    const KLS_byte left:2;
    const KLS_byte right:2;
}KLS_t_SOLVE_OP;

typedef struct{
    const char *left, *op, *right;
}KLS_t_SOLVE_MATCH;

#define _ADD_OPS(...) KLS_$(KLS_t_SOLVE_OP)(__VA_ARGS__,_ADD_OP(0,NULL,0))
#define _ADD_OP(left,op,right) {op,op?((sizeof op)-1):0,left,right}
static const KLS_t_SOLVE_OP *_KLS_solveOps[]={
    _ADD_OPS( _ADD_OP(0,"!",1),     _ADD_OP(0,"~",1) ), // !2 [=0]
    _ADD_OPS( _ADD_OP(1,"!",0) ), // 5! [=120]
    _ADD_OPS( _ADD_OP(0,"asin",1),  _ADD_OP(0,"acos",1), _ADD_OP(0,"atan",1),
              _ADD_OP(0,"sin",1),   _ADD_OP(0,"cos",1),  _ADD_OP(0,"tan",1),
              _ADD_OP(0,"log10",1), _ADD_OP(0,"log2",1), _ADD_OP(0,"log",1)
            ),
    _ADD_OPS( _ADD_OP(1,"@",1) ), // pow(l,r)
    _ADD_OPS( _ADD_OP(1,"*",1),  _ADD_OP(1,"/",1),  _ADD_OP(1,"%",1) ), // 2*2
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

const char *_KLS_solveCalcOp(const char *frmt,const char *l,const char *op, const char *r, char *tmp){
    KLS_byte chk=(!!l) | ((!!r)<<1);
    switch(*op){
        case '+': sprintf(tmp,frmt,strtod(l,0) + strtod(r,0)); return tmp;
        case '-': sprintf(tmp,frmt,strtod(l,0) - strtod(r,0)); return tmp;
        case '*': sprintf(tmp,frmt,strtod(l,0) * strtod(r,0)); return tmp;
        case '/': sprintf(tmp,frmt,strtod(l,0) / strtod(r,0)); return tmp;
        case '@': sprintf(tmp,frmt,pow(strtod(l,0),strtod(r,0))); return tmp;
        case '%': sprintf(tmp,frmt,KLS_mod(strtod(l,0),strtod(r,0))); return tmp;
        case '~':{
            int64_t vr=strtod(r,0);
            if(vr>INT_MAX) sprintf(tmp,KLS_FORMAT_LONG(d),~vr);
            else sprintf(tmp,"%d",~(int32_t)vr); 
            return tmp;
        }
        case '^':{
            int64_t vl=strtod(l,0),vr=strtod(r,0);
            if(vl>INT_MAX || vr>INT_MAX) sprintf(tmp,KLS_FORMAT_LONG(d),vl ^ vr);
            else sprintf(tmp,"%d",(int32_t)vl ^ (int32_t)vr); 
            return tmp;
        }
        case '!':
            switch(chk){
                case 1:{
                    double v=strtod(l,0);
                    if(v<0.) sprintf(tmp,"-" KLS_FORMAT_LONG(u),_KLS_solveFac(-v));
                    else sprintf(tmp,KLS_FORMAT_LONG(u),_KLS_solveFac(v));
                    return tmp;
                }
                case 2: tmp[0]='0'+!strtod(r,0); tmp[1]=0;return tmp; // logical negative
                case 3: break;
            }
    }
    if(!strncmp(op,"log10",5)) {sprintf(tmp,frmt,log10(strtod(r,0)*M_PI/180.)); return tmp;}
    if(!strncmp(op,"log2",4)) {sprintf(tmp,frmt,log2(strtod(r,0)*M_PI/180.)); return tmp;}
    if(!strncmp(op,"asin",4)) {sprintf(tmp,frmt,asin(strtod(r,0)*M_PI/180.)); return tmp;}
    if(!strncmp(op,"acos",4)) {sprintf(tmp,frmt,acos(strtod(r,0)*M_PI/180.)); return tmp;}
    if(!strncmp(op,"atan",4)) {sprintf(tmp,frmt,atan(strtod(r,0)*M_PI/180.)); return tmp;}
    if(!strncmp(op,"log",3)) {sprintf(tmp,frmt,log(strtod(r,0)*M_PI/180.)); return tmp;}
    if(!strncmp(op,"sin",3)) {sprintf(tmp,frmt,sin(strtod(r,0)*M_PI/180.)); return tmp;}
    if(!strncmp(op,"cos",3)) {sprintf(tmp,frmt,cos(strtod(r,0)*M_PI/180.)); return tmp;}
    if(!strncmp(op,"tan",3)) {sprintf(tmp,frmt,tan(strtod(r,0)*M_PI/180.)); return tmp;}
    if(!strncmp(op,"||",2)) {tmp[0]='0'+(strtod(l,0) || strtod(r,0)); tmp[1]=0; return tmp;}
    if(!strncmp(op,"&&",2)) {tmp[0]='0'+(strtod(l,0) && strtod(r,0)); tmp[1]=0; return tmp;}
    if(!strncmp(op,">=",2)) {tmp[0]='0'+(strtod(l,0) >= strtod(r,0)); tmp[1]=0; return tmp;}
    if(!strncmp(op,"<=",2)) {tmp[0]='0'+(strtod(l,0) <= strtod(r,0)); tmp[1]=0; return tmp;}
    if(!strncmp(op,"==",2)) {tmp[0]='0'+(strtod(l,0) == strtod(r,0)); tmp[1]=0; return tmp;}
    if(!strncmp(op,"!=",2)) {tmp[0]='0'+(strtod(l,0) != strtod(r,0)); tmp[1]=0; return tmp;}
    if(!strncmp(op,"<<",2)) {\
        int64_t vl=strtod(l,0);int vr=strtod(r,0);\
        if(vl>INT_MAX) sprintf(tmp,KLS_FORMAT_LONG(d),vl << vr);\
            else sprintf(tmp,"%d",((int32_t)vl) << vr);\
        return tmp;\
    }
    if(!strncmp(op,">>",2)) {\
        int64_t vl=strtod(l,0);int vr=strtod(r,0);\
        if(vl>INT_MAX) sprintf(tmp,KLS_FORMAT_LONG(d),vl >> vr);\
            else sprintf(tmp,"%d",((int32_t)vl) >> vr);\
        return tmp;\
    }

    if(*op=='>') {tmp[0]='0'+(strtod(l,0) > strtod(r,0)); tmp[1]=0; return tmp;}
    if(*op=='<') {tmp[0]='0'+(strtod(l,0) < strtod(r,0)); tmp[1]=0; return tmp;}
    if(*op=='&') {\
        int64_t vl=strtod(l,0),vr=strtod(r,0);\
        if(vl>INT_MAX || vr>INT_MAX) sprintf(tmp,KLS_FORMAT_LONG(d),vl & vr);\
            else sprintf(tmp,"%d",(int32_t)vl & (int32_t)vr);\
        return tmp;\
    }
    if(*op=='|') {\
        int64_t vl=strtod(l,0),vr=strtod(r,0);\
        if(vl>INT_MAX || vr>INT_MAX) sprintf(tmp,KLS_FORMAT_LONG(d),vl | vr);\
            else sprintf(tmp,"%d",(int32_t)vl | (int32_t)vr);\
        return tmp;\
    }
    return NULL;
}

KLS_byte _KLS_solveReplace(char *str,const char *from,const char *to,char *tmp){
    if(str && from && to){
        unsigned int len=strlen(from);
        const char *begin=str,*end;
        *tmp=0;
        //printf("replace in %s: \'%s\' to \'%s\'\n",str,from,to);
        while((end=strstr(begin,from))){
            strncat(tmp,begin,KLS_PTRSUB(end,begin));
            strcat(tmp,to);
            begin=end+len;
        }
        if(begin==str) return 0;
        strcat(tmp,begin);
        *str=0; strcat(str,tmp);
        return 1;
    }
    return 0;
}

void _KLS_solveRemoveSymbols(char *str,const char *symbols,char *tmp){
    const char *s, *_str=str;
    KLS_byte chk;
    *tmp=0;
    while(*_str){
        s=symbols; chk=1;
        while(*s) chk&=(*_str!=*(s++));
        if(chk) strncat(tmp,_str,1);
        ++_str;
    }
    *str=0; strcat(str,tmp);
}

KLS_byte _KLS_solveGetBreckets(KLS_t_SOLVE *s){
    if(s->str){
        char l;
        const char *begin=s->str,*str=begin;
        while( (l=*str) ){
            if(l=='('){
                begin=str++;
                continue;
            }
            if(l==')'){
                unsigned int len=KLS_PTRSUB(str,begin)+1;
                *s->breckets=0; strncat(s->breckets,begin,len);
                *s->unBreckets=0; strncat(s->unBreckets,begin+1,len-2);
                return 1;
            }
            ++str;
        }
    }
    return 0;
}

const char *_KLS_solveFindLeft(const char *begin){
    char c;
    KLS_byte cnt=0;
    while( (c=*(--begin)) && ((c>='0' && c<='9') || c=='.') ) ++cnt;
    if(!cnt) return NULL;
    if(c!='-') ++begin;
    return begin;
}

const char *_KLS_solveFindEnd(const char *end){
    char c=*(end++);
    KLS_byte cnt=0;
    if(c>='0' && c<='9') ++cnt;
    else if(c!='-') return NULL;
    while( (c=*(end++)) && ((c>='0' && c<='9') || c=='.') ) ++cnt;
    if(cnt) return --end;
    return NULL;
}

KLS_byte _KLS_solveFindVal(const char *str){
    char c;
    KLS_byte ret=1,cntDot=0,cntOp=0;
    while( (c=*(str++)) ){
        cntDot+=(c=='.');
        cntOp+=(c=='-' || c=='+');
        ret&=((c>='0' && c<='9') || (c=='.') || (c=='-') || (c=='+'));
        ret&=cntDot<2 && cntOp<2;
    }
    return ret;
}

KLS_byte _KLS_solveFindOp(const char *str,const KLS_t_SOLVE_OP *op,KLS_t_SOLVE_MATCH *m, char *replace){
    const char *begin, *end;
    const KLS_t_SOLVE_OP *o;
    *replace=0;
    memset(m,0,sizeof(*m));
    while(*str){
        o=op;
        while( (m->op=o->op) ){
            begin=str;
            if(!strncmp(begin,m->op,o->opLen)){
                end=begin+o->opLen;
                if(o->left){
                    if( !(begin=_KLS_solveFindLeft(begin)) )
                        return 0;
                    m->left=begin;
                }
                if(o->right){
                    m->right=end;
                    if( !(end=_KLS_solveFindEnd(end)) ){
                        m->right=NULL;
                        return 0;
                    }
                }
                strncat(replace,begin,KLS_PTRSUB(end,begin));
                return 1;
            }
            ++o;
        }
        ++str;
    }
    return 0;
}

KLS_byte _KLS_solveTry(KLS_t_SOLVE *s){
    int i;
    KLS_t_SOLVE_MATCH m;
    while(_KLS_solveGetBreckets(s)){
        for(i=0;i<KLS_ARRAY_LEN(_KLS_solveOps);++i)
            while(_KLS_solveFindOp(s->unBreckets,_KLS_solveOps[i],&m,s->tmp2))
                if( !_KLS_solveReplace(s->unBreckets,s->tmp2,_KLS_solveCalcOp(s->precision,m.left,m.op,m.right,s->tmp3),s->tmp) )
                    return 0;
        _KLS_solveReplace(s->str,s->breckets,s->unBreckets,s->tmp);
    }
    return 1;
}

KLS_size _KLS_solveAddLen(const char *str,const char *precision){
    int ofs=0;
    float tmp;
    char buf[64]={0};
    KLS_size size=0;
    for(;*str;++str) if(*str>='0' && *str<='9') break;
    while(sscanf(str,"%f%n",&tmp,&ofs)==1){
        str+=ofs;
        ofs=sprintf(buf,precision,tmp)-ofs;
        if(ofs>0) size+=ofs;
        for(;*str;++str)
            if(*str>='0' && *str<='9')
                break;
    }
    return size;
}

KLS_byte _KLS_solveMake(const char *str,KLS_t_SOLVE *s,int precision){
    memset(s,0,sizeof(*s));
    sprintf(s->precision,"%%.%df",precision<0?6:precision);
    if(str){
        unsigned int len1=(strlen(str)+_KLS_solveAddLen(str,s->precision)+3+3), len2=128;
        if( (s->str=KLS_malloc((len1<<2)+(len2<<1))) ){
            *s->str=0;
            s->breckets=s->str+len1;         s->breckets[-1]=s->breckets[0]=0;
            s->unBreckets=s->breckets+len1;  s->unBreckets[-1]=s->unBreckets[0]=0;
            s->tmp=s->unBreckets+len1;       s->tmp[-1]=s->tmp[0]=0;
            s->tmp2=s->tmp+len1;             s->tmp2[-1]=s->tmp2[0]=0;
            s->tmp3=s->tmp2+len2;            s->tmp3[-1]=s->tmp3[0]=0;
            strcat(s->str,"("); strcat(s->str,str); strcat(s->str,")");
            _KLS_solveRemoveSymbols(s->str,"\t\n ",s->tmp);
            return 1;
        }
    }
    return 0;
}

double KLS_stringSolve(const char *str,int floatPrecision){
    double answer=KLS_NAN;
    KLS_t_SOLVE s;
    if(_KLS_solveMake(str,&s,floatPrecision)){
        if(_KLS_solveTry(&s) && _KLS_solveFindVal(s.str))
            answer=strtod(s.str,0);
        KLS_freeData(s.str);
    }
    return answer;
}
