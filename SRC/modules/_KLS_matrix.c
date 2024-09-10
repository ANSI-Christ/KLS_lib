
KLS_t_MATRIX KLS_matrixNew(void *data,unsigned int countRow,unsigned int countColumn,unsigned int elementSize,void(*deleter)(void *element)){
    KLS_t_MATRIX m={data?data:KLS_malloc(((KLS_size)countRow)*countColumn*elementSize),deleter,countRow,countColumn,elementSize,{countRow,countColumn,countRow,countColumn},0,0,0,!data};
    if(!data && m.data && deleter) memset(m.data,0,((KLS_size)countRow)*countColumn*elementSize);
    return m;
}

KLS_t_MATRIX KLS_matrixGetMatrix(const KLS_t_MATRIX *matrix,int row,int column,unsigned int rows,unsigned int columns,KLS_byte options){
    KLS_t_MATRIX m={0};
    if(matrix && matrix->data){
        m=*matrix;
        m.rows=rows;
        m.columns=columns;
        m.subRow+=row;
        m.subColumn+=column;
        // FIX ME write easier
        if(m.options&KLS_MATRIX_SUBUNLIM_H){
            m._.bc=m.subColumn<0?0:m.subColumn;
            if(m.subColumn + columns < m._.rc) m._.bw=columns;
            else m._.bw = m._.rc-m._.bc;
            if((int)m._.bw<0) m._.bw=0;
        }else{
            if((int)m._.bc<m.subColumn) m._.bc=m.subColumn;
            m._.bw=columns+m.subColumn;
            if(m._.bw>(columns=matrix->_.bw+matrix->_.bc)) m._.bw=columns;
            if((int)(m._.bw-=m._.bc)<0) m._.bw=0;
        }
        if(m.options&KLS_MATRIX_SUBUNLIM_V){
            m._.br=m.subRow<0?0:m.subRow;
            if (m.subRow + rows < m._.rr)m._.bh=rows;
            else m._.bh = m._.rr - m._.br;
            if((int)m._.bh<0) m._.bh=0;
        }else{
            if((int)m._.br<m.subRow) m._.br=m.subRow;
            m._.bh=rows+m.subRow;
            if(m._.bh>(rows=matrix->_.bh+matrix->_.br)) m._.bh=rows;
            if((int)(m._.bh-=m._.br)<0) m._.bh=0;
        }
        m.options=16|options;
        m._free=0;
    }
    return m;
}

void KLS_matrixFree(KLS_t_MATRIX *matrix){
    if(!matrix)return;
    if(matrix->deleter && matrix->data){
        void *ptr=matrix->data;
        KLS_size cnt=(KLS_size)matrix->_.rr*matrix->_.rc;
        while(cnt){
            matrix->deleter(ptr);
            ptr+=matrix->elSize;
            --cnt;
        }
    }
    if(matrix->_free) KLS_freeData(matrix->data);
    memset(matrix,0,sizeof(KLS_t_MATRIX));
}

int _KLS_matrixLoop(int val,int size){
    val%=size; if(val<0) val+=size; return val;
}

void *KLS_matrixAt_____(const KLS_t_MATRIX *m,unsigned int r,unsigned int c){
    if (r<m->rows && c<m->columns)
        return m->data+(r*m->_.rc + c)*m->elSize;
    return NULL;
}

void *KLS_matrixAt____c(const KLS_t_MATRIX *m,unsigned int r,int c){
    if(r<m->rows)
        return m->data+(r*m->_.rc+_KLS_matrixLoop(c,m->columns))*m->elSize;
    return NULL;
}

void *KLS_matrixAt___r_(const KLS_t_MATRIX *m,int r,unsigned int c){
    if(c<m->columns)
        return m->data+(_KLS_matrixLoop(r,m->rows)*m->_.rc+c)*m->elSize;
    return NULL;
}

void *KLS_matrixAt___rc(const KLS_t_MATRIX *m,int r,int c){
    return m->data+(_KLS_matrixLoop(r,m->rows)*m->_.rc+_KLS_matrixLoop(c,m->columns))*m->elSize;
}

void *KLS_matrixAtsvh__(const KLS_t_MATRIX *m,unsigned int r,unsigned int c){
    if(r<m->rows && c<m->columns)
        return m->data+(_KLS_matrixLoop(r+m->subRow,m->_.rr)*m->_.rc+_KLS_matrixLoop(c+m->subColumn,m->_.rc))*m->elSize;
    return NULL;
}

