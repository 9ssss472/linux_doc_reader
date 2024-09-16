#include "page_manager.h"
#include <stdlib.h>
#include "pic_operation.h"
#include "file.h"
#include <stdio.h>
#include "config.h"
#include "display.h"
#include "input.h"

T_PicPos g_tMainPageBmpPos[] = {
    {0,0,0,0,"browse_mode.bmp"},
    {0, 0, 0, 0, "continue_mod.bmp"},
	{0, 0, 0, 0, "setting.bmp"},
	{0, 0, 0, 0, NULL},
};


static int MainPageRun(void);
static int GetMainPageInput(PT_PageLayout ptPageLayout, PT_EventOpr ptInputEvent );

T_PageOpr g_tMainPageOpr = {
    .name = "main",
    .Run = MainPageRun,
    .GetInputEvent = GetMainPageInput,
};

static T_PageLayout g_tMainPageLayout = {
	.iMaxTotalBytes = 0,
	.atLayout       = g_tMainPageBmpPos,
};



static int ShowMainPage(void)
{
    PT_VideoMem ptVideoMem;
    T_PixelDatas tPixelDatas, tPixelZoomDatas;
    
    PT_PicPos ptPicPos = g_tMainPageBmpPos;

    int iXres,iYres,iBpp;
    int iConWidth, iConHeight;
    int iX, iY;
    int iError;


    ptVideoMem = GetFreeVideoMem(ID("main"), 1);
    if(ptVideoMem == NULL)
    {
        DBG_PRINTF("GetFreeVideoMem failed\r\n");
        return -1;
    }

    if(ptVideoMem ->ePicState != PS_GENERATED )
    {
        GetDispResolution(&iXres, &iYres, &iBpp);

        iConHeight = iYres /10 * 2;
        iConWidth = iConHeight * 2;

        iX = (iXres - iConWidth) / 2;
        iY = iYres / 10;

        tPixelZoomDatas.iWidth = iConWidth;
        tPixelZoomDatas.iHeight = iConHeight;
        tPixelZoomDatas.iBpp = iBpp;
        tPixelZoomDatas.iLineBytes = iConWidth * iBpp / 8;
        tPixelZoomDatas.iTotalBytes = iConWidth * iConHeight * iBpp /8;
        tPixelZoomDatas.aucPixelDatas = malloc(tPixelZoomDatas.iTotalBytes);

        if(tPixelZoomDatas.aucPixelDatas == NULL)
        {
            DBG_PRINTF("malloc tPixelZoomDatas.aucPixelDatas failed\r\n");
            return -1;
        }

        while(ptPicPos ->PicName)
        {
            ptPicPos ->iLeftTopX = iX;
            ptPicPos ->iLeftTopY = iY;
            ptPicPos ->iRightBotX = iX + iConWidth -1;
            ptPicPos ->iRightBotY = iY + iConHeight -1;

            iError = GetFilePixel(ptPicPos ->PicName, &tPixelDatas);
            if (iError < 0)
            {
                DBG_PRINTF("GetFilePixel error\r\n");
                return -1;
            }

            PicZoom(&tPixelDatas, &tPixelZoomDatas);
            PicMerge(ptPicPos ->iLeftTopX, ptPicPos ->iLeftTopY, &tPixelZoomDatas, &ptVideoMem ->tPixelDatas);
            free(tPixelDatas.aucPixelDatas);
            iY += iYres / 10 * 3;
            ptPicPos ++;
        }
        ptVideoMem ->ePicState = PS_GENERATED;
        free(tPixelZoomDatas.aucPixelDatas);
    }

    FlushPageMem2FB( ptVideoMem);

    FreePageMem(ptVideoMem);

    return 0;

}



static int MainPageRun(void)
{
    int iError;
    int iRet;
    PT_PicPos ptPicPos = g_tMainPageLayout.atLayout;
    T_EventOpr tInputEvent;
    int Index = 0;
    int isPress = 0;
    iError = ShowMainPage();
    if(iError < 0)
    {
        DBG_PRINTF("show main page failed\r\n");
        return -1;
    }

    while(1)
    {
        iRet = GetMainPageInput(&g_tMainPageLayout, &tInputEvent);
        if(tInputEvent.iPresure == 0)
        {
            if(isPress)
            {
                isPress = 0;
                ReleaseButtom(&ptPicPos[Index]);

                switch (Index)
                {
                case 0:
                    /*浏览模式*/
                    DBG_PRINTF("%s %s %d\r\n", __FILE__, __FUNCTION__, __LINE__);
                    Page("manul") ->Run();
                    ShowMainPage();
                    break;
                case 1:
                    /*连播模式*/
                    Page("auto") ->Run();
                    ShowMainPage();
                    break;
                case 2:
                    /*设置*/
                    Page("setting") ->Run();
                    ShowMainPage();
                    break;
                
                default:
                    break;
                }
            }
            
        }
        else
        {
            if (iRet >= 0)
            {
                if(!isPress)
                {
                    isPress = 1;
                    Index = iRet;
                    PushButtom(&ptPicPos[Index]);
                }
            }
           
        }
        
    }
    return 0;

}



static int GetMainPageInput(PT_PageLayout ptPageLayout, PT_EventOpr ptInputEvent )
{
    T_EventOpr tEvent;
     
    int i = 0;

    PT_PicPos ptPicPos = ptPageLayout ->atLayout;

    GetInputEvent(&tEvent);

    if(tEvent.iType == INPUT_TYPE_STDIN)
    {
        return -1;
    }

    while(ptPicPos ->PicName)
    {
        if ((tEvent.iX > ptPicPos->iLeftTopX) && (tEvent.iY > ptPicPos->iLeftTopY) \
        && (tEvent.iX < ptPicPos ->iRightBotX) && (tEvent.iY < ptPicPos ->iRightBotY ) )
        {
            *ptInputEvent = tEvent;
            
            DBG_PRINTF("i = %d\r\n",i);
            return i;

        }
        else
        {    
            i++;
            ptPicPos ++;
        }
    }

    return -1;
}

int MainPageInit(void)
{
    return RegisterPageOpr(&g_tMainPageOpr);
}