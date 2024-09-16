/*1.获取显示文件的编码类型
**2.获取编码*/
#include "encoding_manager.h"
#include <malloc.h>
#include "config.h"

static PT_EncodingOpr g_ptEncodingOprHead = NULL;


/*
*@brief:初始化链表头，把新的节点插入到链表尾部
*@arg:T_EncodingOprT_EncodingOpr结构体类型的指针
*/
int registerEncodingOpr( PT_EncodingOpr ptEncodingOpr)
{
    PT_EncodingOpr ptTmp;

    if(!g_ptEncodingOprHead)
    {
        g_ptEncodingOprHead = ptEncodingOpr;
        ptEncodingOpr ->pNext = NULL;
    }
    else
    {
        ptTmp = g_ptEncodingOprHead;
        while (ptTmp ->pNext)
        {
           ptTmp = ptTmp ->pNext;
        }
        ptTmp -> pNext = ptEncodingOpr;
        ptEncodingOpr ->pNext =NULL;
        
    }
    
    return 0;
}

/*
*@brief:把链表中的各个节点的name成员打印出来
*/
void showEncodingOpr(void)
{
    int i= 0;
    PT_EncodingOpr ptTmp = g_ptEncodingOprHead;

    while(ptTmp)
    {
        printf("%02d: %s\r\n", i++, ptTmp ->name);
        ptTmp = ptTmp ->pNext;
    }
}


/*
*@brief:从链表中筛选出支持该文件编码的节点
*@arg:文件内存首地址
*@ret:链表中的某个节点的指针
*/
PT_EncodingOpr selectOprForFile(unsigned char * pucBuffer)
{
    PT_EncodingOpr ptTmp = g_ptEncodingOprHead;
    while(ptTmp)
    {
        if(ptTmp ->isSuport(pucBuffer))
        {
            return ptTmp;
        }
        else
        {
           ptTmp = ptTmp ->pNext;
        } 
    }
    return NULL;

}


/*
*@brief:为PT_EncodingOpr -> ptFontOprSupportedHead插入新的PT_FontOpr类型的节点，头插法；
PT_FontOpr是为PT_EncodingOpr支持的操作类型
*/
int addFontOprForEncoding(PT_EncodingOpr encodingOpr, PT_FontOpr fontOpr)
{
    PT_FontOpr ptFontOprCpy;


    if(!encodingOpr || !fontOpr)
    {
        return -1;
    }
    else
    {
        ptFontOprCpy = malloc(sizeof(T_FontOpr));

        if(ptFontOprCpy)
        {
            memcpy(ptFontOprCpy, fontOpr, sizeof(T_FontOpr));
            ptFontOprCpy -> pNext = encodingOpr ->  ptFontOprSupportHead ;
            encodingOpr ->  ptFontOprSupportHead = ptFontOprCpy;
        }
        else
        {
            return -1;
        }
        
    }
    
    return 0;
}

/*
*@brief:给PT_EncodingOpr -> ptFontOprSupportedHead删除一个PT_FontOpr类型的节点
*/
int delFontOprForEncoding(PT_EncodingOpr encodingOpr, PT_FontOpr fontOpr)
{
    if(!encodingOpr || !fontOpr)
    {
        return -1;
    }
    else
    {
        PT_FontOpr ptTmp = encodingOpr -> ptFontOprSupportHead;
        

        if(strcmp(ptTmp -> name, fontOpr -> name) == 0)
        {
            encodingOpr -> ptFontOprSupportHead = ptTmp -> pNext  ;
            DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            free(ptTmp);
            DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            return 0;
        }

        PT_FontOpr ptNext = ptTmp -> pNext;

        while(ptNext)
        {
            if(strcmp(ptNext -> name, fontOpr -> name) == 0)
            {
                ptTmp -> pNext = ptNext -> pNext;
                DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                free(ptNext);
                DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                return 0;
            }
            else
            {
                ptTmp = ptNext;
                ptNext = ptNext -> pNext;
            }
        }
        return -1;
    }

    return 0;
}


/* 先写一个简单的版本, 只能处理ASCII字符串 */
int GetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode)
{
	unsigned char *pucBuf = pucBufStart;
	unsigned char c = *pucBuf;
	
	if ((pucBuf < pucBufEnd) && (c < (unsigned char)0x80))
	{
		/* 返回ASCII码 */
		*pdwCode = (unsigned int)c;
		return 1;
	}

	if (((pucBuf + 1) < pucBufEnd) && (c >= (unsigned char)0x80))
	{
		/* 返回GBK码 */
		*pdwCode = pucBuf[0] + (((unsigned int)pucBuf[1])<<8);
		return 2;
	}

	if (pucBuf < pucBufEnd)
	{
		/* 可能文件有损坏, 但是还是返回一个码, 即使它是错误的 */
		*pdwCode = (unsigned int)c;
		return 1;
	}
	else
	{
		/* 文件处理完毕 */
		return 0;
	}
}

int EncodingInit(void)
{
    AscilEncodingInit();
    Utf8EncodingInit();
    Utf16beInit();
    Utf16leInit();

    return 0;
}




