#include <ctype.h>

#define KLS_REGEX_SET       1
#define KLS_REGEX_CHAR      2
#define KLS_REGEX_GROUP     3
#define KLS_REGEX_EMPTY     4
#define KLS_REGEX_SPECIAL   5

#define KLS_REGEX_IGNCASE     1
#define KLS_REGEX_MULTILINE   2

#define KLS_REGEX_WEAK        1
#define KLS_REGEX_POSSESIVE   2


typedef struct _KLS_t_REGEX{
    struct _KLS_t_REGEX *next, *alt;
    unsigned int range[2], key;
    KLS_byte type, flags, greedy, _lock;
}KLS_t_REGEX;

typedef struct{
    const char *s, *err;
    KLS_t_REGEX *storage, *i, *last;
    unsigned int groups;
    KLS_byte flags;
}KLS_t_REGEX_COMPILE;

void _KLS_regexPrintGreedy(FILE *f,const char *s,const KLS_t_REGEX *n){
    char c=' ';
    if(n->greedy==KLS_REGEX_WEAK) c='?';
    if(n->greedy==KLS_REGEX_POSSESIVE) c='+';
    fprintf(f,"%s{%u,%u}%c 0x%X\n",s,n->range[0],n->range[1],c,n->flags);
}

void _KLS_regexPrint(FILE *f,const KLS_t_REGEX *r,unsigned int deep){
    const KLS_t_REGEX *n=r+1, *a=n->alt;
    fprintf(f,"%*cG(%u)",deep,' ',r->key);
    _KLS_regexPrintGreedy(f," ",r);
    deep+=4;
    while(n){
        switch(n->type){
            case KLS_REGEX_SET:{
                const KLS_t_REGEX *i=n+1;
                fprintf(f,"%*cS(%c)",deep,' ',n->key?'-':'+'); _KLS_regexPrintGreedy(f," ",n);
                do{
                    switch(i->type){
                        case KLS_REGEX_GROUP: _KLS_regexPrint(f,i,deep+4); break;
                        case KLS_REGEX_CHAR:
                            if(i->key<256) fprintf(f,"%*cL(%c)\n",deep+4,' ',(char)i->key);
                            else fprintf(f,"%*cL(%c-%c)\n",deep+4,' ',((char*)&i->key)[0],((char*)&i->key)[2]);
                            break;
                        case KLS_REGEX_SPECIAL: fprintf(f,"%*c\\(%c)\n",deep+4,' ',i->key); break;
                    }
                }while((i=i->alt));
                break;
            }
            case KLS_REGEX_CHAR: fprintf(f,"%*cL(%c)",deep,' ',n->key); _KLS_regexPrintGreedy(f," ",n); break;
            case KLS_REGEX_GROUP: _KLS_regexPrint(f,n,deep); break;
            case KLS_REGEX_EMPTY: fprintf(f,"%*c__EMPTY_BRANCH__\n",deep,' '); break;
            case KLS_REGEX_SPECIAL: fprintf(f,"%*c\\(%c)",deep,' ',n->key); _KLS_regexPrintGreedy(f," ",n); break;
            default: return;
        }
        if(n->next){
            if(n->next==r){
                if((n=a)){
                    a=a->alt;
                    fprintf(f,"%*cOR\n",deep-2,' ');
                }
                continue;
            }
            n=n->next;
            continue;
        }
        ++n;
    }
}

void KLS_regexPrint(const void *regex,FILE *f){
    if(!f) f=stdout;
    if(regex){
        _KLS_regexPrint(f,regex,0);
        return;
    } fprintf(f,"no regex!\n");
}

KLS_t_REGEX *KLS_regexNode(KLS_t_REGEX_COMPILE *c,KLS_byte type){
    KLS_t_REGEX *prev=c->last;
    if(prev){
        if(prev->_lock){
            prev->next=c->i;
        }else if((++prev)->type){
            while(prev->alt) prev=prev->alt;
            prev->alt=c->i;
        }
    }
    c->i->type=type;
    c->i->flags=c->flags;
    c->i->range[0]=c->i->range[1]=1;
    return (c->last=c->i++);
}

KLS_byte KLS_regexSetRange(KLS_t_REGEX_COMPILE *c,unsigned int min,unsigned int max){
    unsigned int l=1;
    if(min==max){
        l=0;
        if(sscanf(c->s,"%u,%u}%n",&min,&max,&l)>1 && l){
            char s[22]; sprintf(s,"%u,%u",min,max);
            if(strncmp(c->s,s,l-1) || !max || max<min) return 0;
            c->s+=l;
        }else if(sscanf(c->s,"%u,}%n",&min,&l)>0 && l){
            char s[11]; sprintf(s,"%u",min);
            if(strncmp(c->s,s,l-2)) return 0;
            max=-1; c->s+=l;
        }else if(sscanf(c->s,"%u}%n",&min,&l)>0  && l){
            char s[11]; sprintf(s,"%u",min);
            if(strncmp(c->s,s,l-1) || !min) return 0;
            max=min; c->s+=l;
        }
    }
    if(l){
        switch(*c->s){
            case '?': ++c->s; c->last->greedy=KLS_REGEX_WEAK; break;
            case '+': ++c->s; c->last->greedy=KLS_REGEX_POSSESIVE; break;
        }
        if(*c->s && strchr("*+?{",*c->s)) return 0;
        c->last->range[0]=min;
        c->last->range[1]=max;
        return 1;
    }
    return 0;
}

