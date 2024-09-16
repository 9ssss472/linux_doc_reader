#include "page_manager.h"
#include <stdlib.h>
#include "pic_operation.h"
#include "file.h"
#include <stdio.h>
#include "config.h"
#include "display.h"
#include "input.h"
#include "string.h"
 #include <unistd.h>

static T_PageCfg g_tPageCfg;

static int g_bAutoPlayThreadShouldExit = 0;
static pthread_t g_tAutoPlayThreadID;
static pthread_mutex_t g_tAutoPlayThreadMutex  = PTHREAD_MUTEX_INITIALIZER; /* 互斥量 */

PT_DirContent *pptDirContents;

static int g_iFileCountHaveGet = 0;
static int g_iCurFileNumber = 0;

static void ResetAutoPlayFile(void)
{
    g_iCurFileNumber = 0;
    g_iFileCountHaveGet = 0;
}

static int GetNextAutoPlayFile(char *strFileName)
{
    if(g_iCurFileNumber > g_iFileCountHaveGet -1)
    {
        g_iCurFileNumber = 0;
    }
    snprintf(strFileName, 256, "%s/%s", g_tPageCfg.strSeletedDir, pptDirContents[g_iCurFileNumber ++] ->strName);
    strFileName[255] = '\0';
    return 0;
}


/* bCur = 1 表示必须获得videomem, 因为这是马上就要在LCD上显示出来的
 * bCur = 0 表示这是做准备用的, 有可能无法获得videomem
 */
static PT_VideoMem PrepareNextPicture(int bCur)
{
	T_PixelDatas tOriginIconPixelDatas;
	T_PixelDatas tPicPixelDatas;
    PT_VideoMem ptVideoMem;
	int iError;
	int iXres, iYres, iBpp;
    int iTopLeftX, iTopLeftY;
    float k;
    char strFileName[256];
    
	GetDispResolution(&iXres, &iYres, &iBpp);
    
	/* 获得显存 */
	ptVideoMem = GetFreeVideoMem(-1, bCur);
	if (ptVideoMem == NULL)
	{
		DBG_PRINTF("can't get video mem for browse page!\n");
		return NULL;
	}
    ClearVideoMem(ptVideoMem, COLOR_BACKGROUND);

    while (1)
    {
        iError = GetNextAutoPlayFile(strFileName);
        if (iError)
        {
            DBG_PRINTF("GetNextAutoPlayFile error\n");
            FreePageMem(ptVideoMem);
            return NULL;
        }

        
        
        iError = GetPixelDatasFrmFile(strFileName, &tOriginIconPixelDatas);
        if (0 == iError)
        {
            break;
        }
    }

    /* 把图片按比例缩放到LCD屏幕上, 居中显示 */
    k = (float)tOriginIconPixelDatas.iHeight / tOriginIconPixelDatas.iWidth;
    tPicPixelDatas.iWidth  = iXres;
    tPicPixelDatas.iHeight = iXres * k;
    if (tPicPixelDatas.iHeight > iYres)
    {
        tPicPixelDatas.iWidth  = iYres / k;
        tPicPixelDatas.iHeight = iYres;
    }
    tPicPixelDatas.iBpp        = iBpp;
    tPicPixelDatas.iLineBytes  = tPicPixelDatas.iWidth * tPicPixelDatas.iBpp / 8;
    tPicPixelDatas.iTotalBytes = tPicPixelDatas.iLineBytes * tPicPixelDatas.iHeight;
    tPicPixelDatas.aucPixelDatas = malloc(tPicPixelDatas.iTotalBytes);
    if (tPicPixelDatas.aucPixelDatas == NULL)
    {
        FreePageMem(ptVideoMem);
        return NULL;
    }

    PicZoom(&tOriginIconPixelDatas, &tPicPixelDatas);

    /* 算出居中显示时左上角坐标 */
    iTopLeftX = (iXres - tPicPixelDatas.iWidth) / 2;
    iTopLeftY = (iYres - tPicPixelDatas.iHeight) / 2;
    PicMerge(iTopLeftX, iTopLeftY, &tPicPixelDatas, &ptVideoMem->tPixelDatas);
    FreePixelDatasFrmFile(&tOriginIconPixelDatas);
    free(tPicPixelDatas.aucPixelDatas);

    return ptVideoMem;
}

static void *AutoPlayThreadFunction(void *pVoid)
{
    int bExit;
    PT_VideoMem ptVideoMem;

    ResetAutoPlayFile();

    int iNum;

    printf("g_tPageCfg.strSeletedDir = %s\r\n",g_tPageCfg.strSeletedDir);

    GetDirContents(g_tPageCfg.strSeletedDir, &pptDirContents,&iNum);
    g_iFileCountHaveGet = iNum;
    while (1)
    {
        /* 1. 先判断是否要退出 */
        pthread_mutex_lock(&g_tAutoPlayThreadMutex);
        bExit = g_bAutoPlayThreadShouldExit;
        pthread_mutex_unlock(&g_tAutoPlayThreadMutex);

        if (bExit)
        {
            return NULL;
        }

        /* 2. 准备要显示的图片 */
        ptVideoMem = PrepareNextPicture(0);

        /* 3. 时间到后就显示出来 */
        if (ptVideoMem == NULL)
        {
            ptVideoMem = PrepareNextPicture(1);
        }

    	/* 刷到设备上去 */
    	FlushPageMem2FB(ptVideoMem);

    	/* 解放显存 */
    	FreePageMem(ptVideoMem);        

        sleep(g_tPageCfg.iIntervalSecond);       /* 先用休眠来代替 */
    }
    return NULL;
}
static int AutoPageRun(void)
{
	T_EventOpr tInputEvent;
	int iRet;
    
    g_bAutoPlayThreadShouldExit = 0;
        /* 获得配置值: 显示哪一个目录, 显示图片的间隔 */
    GetPageCfg(&g_tPageCfg);
        
    /* 1. 启动一个线程来连续显示图片 */
    pthread_create(&g_tAutoPlayThreadID, NULL, AutoPlayThreadFunction, NULL);

    /* 2. 当前线程等待触摸屏输入, 先做简单点: 如果点击了触摸屏, 让线程退出 */
    while (1)
    {
        iRet = GetInputEvent(&tInputEvent);
        if (0 == iRet)
        {
            pthread_mutex_lock(&g_tAutoPlayThreadMutex);
            g_bAutoPlayThreadShouldExit = 1;   /* AutoPlayThreadFunction线程检测到这个变量为1后会退出 */
            pthread_mutex_unlock(&g_tAutoPlayThreadMutex);
            sleep(2); /* 也许AutoPlayThreadFunction线程退出没那么快,在这里稍等一会 */
            return 0;
        }
    }
    return 0;
}
static T_PageOpr g_tAutoPageAction = {
	.name          = "auto",
	.Run           = AutoPageRun,
};

int AutoPageInit(void)
{
	return RegisterPageOpr(&g_tAutoPageAction);
}


