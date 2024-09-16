#include "page_manager.h"
#include <stdlib.h>
#include "pic_operation.h"
#include "file.h"
#include <stdio.h>
#include "config.h"
#include "display.h"
#include "input.h"
#include "string.h"


T_PicPos g_tSettingPageBmpPos[] = {
    {0,0,0,0,"select_fold.bmp"},
    {0, 0, 0, 0, "interval.bmp"},
	{0, 0, 0, 0, "return.bmp"},
	{0, 0, 0, 0, NULL},
};

static int SettingPageRun(void);
static int GetSettingPageInput(PT_PageLayout ptPageLayout, PT_EventOpr ptInputEvent );

T_PageOpr g_tSettingPageOpr = {
    .name = "setting",
    .Run = SettingPageRun,
    .GetInputEvent = GetSettingPageInput,
};

static T_PageLayout g_tSettingPageLayout = {
	.iMaxTotalBytes = 0,
	.atLayout       = g_tSettingPageBmpPos,
};


static int ShowSettingPage(void)
{
    PT_VideoMem ptVideoMem;
    T_PixelDatas tPixelDatas, tPixelZoomDatas;
    
    PT_PicPos ptPicPos = g_tSettingPageBmpPos;

    int iXres,iYres,iBpp;
    int iConWidth, iConHeight;
    int iX, iY;
    int iError;


    ptVideoMem = GetFreeVideoMem(ID("setting"), 1);
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

            if(strcmp(ptPicPos ->PicName, "return.bmp") == 0)
            {
                ptPicPos ->iLeftTopX = (iXres -iConWidth /2) /2;
                ptPicPos ->iRightBotX = ptPicPos ->iLeftTopX + iConWidth / 2;
                tPixelZoomDatas.iWidth = iConWidth / 2;
                tPixelZoomDatas.iLineBytes = iConWidth / 2 * iBpp / 8;
                tPixelZoomDatas.iTotalBytes = iConWidth / 2 * iConHeight * iBpp /8;
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


static int SettingPageRun(void)
{
    int iError;
    int iRet;
    PT_PicPos ptPicPos = g_tSettingPageLayout.atLayout;
    T_EventOpr tInputEvent;
    int Index = 0;
    int isPress = 0;
    iError = ShowSettingPage();
    if(iError < 0)
    {
        DBG_PRINTF("show setting page failed\r\n");
        return -1;
    }

    while(1)
    {
        iRet = GetSettingPageInput(&g_tSettingPageLayout, &tInputEvent);
        if(tInputEvent.iPresure == 0)
        {
            if(isPress)
            {
                
                isPress = 0;
                ReleaseButtom(&ptPicPos[Index]);

                switch (Index)
                {
                case 0:
                    /*选择目录*/
                    Page("browse")->Run();
                    ShowSettingPage();
                    break;
                case 1:
                    Page("interval") ->Run();
                    ShowSettingPage();
                    /*设置间隔*/
                    break;
                case 2:
                    /*返回*/
                    return 0;

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



static int GetSettingPageInput(PT_PageLayout ptPageLayout, PT_EventOpr ptInputEvent )
{
    T_EventOpr tEvent;
     
    int i = 0;

    PT_PicPos ptPicPos = ptPageLayout->atLayout;

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

int SettingPageInit(void)
{
    return RegisterPageOpr(&g_tSettingPageOpr);
}