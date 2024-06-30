/*
 line from points to ABC:
 A = y2 - y1
 B = x1 - x2
 C = y1*x2 - x1*y2
*/

double KLS_lendir_x(double len,double angle)                  { return cos(angle*M_PI/180.)*len;                }
double KLS_lendir_y(double len,double angle)                  { return sin(angle*M_PI/180.)*len;                }
double KLS_lendir_x_rad(double len,double angle_rad)          { return cos(angle_rad)*len;                      }
double KLS_lendir_y_rad(double len,double angle_rad)          { return sin(angle_rad)*len;                      }
double KLS_p2dist(double x1,double y1,double x2,double y2)    { x2-=x1; y2-=y1; return sqrt(x2*x2 + y2*y2);     }
double KLS_p2dir(double x1,double y1,double x2,double y2)     { return (180./M_PI)*(atan2(y2-y1,x2-x1));        }
double KLS_p2dir_rad(double x1,double y1,double x2,double y2) { return atan2(y2-y1,x2-x1);                      }
double KLS_angleTo360(double angle)                           { return angle - 360.*( (KLS_long)(angle/360.) ) + 360.*(angle<0.); }
double KLS_angleTo360_rad(double angle_rad)                   { return angle_rad - M_PI*2.*( (KLS_long)(angle_rad/(M_PI*2.)) ) + M_PI*2.*(angle_rad<0.); }

double KLS_interpol(double x1,double y1,double x2,double y2,double y3){
    if(fabs( (y2-=y1) )<1.e-5) return x1;
    return x1+(y3-y1)*((x2-x1)/(y2));
}

double KLS_angle2Range(double angle,double angleRange){
    angle=KLS_angleTo360(angle);
    if(angle>angleRange) angle-=360.;
    return angle;
}

double KLS_angle2Range_rad(double angle_rad,double angleRange_rad){
    angle_rad=KLS_angleTo360_rad(angle_rad);
    if(angle_rad>angleRange_rad) angle_rad-=M_PI*2.;
    return angle_rad;
}

KLS_byte KLS_angleInSector(double angle,double angleSector,double angleSectorRot){
    angle=KLS_angleTo360(angle-angleSector-angleSectorRot*(angleSectorRot<0));
    return angle>=0. && angle<=fabs(angleSectorRot);
}

KLS_byte KLS_angleInSector_rad(double angle_rad,double angleSector_rad,double angleSectorRot_rad){
    angle_rad=KLS_angleTo360_rad(angle_rad-angleSector_rad-angleSectorRot_rad*(angleSectorRot_rad<0));
    return angle_rad>=0. && angle_rad<=fabs(angleSectorRot_rad);
}

KLS_t_POINT KLS_point(double x,double y){
    return (KLS_t_POINT){x,y};
}

double KLS_point2Line(double x,double y,double x1,double y1,double x2,double y2){
    double dx=x2-x1, dy=y2-y1;
    x2=dy*x-dx*y+x2*y1-x1*y2;
    y2=sqrt(dx*dx+dy*dy);
    if(y2<1.e-5) return KLS_p2dist(x,y,x1,y1);
    return x2/y2;
}

signed char _KLS_crossLineLine(double a1,double b1,double a2,double b2,double c2,KLS_t_POINT p[static 1]){
    double dxy=a2*b1-b2*a1;
    if(fabs(dxy)<1.e-5) return -(fabs(c2)<1.e-5);
    c2/=dxy; p->x=-b1*c2; p->y=a1*c2;
    return 1;
}

signed char _KLS_crossRoundLine(double r,double a,double b,double c,KLS_t_POINT p[static 2]){
    double l=sqrt(a*a+b*b), d=-c/l;
    if(fabs(d)>r) return 0;
    a/=l; b/=l;
    p[0].x=p[1].x=d*a;
    p[0].y=p[1].y=d*b;
    d=sqrt(r*r-d*d);
    a*=d; b*=d;
    p[0].x+=b;
    p[0].y-=a;
    if(d){
        p[1].x-=b;
        p[1].y+=a;
        return 2;
    }
    return 1;
}