void *KLS_matrixAtsvh_c(const KLS_t_MATRIX *m,unsigned int r,int c){
    if(r<m->rows){
        c=_KLS_matrixLoop(c,m->columns);
        return m->data+(_KLS_matrixLoop(r+m->subRow,m->_.rr)*m->_.rc+_KLS_matrixLoop(c+m->subColumn,m->_.rc))*m->elSize;
    } return NULL;
}

void *KLS_matrixAtsvhr_(const KLS_t_MATRIX *m,int r,unsigned int c){
    if(c<m->columns){
        r=_KLS_matrixLoop(r,m->rows);
        return m->data+(_KLS_matrixLoop(r+m->subRow,m->_.rr)*m->_.rc+_KLS_matrixLoop(c+m->subColumn,m->_.rc))*m->elSize;
    } return NULL;
}

void *KLS_matrixAtsvhrc(const KLS_t_MATRIX *m,int r,int c){
    r=_KLS_matrixLoop(r,m->rows);
    c=_KLS_matrixLoop(c,m->columns);
    return m->data+(_KLS_matrixLoop(r+m->subRow,m->_.rr)*m->_.rc+_KLS_matrixLoop(c+m->subColumn,m->_.rc))*m->elSize;
}

void *KLS_matrixAtsv___(const KLS_t_MATRIX *m,unsigned int r,unsigned int c){
    if(r<m->rows && c<m->columns){
        r+=m->subRow;
        if(r-m->_.br<m->_.bh)
            return m->data+(r*m->_.rc+_KLS_matrixLoop(c+m->subColumn,m->_.rc))*m->elSize;
    } return NULL;
}

void *KLS_matrixAtsv__c(const KLS_t_MATRIX *m,unsigned int r,int c){
    if(r<m->rows){
        r+=m->subRow;
        if(r-m->_.br<m->_.bh){
            c=_KLS_matrixLoop(c,m->columns)+m->subColumn;
            return m->data+(r*m->_.rc+_KLS_matrixLoop(c,m->_.rc))*m->elSize;
    }} return NULL;
}

void *KLS_matrixAtsv_r_(const KLS_t_MATRIX *m,int r,unsigned int c){
    if(c<m->columns){
        r=_KLS_matrixLoop(r,m->rows)+m->subRow;
        if(r-m->_.br<m->_.bh)
            return m->data+(r*m->_.rc+_KLS_matrixLoop(c+m->subColumn,m->_.rc))*m->elSize;
    } return NULL;
}

void *KLS_matrixAtsv_rc(const KLS_t_MATRIX *m,int r,int c){
    r=_KLS_matrixLoop(r,m->rows)+m->subRow;
    if(r - m->_.br < m->_.bh){
        c=_KLS_matrixLoop(c,m->columns)+m->subColumn;
        return m->data+(r*m->_.rc+_KLS_matrixLoop(c,m->_.rc))*m->elSize;
    } return NULL;
}

void *KLS_matrixAts_h__(const KLS_t_MATRIX *m,unsigned int r,unsigned int c){
    if(r<m->rows && c<m->columns){
        c+=m->subColumn;
        if(c-m->_.bc<m->_.bw)
            return m->data+(_KLS_matrixLoop(r+m->subRow,m->_.rr)*m->_.rc+c)*m->elSize;
    } return NULL;
}

void *KLS_matrixAts_h_c(const KLS_t_MATRIX *m,unsigned int r,int c){
    if(r<m->rows){
        c=_KLS_matrixLoop(c,m->columns)+m->subColumn;
        if(c-m->_.bc<m->_.bw)
            return m->data+(_KLS_matrixLoop(r+m->subRow,m->_.rr)*m->_.rc+c)*m->elSize;
    } return NULL;
}

void *KLS_matrixAts_hr_(const KLS_t_MATRIX *m,int r,unsigned int c){
    if(c<m->columns){
        c+=m->subColumn;
        if(c-m->_.bc<m->_.bw){
            r=_KLS_matrixLoop(r,m->rows)+m->subRow;
            return m->data+(_KLS_matrixLoop(r,m->_.rr)*m->_.rc+c)*m->elSize;
    }} return NULL;
}

