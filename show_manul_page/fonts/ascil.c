#include <fonts_manager.h>
#include "asicil.h"



static int ASCILFontInit(char * pcFontFile, unsigned int dwFontSize);
static int ASCILGetFontBitmap(unsigned int dwcode, PT_FontBitMap ptFontBitMap);

static T_FontOpr ascilBitMap ={
    .name = "ascil",
    .FontInit = ASCILFontInit,
    .GetFontBitmap = ASCILGetFontBitmap,
};



int ASCILFontInit(char * pcFontFile, unsigned int dwFontSize)
{
    if(dwFontSize != 16)
    {
        return -1;
    }

    return 0;
}


/*
*@brief:跟据参数一获取到编码对应的点阵，存储到参数二中
*/
int ASCILGetFontBitmap(unsigned int dwcode, PT_FontBitMap ptFontBitMap)
{
    int iPenX = ptFontBitMap ->iCurOriginX;
    int iPenY = ptFontBitMap ->iCurOriginY;

    if(dwcode > 0x80)
    {
        return -1;
    }
    else
    {
        ptFontBitMap->pucBuffer = (unsigned char* )&fontdata_8x16[dwcode * 16];
        ptFontBitMap ->iXLeft = iPenX;
        ptFontBitMap ->iYTop = iPenY - 16;
        ptFontBitMap ->iXMax = iPenX + 8;
        ptFontBitMap ->iYMax = iPenY;
        ptFontBitMap ->iNextOriginX = iPenX + 8;
        ptFontBitMap ->iNextOriginY = iPenY;
        ptFontBitMap ->iPitch = 1;

    }

    return 0;
}

int ascilInit(void )
{
    return registerFontOpr(&ascilBitMap);
}