signed char KLS_crossRoundRound(double r1,double x1,double y1,double r2,double x2,double y2,KLS_t_POINT p[static 2]){
    double dx=x2-x1, dy=y2-y1, d=sqrt(dx*dx+dy*dy);
    if(d>(r1+r2) || d<fabs(r1-r2)) return 0;
    if(fabs(d)<1.e-5) return -!KLS_CMP(r1,r2,1.e-5);
    r1*=r1;
    x2=(r1-r2*r2+d*d)/(d*2.);
    y2=sqrt(r1-x2*x2);
    dx/=d; dy/=d;
    p[0].x=p[1].x=x1+x2*dx;
    p[0].y=p[1].y=y1+x2*dy;
    p[0].x+=y2*dy;
    p[0].y-=y2*dx;
    if(y2){
        p[1].x-=y2*dy;
        p[1].y+=y2*dx;
        return 2;
    }
    return 1;
}

signed char KLS_crossLineLine(double x11,double y11,double x12,double y12,double x21,double y21,double x22,double y22,KLS_t_POINT p[static 1]){
    signed char s; x12-=x11; x21-=x11; x22-=x11; y12-=y11; y21-=y11; y22-=y11;
    if( (s=_KLS_crossLineLine(y12,-x12,y22-y21,x21-x22,y21*x22-x21*y22,p))>0 ){
        p->x+=x11; p->y+=y11;
    } return s;
}

signed char KLS_crossSliceLine(double x11,double y11,double x12,double y12,double x21,double y21,double x22,double y22,KLS_t_POINT p[static 1]){
    signed char s; x12-=x11; x21-=x11; x22-=x11; y12-=y11; y21-=y11; y22-=y11;
    if( (s=_KLS_crossLineLine(y12,-x12,y22-y21,x21-x22,y21*x22-x21*y22,p))>0
    && KLS_pointMaskSlice(p->x,p->y,0,0,x12,y12)&14 ){
        p->x+=x11; p->y+=y11;
    } return s;
}

signed char KLS_crossSliceSlice(double x11,double y11,double x12,double y12,double x21,double y21,double x22,double y22,KLS_t_POINT p[static 1]){
    signed char s; x12-=x11; x21-=x11; x22-=x11; y12-=y11; y21-=y11; y22-=y11;
    if( (s=_KLS_crossLineLine(y12,-x12,y22-y21,x21-x22,y21*x22-x21*y22,p))>0 ){
        if(KLS_pointMaskSlice(p->x,p->y,0,0,x12,y12)&14 && KLS_pointMaskSlice(p->x,p->y,x21,y21,x22,y22)&14){
            p->x+=x11; p->y+=y11;
        }
    }else if(s<0)
        #define _KLS_CSS_CASE(_e1_,_e2_) ((_e1_)<<5) | (_e2_)
        switch(_KLS_CSS_CASE(KLS_pointMaskSlice(0,0,x21,y21,x22,y22), KLS_pointMaskSlice(x12,y12,x21,y21,x22,y22))){
            case _KLS_CSS_CASE(1,1): case _KLS_CSS_CASE(16,16): return 0;
            case _KLS_CSS_CASE(2,1): case _KLS_CSS_CASE(8,16): p->x=x11; p->y=y11; return 1;
            case _KLS_CSS_CASE(1,2): case _KLS_CSS_CASE(16,8): p->x=x12+x11; p->y=y12+y11; return 1;
        }
        #undef _KLS_CSS_CASE
    return s;
}

signed char KLS_crossRoundLine(double r,double x0,double y0,double x1,double y1,double x2,double y2,KLS_t_POINT p[static 2]){
    signed char s; x1-=x0; x2-=x0; y1-=y0; y2-=y0;
    if( (s=_KLS_crossRoundLine(r,y2-y1,x1-x2,y1*x2-x1*y2,p))>0 ){
        signed char i=0;
        while(i<s){
            p[i].x+=x0; p[i].y+=y0; ++i;
        }
    } return s;
}