void *KLS_matrixAts_hrc(const KLS_t_MATRIX *m,int r,int c){
    c=_KLS_matrixLoop(c,m->columns)+m->subColumn;
    if(c - m->_.bc<m->_.bw){
        r=_KLS_matrixLoop(r,m->rows)+m->subRow;
        return m->data+(_KLS_matrixLoop(r,m->_.rr)*m->_.rc+c)*m->elSize;
    } return NULL;
}

void *KLS_matrixAts____(const KLS_t_MATRIX *m,unsigned int r,unsigned int c){
    if(r<m->rows && c<m->columns){
        r+=m->subRow;
        if(r-m->_.br<m->_.bh){
            c+=m->subColumn;
            if(c-m->_.bc<m->_.bw)
                return m->data+(r*m->_.rc+c)*m->elSize;
    }} return NULL;
}

void *KLS_matrixAts___c(const KLS_t_MATRIX *m,unsigned int r,int c){
    if(r<m->rows){
        r+=m->subRow;
        if(r-m->_.br<m->_.bh){
            c=_KLS_matrixLoop(c,m->columns)+m->subColumn;
            if(c-m->_.bc<m->_.bw)
                return m->data+(r*m->_.rc+c)*m->elSize;
    }} return NULL;
}

void *KLS_matrixAts__r_(const KLS_t_MATRIX *m,int r,unsigned int c){
    if(c>=m->columns){
        c+=m->subColumn;
        if(c-m->_.bc<m->_.bw){
            r=_KLS_matrixLoop(r,m->rows)+m->subRow;
            if(r-m->_.br < m->_.bh)
                return m->data + (r*m->_.rc+c)*m->elSize;
    }} return NULL;
}

void *KLS_matrixAts__rc(const KLS_t_MATRIX *m,int r,int c){
    r=_KLS_matrixLoop(r,m->rows)+m->subRow;
    if (r-m->_.br<m->_.bh) {
        c=_KLS_matrixLoop(c,m->columns)+m->subColumn;
        if (c-m->_.bc<m->_.bw)
            return m->data + (r*m->_.rc+c)*m->elSize;
    } return NULL;
}

static void *(*_KLS_matrixAt[])(const KLS_t_MATRIX *matrix,int row,int column)={
    (void*)KLS_matrixAt_____,(void*)KLS_matrixAt____c,(void*)KLS_matrixAt___r_,(void*)KLS_matrixAt___rc,
    (void*)KLS_matrixAt_____,(void*)KLS_matrixAt____c,(void*)KLS_matrixAt___r_,(void*)KLS_matrixAt___rc,
    (void*)KLS_matrixAt_____,(void*)KLS_matrixAt____c,(void*)KLS_matrixAt___r_,(void*)KLS_matrixAt___rc,
    (void*)KLS_matrixAt_____,(void*)KLS_matrixAt____c,(void*)KLS_matrixAt___r_,(void*)KLS_matrixAt___rc,
    (void*)KLS_matrixAts____,(void*)KLS_matrixAts___c,(void*)KLS_matrixAts__r_,(void*)KLS_matrixAts__rc,
    (void*)KLS_matrixAts_h__,(void*)KLS_matrixAts_h_c,(void*)KLS_matrixAts_hr_,(void*)KLS_matrixAts_hrc,
    (void*)KLS_matrixAtsv___,(void*)KLS_matrixAtsv__c,(void*)KLS_matrixAtsv_r_,(void*)KLS_matrixAtsv_rc,
    (void*)KLS_matrixAtsvh__,(void*)KLS_matrixAtsvh_c,(void*)KLS_matrixAtsvhr_,(void*)KLS_matrixAtsvhrc,
};

void *KLS_matrixAt(const KLS_t_MATRIX *matrix,int row,int column){
    return _KLS_matrixAt[matrix->options](matrix,row,column);
}

void KLS_matrixPutElement(KLS_t_MATRIX *matrix,int row, int column,const void *element){
    void *ptr=KLS_matrixAt(matrix,row,column);
    if(ptr && element){
        if(matrix->deleter) matrix->deleter(ptr);
        memcpy(ptr,element,matrix->elSize);
    }
}

