
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

void _KLS_fsRemover(const char *n,KLS_byte t,void *arg){
    t==KLS_FS_TYPE_FOLDER ? KLS_fsRemove(n) : remove(n);
}

KLS_byte KLS_fsRemove(const char *name){
    KLS_fsContentForEach(name,_KLS_fsRemover,NULL);
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
    struct stat data;
    if(info && fileName && !stat(fileName,&data)){
        info->size=data.st_size;
        info->create=KLS_dateTimeFrom(data.st_ctime);
        info->access=KLS_dateTimeFrom(data.st_atime);
        info->mod=KLS_dateTimeFrom(data.st_mtime);
        if(S_ISDIR(data.st_mode))      info->type=KLS_FS_TYPE_FOLDER;
        else if(S_ISREG(data.st_mode)) info->type=KLS_FS_TYPE_FILE;
#ifdef S_ISLNK
        else if(S_ISLNK(data.st_mode)) info->type=KLS_FS_TYPE_LINK;
#endif
        else info->type=KLS_FS_TYPE_UNKNOWN;
        return 1;
    }
    if(info) info->type=KLS_FS_TYPE_ERROR;
    return 0;
}

void KLS_fsInfoPrint(const KLS_t_FS_INFO *info,FILE *f){
#define _FSTP(tp) [KLS_FS_TYPE_##tp]=#tp
    const char *types[]={_FSTP(ERROR),_FSTP(UNKNOWN),_FSTP(FILE),_FSTP(FOLDER),_FSTP(LINK)};
#undef _FSTP
    if(info){
        if(!f) f=stdout;
        if(info->type==KLS_FS_TYPE_ERROR){
            fprintf(f,"%s",types[info->type]);
            return;
        }
        fprintf(f,"%s [" KLS_FORMAT_LONG(u) "]",types[info->type],info->size);
        fprintf(f,"\n  create: "); KLS_dateTimePrint(&info->create,f);
        fprintf(f,"\n  access: "); KLS_dateTimePrint(&info->access,f);
        fprintf(f,"\n  mod:    "); KLS_dateTimePrint(&info->mod,f);
        fprintf(f,"\n");
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
        fprintf(f,"owner[%c%c%c] ", rules->owner.r?'R':' ', rules->owner.w?'W':' ', rules->owner.e?'E':' ');
        fprintf(f,"groop[%c%c%c] ", rules->groop.r?'R':' ', rules->groop.w?'W':' ', rules->groop.e?'E':' ');
        fprintf(f,"other[%c%c%c]", rules->other.r?'R':' ', rules->other.w?'W':' ', rules->other.e?'E':' ');
    }
}
