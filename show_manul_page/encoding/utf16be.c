#include "encoding_manager.h"
#include "fonts_manager.h"
#include <string.h>

static int isUtf16be(unsigned char * pcBufferHead);
static int getUtf16beFrmBuf(unsigned char * pcBufferStart, unsigned char *pucBufferEnd, unsigned int * pdwCode);

static T_EncodingOpr g_tUtf16beEncodingOpr = {
    .name = "utf16be",
    .headlen = 2,
    .isSuport = isUtf16be,
    .getCodeFrmBuf = getUtf16beFrmBuf,
}; 

/*
*@breif:判断是否支持utf16be
*@ret:支持返回1,否则返回-1
*/
static int isUtf16be(unsigned char * pcBufferHead)
{
    const char aHead[] = {0xFE, 0xFF, 0};

    if(strncmp((const char *)pcBufferHead, aHead, 2) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
    
}


static int getUtf16beFrmBuf(unsigned char * pcBufferStart, unsigned char *pucBufferEnd, unsigned int * pdwCode)
{

    if(pcBufferStart + 1  <= pucBufferEnd)
    {
        *pdwCode = (((unsigned int)pcBufferStart[0]) << 8) + pcBufferStart[1];
        return 2;
    }
    else
    {
        return 0;
    }
}


int Utf16beInit(void)
{
    addFontOprForEncoding(&g_tUtf16beEncodingOpr, getFontOpr("freetype"));
    addFontOprForEncoding(&g_tUtf16beEncodingOpr, getFontOpr("ascil"));
    return registerEncodingOpr(&g_tUtf16beEncodingOpr);
}