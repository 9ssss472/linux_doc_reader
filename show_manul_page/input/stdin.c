#include "input.h"
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include "config.h"

static int StdinInit(void);
static int GetStdinEvent(PT_EventOpr ptEventOpr);

T_InputOpr T_StdinOpr =  {
    .name = "stdin",
    .InputDevInit = StdinInit,
    .GetInputEvent = GetStdinEvent,
};


static int StdinInit(void)
{
    struct termios tTTYState;

    tcgetattr(STDIN_FILENO, &tTTYState);

    tTTYState.c_lflag &= ~ICANON;
    tTTYState.c_cc[VMIN] = 1;

    tcsetattr(STDIN_FILENO, TCSANOW, &tTTYState);

    return 0;
}

static int GetStdinEvent(PT_EventOpr ptEventOpr)
{

    char c;
    
    c = fgetc(stdin);
    ptEventOpr ->iType = INPUT_TYPE_STDIN;

    gettimeofday(&ptEventOpr->tTime, NULL);

    
#if 0

    switch (c)
    {
    case 'n':
        ptEventOpr ->iVal= INPUT_VALUE_DOWN;
        break;
    case 'u':
        ptEventOpr ->iVal= INPUT_VALUE_UP;
        break;
    case 'q':
        ptEventOpr ->iVal= INPUT_VALUE_EXIT;
        break;
    
    default:
        ptEventOpr ->iVal= INPUT_VALUE_UNKNOWN;
        break;
    }
#endif

    return 0;

}

int registerStdin(void)
{
    return registerInputOpr(&T_StdinOpr);
}


