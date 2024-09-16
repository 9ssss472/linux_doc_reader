#include "debug_manager.h"
#include <stdio.h>
#include <string.h>

static int StdoutInit(void);
static int StdoutPrint(char * buf);


static T_DebugOpr T_StdoutOpr = {
    .name = "stdout",
    .isCanUse = 1,
    .DebugPrint = StdoutPrint,
    .DebugInit = StdoutInit,
};


static int StdoutPrint(char * buf)
{
    printf("%s", buf);

    return strlen(buf);
}

static int StdoutInit(void)
{
    return 0;
}

int RegisterStdoutOpr(void)
{
    return RegisterDebugOpr(&T_StdoutOpr);
}