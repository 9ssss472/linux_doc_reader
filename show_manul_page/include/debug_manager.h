#ifndef  _DEBUG__H
#define  _DEBUG__H

typedef struct  tDebugOpr
{
    int isCanUse;
    char *name;
    int (*DebugInit)(void);
    int (*DebugPrint)(char * buf);
    struct tDebugOpr *pNext;
}T_DebugOpr, *PT_DebugOpr;


#define DEFAULT_DBGLEVEL  4

int RegisterDebugOpr(PT_DebugOpr ptDebugOpr);
int SetDebugLevel(char * pcBuf);
int SetDebugChanel(char * strBuf);
int RegisterNetPrintOpr(void);
int DebugInit(void);
int DebugChanelInit(void);
int DebugPrint(const char * pcFormat, ...);
int ShowInputOpr(void);
int RegisterStdoutOpr(void);
#endif // ! _DEBUG__H
