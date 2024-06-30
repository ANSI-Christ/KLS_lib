
KLS_COLOR _KLS_rgb8(KLS_byte r,KLS_byte g,KLS_byte b){return ((r>>5)<<5)|((g>>5)<<2)|(b>>6);}
KLS_COLOR _KLS_rgb15(KLS_byte r,KLS_byte g,KLS_byte b){return ((r>>3)<<10)|((g>>3)<<5)|(b>>3);}
KLS_COLOR _KLS_rgb16(KLS_byte r,KLS_byte g,KLS_byte b){return ((r>>3)<<11)|((g>>2)<<5)|(b>>3);}
KLS_COLOR _KLS_rgb24(KLS_byte r,KLS_byte g,KLS_byte b){return (r<<16)|(g<<8)|b;}
KLS_COLOR _KLS_rgb32(KLS_byte r,KLS_byte g,KLS_byte b){return (r<<16)|(g<<8)|b;}
void _KLS_rgbGetInfo(int bits){
#define _KLS_RGB_CASE(bits,bytes) case bits: _KLS_rgb=_KLS_rgb##bits; break
    switch((KLS_COLOR_BITS=bits)){
        _KLS_RGB_CASE(8,1); _KLS_RGB_CASE(15,2); _KLS_RGB_CASE(16,2); _KLS_RGB_CASE(24,3); _KLS_RGB_CASE(32,4);
    }
#undef _KLS_RGB_CASE
}

void _KLS_drawInterpol(const KLS_t_CANVAS *canvas,double *x, double *y){
    *x=KLS_interpol(0,canvas->left,canvas->m.columns,canvas->right,*x);
    *y=KLS_interpol(0,canvas->up,canvas->m.rows,canvas->down,*y);
}

KLS_t_CANVAS KLS_canvasNewExt(void *buffer,unsigned int pixWidth,unsigned int pixHeight,unsigned char colorSize,double left,double up,double right,double down){
    KLS_t_CANVAS c={{0}};
    c.m=KLS_matrixNew(buffer,pixHeight,pixWidth,colorSize,NULL);
    c.up=up; c.left=left; c.down=down; c.right=right;
    c._dv=pixHeight/fabs(c.up-c.down); c._dh=pixWidth/fabs(c.left-c.right);
    return c;
}

KLS_t_CANVAS KLS_canvasNew(void *buffer,unsigned int pixWidth,unsigned int pixHeight,double left,double up,double right,double down){
    return KLS_canvasNewExt(buffer,pixWidth,pixHeight,sizeof(KLS_COLOR),left,up,right,down);
}

KLS_t_CANVAS KLS_canvasSub(const KLS_t_CANVAS *canvas,int pixelX,int pixelY,unsigned int pixWidth,unsigned int pixHeight,double left,double up,double right,double down){
    KLS_t_CANVAS c={{0}};
    if(canvas && canvas->m.data){
        c.m=KLS_matrixGetMatrix(&canvas->m,pixelY,pixelX,pixHeight,pixWidth);
        c.up=up; c.left=left; c.down=down; c.right=right;
        c._dv=pixHeight/fabs(c.up-c.down); c._dh=pixWidth/fabs(c.left-c.right);
    }
    return c;
}
KLS_t_CANVAS KLS_canvasSub2(const KLS_t_CANVAS *canvas,double left,double up,double right,double down){
    KLS_t_CANVAS c={{0}};
    if(canvas && canvas->m.data){
        double x=left,y=up,h=down,w=right;
        _KLS_drawInterpol(canvas,&x,&y);
        _KLS_drawInterpol(canvas,&w,&h);
        w-=x; h-=y;
        if(w>0 && h>0){
            c.m=KLS_matrixGetMatrix(&canvas->m,y,x,h,w);
            c.up=up; c.left=left; c.down=down; c.right=right;
            c._dv=h/fabs(c.up-c.down); c._dh=w/fabs(c.left-c.right);
        }
    }
    return c;
}

