#include <encoding_manager.h>

static int getAscilCode(unsigned char * pcBufferStart, unsigned char *pucBufferEnd, unsigned int * pdwCode);
static int isAscilCoding( unsigned char * pcBufferHead);


static T_EncodingOpr g_tAscilEncodingOpr = {
    .name = "ascil",
    .headlen = 0,
    .getCodeFrmBuf = getAscilCode,
    .isSuport = isAscilCoding,
};


/*
*@brief:判断文件是否是使用ascil编码
*@arg:文件开头指针
*@ret:是ascil编码返回1,不是返回0
*/
static int isAscilCoding( unsigned char * pcBufferHead)
{
	const  char aStrUtf8[]    = {0xEF, 0xBB, 0xBF, 0};
	const  char aStrUtf16le[] = {0xFF, 0xFE, 0};
	const  char aStrUtf16be[] = {0xFE, 0xFF, 0};

    if(strncmp((const char *)pcBufferHead, aStrUtf8, 3) == 0 )
    {
        return 0;
    }
    else if(strncmp((const char *)pcBufferHead,aStrUtf16le, 2 ) == 0)
    {
        return 0;
    }
    else if(strncmp((const char *)pcBufferHead,aStrUtf16be, 2 ) == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
    
    return 0;
}


/*
*brief:获取ascil的编码或GBK的编码，编码首地址存到pdwCode中
*arg1:
*/
static int getAscilCode(unsigned char * pcBufferStart, unsigned char *pucBufferEnd, unsigned int * pdwCode)
{
    unsigned char * pucBuffer= pcBufferStart;
    unsigned char c = *pcBufferStart;

    if(pucBuffer < pucBufferEnd && c < 0x80)
    {
        /*返回ascil码*/
        *pdwCode = (unsigned int )c;
        return 1;
    }
    else if(pucBuffer + 1 < pucBufferEnd && c >= 0x80)
    {
        /*返回GBK码*/
        *pdwCode = pucBuffer[0] + (unsigned int)(pucBuffer[1] << 8);
        return 2;
    }

    if(pucBuffer < pucBufferEnd)
    {
        /*文件出错*/
        *pdwCode = (unsigned int )c;
        return 1;
    }
    else
    {
        /*文件结束 */
        return 0;
    }
    
}

int AscilEncodingInit(void)
{
    addFontOprForEncoding(&g_tAscilEncodingOpr, getFontOpr("ascil"));
    addFontOprForEncoding(&g_tAscilEncodingOpr, getFontOpr("freetype"));
    addFontOprForEncoding(&g_tAscilEncodingOpr, getFontOpr("GBK"));
    return registerEncodingOpr(&g_tAscilEncodingOpr);

}