signed char KLS_crossRoundSlice(double r,double x0,double y0,double x1,double y1,double x2,double y2,KLS_t_POINT p[static 2]){
    signed char s; x1-=x0; x2-=x0; y1-=y0; y2-=y0;
    if( (s=_KLS_crossRoundLine(r,y2-y1,x1-x2,y1*x2-x1*y2,p))>0 ){
        signed char i=0;
        while(i<s){
            if(KLS_pointMaskSlice(p[i].x,p[i].y,x1,y1,x2,y2)&17){
                s-=KLS_arrayRemove(p,s,sizeof(*p),i);
                continue;
            }
            p[i].x+=x0; p[i].y+=y0; ++i;
        }
    } return s;
}

signed char KLS_crossArcLine(double r,double x0,double y0,double angle_rad,double angleRot_rad,double x1,double y1,double x2,double y2,KLS_t_POINT p[static 2]){
    signed char s; x1-=x0; x2-=x0; y1-=y0; y2-=y0;
    if( (s=_KLS_crossRoundLine(r,y2-y1,x1-x2,y1*x2-x1*y2,p))>0 ){
        signed char i=0;
        while(i<s){
            if(KLS_angleInSector_rad(KLS_p2dir_rad(0.,0.,p[i].x,p[i].y),angle_rad,angleRot_rad)){
                p[i].x+=x0; p[i].y+=y0; ++i;
                continue;
            }
            s-=KLS_arrayRemove(p,s,sizeof(*p),i);
        }
    } return s;
}

signed char KLS_crossArcSlice(double r,double x0,double y0,double angle_rad,double angleRot_rad,double x1,double y1,double x2,double y2,KLS_t_POINT p[static 2]){
    signed char s; x1-=x0; x2-=x0; y1-=y0; y2-=y0;
    if( (s=_KLS_crossRoundLine(r,y2-y1,x1-x2,y1*x2-x1*y2,p))>0 ){
        signed char i=0;
        while(i<s){
            if(KLS_pointMaskSlice(p[i].x,p[i].y,x1,y1,x2,y2)&17 || !KLS_angleInSector_rad(KLS_p2dir_rad(0.,0.,p[i].x,p[i].y),angle_rad,angleRot_rad)){
                s-=KLS_arrayRemove(p,s,sizeof(*p),i);
                continue;
            }
            p[i].x+=x0; p[i].y+=y0; ++i;
        }
    } return s;
}

KLS_byte KLS_pointMaskSlice(double x,double y,double x1,double y1,double x2,double y2){
    x2-=x1; x-=x1; y2-=y1; y-=y1; x1=x-x2/2; y1=y-y2/2;
    y=sqrt(x2*x2+y2*y2);
    x=y/2.-sqrt(x1*x1+y1*y1);
    if(y<1.e-2) x*=1.e7;
    if(x>1.e-5) return 4;
    if(x<-1.e-5){
        if(x2*x1>-y2*y1) return 16;
        return 1;
    }
    if(x2*x1>-y2*y1) return 8;
    return 2;
}

KLS_byte KLS_pointInLine(double x,double y,double x1,double y1,double x2,double y2){
    return !KLS_CMP((x-x1)*(y2-y1),(y-y1)*(x2-x1),1.e-5);
}

KLS_byte KLS_pointInSlice(double x,double y,double x1,double y1,double x2,double y2){
    return KLS_pointInLine(x,y,x1,y1,x2,y2) && KLS_pointMaskSlice(x,y,x1,y1,x2,y2)&14;
}

KLS_byte KLS_pointInRay(double x,double y,double x1,double y1,double x2,double y2){
    return KLS_pointInLine(x,y,x1,y1,x2,y2) && KLS_pointMaskSlice(x,y,x1,y1,x2,y2)&30;
}

KLS_byte KLS_pointInPoly(double x,double y,const KLS_t_POINT *poly,int count){
    int i,j;
    double angle=0.,ax,ay,bx,by,ang;
    if(!memcmp(poly,poly+count-1,sizeof(KLS_t_POINT)))
        --count;
    for (i=0,j=count-1;i<count;j=i,++i){
        if(KLS_pointInSlice(x,y, poly[i].x,poly[i].y,poly[j].x,poly[j].y))
            return 2;
        ax=poly[j].x-x;
        ay=poly[j].y-y;
        bx=poly[i].x-x;
        by=poly[i].y-y;
        if( (ang=(ax*bx+ay*by)/(sqrt(ax*ax+ay*ay)*sqrt(bx*bx+by*by))) <= 1. )
            angle+=KLS_SIGN(ax*by-ay*bx)*acos(ang)*180./M_PI;
    }
    return !KLS_CMP(fabs(angle/360.),1.,1.e-5);
}