void KLS_canvasFree(KLS_t_CANVAS *canvas){
    if(canvas){
        KLS_matrixFree(&canvas->m);
        memset(canvas,0,sizeof(*canvas));
    }
}

void *KLS_canvasAtPix(const KLS_t_CANVAS *canvas,int pixelX,int pixelY,KLS_t_POINT *point){
    void *ptr=canvas?KLS_matrixAt(&canvas->m,pixelY,pixelX):NULL;
    if(point){
        if(ptr){
            point->x=KLS_interpol(canvas->left,0,canvas->right,canvas->m.columns,pixelX);
            point->y=KLS_interpol(canvas->up,0,canvas->down,canvas->m.rows,pixelY);
            return ptr;
        }
        point->x=point->y=KLS_NAN;
    }
    return ptr;
}


void KLS_canvasBMPf(const KLS_t_CANVAS *canvas,const char *format,...){
    char *str=KLS_stringv(format);
    KLS_canvasBMP(canvas,str);
    KLS_free(str);
}

void KLS_canvasBMP(const KLS_t_CANVAS *canvas,const char *fileName){
    #define _KLS_BMP_WR(t,...) ({KLS_TYPEOF(t) _tmp_=(__VA_ARGS__); fwrite(&_tmp_,1,sizeof(_tmp_),f);})
    if(canvas && canvas->m.data && fileName){
        int row,c;
        const char extraColor=0;
        const int palitraCount=(canvas->m.elSize==1)*256, palitraSize=palitraCount*sizeof(int);
        KLS_byte extraBytes=((KLS_size)canvas->m.columns*(4-canvas->m.elSize))%4;
        FILE *f=fopen(fileName,"wb");
        if(!f) return;

        fwrite((KLS_endianGet()==KLS_ENDIAN_LITTLE)?"BM":"MB",2,1,f);
        _KLS_BMP_WR(unsigned int, 14+40+palitraSize+(KLS_size)canvas->m.elSize*canvas->m.rows*canvas->m.columns+canvas->m.rows*extraBytes);
        _KLS_BMP_WR(unsigned int, 0);
        _KLS_BMP_WR(unsigned int, 14+40+palitraSize);

        _KLS_BMP_WR(unsigned int, 40);
        _KLS_BMP_WR(unsigned int, canvas->m.columns);
        _KLS_BMP_WR(unsigned int, canvas->m.rows);
        _KLS_BMP_WR(unsigned short, 1);
        _KLS_BMP_WR(unsigned short, canvas->m.elSize*8);
        _KLS_BMP_WR(unsigned int, 0);
        _KLS_BMP_WR(unsigned int, 0);
        _KLS_BMP_WR(unsigned int, 0);
        _KLS_BMP_WR(unsigned int, 0);
        _KLS_BMP_WR(unsigned int, 1<<(canvas->m.elSize*8));
        _KLS_BMP_WR(unsigned int, 0);

        for(c=0;c<palitraCount;++c){
            KLS_COLOR color=((KLS_COLOR)(c&KLS_0b00000011)<<16) | ((KLS_COLOR)(c&KLS_0b00011100)<<8) | ((KLS_COLOR)(c&KLS_0b11100000));
            fwrite(&color,sizeof(color),1,f);
        }
        for(row=canvas->m.rows-1;row>-1;--row){
            for(c=0;c<canvas->m.columns;++c)
                fwrite(KLS_matrixAt(&canvas->m,row,c),canvas->m.elSize,1,f);
            for(c=0;c<extraBytes;++c)
                fwrite(&extraColor,1,1,f);
        }
        KLS_freeFile(f);
    }
    #undef _KLS_BMP_WR
}


void KLS_canvasText(KLS_t_CANVAS *canvas,double x,double y,const KLS_t_FONT *font,KLS_byte align,const void *color,const char *text){
    if(!canvas || !canvas->m.data || !text) return;
    _KLS_drawInterpol(canvas,&x,&y);
    KLS_matrixPutString(&canvas->m,y,x,color,font,align,text);
}

