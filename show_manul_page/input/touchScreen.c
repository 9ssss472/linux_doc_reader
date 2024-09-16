#include "input.h"
#include <tslib.h>
#include "display.h"
#include "config.h"

static int TouchScreenInit(void);
static int GetTouchScreenEvent(PT_EventOpr ptEventOpr);

T_InputOpr T_TouchScreenOpr =  {
    .name = "TouchScreen",
    .InputDevInit = TouchScreenInit,
    .GetInputEvent = GetTouchScreenEvent,
};

struct tsdev * g_tsdev;
int g_iXres, g_iYres;

static int TouchScreenInit(void)
{
    int iBpp;
    g_tsdev = ts_setup(NULL, 0);
    if(g_tsdev == NULL)
    {
        DBG_PRINTF("ts_setup failed!\r\n");
        return -1;
    }

    if(GetDispResolution(&g_iXres, &g_iYres, &iBpp))
    {
        DBG_PRINTF("getResolution failed\r\n");
        return -1;
    }

    return 0;
}


static int isOutOf1ms(struct timeval *preTime, struct timeval *curTime)
{
    unsigned int preMs, curMs;

    preMs = preTime ->tv_sec * 1000 + preTime ->tv_usec /1000;
    curMs = curTime ->tv_sec * 1000 + curTime ->tv_usec /1000;

    return (curMs > preMs + 1);
}


static int GetTouchScreenEvent(PT_EventOpr ptEventOpr)
{
    struct ts_sample tSample;
    static struct timeval tPreTime;
    int iRet;


    iRet = ts_read(g_tsdev, &tSample, 1);
    if(iRet < 0)
    {
        return -1;
    }

    

    if(isOutOf1ms(&tPreTime, &tSample.tv))
    {
        tPreTime = tSample.tv;

        ptEventOpr ->tTime = tSample.tv;
        ptEventOpr ->iType = INPUT_TYPE_TOUCHSCREEN;
        ptEventOpr ->iX = tSample.x;
        ptEventOpr ->iY = tSample.y;
        ptEventOpr ->iPresure = tSample.pressure;
    }
    else
    {
        return -1;
    }

    return 0;
}


int registerTouchScreen(void)
{
    return registerInputOpr(&T_TouchScreenOpr);
}