void KLS_matrixPutLine(KLS_t_MATRIX *matrix,int row1,int column1,int row2,int column2,const void *element,unsigned int width){
    const int deltaX=abs(row2-row1),
            deltaY=abs(column2-column1),
            signX=row1 < row2 ? 1 : -1,
            signY=column1 < column2 ? 1 : -1;
    int error=deltaX-deltaY;
    KLS_matrixPutPoint(matrix,row2,column2,element,width);
    while(row1!=row2 || column1!=column2){
        const int error2=error<<1;
        KLS_matrixPutPoint(matrix,row1,column1,element,width);
        if(error2>-deltaY){
            error-=deltaY;
            row1+=signX;
        }
        if(error2<deltaX){
            error+=deltaX;
            column1+=signY;
        }
    }
}

void KLS_matrixPutRect(KLS_t_MATRIX *matrix,int row_1,int column_1,int row_2,int column_2,const void *element,unsigned int width,const void *fill){
    int i,j;
    if(row_2<row_1) KLS_swap(&row_1,&row_2,sizeof(int));
    if(column_2<column_1) KLS_swap(&column_1,&column_2,sizeof(int));
    if(fill)
        for(i=row_1;i<=row_2;++i)
            for(j=column_1;j<=column_2;++j)
                KLS_matrixPutElement(matrix,i,j,fill);
    for(i=row_1;i<=row_2;++i){
        KLS_matrixPutPoint(matrix,i,column_1,element,width);
        KLS_matrixPutPoint(matrix,i,column_2,element,width);
    }
    for(i=column_1;i<=column_2;++i){
        KLS_matrixPutPoint(matrix,row_1,i,element,width);
        KLS_matrixPutPoint(matrix,row_2,i,element,width);
    }
}

void _KLS_matrixPutPiece4(KLS_t_MATRIX *matrix,int row,int column,int r,int c,const void *element,unsigned int width){
    KLS_matrixPutPoint(matrix,row+r,column+c,element,width);
    KLS_matrixPutPoint(matrix,row+r,column-c,element,width);
    KLS_matrixPutPoint(matrix,row-r,column+c,element,width);
    KLS_matrixPutPoint(matrix,row-r,column-c,element,width);
}

void KLS_matrixPutRound(KLS_t_MATRIX *matrix,int row,int column,unsigned int radius,const void *element,unsigned int width,const void *fill){
    int x=0,y=radius,delta=3-2*radius;
    while(x<y) {
        _KLS_matrixPutPiece4(matrix,row,column,x,y,element,width);
        _KLS_matrixPutPiece4(matrix,row,column,y,x,element,width);
        if (delta<0) delta+=(x<<2)+6;
        else delta+=((x-(y--))<<2)+10;
        ++x;
    }
    if(x==y) _KLS_matrixPutPiece4(matrix,row,column,x,y,element,width);
    if(fill) KLS_matrixFillArea(matrix,row,column,fill,element);
}

void KLS_matrixPutEllipse(KLS_t_MATRIX *matrix,int row,int column,KLS_long rowCount,KLS_long columnCount, const void *element,unsigned int width,const void *fill){
    int r=0,c=columnCount;
    KLS_long two_a_square,two_b_square,four_a_square,four_b_square,d;
    rowCount*=rowCount;
    columnCount*=columnCount;
    two_a_square=rowCount<<1;
    four_a_square=rowCount<<2;
    four_b_square=columnCount<<2;
    two_b_square=columnCount<<1;
    d=two_a_square*((c-1)*c)+rowCount+two_b_square*(1-rowCount);
    while(rowCount*c>columnCount*r){
        _KLS_matrixPutPiece4(matrix,row,column,r,c,element,width);
        if (d>=0) d-=four_a_square*(--c);
        d+=two_b_square*(3+(r<<1)); ++r;
    }
    d=two_b_square*(r+1)*r+two_a_square*(c*(c-2)+1)+(1-two_a_square)*columnCount;
    while(c+1){
        _KLS_matrixPutPiece4(matrix,row,column,r,c,element,width);
        if(d<=0){d+=four_b_square*r;++r;}
        d+=two_a_square*(3-((--c)<<1));
    }
    if(fill) KLS_matrixFillArea(matrix,row,column,fill,element);
}