void KLS_canvasTextf(KLS_t_CANVAS *canvas,double x,double y,const KLS_t_FONT *font,KLS_byte align,const void *color,const char *format,...){
    if(canvas && canvas->m.data && format){
        char *str=KLS_stringv(format);
        _KLS_drawInterpol(canvas,&x,&y);
        KLS_matrixPutString(&canvas->m,y,x,color,font,align,str);
        KLS_free(str);
    }
}

KLS_t_CANVAS KLS_canvasPlot(KLS_t_CANVAS *canvas,const KLS_t_AXIS *axisX,const KLS_t_AXIS *axisY,const void *colorAxis,const void *colorBackground){
    KLS_t_CANVAS s={{0}};
    KLS_t_AXIS _axis={0};
    if(!axisX) axisX=&_axis;
    if(!axisY) axisY=&_axis;
    if(canvas && canvas->m.data && colorAxis && colorBackground){
        const char *frmX=axisX->format ? axisX->format : "%.0f";
        const char *frmY=axisY->format ? axisY->format : "%.0f";
        const KLS_t_FONT optX=KLS_fontBase, optY=optX;
        const int gridX=!!axisX->gridOn, gridY=!!axisY->gridOn,
                ofsUp=0+optY.intervalRow+optY.height/2,
                ofsLeft=5+KLS_MAX(KLS_stringLen(frmY,canvas->up),KLS_stringLen(frmY,canvas->down))*(optY.width+optY.intervalSymbol),
                ofsRight=1+ofsLeft+KLS_stringLen(frmX,canvas->right)*(optY.width+optY.intervalSymbol)/2,
                ofsDown=3+ofsUp+(optX.height+optX.intervalRow);
        const double stepHor=(canvas->right-canvas->left)/(axisX->division+1), stepVert=(canvas->up-canvas->down)/(axisY->division+1);
        int i,cnt,pos,_row;
        double step;
        char str[64];
        KLS_canvasClear(canvas,colorBackground);
        KLS_matrixPutRect(&canvas->m,0,0,canvas->m.rows-1,canvas->m.columns-1,colorAxis,1,NULL);
        s=KLS_canvasSub(canvas,ofsLeft,ofsUp,canvas->m.columns-ofsRight,canvas->m.rows-ofsDown,0,0,1,1);
        KLS_matrixPutRect(&s.m,0,0,s.m.rows-1,s.m.columns-1,colorAxis,1,NULL);
        for(i=0, cnt=axisY->division+2, step=canvas->up; i<cnt; ++i, step-=stepVert){
            pos=KLS_interpol(ofsUp,canvas->up,ofsUp+s.m.rows-1,canvas->down,step);
            snprintf(str,KLS_ARRAY_LEN(str)-1,frmY,step);
            KLS_matrixPutString(&canvas->m,pos,ofsLeft-2-2,colorAxis,&optY,KLS_ALIGN_H_RIGHT | KLS_ALIGN_V_MID,str);
            KLS_matrixPutLine(&canvas->m,pos,ofsLeft-2,pos,ofsLeft+(s.m.columns-2)*gridY,colorAxis,1);
        }
        _row=ofsUp+s.m.rows;
        for(i=0, cnt=axisX->division+2, step=canvas->left; i<cnt; ++i, step+=stepHor){
            pos=KLS_interpol(ofsLeft,canvas->left,ofsLeft+s.m.columns-1,canvas->right,step);
            snprintf(str,KLS_ARRAY_LEN(str)-1,frmX,step);
            KLS_matrixPutString(&canvas->m,_row+2+3,pos,colorAxis,&optX,KLS_ALIGN_H_MID | KLS_ALIGN_V_TOP,str);
            KLS_matrixPutLine(&canvas->m,_row+1,pos,_row-s.m.rows*gridX,pos,colorAxis,1);
        }
        s=KLS_canvasSub(&s,1,1,s.m.columns-2,s.m.rows-2,canvas->left,canvas->up,canvas->right,canvas->down);
    }
    return s;
}

