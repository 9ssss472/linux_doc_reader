#include "page_manager.h"
#include <stdlib.h>
#include "pic_operation.h"
#include "file.h"
#include <stdio.h>
#include "config.h"
#include "display.h"
#include "input.h"
#include "string.h"
#include "fonts_manager.h"

T_PicPos g_tIntervalPageBmpPos[] = {
	{0, 0, 0, 0, "inc.bmp"},
	{0, 0, 0, 0, "time.bmp"},
	{0, 0, 0, 0, "dec.bmp"},
	{0, 0, 0, 0, "ok.bmp"},
	{0, 0, 0, 0, "cancel.bmp"},
	{0, 0, 0, 0, NULL},
};

static int IntervalPageRun(void);
static int GetIntervalPageInput(PT_PageLayout ptPageLayout, PT_EventOpr ptInputEvent );

T_PageOpr g_tIntervalPageOpr = {
    .name = "interval",
    .Run = IntervalPageRun,
    .GetInputEvent = GetIntervalPageInput,

};

static T_PageLayout g_tIntervalPageLayout = {
	.iMaxTotalBytes = 0,
	.atLayout       = g_tIntervalPageBmpPos,
};

static int g_iIntervalSecond = 7;


void GetIntervalTime(int *piIntervalSecond)
{
    *piIntervalSecond = g_iIntervalSecond;
}

static int ShowIntervalPage(void)
{
    PT_VideoMem ptVideoMem;
    PT_PicPos ptPicPos = g_tIntervalPageBmpPos;
    T_PixelDatas  tPixelZoomDatas, tPixelDatas;
    PT_PixelDatas ptPixelDatas;
    int iHeight, iWidth;
    int iXres,iYres,iBpp;
    int iY, iX;
    int iRet;

    ptVideoMem = GetFreeVideoMem(ID("interval"), 1);
    if(!ptVideoMem)
    {
        DBG_PRINTF("GetFreeVideoMem failed\r\n");
        return -1;
    }
    DBG_PRINTF("%s %s %d\r\n", __FILE__, __FUNCTION__, __LINE__);
    if(ptVideoMem ->ePicState != PS_GENERATED)
    {
        ptPixelDatas =  &ptVideoMem ->tPixelDatas;
        GetDispResolution(&iXres, &iYres, &iBpp);

        iHeight = iYres /10 * 1.5;
        iWidth = iHeight * 2;

        ptPicPos->iLeftTopX = (iXres - iWidth / 2) / 2 - 1;
        ptPicPos->iLeftTopY = iYres / 10 - 1;
        ptPicPos->iRightBotX = ptPicPos->iLeftTopX + iWidth /2;
        ptPicPos->iRightBotY = ptPicPos->iLeftTopY + iHeight;

        iY = ptPicPos->iRightBotY + 1;
        ptPicPos++;

        ptPicPos->iLeftTopX = (iXres - iWidth) / 2 - 1;
        ptPicPos->iLeftTopY = iY;
        ptPicPos->iRightBotX = ptPicPos->iLeftTopX + iWidth;
        ptPicPos->iRightBotY = ptPicPos->iLeftTopY + iHeight;

        iY = ptPicPos->iRightBotY + 1;
        ptPicPos++;

        ptPicPos->iLeftTopX = (iXres - iWidth / 2 ) / 2 - 1;
        ptPicPos->iLeftTopY = iY;
        ptPicPos->iRightBotX = ptPicPos->iLeftTopX + iWidth /2;
        ptPicPos->iRightBotY = ptPicPos->iLeftTopY + iHeight;

        iY = ptPicPos->iRightBotY + iHeight;
        iX = (iXres - iWidth  ) / 4 - 1;
        ptPicPos++;

        while(ptPicPos ->PicName != NULL)
        {
            ptPicPos->iLeftTopX = iX;
            ptPicPos->iLeftTopY = iY;
            ptPicPos->iRightBotY = ptPicPos->iLeftTopY + iHeight;
            ptPicPos->iRightBotX = ptPicPos->iLeftTopX + iWidth;
            ptPicPos++;
            iX = iXres - 2*iX ;
        }
        ptPicPos = g_tIntervalPageBmpPos;

        tPixelZoomDatas.iHeight = iHeight;
        tPixelZoomDatas.iBpp = iBpp;
        tPixelZoomDatas.iTotalBytes = iWidth * iHeight * iBpp /8;

        DBG_PRINTF("%s %s %d\r\n", __FILE__, __FUNCTION__, __LINE__);

        tPixelZoomDatas.aucPixelDatas = malloc(tPixelZoomDatas.iTotalBytes * 2);
        if(tPixelZoomDatas.aucPixelDatas == NULL)
        {
            perror("malloc failed\r\n");
            return -1;
        }

        DBG_PRINTF("%s %s %d\r\n", __FILE__, __FUNCTION__, __LINE__);
        while(ptPicPos ->PicName != NULL)
        {
            iRet =  GetFilePixel(ptPicPos ->PicName, &tPixelDatas);
            if(iRet)
            {
                DBG_PRINTF("GetFilePixel failed\r\n");
                return -1;
            }

            

            if(strcmp(ptPicPos ->PicName , "inc.bmp") == 0 || strcmp(ptPicPos ->PicName , "dec.bmp") == 0)
            {
                tPixelZoomDatas.iWidth = iWidth / 2;
            }
            else
            {
                tPixelZoomDatas.iWidth = iWidth;
                
            }
            tPixelZoomDatas.iLineBytes = tPixelZoomDatas.iWidth * iBpp / 8;
            tPixelZoomDatas.iTotalBytes = tPixelZoomDatas.iLineBytes * tPixelZoomDatas.iHeight;
            
            PicZoom(&tPixelDatas, &tPixelZoomDatas);

            PicMerge(ptPicPos ->iLeftTopX, ptPicPos ->iLeftTopY,  &tPixelZoomDatas, ptPixelDatas);

            ptPicPos ++;
            free(tPixelDatas.aucPixelDatas);    
        }
        free(tPixelZoomDatas.aucPixelDatas);
        ptVideoMem ->eVieoMemState = PS_GENERATED;

    }

    FlushPageMem2FB(ptVideoMem);
    FreePageMem(ptVideoMem);
    return 0;
}