//FIX ME work slowly
void _KLS_matrixPutPiece4Angle(KLS_t_MATRIX *matrix,int row,int column,int r,int c,const void *element,unsigned int width,float angle_rad,float angleRot_rad){
    if(KLS_angleInSector_rad(atan2(-r,c),angle_rad,angleRot_rad))  KLS_matrixPutPoint(matrix,row+r,column+c,element,width);
    if(KLS_angleInSector_rad(atan2(-r,-c),angle_rad,angleRot_rad)) KLS_matrixPutPoint(matrix,row+r,column-c,element,width);
    if(KLS_angleInSector_rad(atan2(r,c),angle_rad,angleRot_rad))   KLS_matrixPutPoint(matrix,row-r,column+c,element,width);
    if(KLS_angleInSector_rad(atan2(r,-c),angle_rad,angleRot_rad))  KLS_matrixPutPoint(matrix,row-r,column-c,element,width);
}

void KLS_matrixPutArcRound(KLS_t_MATRIX *matrix,int row,int column,unsigned int radius,float angle_rad,float angleRot_rad,const void *element,unsigned int width){
    int x=0,y=radius,delta=3-2*radius;
    while(x<y) {
        _KLS_matrixPutPiece4Angle(matrix,row,column,x,y,element,width,angle_rad,angleRot_rad);
        _KLS_matrixPutPiece4Angle(matrix,row,column,y,x,element,width,angle_rad,angleRot_rad);
        if (delta<0) delta+=(x<<2)+6;
        else delta+=((x-(y--))<<2)+10;
        ++x;
    }
    if(x==y) _KLS_matrixPutPiece4Angle(matrix,row,column,x,y,element,width,angle_rad,angleRot_rad);
}

void KLS_matrixPutArcEllipse(KLS_t_MATRIX *matrix,int row,int column,KLS_long rowCount,KLS_long columnCount,float angle_rad,float angleRot_rad,const void *element,unsigned int width){
    int r=0,c=columnCount;
    KLS_long two_a_square,two_b_square,four_a_square,four_b_square,d;
    rowCount*=rowCount;
    columnCount*=columnCount;
    two_a_square=rowCount<<1;
    four_a_square=rowCount<<2;
    four_b_square=columnCount<<2;
    two_b_square=columnCount<<1;
    d=two_a_square*((c-1)*c)+rowCount+two_b_square*(1-rowCount);
    while(rowCount*c>columnCount*r){
        _KLS_matrixPutPiece4Angle(matrix,row,column,r,c,element,width,angle_rad,angleRot_rad);
        if (d>=0) d-=four_a_square*(--c);
        d+=two_b_square*(3+(r<<1)); ++r;
    }
    d=two_b_square*(r+1)*r+two_a_square*(c*(c-2)+1)+(1-two_a_square)*columnCount;
    while (c + 1){
        _KLS_matrixPutPiece4Angle(matrix,row,column,r,c,element,width,angle_rad,angleRot_rad);
        if (d<=0){d+=four_b_square*r; ++r;}
        d+=two_a_square*(3-((--c)<<1));
    }
}


int _KLS_matrixFillLine(KLS_t_MATRIX *matrix,int row,int column,int dir,int left,int right,const void *element,const void *elementOfBorder){
    int xl=column, xr=column;
    const void *c;
    while ((c=KLS_matrixAt(matrix,row,--xl)) && memcmp(c,elementOfBorder,matrix->elSize) && memcmp(c,element,matrix->elSize));
    while ((c=KLS_matrixAt(matrix,row,++xr)) && memcmp(c,elementOfBorder,matrix->elSize) && memcmp(c,element,matrix->elSize));
    for(column=++xl,--xr;column<=xr;++column)
        KLS_matrixPutElement(matrix,row,column,element);
    for (column=xl; column<=xr; ++column)
        if((c=KLS_matrixAt(matrix,row+dir,column)) && memcmp(c,elementOfBorder,matrix->elSize) && memcmp(c,element,matrix->elSize))
            _KLS_matrixFillLine(matrix,row+dir,column,dir,xl,xr,element,elementOfBorder);
    for (column=xl; column<left; ++column)
        if((c=KLS_matrixAt(matrix,row-dir,column)) && memcmp(c,elementOfBorder,matrix->elSize) && memcmp(c,element,matrix->elSize))
            _KLS_matrixFillLine(matrix,row-dir,column,-dir,xl,xr,element,elementOfBorder);
    for (column=right; column<xr; ++column)
        if((c=KLS_matrixAt(matrix,row-dir,column)) && memcmp(c,elementOfBorder,matrix->elSize) && memcmp(c,element,matrix->elSize))
            _KLS_matrixFillLine(matrix,row-dir,column,-dir,xl,xr,element,elementOfBorder);
    return xr;
}