double KLS_polySquad(const KLS_t_POINT *poly,int len){
    double res=0.;
    unsigned int i;
    for(i=0,--len;i<len;++i)
        res+=(poly[i+1].x+poly[i].x)*(poly[i+1].y-poly[i].y);
    res+=(poly[0].x+poly[len].x)*(poly[0].y-poly[len].y);
    return fabs(res)/2.;
}

int _KLS_polyBounceCheck(const KLS_t_POINT *poly,int st, int end,double delta){
    KLS_t_POINT p[1]={};
    double dir=KLS_p2dir(poly[st].x,poly[st].y,poly[end].x,poly[end].y)+90.,
        x=KLS_lendir_x(100.,dir), y=KLS_lendir_y(100.,dir),distPrev=-1.,dist;
    int i,max=0;
    for(i=st+1;i<end;++i){
        if(KLS_crossLineLine(poly[i].x,poly[i].y,poly[i].x+x,poly[i].y+y,poly[st].x,poly[st].y,poly[end].x,poly[end].y,p)>0){
            dist=KLS_p2dist(p[0].x,p[0].y,poly[i].x,poly[i].y);
            if(dist>=delta && dist>distPrev){
                max=i;
                distPrev=dist;
            }
        }
    }
    return max;
}

KLS_t_VECTOR KLS_polyBounce(const KLS_t_POINT *poly,int len,double delta){
    int i=0,j=len-1;
    KLS_t_VECTOR v=KLS_vectorNew(0,sizeof(KLS_t_POINT),NULL);
    if(!poly || len<2) return v;
    j-=!memcmp(poly,poly+j,sizeof(KLS_t_POINT));
    KLS_vectorPushBack(&v,&i);
    KLS_vectorPushBack(&v,&j);
    for(i=1;i<v.size;++i)
        if((j=_KLS_polyBounceCheck(poly,*(int*)KLS_vectorAt(&v,i-1),*(int*)KLS_vectorAt(&v,i),delta))){
            KLS_vectorInsert(&v,i,&j);
            i=0;
        }
    for(i=0;i<v.size;++i){
        KLS_t_POINT *ptr=KLS_vectorAt(&v,i);
        *ptr=poly[*(int*)ptr];
    }
    return v;
}

KLS_t_VECTOR KLS_polyBounce_v2(const KLS_t_POINT *poly,int len,double delta){
    int st=0,end=2;
    KLS_t_VECTOR v=KLS_vectorNew(0,sizeof(KLS_t_POINT),NULL);
    if(!poly || len<2) return v;
    len-=!memcmp(poly,poly+len-1,sizeof(KLS_t_POINT));
    KLS_vectorPushBack(&v,poly);
    while(end<len){
        if(_KLS_polyBounceCheck(poly,st,end,delta)){
            st=(end++)-1;
            KLS_vectorPushBack(&v,poly+st);
            continue;
        }
        ++end;
    }
    KLS_vectorPushBack(&v,poly+len-1);
    return v;
}

KLS_byte KLS_polyIsConcave(const KLS_t_POINT *poly,int len){
    KLS_t_POINT unused;
    int i,j,m,n,k,cnt;
    if(!memcmp(poly,poly+len-1,sizeof(KLS_t_POINT)))
        --len;
    for(cnt=len-3,i=0;i<len;++i){
        m=(i+1)%len;
        for(j=0;j<cnt;++j){
            n=(j+i+2)%len;
            k=(j+i+3)%len;
            if(KLS_crossSliceLine(poly[n].x,poly[n].y,poly[k].x,poly[k].y, poly[i].x,poly[i].y,poly[m].x,poly[m].y,&unused)>0)
                return 1;
        }
    }
    return 0;
}

