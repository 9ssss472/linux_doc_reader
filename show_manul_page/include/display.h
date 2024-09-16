#ifndef _DISP_MANAGER_H
#define _DISP_MANAGER_H

#include "pic_operation.h"

typedef enum {
    VMS_FREE = 0,
    VMS_USED_FOR_PREPARE,
    VMS_USED_FOR_CUR,
}E_VideoMemState;

typedef enum {
    PS_BLANK = 0,
    PS_GENERATING,
    PS_GENERATED,
}E_PicState;

typedef struct VideoMem {
    int ID;
    int bDevFrameBuffer;
    E_VideoMemState eVieoMemState;
    E_PicState ePicState;
    T_PixelDatas tPixelDatas;
    struct VideoMem * ptNext;
}T_VideoMem, *PT_VideoMem;

typedef struct dispOpr{
    char *name;
    int xres;
    int yres;
    int bpp;
    int iLineBytes;
    unsigned char * pucDispMemBase;
    int (*dispInit)(void);
    int (*drawOnePixel)(int x, int y, int color);
    int (*cleanScreen)(unsigned int color);
    int (*ShowPage)(PT_VideoMem ptVideoMem);
    struct dispOpr *pNext;
}T_dispOpr, *PT_dispOpr; 


void FlushPageMem2FB(PT_VideoMem ptPageVideoMem);
int registerDisOpr(PT_dispOpr dispOpr);
int FbInit(void);
int DisplayInit(void);
PT_dispOpr getDispOpr(char * name);
int showDispOpr(void);
int getResolution(int *xRes, int *yRes);
int SelectAndInitDefautDisp(char * name);
int AllocateVidieoMem(int iNum);
PT_VideoMem GetFreeVideoMem(int ID, int iBur);
int GetDispResolution(int *piXres, int *piYres, int *piBpp);
PT_dispOpr GetDefautDispOpr(void);
void FreePageMem(PT_VideoMem ptPageVideoMem);
PT_VideoMem GetDevVideoMem(void);
void ClearVideoMem(PT_VideoMem ptVideoMem, unsigned int dwColor);
#endif 