void KLS_matrixFillArea(KLS_t_MATRIX *matrix,int row,int column,const void *element,const void *elementOfBorder){
    if( !KLS_matrixAt(matrix,row,column) || !element) return;
    if(elementOfBorder) _KLS_matrixFillLine(matrix,row,column,1,column,column,element,elementOfBorder);
    else
        for(row=0;row<matrix->rows;++row)
            for(column=0;column<matrix->columns;++column)
                KLS_matrixPutElement(matrix,row,column,element);
}

void _KLS_matrixHorLine(KLS_t_MATRIX *m,int r, int c1,int c2,const void *element){for(;c1<=c2;++c1) KLS_matrixPutElement(m,r,c1,element);}

void KLS_matrixPutPoint(KLS_t_MATRIX *matrix,int row,int column,const void *element,unsigned int width){
    if(width-->1){
        int x=0,y=width,delta=3-2*width;
        while(x<=y) {
            _KLS_matrixHorLine(matrix,row+x,column-y,column+y,element);
            _KLS_matrixHorLine(matrix,row-x,column-y,column+y,element);
            _KLS_matrixHorLine(matrix,row+y,column-x,column+x,element);
            _KLS_matrixHorLine(matrix,row-y,column-x,column+x,element);
            if (delta<0) delta+=(x<<2)+6;
            else{delta+=((x-y)<<2)+10; --y;}
            ++x;
        }
    }else KLS_matrixPutElement(matrix,row,column,element);
}

void KLS_matrixPutMatrix(KLS_t_MATRIX *matrix,int row,int column,const KLS_t_MATRIX *element,void(*putter)(void *elementDst,const void *elementSrc)){
    if(matrix && element){
        int i,j;
        void *dst;
        if(putter){
            for(i=0;i<element->rows;++i)
                for(j=0;j<element->columns;++j)
                    if( (dst=KLS_matrixAt(matrix,row+i,column+j)) )
                        putter(dst,KLS_matrixAt(element,i,j));
        }else if(matrix->elSize==element->elSize){
            for(i=0;i<element->rows;++i)
                for(j=0;j<element->columns;++j)
                    KLS_matrixPutElement(matrix,row+i,column+j,element);
        }
    }
}

void _KLS_matrixSS_BB(const KLS_t_MATRIX * const from,KLS_t_MATRIX * const to,void * const tmp,KLS_byte(*transformer)(const void *elFrom,void *elTo,void *arg),void *arg){
    unsigned int is,js,ib,jb;
    const float r=((double)to->rows)/from->rows, c=((double)to->columns)/from->columns;
    for(is=0;is<from->rows;++is){
        const unsigned int rSt=round(r*is), rEnd=round(r*(is+1));
        for(js=0;js<from->columns;++js){
            const unsigned int cSt=round(c*js), cEnd=round(c*(js+1));
            if(transformer(KLS_matrixAt(from,is,js),tmp,arg))
                for(ib=rSt;ib<rEnd;++ib)
                    for(jb=cSt;jb<cEnd;++jb)
                        KLS_matrixPutElement(to,ib,jb,tmp);
        }
    }
}

void _KLS_matrixBS_SB(const KLS_t_MATRIX * const from,KLS_t_MATRIX * const to,void * const tmp,KLS_byte(*transformer)(const void *elFrom,void *elTo,void *arg),void *arg){
    unsigned int is,js,jb;
    const float r=((double)from->rows)/to->rows, c=((double)to->columns)/from->columns;
    for(js=0;js<from->columns;++js){
        const unsigned int cSt=round(c*js), cEnd=round(c*(js+1));
        for(is=0;is<to->rows;++is)
            if(transformer(KLS_matrixAt(from,round(is*r),js),tmp,arg))
                for(jb=cSt;jb<cEnd;++jb)
                    KLS_matrixPutElement(to,is,jb,tmp);
    }
}