KLS_byte KLS_polyIsIntersected(const KLS_t_POINT *poly,int len){
    KLS_t_POINT unused;
    int i,j,m,n,k,cnt;
    if(!memcmp(poly,poly+len-1,sizeof(KLS_t_POINT)))
        --len;
    for(cnt=len-3,i=0;i<len;++i){
        m=(i+1)%len;
        for(j=0;j<cnt;++j){
            n=(j+i+2)%len;
            k=(j+i+3)%len;
            if(KLS_crossSliceSlice(poly[n].x,poly[n].y,poly[k].x,poly[k].y, poly[i].x,poly[i].y,poly[m].x,poly[m].y,&unused)>0)
                return 1;
        }
    }
    return 0;
}

KLS_t_POINT KLS_polyCentroid(const KLS_t_POINT *poly,int len){
    int i,j;
    double s=0.0,tmp;
    KLS_t_POINT p={0.0, 0.0};
    if(!memcmp(poly,poly+len-1,sizeof(KLS_t_POINT)))
        --len;
    for(i=0,j=len-1;i<len;j=i,++i){
        tmp=poly[j].x*poly[i].y-poly[i].x*poly[j].y;
        p.x+=(poly[j].x+poly[i].x)*tmp;
        p.y+=(poly[j].y+poly[i].y)*tmp;
        s+=tmp;
    }
    if(s){ s*=3.0; p.x/=s; p.y/=s; return p; }
    return poly[0];
}

KLS_t_VECTOR KLS_polyStretch(const KLS_t_POINT *poly,int len,double delta){
    KLS_t_VECTOR v={NULL,};
    if(poly && len>2){
        signed char znak=KLS_SIGN(delta),c;
        int i,j,k;
        double dir[2];
        KLS_t_POINT p[4];
        delta=fabs(delta);
        v=KLS_vectorNew(len,sizeof(*poly),NULL);
        // check sign of stretching
        for(i=0,j=len-1,k=1;i<len;j=i,++i,k=(k+1)%len){
            dir[0]=KLS_p2dir_rad(poly[j].x,poly[j].y,poly[i].x,poly[i].y)+M_PI_2;
            dir[1]=KLS_p2dir_rad(poly[i].x,poly[i].y,poly[k].x,poly[k].y)+M_PI_2;
            if(KLS_mod(dir[0]-dir[1],M_PI)){
                p[3].x=cos(dir[0]); p[3].y=sin(dir[0]);
                p[0]=KLS_point(poly[j].x+p[3].x,poly[j].y+p[3].y);
                p[1]=KLS_point(poly[i].x+p[3].x,poly[i].y+p[3].y);
                dir[0]=cos(dir[1]); dir[1]=sin(dir[1]);
                p[2]=KLS_point(poly[i].x+dir[0],poly[i].y+dir[1]);
                p[3]=KLS_point(poly[k].x+dir[0],poly[k].y+dir[1]);
                KLS_crossLineLine(p[0].x,p[0].y,p[1].x,p[1].y, p[2].x,p[2].y,p[3].x,p[3].y,p);
                if(KLS_pointInPoly(p->x,p->y,poly,len))
                    znak*=-1;
                break;
            }
        }
        //stretch
        for(i=0,j=len-1,k=1;i<len;j=i,++i,k=(k+1)%len){
            dir[0]=KLS_p2dir_rad(poly[j].x,poly[j].y,poly[i].x,poly[i].y)+M_PI_2*znak;
            dir[1]=KLS_p2dir_rad(poly[i].x,poly[i].y,poly[k].x,poly[k].y)+M_PI_2*znak;
            p[0]=KLS_point(poly[j].x+KLS_lendir_x_rad(delta,dir[0]),poly[j].y+KLS_lendir_y_rad(delta,dir[0]));
            p[1]=KLS_point(poly[i].x+KLS_lendir_x_rad(delta,dir[0]),poly[i].y+KLS_lendir_y_rad(delta,dir[0]));
            p[2]=KLS_point(poly[i].x+KLS_lendir_x_rad(delta,dir[1]),poly[i].y+KLS_lendir_y_rad(delta,dir[1]));
            p[3]=KLS_point(poly[k].x+KLS_lendir_x_rad(delta,dir[1]),poly[k].y+KLS_lendir_y_rad(delta,dir[1]));
            if( (c=KLS_crossLineLine(p[0].x,p[0].y,p[1].x,p[1].y, p[2].x,p[2].y,p[3].x,p[3].y,p))>-1 ){
                if(c){
                    KLS_vectorPushBack(&v,p);
                    continue;
                }
                dir[0]-=M_PI_2*znak;
                dir[1]+=M_PI_2*znak;
                p[0]=KLS_point(p[1].x+KLS_lendir_x_rad(delta,dir[0]),p[1].y+KLS_lendir_y_rad(delta,dir[0]));
                p[1]=KLS_point(p[2].x+KLS_lendir_x_rad(delta,dir[1]),p[2].y+KLS_lendir_y_rad(delta,dir[1]));
                KLS_vectorPushBack(&v,p);
                KLS_vectorPushBack(&v,p+1);
            }
        }
    }
    return v;
}


