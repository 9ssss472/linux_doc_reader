#include "debug_manager.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


static PT_DebugOpr PT_DebugOprHead = NULL;

static int g_iDbgLevelLimit = 8;
/*
*@brief:为链表添加一给新的PT_DebugOpr类型的节点
*/       
int RegisterDebugOpr(PT_DebugOpr ptDebugOpr)
{

    if(!PT_DebugOprHead)   
    {
        PT_DebugOprHead = ptDebugOpr;
        PT_DebugOprHead -> pNext = NULL;
    }
    else
    {
        PT_DebugOpr ptTmp = PT_DebugOprHead;
        while(ptTmp -> pNext)
        {
            ptTmp = ptTmp ->pNext;
        }
        ptTmp ->pNext = ptDebugOpr;
        ptDebugOpr ->pNext =NULL;
    }
    return 0;
}

/*
*@brief:打印链表上各个节点的name
*/
int ShowInputOpr(void)
{
    PT_DebugOpr ptTmp = PT_DebugOprHead;
    int i = 0;
    while(ptTmp)
    {
        printf("%02d: %s\r\n", i++, ptTmp ->name );
        ptTmp  = ptTmp -> pNext;
    }

    return 0;
}


/*
*@brief:获取name符合参数一的节点
*/
PT_DebugOpr getDebugOpr(char * name)
{
    PT_DebugOpr PT_Tmp = PT_DebugOprHead;

    while(PT_Tmp)
    {
        if(strcmp(name, PT_Tmp ->name) == 0)
        {
            return PT_Tmp;
        }

        PT_Tmp = PT_Tmp ->pNext;
    }
    return NULL;
}


/*
*@brief:设置打印等级;strBuf = "dbglevel=6" 
*/
int SetDebugLevel(char * pcBuf)
{
    g_iDbgLevelLimit = pcBuf[9] - '0';
	return 0;
}

/*
*@brief:设置打印通道打开和关闭(网络打印通道，标准输出打印通道)
* stdout=0             : 关闭stdout打印
* stdout=1             : 打开stdout打印
* netprint=0           : 关闭netprint打印
* netprint=1           : 打开netprint打印
*/
int SetDebugChanel(char * strBuf)
{
    PT_DebugOpr ptTmp;
    char * pstrTmp;
    char tmpBuf[128];

    printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

    pstrTmp = strchr(strBuf, '=');
    if(!pstrTmp)
    {
        return -1;
    }
    else
    {
        printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

        strncpy(tmpBuf, strBuf,pstrTmp - strBuf);
        tmpBuf[pstrTmp - strBuf] = '\0';
        printf("tmpBuf = %s", tmpBuf);

        ptTmp = getDebugOpr(tmpBuf);
        if(!ptTmp)
        {
            return -1;
        }

        if(pstrTmp[1] == '0')
            ptTmp ->isCanUse = 0;
        else       
            ptTmp ->isCanUse = 1;

        return 0;
        
    }
}


/*
*@brief:调用各种debug方式的DebugPrint函数
*/
int DebugPrint(const char * pcFormat, ...)
{
    va_list tArg;
    char strTmpBuf[1000];
    char *pcTmp;
    int iDbgLevel = DEFAULT_DBGLEVEL;
    int iNum;
    PT_DebugOpr ptTmp = PT_DebugOprHead;

    va_start(tArg, pcFormat);
    iNum = vsprintf(strTmpBuf, pcFormat, tArg);
    va_end(tArg);
    strTmpBuf[iNum] = '\0';

    pcTmp = strTmpBuf;

    if(strTmpBuf[0] == '<' && strTmpBuf[2] == '>')
    {
        iDbgLevel = strTmpBuf[1] - '0';
        if(iDbgLevel >= 0 && iDbgLevel <= 8)
        {
            pcTmp = strTmpBuf +3;
        }
        else
        {
            iDbgLevel = DEFAULT_DBGLEVEL;
        }
    }

    
    if(iDbgLevel > g_iDbgLevelLimit)
    {
        return -1;
    }

    while (ptTmp)
    {
        if(ptTmp ->isCanUse)
        {
            ptTmp ->DebugPrint(pcTmp);
        }
        
        ptTmp = ptTmp ->pNext;
    }
    
    return 0;

}

/*
*@breif:调用各DEBUG通道的初始化函数
*/
int DebugChanelInit(void)
{
    PT_DebugOpr ptTmp = PT_DebugOprHead;

    while(ptTmp)
    {
        if(ptTmp ->isCanUse )
        {
            ptTmp ->DebugInit();
        }
        
        ptTmp = ptTmp->pNext;
    }

    return 0;
}

int DebugInit(void)
{
    int iError = 0;
    iError = RegisterStdoutOpr();
    iError |= RegisterNetPrintOpr();
    return iError;
}