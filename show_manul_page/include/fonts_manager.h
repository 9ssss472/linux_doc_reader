#ifndef _FONTS_MANAGER_H
#define _FONTS_MANAGER_H

#include <stdio.h>


typedef struct fontBitMap{
    int iXLeft;  /*左上角的X*/
    int iYTop;   /*顶部的Y*/
    int iXMax;   /*右下角的X*/
    int iYMax;   /*右下角的Y*/
    int iPitch;
    int iCurOriginX;  /*当前位置的左下角X*/
    int iCurOriginY;  /*当前位置的左下角Y*/
    int iNextOriginX;
    int iNextOriginY;
    int iBpp;
    unsigned char * pucBuffer;
}T_FontBitMap, *PT_FontBitMap;

typedef struct fontOpr{
    char * name;
    struct fontOpr * pNext;
    int (*FontInit)(char * pcFontFile, unsigned int dwFontSize);
    int (*GetFontBitmap)(unsigned int dwcode, PT_FontBitMap ptFontBitMap);
    void (*SetFontSize)(int iSize);
}T_FontOpr, *PT_FontOpr;

int registerFontOpr(PT_FontOpr fontOpr);
PT_FontOpr getFontOpr(char * name);
int ascilInit(void );
int FreeTypeInit(void);
int GBKInit(void);
int FontInit(void);
int showFontOpr(void);
int GetFontBitmap(unsigned int dwCode, PT_FontBitMap ptFontBitMap);
void SetFontSize(unsigned int dwFontSize);
int SetFontDetail(char * fontName, char * filename,int iWordSize);
#endif /* _FONTS_MANAGER_H */