void _KLS_matrixSB_BS(const KLS_t_MATRIX * const from,KLS_t_MATRIX * const to,void * const tmp,KLS_byte(*transformer)(const void *elFrom,void *elTo,void *arg),void *arg){
    unsigned int is,js,ib;
    const float r=((double)to->rows)/from->rows, c=((double)from->columns)/to->columns;
    for(is=0;is<from->rows;++is){
        const unsigned int rSt=round(r*is), rEnd=round(r*(is+1));
        for(js=0;js<to->columns;++js)
            if(transformer(KLS_matrixAt(from,is,round(js*c)),tmp,arg))
                for(ib=rSt;ib<rEnd;++ib)
                    KLS_matrixPutElement(to,ib,js,tmp);
    }
}

void _KLS_matrixBB_SS(const KLS_t_MATRIX * const from,KLS_t_MATRIX * const to,void * const tmp,KLS_byte(*transformer)(const void *elFrom,void *elTo,void *arg),void *arg){
    unsigned int is,js;
    const float r=((double)from->rows)/to->rows, c=((double)from->columns)/to->columns;
    for(is=0;is<to->rows;++is)
        for(js=0;js<to->columns;++js)
            transformer(KLS_matrixAt(from,round(is*r),round(js*c)),KLS_matrixAt(to,is,js),arg);
}

KLS_byte _KLS_matrixTransformerDefault(const void * const src,void * const dst,const KLS_TYPEOF(KLS_ABSTRACT(KLS_t_MATRIX)->elSize) *size){ if(dst && src){memcpy(dst,src,*size); return 1;} return 0; }

KLS_byte KLS_matrixTransform(const KLS_t_MATRIX * const from,KLS_t_MATRIX * const to,KLS_byte(*transformer)(const void *elFrom,void *elTo,void *arg),void *arg){
    if(from && from->data && to && to->data){
        if(!transformer){
            if(from->elSize==to->elSize){
                transformer=(void*)_KLS_matrixTransformerDefault;
                arg=KLS_UNCONST(&from->elSize);
            }else return 0;
        }
        if(from->rows==to->rows && from->columns==to->columns){
            unsigned int i,j;
            for(i=0;i<from->rows;++i)
                for(j=0;j<from->columns;++j)
                    transformer(KLS_matrixAt(from,i,j),KLS_matrixAt(to,i,j),arg);
        }else{
            char local[256], *p=local;
            if(to->elSize>sizeof(local) && !(p=malloc(to->elSize))){
                errno=ENOMEM; return 0;
            }
            ((void(*[2][2])(const KLS_t_MATRIX*,KLS_t_MATRIX*,void*,KLS_byte(*)(const void*,void*,void*),void*)){
                {_KLS_matrixSS_BB,_KLS_matrixSB_BS},
                {_KLS_matrixBS_SB,_KLS_matrixBB_SS},
            })[from->rows>to->rows][from->columns>to->columns](from,to,p,transformer,arg);
            if(p!=local) free(p);
        }
        return 1;
    } return 0;
}


int _KLS_matrixGetRowAlign(const char *string,unsigned int ofs,unsigned int interval,KLS_byte align){
    int res=1;
    for(;*string;++string)
        if(*string=='\n')
            ++res;
    res*=ofs; res-=interval;
    return res-((align*res)>>1)+((align==2)-1);
}

int _KLS_matrixGetColumnAlign(const char *string,unsigned int ofs,unsigned int interval,KLS_byte align){
    int res=0;
    for(;*string && *string!='\n';++string)
        ++res;
    res*=ofs; res-=interval;
    return (align*res)>>1;
}

struct _KLS_t_MATRIX_LETTER{
    const void * const element;
    const KLS_size size;
};

static KLS_byte _KLS_matrixLetter(const char * const from,void * const to,const struct _KLS_t_MATRIX_LETTER * const element){
    if(to && *from){
        memcpy(to,element->element,element->size);
        return 1;
    } return 0;
}

