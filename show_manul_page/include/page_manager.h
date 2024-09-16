#ifndef _PAGE_MANAGER_H
#define _PAGE_MANAGER_H


#include "input.h"
#include "display.h"
#include "pic_operation.h"

typedef struct tPicPos{
    int iLeftTopX;
    int iLeftTopY;
    int iRightBotX;
    int iRightBotY;
    char *PicName;
}T_PicPos, *PT_PicPos;



typedef struct PageLayout {
	int iLeftTopX;        /* 这个区域的左上角、右下角坐标 */
	int iLeftTopY;
	int iRightBotX;
	int iRightBotY;
	int iBpp;
	int iMaxTotalBytes;
	PT_PicPos atLayout;  /* 这个区域分成好几个小区域 */
}T_PageLayout, *PT_PageLayout;

typedef struct tPageOpr{
    char *name;
    int (*Run)(void);
    int (*GetInputEvent)(PT_PageLayout ptPageLayout, PT_EventOpr ptInputEvent);
    struct tPageOpr *pNext;
}T_PageOpr, *PT_PageOpr;

typedef struct PageCfg {
    int iIntervalSecond;
    char strSeletedDir[256];
}T_PageCfg, *PT_PageCfg;


int ID(char * name);
int RegisterPageOpr(PT_PageOpr ptPageOpr);
int MainPageInit(void);
int PageInit(void);
PT_PageOpr Page(char *PageName);
int ConvertPixelData(T_PicPos tPicPos);
int PushButtom(PT_PicPos ptPicPos);
int ReleaseButtom(PT_PicPos ptPicPos);
int SettingPageInit(void);
int IntervalPageInit(void);
int MergerStringToCenterOfRectangleInVideoMem(int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY, unsigned char *pucTextString, PT_VideoMem ptVideoMem);
void ClearRectangleInVideoMem(int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY, PT_VideoMem ptVideoMem, unsigned int dwColor);
void FreePixelDatasForIcon(PT_PixelDatas ptPixelDatas);
int BrowsePageInit(void);
int GeneratePage(PT_PageLayout ptPageLayout, PT_VideoMem ptVideoMem);
int GenericGetInputEvent(PT_PageLayout ptPageLayout, PT_EventOpr ptInputEvent);
int AutoPageInit(void);
int GetPixelDatasFrmFile(char *strFileName, PT_PixelDatas ptPixelDatas);
void FreePixelDatasFrmFile(PT_PixelDatas ptPixelDatas);
void GetIntervalTime(int *piIntervalSecond);
void GetSelectedDir(char *strSeletedDir);
void GetPageCfg(PT_PageCfg ptPageCfg);
int ManulPageInit(void);
int TimeMSBetween(struct timeval tTimeStart, struct timeval tTimeEnd);
#endif // ! PAGE_MANAGER_H