KLS_byte KLS_regexCompileSpecial(KLS_t_REGEX_COMPILE *c){
    #define _RGX_CASE(_t_,...) if(*c->s && strchr(__VA_ARGS__,*c->s)){KLS_regexNode(c,KLS_REGEX_##_t_)->key=*(c->s++); return 1;}
    _RGX_CASE(CHAR,"^$.+*?[](){}\\")
    _RGX_CASE(SPECIAL,"aAcCdDpPsSwWxXul")
    #undef _RGX_CASE
    #define _RGX_CASE(_t_,_c_,_v_) if(*c->s==_c_){KLS_regexNode(c,KLS_REGEX_##_t_)->key=_v_; ++c->s; return 1;}
    _RGX_CASE(CHAR,'n','\n') _RGX_CASE(CHAR,'t','\t') _RGX_CASE(CHAR,'r','\r')
    _RGX_CASE(CHAR,'f','\f') _RGX_CASE(CHAR,'v','\v')
    #undef _RGX_CASE
    return 0;
}

KLS_byte _KLS_regexCompileGroupOrFlags(KLS_t_REGEX_COMPILE *c);

KLS_byte KLS_regexCompileSet(KLS_t_REGEX_COMPILE *c){
    KLS_t_REGEX *n[2]={NULL}, *set=KLS_regexNode(c,KLS_REGEX_SET);
    KLS_byte i=0;
    c->s+=(set->key=(*c->s=='^'));
    while(1){
        if(!*c->s || strchr(".+*?{})[",*c->s))
            return 0;
        c->last=set;
        switch(*(c->s++)){
            case '(':
                if(_KLS_regexCompileGroupOrFlags(c)){n[i]=c->last; break;}
                return 0;
            case ']':
                if(set[1].type){set->_lock=1; return 1;}
                return 0;
            case '\\':
                if(KLS_regexCompileSpecial(c)){n[i]=c->last; break;}
                return 0;
            case '^':
            case '&': (n[i]=KLS_regexNode(c,KLS_REGEX_SPECIAL))->key=c->s[-1]; break;
            default: (n[i]=KLS_regexNode(c,KLS_REGEX_CHAR))->key=c->s[-1]; break;
        }
        if(n[1]){
            if(n[1]->type==KLS_REGEX_SPECIAL) return 0;
            if(n[0]->key>n[1]->key) return 0;
            ((char*)&n[0]->key)[2]=((char*)&n[1]->key)[0];
            memset(n[1],0,sizeof(**n));
            n[0]->alt=n[1]=NULL; i=0;
            continue;
        }
        if(*c->s=='-'){
            if(*(++c->s)==']'){
                c->last=set;
                (n[0]=KLS_regexNode(c,KLS_REGEX_CHAR))->key=c->s[-1];
                continue;
            }
            if(n[0]->type==KLS_REGEX_SPECIAL) return 0;
            i=1;
        }
    }
}

KLS_byte KLS_regexCompileFlags(KLS_t_REGEX_COMPILE *c){
    #define _RGX_FLAG(_m_) if(state) c->flags|=KLS_REGEX_##_m_; else c->flags&=~KLS_REGEX_##_m_; break
    KLS_byte state=1;
    ++c->s;
    while(1)
        switch(*(c->s++)){
            case 'i': _RGX_FLAG(IGNCASE);
            case 's': break;
            case 'm': break;
            case '-': state=0; break;
            case ')': return 1;
            case ':': return 2;
            default: return 0;
        }
    #undef _RGX_FLAG
}