void KLS_matrixPutString(KLS_t_MATRIX *matrix,int row,int column,const void *element,const KLS_t_FONT *font,KLS_byte align,const char *string){
    if(string){
        if(!font) font=&KLS_fontBase;
        if(!align) align=KLS_ALIGN_H_LEFT | KLS_ALIGN_V_TOP;
        {
        const KLS_byte alignC=(align&7)>>1;
        const KLS_byte alignR=((align>>3)&7)>>1;
        const KLS_byte symbWidth=((font->bitmap->symbolWidth-1)>>3)+1;
        const KLS_byte symbHeight=font->bitmap->symbolHeight*symbWidth;

        const unsigned int mask=1<<(font->bitmap->symbolWidth-1);
        const unsigned int ofsR=font->height+font->intervalRow;
        const unsigned int ofsC=font->width+font->intervalSymbol;

        int r=row-_KLS_matrixGetRowAlign(string,ofsR,font->intervalRow,alignR);
        int c=column-_KLS_matrixGetColumnAlign(string,ofsC,font->intervalSymbol,alignC);

        char buffer[1024];
        char *p=sizeof(buffer)<font->bitmap->symbolHeight*font->bitmap->symbolWidth ? KLS_malloc(font->bitmap->symbolHeight*font->bitmap->symbolWidth) : buffer;

        if(p){
            struct _KLS_t_MATRIX_LETTER elem={element,matrix->elSize};
            int i,j;
            KLS_t_MATRIX symb,letter=KLS_matrixNew(p,font->bitmap->symbolHeight,font->bitmap->symbolWidth,sizeof(char),NULL);
            while(*string){
                unsigned char id=*string;
                const char *bm=font->bitmap->symbols+id*symbHeight;
                ++string;

                if(id=='\n'){
                    r+=ofsR;
                    c=column-_KLS_matrixGetColumnAlign(string,ofsC,font->intervalSymbol,alignC);
                    continue;
                }

                for(i=0,p=letter.data;i<letter.rows;++i,bm+=symbWidth)
                    for(j=0;j<letter.columns;++j,++p)
                        *p=*(bm+((font->bitmap->symbolWidth-1-j)>>3)) & (mask>>j);

                symb=KLS_matrixGetMatrix(matrix,r,c,font->height,font->width,0);
                KLS_matrixTransform(&letter,&symb,(void*)_KLS_matrixLetter,&elem);

                c+=ofsC;
            }
            if(letter.data!=buffer)
                KLS_free(letter.data);
        }
        }
    }
}

void KLS_matrixPutStringf(KLS_t_MATRIX *matrix,int row,int column,const void *element,const KLS_t_FONT *font,KLS_byte align,const char *format,...){
    char *str=KLS_stringv(format);
    KLS_matrixPutString(matrix,row,column,element,font,align,str);
    KLS_free(str);
}

void KLS_matrixPrint(const KLS_t_MATRIX *matrix,void(*printer)(const void *element)){
    if(printer){
        unsigned int i,j;
        for(i=0;i<matrix->rows;++i){
            for(j=0;j<matrix->columns;++j)
                printer(KLS_matrixAt(matrix,i,j));
            printer(NULL);
        }
    }
}

void KLS_stringRect(const char *string,const KLS_t_FONT *font,int *width,int *height){
    *width=*height=0;
    if(string){
        if(!font) font=&KLS_fontBase;
        {
        const unsigned int ofsR=font->height+font->intervalRow;
        const unsigned int ofsC=font->width+font->intervalSymbol;
        const char *p;
        unsigned int len;
        while((p=KLS_stringSep(&string,&len,"\n"))){
            *height+=ofsR;
            if(len>*width) *width=len;
        }
        *width*=ofsC;
        *width-=font->intervalSymbol;
        *height-=font->intervalRow;
        }
    }
}

const char *KLS_stringPosition(const char *string,const KLS_t_FONT *font,KLS_byte align,int x,int y){
    if(string){
        if(!font) font=&KLS_fontBase;
        if(!align) align=KLS_ALIGN_H_LEFT | KLS_ALIGN_V_TOP;
        {
        const KLS_byte alignC=(align&7)>>1;
        const KLS_byte alignR=((align>>3)&7)>>1;

        const unsigned int ofsR=font->height+font->intervalRow;
        const unsigned int ofsC=font->width+font->intervalSymbol;
        int i;

        if( y<(i=-_KLS_matrixGetRowAlign(string,ofsR,font->intervalRow,alignR)) )
            return string; // ???
        while(y>=(i+=ofsR)){
            const char *p=strstr(string,"\n");
            if(!p) return string+strlen(string); // ???
            string=p+1;
        }

        if( x<(i=-_KLS_matrixGetColumnAlign(string,ofsC,font->intervalSymbol,alignC)) )
            return string;
        while(*string && x>=(i+=ofsC)){
            if(*string=='\n') break;
            ++string;
        }
        }
    }
    return string;
}

