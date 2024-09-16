#ifndef __INPUT_H
#define __INPUT_H

#include <sys/time.h>
#include <pthread.h>

#define INPUT_TYPE_STDIN        0
#define INPUT_TYPE_TOUCHSCREEN  1

#define INPUT_VALUE_UP          0   
#define INPUT_VALUE_DOWN        1
#define INPUT_VALUE_EXIT        2
#define INPUT_VALUE_UNKNOWN     -1

typedef struct TeventOpr{
    int iVal;
    int iType;
    int iX;
    int iY;
    int iPresure;
    int iKey;
    struct timeval tTime;
}T_EventOpr, *PT_EventOpr;

typedef struct TinputOpr{
    char *name;
    pthread_t tThreadID;
    int (* InputDevInit)(void);
    int (* GetInputEvent)(PT_EventOpr ptEventOpr);
    struct TinputOpr *pNext;
}T_InputOpr, *PT_InputOpr;


int registerInputOpr(PT_InputOpr InputOpr);
int registerStdin(void);
int registerTouchScreen(void);
int InputInit(void);
int GetInputEvent(PT_EventOpr ptEvent);
int showInputOpr(void);
int AllInputDevicesInit(void);

#endif // !__INPUT_H

