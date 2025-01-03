
// LOOK at ./os_dep

int KLS_fsContentCount(const char *directory){
    DIR *dir=opendir(directory);
    if(dir){
        struct dirent *entry;
        int cnt=0;
        while( (entry=readdir(dir)) )
            cnt+=strcmp(entry->d_name,".") && strcmp(entry->d_name,"..");
        closedir(dir);
        return cnt;
    } return -1;
}

void _KLS_fsRemover(const char *n,KLS_byte t){
    t==KLS_FS_TYPE_FOLDER ? KLS_fsRemove(n) : remove(n);
}

KLS_byte KLS_fsRemove(const char *name){
    KLS_fsContentForEach(name,(void*)_KLS_fsRemover,NULL);
    return name && !remove(name);
}

KLS_byte KLS_fsContentForEach(const char *directory,void(*action)(const char *name,KLS_byte type,void *arg),void *arg){
    DIR *dir=action ? opendir(directory) : NULL;
    if(dir){
        struct dirent *entry;
        struct stat entryInfo;
        char name[PATH_MAX+1];
        const char *pattern=(directory[strlen(directory)-1]=='/' ? "%s%s" : "%s/%s" );
        KLS_byte type;
        while( (entry=readdir(dir)) )
            if(strcmp(entry->d_name,".") && strcmp(entry->d_name,"..")){
                snprintf(name,PATH_MAX,pattern,directory,entry->d_name);
                if(stat(name, &entryInfo))          type=KLS_FS_TYPE_ERROR;
                else if(S_ISDIR(entryInfo.st_mode)) type=KLS_FS_TYPE_FOLDER;
                else if(S_ISREG(entryInfo.st_mode)) type=KLS_FS_TYPE_FILE;
                #ifdef S_ISLNK
                else if(S_ISLNK(entryInfo.st_mode)) type=KLS_FS_TYPE_LINK;
                #endif
                else type=KLS_FS_TYPE_UNKNOWN;
                action(name,type,arg);
            }
        closedir(dir);
        return 1;
    } return 0;
}

KLS_byte KLS_fsInfoGet(const char *fileName,KLS_t_FS_INFO *info){
    if(info){
        struct stat data;
        if(fileName && !stat(fileName,&data)){
            info->size=data.st_size;
            info->create=data.st_ctime;
            info->access=data.st_atime;
            info->mod=data.st_mtime;
            if(S_ISDIR(data.st_mode))      info->type=KLS_FS_TYPE_FOLDER;
            else if(S_ISREG(data.st_mode)) info->type=KLS_FS_TYPE_FILE;
            #ifdef S_ISLNK
            else if(S_ISLNK(data.st_mode)) info->type=KLS_FS_TYPE_LINK;
            #endif
            else info->type=KLS_FS_TYPE_UNKNOWN;
            return 1;
        }
        info->size=0;
        info->type=KLS_FS_TYPE_ERROR;
    }
    return 0;
}

static const char *_KLS_fsInfoType(const int t){
#define _FSTP(tp) case KLS_FS_TYPE_##tp: return #tp
    switch(t){ _FSTP(ERROR);_FSTP(UNKNOWN);_FSTP(FILE);_FSTP(FOLDER);_FSTP(LINK); }
    return "ERROR";
#undef _FSTP
}

void KLS_fsInfoPrint(const KLS_t_FS_INFO *info,FILE *f){
    if(info){
        if(!f) f=stdout;
        if(info->type==KLS_FS_TYPE_ERROR){
            fprintf(f,"%s [%zu]\n",_KLS_fsInfoType(info->type),info->size);
            return;
        }
        {
            char s[3][64];
            struct datetime dt[1];
            datetime_from_epoch(dt,info->create);  datetime_string(dt,"h:m:s / D.M.Y",s[0],sizeof(*s));
            datetime_from_epoch(dt,info->access);  datetime_string(dt,"h:m:s / D.M.Y",s[1],sizeof(*s));
            datetime_from_epoch(dt,info->mod);     datetime_string(dt,"h:m:s / D.M.Y",s[2],sizeof(*s));
            fprintf(f,"%s [%zu]\n  create: %s\n  access: %s\n  mod:    %s\n",_KLS_fsInfoType(info->type),info->size,s[0],s[1],s[2]);
        }
    }
}

KLS_byte KLS_fsRuleGet(const char *path,KLS_t_FS_RULE *rule){
    struct stat info;
    if(!rule || stat(path, &info)){
        rule->owner.error=rule->groop.error=rule->other.error=1;
        return 0;
    }
#define _RULE(md,usr) ( (info.st_mode & S_I##md##usr) == S_I##md##usr )
    rule->owner.r=_RULE(R, USR); rule->owner.w=_RULE(W, USR); rule->owner.e=_RULE(X, USR);
#ifdef S_IRGRP
    rule->groop.r=_RULE(R, GRP); rule->groop.w=_RULE(W, GRP); rule->groop.e=_RULE(X, GRP);
#endif
#ifdef S_IROTH
    rule->other.r=_RULE(R, OTH); rule->other.w=_RULE(W, OTH); rule->other.e=_RULE(X, OTH);
#endif
#undef _RULE
    return 1;
}

void KLS_fsRulePrint(const KLS_t_FS_RULE *rules,FILE *f){
    if(rules){
        if(!f) f=stdout;
        fprintf(f,"owner[%c%c%c] groop[%c%c%c] other[%c%c%c]\n",
            rules->owner.r?'R':' ', rules->owner.w?'W':' ', rules->owner.e?'E':' ',
            rules->groop.r?'R':' ', rules->groop.w?'W':' ', rules->groop.e?'E':' ',
            rules->other.r?'R':' ', rules->other.w?'W':' ', rules->other.e?'E':' '
        );
    }
}
