#include "encoding_manager.h"
#include "fonts_manager.h"
#include <string.h>

static int isUtf16le(unsigned char * pcBufferHead);
static int getUtf16leFrmBuf(unsigned char * pcBufferStart, unsigned char *pucBufferEnd, unsigned int * pdwCode);

static T_EncodingOpr g_tUtf16leEncodingOpr = {
    .name = "utf16be",
    .headlen = 2,
    .isSuport = isUtf16le,
    .getCodeFrmBuf = getUtf16leFrmBuf,
}; 

/*
*@breif:判断是否支持utf16be
*@ret:支持返回1,否则返回-1
*/
static int isUtf16le(unsigned char * pcBufferHead)
{
    char aHead[] = {0xFF, 0xFE, 0};

    if(strncmp((const char *)pcBufferHead, aHead, 2) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
    
}


static int getUtf16leFrmBuf(unsigned char * pcBufferStart, unsigned char *pucBufferEnd, unsigned int * pdwCode)
{

    if(pcBufferStart + 1  <= pucBufferEnd)
    {
        *pdwCode = (((unsigned int)pcBufferStart[1]) << 8) + pcBufferStart[0];
        return 2;
    }
    else
    {
        return 0;
    }
}


int Utf16leInit(void)
{
    addFontOprForEncoding(&g_tUtf16leEncodingOpr, getFontOpr("freetype"));
    addFontOprForEncoding(&g_tUtf16leEncodingOpr, getFontOpr("ascil"));
    return registerEncodingOpr(&g_tUtf16leEncodingOpr);

}