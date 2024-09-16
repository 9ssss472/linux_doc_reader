#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>


static char g_cCommandBuf[128];
static pthread_mutex_t g_tMutex;
static pthread_cond_t g_tCond;
int g_iFd;
struct sockaddr_in g_tServerAddr;
socklen_t g_tSeverAddrLen;
/*
 *  dbglevel=<0-9>
 *  stdout=0|1
 *  netprint=0|1
 *  show // setclient,并且接收打印信息
 */
static void *StdinThreadFunc(void * pVoid)
{

    while(1)
    {

        fgets(g_cCommandBuf, 127, stdin);
        printf("g_cCommandBuf :%s\r\n",g_cCommandBuf);

        pthread_mutex_lock(&g_tMutex);

        pthread_mutex_unlock(&g_tMutex);
        pthread_cond_signal(&g_tCond);
      
       
    }

    return NULL;
}


static void *SocketRecvThreadFunc(void * pVoid)
{
    char cRecvBuf[512];
    int iRecvLen;
    int iAddrLen;
    iAddrLen = sizeof(struct sockaddr);
    while(1)
    { 
        iRecvLen = recvfrom(g_iFd, cRecvBuf, 512, 0,( struct sockaddr *)&g_tServerAddr, &iAddrLen);
        if(iRecvLen > 0 )
        {
            cRecvBuf[iRecvLen] = '\0';
            printf("Recv:%s\r\n",cRecvBuf);
        }

    }

    return NULL;
}

static void *SocketSendThreadFunc(void * pVoid)
{
    int iSendLen;

    while(1)
    {
        pthread_mutex_lock(&g_tMutex);

        pthread_cond_wait(&g_tCond, &g_tMutex);

        iSendLen = sendto(g_iFd, g_cCommandBuf, sizeof(g_cCommandBuf), 0,(struct sockaddr *)&g_tServerAddr, g_tSeverAddrLen );
       
        if(iSendLen == -1)
        {
            perror("sendto");
            return NULL;
        }

        pthread_mutex_unlock(&g_tMutex);
    }

    return NULL;
}



int main(int argc, char** argv)
{
    
    pthread_t tStdinID, tSocketRecvID, tSocketSendID;

    if(argc != 3)
    {
        printf("Usage: %s <server addr> <server port>\r\n", argv[0]);
        return -1;
    }

    
    g_tServerAddr.sin_family = AF_INET;
    g_tServerAddr.sin_port = htons(atoi(argv[2]));
    if(0 == inet_aton(argv[1] , &g_tServerAddr.sin_addr))
    {
        printf("invalid server_ip\n");
		return -1;
    }

    g_tSeverAddrLen = sizeof(g_tServerAddr);

    g_iFd = socket(AF_INET, SOCK_DGRAM, 0);
    if(g_iFd < 0)
    {
        perror("socket");
        return -1;
    }

    sendto(g_iFd, "setclient",9, 0,(struct sockaddr *)&g_tServerAddr, g_tSeverAddrLen );

    pthread_mutex_init(&g_tMutex, NULL);
    pthread_cond_init(&g_tCond, NULL);

    pthread_create(&tSocketSendID, NULL, SocketSendThreadFunc, NULL);

    pthread_create(&tSocketRecvID, NULL, SocketRecvThreadFunc, NULL);

    pthread_create(&tStdinID, NULL, StdinThreadFunc, NULL);

  
    
    pthread_join(tStdinID , NULL);
    pthread_join(tSocketRecvID , NULL);
    pthread_join(tSocketSendID , NULL);

    return 0;  
}