KLS_t_REGEX *KLS_regexCompileGroup(KLS_t_REGEX_COMPILE *c){
    #define _RGX_ERR(_e_) if(!c->err) c->err=(_e_); return NULL
    KLS_t_REGEX *group=KLS_regexNode(c,KLS_REGEX_GROUP);
    KLS_byte flags=c->flags;
    while(1)
        switch(*(c->s++)){
            case 0:
                if(group==c->storage){
                    if(c->last==group) KLS_regexNode(c,KLS_REGEX_EMPTY);
                    c->last->next=c->storage;
                    group->next=c->i;
                    return group;
                } _RGX_ERR("unfinished pattern");
            case '|':
                if(c->last==group) KLS_regexNode(c,KLS_REGEX_EMPTY);
                c->last->next=group;
                c->last=group;
                c->flags=flags; // flags works only for branch of group
                continue;
            case '(':
                if(_KLS_regexCompileGroupOrFlags(c)) continue;
                _RGX_ERR("bad flags");
            case ')':
                if(group!=c->storage){
                    if(c->last==group) KLS_regexNode(c,KLS_REGEX_EMPTY);
                    (c->last->next=group)->_lock=1;
                    c->flags=flags;
                    return group;
                } _RGX_ERR("unexpected bracket");
            case ']': case '}': _RGX_ERR("unexpected symbol");
            case '?': if(c->last!=group && KLS_regexSetRange(c,0,1)) continue; _RGX_ERR("bad range");
            case '*': if(c->last!=group && KLS_regexSetRange(c,0,-1)) continue; _RGX_ERR("bad range");
            case '+': if(c->last!=group && KLS_regexSetRange(c,1,-1)) continue; _RGX_ERR("bad range");
            case '{': if(c->last!=group && KLS_regexSetRange(c,-1,-1)) continue; _RGX_ERR("bad range");
            case '[': if(KLS_regexCompileSet(c)) continue; _RGX_ERR("bad set");
            case '\\': if(KLS_regexCompileSpecial(c)) continue; _RGX_ERR("unknown special");
            case '^': case '&': case '.': KLS_regexNode(c,KLS_REGEX_SPECIAL)->key=c->s[-1]; continue;
            default: KLS_regexNode(c,KLS_REGEX_CHAR)->key=c->s[-1]; continue;
        }
    #undef _RGX_ERR
}

KLS_byte _KLS_regexCompileGroupOrFlags(KLS_t_REGEX_COMPILE *c){
    if(*c->s=='?'){
        KLS_byte save=c->flags;
        switch(KLS_regexCompileFlags(c)){
            case 0: return 0;
            case 1: return 1;
        }
        if( (c->last=KLS_regexCompileGroup(c)) ){
            c->flags=save;
            return 1;
        }
    }else{
        c->i->key=++c->groups; //hack
        if( (c->last=KLS_regexCompileGroup(c)) )
            return 1;
    }
    return 0;
}

KLS_byte _KLS_regexNew(KLS_t_REGEX_COMPILE *c){
    if(c->s && *c->s){
        unsigned int l=strlen(c->s)+1;
        if( (c->storage=KLS_malloc(sizeof(struct _KLS_t_REGEX)*l)) ){
            memset(c->storage,0,sizeof(struct _KLS_t_REGEX)*l);
            (c->i=c->storage)->key=c->groups=1;
            return 1;
        }else c->err="no free memory";
    }else c->err="empty pattern";
    return 0;
}

void *KLS_regexNew(const char *pattern,const char **err){
    KLS_t_REGEX_COMPILE c={pattern};
    if(_KLS_regexNew(&c) && !KLS_regexCompileGroup(&c))
        KLS_freeData(c.storage);
    if(err) *err=c.err;
    return c.storage;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

typedef struct{
    const char *s;
    KLS_t_REGEX *n;
    unsigned int i;
}KLS_t_REGEX_SAVE;

typedef struct{
    const KLS_t_REGEX *last;
    const char *begin,*s,*end;
    KLS_t_REGEX_CAP *cap;
}KLS_t_REGEX_FIND;


int KLS_regexMatchSpecial(KLS_t_REGEX_FIND *f){
    switch(f->last->key){
        case '.': return 1;
        case '^': return f->s==f->begin;
        case '&': return f->s==f->end;
        case 'u': return isupper(*f->s);
        case 'l': return islower(*f->s);
        case 'a': return isalpha(*f->s);
        case 'A': return !isalpha(*f->s);
        case 'c': return iscntrl(*f->s);
        case 'C': return !iscntrl(*f->s);
        case 'd': return isdigit(*f->s);
        case 'D': return !isdigit(*f->s);
        case 'p': return ispunct(*f->s);
        case 'P': return !ispunct(*f->s);
        case 'w': return isalnum(*f->s) || *f->s=='_';
        case 'W': return !(isalnum(*f->s) || *f->s=='_');
        case 'x': return isxdigit(*f->s);
        case 'X': return !isxdigit(*f->s);
    }
    return 0;
}

KLS_byte KLS_regexMatchSet(KLS_t_REGEX_FIND *f){
    const KLS_t_REGEX *l=f->last;
    const KLS_byte ret=!(f->last++)->key;
    do{
        switch(f->last->type){
            case KLS_REGEX_CHAR:{
                const KLS_byte c=*f->s;
                if( (f->last->key>255 ? (c>=((char*)&f->last->key)[0] && c<=((char*)&f->last->key)[2]) : (c==f->last->key)) ){
                    f->last=l; return ret;
                } break;
            }
            case KLS_REGEX_SPECIAL:
                if(KLS_regexMatchSpecial(f)){
                    f->last=l; return ret;
                } break;
        }
    }while((f->last=f->last->alt));
    f->last=l;
    return !ret;
}

KLS_byte KLS_regexFindGroup(void){
    return 0;
}

const char *KLS_regexFind(const void *regex,const char *begin,const char *end,KLS_t_REGEX_CAP *cap){
    return NULL;
}