typedef struct{
    KLS_t_POINT min,max;
}_KLS_t_POLY_ENVELOPE;

typedef struct{
    KLS_t_POINT c;
    double h,d,max;
}_KLS_t_POLY_CELL;

double _KLS_polyLabelSegDistSq(const KLS_t_POINT *p,const KLS_t_POINT *a,const KLS_t_POINT *b){
    KLS_t_POINT c=*a;
    double dx=b->x-c.x, dy=b->y-c.y;
    if(dx || dy){
        double t=((p->x-c.x)*dx+(p->y-c.y)*dy)/(dx*dx+dy*dy);
        if(t>1.0){
            c=*b;
        }else if(t>0.0){
            c.x+=dx*t;
            c.y+=dy*t;
        }
    }
    dx=p->x-c.x;
    dy=p->y-c.y;
    return dx*dx+dy*dy;
}

double _KLS_polyLabelPointDist(const KLS_t_POINT *p,const KLS_t_POINT *poly,int len){
    int i,j;
    double min=1.0/0.0;
    for(i=0,j=len-1;i<len;j=i++)
        min=KLS_MIN(min,_KLS_polyLabelSegDistSq(p,poly+i,poly+j));
    return ((KLS_pointInPoly(p->x,p->y,poly,len)==1)?1:-1)*sqrt(min);
}

_KLS_t_POLY_CELL _KLS_polyLabelMakeCell(double x, double y,double h,const KLS_t_POINT *poly,int len){
    _KLS_t_POLY_CELL cell={{x,y},h};
    cell.d=_KLS_polyLabelPointDist(&cell.c,poly,len);
    cell.max=cell.d+cell.h*1.41421356237;
    return cell;
}

_KLS_t_POLY_CELL _KLS_polyLabelCellCentroid(const KLS_t_POINT *poly,int len){
    KLS_t_POINT p=KLS_polyCentroid(poly,len);
    return _KLS_polyLabelMakeCell(p.x,p.y,0,poly,len);
}

_KLS_t_POLY_ENVELOPE _KLS_polyLabelEnvelope(const KLS_t_POINT *poly,int len){
    int i;
    const KLS_t_POINT *p;
    _KLS_t_POLY_ENVELOPE env={*poly,*poly};
    for(i=0;i<len;++i){
        p=poly+i;
        if(p->x>env.max.x) env.max.x=p->x;
        if(p->y>env.max.y) env.max.y=p->y;
        if(p->x<env.min.x) env.min.x=p->x;
        if(p->y<env.min.y) env.min.y=p->y;
    }
    return env;
}

KLS_byte _KLS_polyLabelCmp(const _KLS_t_POLY_CELL *prev,const _KLS_t_POLY_CELL *next){
    return prev->max > next->max;
}

