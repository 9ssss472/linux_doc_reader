#include "debug_manager.h"
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "config.h"

#define NET_PORT 5678
#define PRINT_BUF_SIZE   (16*1024)

pthread_t g_tSendThreadID, g_tRecvThreadID;

static int  NetPrint(char * buf);
static int NetPrintInit(void);
static int g_iSockFd;
static char * g_pcNetPrintBuf;
static pthread_mutex_t g_tMutex;
static pthread_cond_t g_tCond;
static struct sockaddr_in tSockAddr;
static int g_iWritePos = 0, g_iReadPos = 0;
static int g_isConected = 0;
static struct sockaddr_in g_tClientSock;

static T_DebugOpr T_NetPrintOpr = {
    .name = "netprint",
    .isCanUse = 1,
    .DebugPrint = NetPrint,
    .DebugInit = NetPrintInit,
};

static int isFull(void)
{
    return (((g_iWritePos +1) % PRINT_BUF_SIZE) == g_iReadPos);
}

static int isEmpty(void)
{
   
    return (g_iReadPos == g_iWritePos);
}

static int PutData(char cVal)
{
    if(isFull())
        return -1;
    else
    {
        g_pcNetPrintBuf[g_iWritePos] = cVal;
        g_iWritePos = (g_iWritePos +1) % PRINT_BUF_SIZE;
        return 0;
    }
}

static int GetData(char *cVal)
{
    if(isEmpty())
        return -1;
    else
    {
        *cVal = g_pcNetPrintBuf[g_iReadPos];
        g_iReadPos = (g_iReadPos + 1) % PRINT_BUF_SIZE;
        return 0;
    }
}




/*
*@breif:发送线程函数
*/
static void* SendThreadFunc(void * pVoid)
{
    char cSendBuf[512];
    int i;
    char cVal;
    
    socklen_t t_socklen;
    t_socklen = sizeof(struct sockaddr_in);

    while(1)
    {
        pthread_mutex_lock(&g_tMutex);
        pthread_cond_wait(&g_tCond, &g_tMutex);

        pthread_mutex_unlock(&g_tMutex);

        while(g_isConected && !isEmpty())
        {
            for(i = 0; i < 512; i++)
            {
                if( 0 == GetData(&cVal))
                {
                    cSendBuf[i] = cVal;
                }
                else
                {
                    break;
                }
                
            }
           
        sendto(g_iSockFd, cSendBuf, i,0, (struct sockaddr *)&g_tClientSock, t_socklen );            
        }
      
    }
    return NULL;
}



/*
*@breif:接受线程函数
*/
static void* RecvThreadFunc(void *pVoid)
{
    char cRecvBuf[128];
    int iRecvLen;
    struct sockaddr_in tClientSock;
    socklen_t t_socklen;
    t_socklen = sizeof(struct sockaddr_in);


    while(1)
    {
        iRecvLen = recvfrom(g_iSockFd,cRecvBuf, sizeof(cRecvBuf), 0,(struct sockaddr *)&tClientSock, &t_socklen);

        if(iRecvLen > 0)
        {
            	/* 解析数据:
			 * setclient            : 设置接收打印信息的客户端
			 * dbglevel=0,1,2...    : 修改打印级别
			 * stdout=0             : 关闭stdout打印
			 * stdout=1             : 打开stdout打印
			 * netprint=0           : 关闭netprint打印
			 * netprint=1           : 打开netprint打印
			 */
           
            printf("cRecvBuf:%s",cRecvBuf);
            if(strncmp(cRecvBuf, "setclient", 9) == 0)
            {
                g_isConected = 1;
                g_tClientSock = tClientSock;
                printf("client has connected\r\n");
            }
            else if(strncmp(cRecvBuf, "dbglevel=", 9) == 0)
            {
                printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                SetDebugLevel(cRecvBuf);
            }
            else
            {
                printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                SetDebugChanel(cRecvBuf);
            }
            printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        }
    }

    return NULL;
}


/*
*@brief:把要打印的数据存入Buf,唤醒发送线程
*/
static int  NetPrint(char * pcBuf)
{
    int i;

    for( i = 0; i < strlen(pcBuf); i++)
    {
        if ( 0 != PutData(pcBuf[i]))
        {
            break;
        }
    }

    pthread_mutex_lock(&g_tMutex);
    pthread_cond_signal(&g_tCond);

    pthread_mutex_unlock(&g_tMutex);


    return i;
}


/*
*@breif:初始化UDP服务端，创建发送和接受线程
*/
static int NetPrintInit(void)
{
    int iError;

    tSockAddr.sin_family  = AF_INET;
    tSockAddr.sin_port = htons(NET_PORT);
    memset(tSockAddr.sin_zero , 0, sizeof(tSockAddr.sin_zero));
    tSockAddr.sin_addr.s_addr = INADDR_ANY;

    
    g_iSockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if(g_iSockFd < 0)
    {
        printf("bind error!\n");
        return -1;
    }

    iError = bind(g_iSockFd,(struct sockaddr *)&tSockAddr, sizeof(tSockAddr));
    if(iError < 0)
    {
        printf("bind error!\n");
        return -1;
    }

    g_pcNetPrintBuf = malloc(PRINT_BUF_SIZE);
    if(g_pcNetPrintBuf == NULL)
    {
        close(g_iSockFd);
        return -1;
    }

    pthread_create(&g_tSendThreadID, NULL, SendThreadFunc, NULL);

    pthread_create(&g_tRecvThreadID, NULL, RecvThreadFunc, NULL);


    return 0;
}

int RegisterNetPrintOpr(void)
{
    return RegisterDebugOpr(&T_NetPrintOpr);
}