/* 绘制图标中的数字 */
static int GenerateIntervalPageSpecialIcon(int dwNumber, PT_VideoMem ptVideoMem)
{
	unsigned int dwFontSize;
	char strNumber[3];
	int iError;
	

	dwFontSize = g_tIntervalPageBmpPos[1].iRightBotY - g_tIntervalPageBmpPos[1].iLeftTopY;
	SetFontSize(dwFontSize);

	/* 显示两位数字: 00~59 */
	if (dwNumber > 59)
	{
		return -1;
	}

	snprintf(strNumber, 3, "%02d", dwNumber);
	DBG_PRINTF("strNumber = %s, len = %d\n", strNumber, strlen(strNumber));

	iError = MergerStringToCenterOfRectangleInVideoMem(g_tIntervalPageBmpPos[1].iLeftTopX, g_tIntervalPageBmpPos[1].iLeftTopY, \
                                                        g_tIntervalPageBmpPos[1].iRightBotX, g_tIntervalPageBmpPos[1].iRightBotY, \
                                                        (unsigned char *)strNumber, ptVideoMem);

	return iError;
}

static int isOutOf500ms(struct timeval *preTime, struct timeval *curTime)
{
    unsigned int preMs, curMs;

    preMs = preTime ->tv_sec * 1000 + preTime ->tv_usec /1000;
    curMs = curTime ->tv_sec * 1000 + curTime ->tv_usec /1000;

    return (curMs > preMs + 500);
}

static int IntervalPageRun(void)
{
    int iError;
    int iRet;
    PT_PicPos ptPicPos = g_tIntervalPageLayout.atLayout;
    T_EventOpr tInputEvent;
    int isPress = 0;
    int Index = 0;
    int iNumber = g_iIntervalSecond;
    PT_VideoMem ptVideoMem;
    struct timeval preTime, curTime;

    ptVideoMem = GetDevVideoMem();
    
    iError = ShowIntervalPage();
    if(iError < 0)
    {
        DBG_PRINTF("show interval page failed\r\n");
        return -1;
    }
    GenerateIntervalPageSpecialIcon(iNumber,ptVideoMem);

    while(1)
    {
        iRet = GetIntervalPageInput(&g_tIntervalPageLayout, &tInputEvent);
        if(tInputEvent.iPresure == 0)
        {
            if(isPress)
            {
                isPress = 0;
                ReleaseButtom(&ptPicPos[Index]);

                switch (Index)
                {
                case 0:
                    iNumber ++;
                    if (iNumber == 60)
                    {
                        iNumber = 0;
                    }
                   
                    GenerateIntervalPageSpecialIcon(iNumber,ptVideoMem);
                    break;
                case 1:
                    
                    break;
                case 2:
                    iNumber --;
                    if (iNumber == -1)
                    {
                        iNumber = 59;
                    }
                  
                    GenerateIntervalPageSpecialIcon(iNumber, ptVideoMem);
                    break;
                case 3:
                    g_iIntervalSecond = iNumber;
                    return 0;
                    break;
                case 4:
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
                    preTime = tInputEvent.tTime;
                    PushButtom(&ptPicPos[Index]);
                }
                else
                {
                    curTime = tInputEvent.tTime;
                    if (isOutOf500ms(&preTime, &curTime) )
                    {
                        preTime = curTime;
                        if(Index == 0)
                        {
                            iNumber += 5;
                            if (iNumber >= 60)
                            {
                                iNumber = 0;
                            }
                            GenerateIntervalPageSpecialIcon(iNumber,ptVideoMem);
                        }
                        else
                        {
                            iNumber -= 5;
                            if (iNumber <= -1)
                            {
                                iNumber = 59;
                            }
                  
                            GenerateIntervalPageSpecialIcon(iNumber, ptVideoMem);
                        }
                        
                    }

                }
                
            }

        }  
    }

    return 0;
}



static int GetIntervalPageInput(PT_PageLayout ptPageLayout, PT_EventOpr ptInputEvent )
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


int IntervalPageInit(void)
{
    return RegisterPageOpr(&g_tIntervalPageOpr);
}