KLS_t_POINT KLS_polyLabel(const KLS_t_POINT *poly,int len, double delta){
    _KLS_t_POLY_ENVELOPE env=_KLS_polyLabelEnvelope(poly,len);
    KLS_t_POINT p=env.min, size={env.max.x-env.min.x, env.max.y-env.min.y};
    double x,y,cellSize=KLS_MIN(size.x,size.y),h=cellSize/2.0;
    KLS_t_VECTOR v=KLS_vectorNew(10,sizeof(_KLS_t_POLY_CELL),NULL);
    _KLS_t_POLY_CELL bestCell=_KLS_polyLabelCellCentroid(poly,len), cell=_KLS_polyLabelMakeCell(env.min.x+size.x/2.0,env.min.y+size.y/2.0,0,poly,len), buff;

    if(cell.d>bestCell.d)
        bestCell=cell;

    if(cellSize){
        for(x=env.min.x;x<env.max.x;x+=cellSize)
            for(y=env.min.y;y<env.max.y;y+=cellSize){
                buff=_KLS_polyLabelMakeCell(x+h,y+h,h,poly,len);
                KLS_vectorPushBack(&v,&buff);
            }
        KLS_arraySort(v.data,v.size,v.sizeElement,(void*)_KLS_polyLabelCmp,NULL);

        while(v.size){
            cell=*(_KLS_t_POLY_CELL*)KLS_vectorAt(&v,v.size-1);
            KLS_vectorRemove(&v,v.size-1);
            if(cell.d>bestCell.d)
                bestCell=cell;
            if(cell.max-bestCell.d<=delta)
                continue;
            h=cell.h/2.0;
            buff=_KLS_polyLabelMakeCell(cell.c.x-h,cell.c.y-h,h,poly,len);
            KLS_vectorPushBack(&v,&buff);
            buff=_KLS_polyLabelMakeCell(cell.c.x+h,cell.c.y-h,h,poly,len);
            KLS_vectorPushBack(&v,&buff);
            buff=_KLS_polyLabelMakeCell(cell.c.x-h,cell.c.y+h,h,poly,len);
            KLS_vectorPushBack(&v,&buff);
            buff=_KLS_polyLabelMakeCell(cell.c.x+h,cell.c.y+h,h,poly,len);
            KLS_vectorPushBack(&v,&buff);
            KLS_arraySort(v.data,v.size,v.sizeElement,(void*)_KLS_polyLabelCmp,NULL);
        }
        p=bestCell.c;
    }
    KLS_vectorFree(&v);
    return p;
}



typedef struct{
    char r,c;
}_KLS_t_MATRIX_DIR;

static const _KLS_t_MATRIX_DIR _KLS_matrixPolyDirs[8]={ {-1,0}, {-1,-1}, {0,-1}, {1,-1}, {1,0}, {1,1}, {0,1}, {-1,1} };

void _KLS_matrixGetPolyDir(const KLS_t_MATRIX *matrix,int row,int column,const void *element,KLS_byte *arr,KLS_byte dirPrev,KLS_byte(*comparator)(const void*,...)){
    KLS_byte i,end=dirPrev+8;
    for(i=dirPrev;i<end;++i){
        void *ptr=KLS_matrixAt(matrix,row+_KLS_matrixPolyDirs[i%8].r,column+_KLS_matrixPolyDirs[i%8].c);
        arr[i-dirPrev]=ptr && comparator(ptr,element,matrix->elSize);
    }
}

KLS_byte _KLS_matrixGetPolyDir4(const KLS_t_MATRIX *matrix,int row,int column,const void *element,KLS_byte dirPrev,void *comparator){
    KLS_byte fnd[8];
    _KLS_matrixGetPolyDir(matrix,row,column,element,fnd,dirPrev,comparator);
    if(fnd[0]){
        if(fnd[2]) dirPrev+=2;
        else dirPrev+=0;
    }else{
        if(fnd[2])
            dirPrev+=2;
        else{
            if(fnd[6]) dirPrev+=6;
            else dirPrev+=4;
        }
    }
    return dirPrev%8;
}

