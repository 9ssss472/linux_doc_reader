#include "encoding_manager.h"
#include "config.h"

static int UTF8GetCodeFrmBuf(unsigned char * pcBufferStart, unsigned char *pucBufferEnd, unsigned int * pdwCode);
static int isUtf8(unsigned char * pcBufferHead);

static T_EncodingOpr T_Utf8EncodingOpr ={
    .name = "utf8",
    .headlen = 3,
    .getCodeFrmBuf = UTF8GetCodeFrmBuf,
    .isSuport = isUtf8,
};

/*
*@brief:判断是否是utf8编码
*/
static int isUtf8(unsigned char * pcBufferHead)
{
    const char aUtf8Head[] = {0xEF, 0xBB, 0xBF, 0};

    DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    if (strncmp((const char *)pcBufferHead, aUtf8Head, 3) == 0)
    {
        DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return 1;
    }
    DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    return 0;

}

static int CountOneBit(char c)
{
    int count = 0;
    int i;

    for(i = 7; i >= 0 ; i--)
    {
        if(c & (1 << i))
        {
            count ++;
        }
        else
        {
            break;
        }
    }
    return count;
}

/*
*给字符取码存到pdwCode里
*/
static int UTF8GetCodeFrmBuf(unsigned char * pcBufferStart, unsigned char *pucBufferEnd, unsigned int * pdwCode)
{
    unsigned char * pcBuffer = pcBufferStart;
    unsigned char iVal = *pcBuffer;
    int iCount;
    unsigned int iSum = 0;
    int i = 0;

    if(pcBufferStart >= pucBufferEnd)
    {
        return 0;
    }

    iCount = CountOneBit(iVal);

    if(pcBufferStart + iCount >= pucBufferEnd)
    {
        return 0;
    }

    if(iCount == 0)
    {
        *pdwCode = pcBuffer[0];
        return 1;
    }
    else
    {
        iVal = iVal << iCount;
        iVal = iVal >> iCount;
        iSum += iVal;

        for(i = 1; i < iCount; i++)
        {
            iVal = pcBuffer[i] & 0x3f;
            iSum = iSum << 6;
          
            iSum += iVal;

        }
        //printf("iSum: %x\r\n", iSum);
        *pdwCode = iSum;
        return iCount;
    }

    return 0;
}

int Utf8EncodingInit(void)
{
    addFontOprForEncoding(&T_Utf8EncodingOpr, getFontOpr("freetype"));
    addFontOprForEncoding(&T_Utf8EncodingOpr, getFontOpr("ascil"));
    return registerEncodingOpr(&T_Utf8EncodingOpr);

}