void KLS_canvasPoint(KLS_t_CANVAS *canvas,double x,double y,const void *color,unsigned int width){
    if(!canvas || !canvas->m.data || !color) return;
    _KLS_drawInterpol(canvas,&x,&y);
    KLS_matrixPutPoint(&canvas->m,y,x,color,width);
}

void KLS_canvasLine(KLS_t_CANVAS *canvas,double x1,double y1,double x2,double y2,const void *color,unsigned int width){
    if(!canvas || !canvas->m.data || !color) return;
    _KLS_drawInterpol(canvas,&x1,&y1);
    _KLS_drawInterpol(canvas,&x2,&y2);
    KLS_matrixPutLine(&canvas->m,y1,x1,y2,x2,color,width);
}

void KLS_canvasRound(KLS_t_CANVAS *canvas,double x,double y,double r,const void *color,unsigned int width,const void *colorFill){
    if(!canvas || !canvas->m.data || !color) return;
    _KLS_drawInterpol(canvas,&x,&y);
    KLS_matrixPutEllipse(&canvas->m,y,x,r*canvas->_dv,r*canvas->_dh,color,width,colorFill);
}

void KLS_canvasEllipse(KLS_t_CANVAS *canvas,double x,double y,double xWidth,double yHeight,const void *color,unsigned int width,const void *colorFill){
    if(!canvas || !canvas->m.data || !color) return;
    _KLS_drawInterpol(canvas,&x,&y);
    KLS_matrixPutEllipse(&canvas->m,y,x,canvas->_dv*yHeight,canvas->_dh*xWidth,color,width,colorFill);
}

void KLS_canvasArc(KLS_t_CANVAS *canvas,double x,double y,double r,float angle, float angleRot,const void *color,unsigned int width){
    if(!canvas || !canvas->m.data || !color) return;
    _KLS_drawInterpol(canvas,&x,&y);
    KLS_matrixPutArcEllipse(&canvas->m,y,x,r*canvas->_dv,r*canvas->_dh,angle*M_PI/180.,angleRot*M_PI/180.,color,width);
}

void KLS_canvasArcEllipse(KLS_t_CANVAS *canvas,double x,double y,double xWidth,double yHeight,float angle, float angleRot,const void *color,unsigned int width){
    if(!canvas || !canvas->m.data || !color) return;
    _KLS_drawInterpol(canvas,&x,&y);
    KLS_matrixPutArcEllipse(&canvas->m,y,x,canvas->_dv*yHeight,canvas->_dh*xWidth,angle*M_PI/180.,angleRot*M_PI/180.,color,width);
}

void KLS_canvasRect(KLS_t_CANVAS *canvas,double x1,double y1,double x2,double y2,const void *color,unsigned int width,const void *colorFill){
    if(!canvas || !canvas->m.data || !color) return;
    _KLS_drawInterpol(canvas,&x1,&y1);
    _KLS_drawInterpol(canvas,&x2,&y2);
    KLS_matrixPutRect(&canvas->m,y1,x1,y2,x2,color,width,colorFill);
}

void KLS_canvasClear(KLS_t_CANVAS *canvas,const void *color){
    if(!canvas || !canvas->m.data || !color) return;
    KLS_matrixFillArea(&canvas->m,0,0,color,NULL);
}

void KLS_canvas2Display(KLS_t_CANVAS *canvas,GUI_t_DISPLAY *display){
    if(canvas && canvas->m.data && display && display->m.data){
        unsigned int x,y,r=KLS_MIN(display->m.rows,canvas->m.rows),c=KLS_MIN(display->m.columns,canvas->m.columns);
        for(y=0;y<r;++y)
            for(x=0;x<c;++x)
                memcpy(KLS_matrixAt(&display->m,y,x),KLS_matrixAt(&canvas->m,y,x),sizeof(KLS_COLOR));
        GUI_displayDraw(display);
    }
}