KLS_byte _KLS_matrixGetPolyDir8(const KLS_t_MATRIX *matrix,int row,int column,const void *element,KLS_byte dirPrev,void *comparator){
    KLS_byte fnd[8];
    _KLS_matrixGetPolyDir(matrix,row,column,element,fnd,dirPrev,comparator);
    if(fnd[0]){
        if(fnd[2]) dirPrev+=2;
        else if(fnd[1]) dirPrev+=1;
        else dirPrev+=0;
    }else{
        if(fnd[2]) dirPrev+=2;
        else if(fnd[1]) dirPrev+=1;
        else{
            if(fnd[7]) dirPrev+=7;
            else if(fnd[6]) dirPrev+=6;
            else if(fnd[5]) dirPrev+=5;
            else dirPrev+=4;
        }
    }
    return dirPrev%8;
}

KLS_byte _KLS_polyMemCmp(const void *p1,const void *p2,size_t n){ return !memcmp(p1,p2,n); }

KLS_t_VECTOR KLS_polyFromMatrix(const KLS_t_MATRIX *matrix,int row,int column,const void *element,KLS_byte chainMode,KLS_byte(*comparator)(const void *matrixElement,const void *element)){
    KLS_t_VECTOR v={0};
    if(matrix && (element || comparator)){
        void *ptr;
        int r=row, c=column;
        KLS_t_POINT p;
        KLS_byte (*getDir)(const KLS_t_MATRIX*,int,int,const void*,KLS_byte,void*) = (chainMode>7)?_KLS_matrixGetPolyDir8:_KLS_matrixGetPolyDir4;
        KLS_byte dir=getDir(matrix,r,c,element,0,comparator), dirNext;
        KLS_byte (*_cmp)(const void*,...)=comparator ? (void*)comparator : (void*)_KLS_polyMemCmp;
        v=KLS_vectorNew(40,sizeof(KLS_t_POINT),NULL);
        if(KLS_matrixAt(matrix,r,c)) do{
            p=KLS_point(c,r);
            KLS_vectorPushBack(&v,&p);
            do{
                r+=_KLS_matrixPolyDirs[dir].r;
                c+=_KLS_matrixPolyDirs[dir].c;
                ptr=KLS_matrixAt(matrix,r,c);
                if(ptr && !_cmp(ptr,element,matrix->elSize)){
                    KLS_vectorPushBack(&v,KLS_vectorAt(&v,v.size-1));
                    return v;
                }
                if(r==row && c==column)
                    return v;
                dirNext=getDir(matrix,r,c,element,dir,_cmp);
            }while(dirNext==dir);
            dir=dirNext;
        }while(!(r==row && c==column));
    }
    return v;
}

KLS_t_VECTOR KLS_polyFromMatrixAll(const KLS_t_MATRIX *matrix,const void *element,KLS_byte chainMode,KLS_byte(*comparator)(const void *matrixElement,const void *element)){
    KLS_t_VECTOR ret={0};
    if(matrix && (element || comparator)){
        int i,j,k,flag;
        KLS_byte (*_cmp)(const void*,...)=comparator ? (void*)comparator : (void*)_KLS_polyMemCmp;
        ret=KLS_vectorNew(1,sizeof(KLS_t_VECTOR),(void*)KLS_vectorFree);
        for(i=0;i<matrix->rows;++i)
            for(j=0;j<matrix->columns;++j){
                void *ptr=KLS_matrixAt(matrix,i,j);
                if(ptr && _cmp(ptr,element,matrix->elSize)){
                    for(flag=1,k=0;k<ret.size;++k){
                        KLS_t_VECTOR *vptr=KLS_vectorAt(&ret,k);
                        if(KLS_pointInPoly(j,i,vptr->data,vptr->size)){
                            flag=0;
                            break;
                        }
                    }
                    if(flag){
                        KLS_t_VECTOR v=KLS_polyFromMatrix(matrix,i,j,element,chainMode,(void*)_cmp);
                        KLS_vectorPushBack(&ret,&v);
                    }
                }
            }
    }
    return ret;
}

void KLS_pointsPrint(const KLS_t_POINT *p,unsigned int count,FILE *f){
    if(!f) f=stdout;
    if(count==-1){fprintf(f,"inf\n");return;}
    fprintf(f,"%u\n",count);
    while(count--)
        fprintf(f,"(%f, %f)\n",p[count].x, p[count].y);
}
