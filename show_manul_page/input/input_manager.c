#include "input.h"
#include "stdlib.h"
#include "stdio.h"
#include <pthread.h>
#include "config.h"

static PT_InputOpr PT_InputOprHead = NULL;

pthread_mutex_t g_tMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_tCond = PTHREAD_COND_INITIALIZER;
T_EventOpr g_tEvent;

/*
*@brief:为链表添加一给新的PT_InputOpr类型的节点
*/       
int registerInputOpr(PT_InputOpr InputOpr)
{

    if(!PT_InputOprHead)   
    {
        PT_InputOprHead = InputOpr;
        PT_InputOprHead -> pNext = NULL;
    }
    else
    {
        PT_InputOpr ptTmp = PT_InputOprHead;
        while(ptTmp -> pNext)
        {
            ptTmp = ptTmp ->pNext;
        }
        ptTmp ->pNext = InputOpr;
        InputOpr ->pNext =NULL;
    }
    return 0;
}

/*
*@brief:打印链表上各个节点的name
*/
int showInputOpr(void)
{
    PT_InputOpr ptTmp = PT_InputOprHead;
    int i = 0;
    while(ptTmp )
    {
        DBG_PRINTF("%02d: %s\r\n", i++, ptTmp ->name );
        ptTmp  = ptTmp -> pNext;
    }

    return 0;
}

/*
*@breif:使主线程进入休眠，等待输入事件唤醒
*/
int GetInputEvent(PT_EventOpr ptEvent)
{
    pthread_mutex_lock(&g_tMutex);

    pthread_cond_wait(&g_tCond, &g_tMutex);

    *ptEvent = g_tEvent;

    pthread_mutex_unlock(&g_tMutex);

    return 0;
}

/*
*@brief:当有输入事件时，唤醒主线程;没有输入事件时进入阻塞
*/
static void* InputEventThreadFunc(void * arg)
{
    int (* GetInputEvent)(PT_EventOpr ptEventOpr);
    GetInputEvent = (int (*)(PT_EventOpr))arg;

    T_EventOpr tEventOpr;

    while(1)
    {
        if ( 0 == GetInputEvent(&tEventOpr))
        {
            pthread_mutex_lock(&g_tMutex);

            g_tEvent = tEventOpr;

            pthread_cond_signal(&g_tCond);

            pthread_mutex_unlock(&g_tMutex);

        }

    }
    return NULL;
}


int AllInputDevicesInit(void)
{
	PT_InputOpr ptTmp = PT_InputOprHead;
	int iError = -1;

	while (ptTmp)
	{
		if (0 == ptTmp->InputDevInit())
		{
            pthread_create(&ptTmp ->tThreadID, NULL, InputEventThreadFunc, ptTmp ->GetInputEvent);
			iError = 0;
		}
		ptTmp = ptTmp->pNext;
	}
	return iError;
}


int InputInit(void)
{
    int iError = 0;

    iError = registerStdin();
    iError |= registerTouchScreen();

    return iError;
}



