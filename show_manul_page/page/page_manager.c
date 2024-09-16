#include "page_manager.h"
#include <string.h>
#include "config.h"
#include "display.h"
#include <stdlib.h>

PT_PageOpr ptPageOprHead = NULL;

int RegisterPageOpr(PT_PageOpr ptPageOpr)
{
    PT_PageOpr ptPageTmp = ptPageOprHead;

    if(ptPageOprHead == NULL)
    {
        ptPageOprHead = ptPageOpr;
    }
    else
    {
        while(ptPageTmp -> pNext)
        {
            ptPageTmp = ptPageTmp ->pNext;
        }

        ptPageTmp ->pNext = ptPageOpr;
    }
    
    return 0;
}

int ID(char * name)
{
    return ((int)name[0] + (int)name[1] + (int)name[2] + (int)name[3]);
}

int GeneratePage(PT_PageLayout ptPageLayout, PT_VideoMem ptVideoMem)
{
	T_PixelDatas tOriginIconPixelDatas;
	T_PixelDatas tIconPixelDatas;
	int iError;
	PT_PicPos atLayout = ptPageLayout->atLayout;
		
	/* 描画数据 */
	if (ptVideoMem->ePicState != PS_GENERATED)
	{
		ClearVideoMem(ptVideoMem, COLOR_BACKGROUND);

		tIconPixelDatas.iBpp          = ptPageLayout->iBpp;
		tIconPixelDatas.aucPixelDatas = malloc(ptPageLayout->iMaxTotalBytes);
		if (tIconPixelDatas.aucPixelDatas == NULL)
		{
			return -1;
		}

		while (atLayout->PicName)
		{
			iError = GetFilePixel(atLayout->PicName, &tOriginIconPixelDatas);
			if (iError)
			{
				DBG_PRINTF("GetFilePixel %s error!\n", atLayout->PicName);
				return -1;
			}
 			tIconPixelDatas.iHeight = atLayout->iRightBotY - atLayout->iLeftTopY + 1;
			tIconPixelDatas.iWidth  = atLayout->iRightBotX - atLayout->iLeftTopX+ 1;
			tIconPixelDatas.iLineBytes  = tIconPixelDatas.iWidth * tIconPixelDatas.iBpp / 8;
			tIconPixelDatas.iTotalBytes = tIconPixelDatas.iLineBytes * tIconPixelDatas.iHeight;
 			PicZoom(&tOriginIconPixelDatas, &tIconPixelDatas);
 			PicMerge(atLayout->iLeftTopX, atLayout->iLeftTopY, &tIconPixelDatas, &ptVideoMem->tPixelDatas);
 			FreePixelDatasForIcon(&tOriginIconPixelDatas);
 			atLayout++;
		}
		free(tIconPixelDatas.aucPixelDatas);
		ptVideoMem->ePicState = PS_GENERATED;
	}

	return 0;
}



PT_PageOpr Page(char *PageName)
{
    PT_PageOpr ptPageOpr;
    ptPageOpr = ptPageOprHead;
    while(ptPageOpr)
    {
        if(strcmp(PageName, ptPageOpr ->name) == 0)
        {
            return ptPageOpr;
        }

        ptPageOpr = ptPageOpr ->pNext;
    }

    return NULL;
}


int GenericGetInputEvent(PT_PageLayout ptPageLayout, PT_EventOpr ptInputEvent)
{
	T_EventOpr tInputEvent;
	int iRet;
	int i = 0;
	PT_PicPos atLayout = ptPageLayout->atLayout;
	
	/* 获得原始的触摸屏数据 
	 * 它是调用input_manager.c的函数，此函数会让当前线否休眠
	 * 当触摸屏线程获得数据后，会把它唤醒
	 */
	iRet = GetInputEvent(&tInputEvent);
	if (iRet)
	{
		return -1;
	}

	if (tInputEvent.iType != INPUT_TYPE_TOUCHSCREEN)
	{
		return -1;
	}

	*ptInputEvent = tInputEvent;
	
	/* 处理数据 */
	/* 确定触点位于哪一个按钮上 */
	while (atLayout[i].PicName)
	{
		if ((tInputEvent.iX >= atLayout[i].iLeftTopX) && (tInputEvent.iX <= atLayout[i].iRightBotX) && \
			 (tInputEvent.iY >= atLayout[i].iLeftTopY) && (tInputEvent.iY <= atLayout[i].iRightBotY))
		{
			/* 找到了被点中的按钮 */
			return i;
		}
		else
		{
			i++;
		}			
	}

	/* 触点没有落在按钮上 */
	return -1;
}

void GetPageCfg(PT_PageCfg ptPageCfg)
{
    GetSelectedDir(ptPageCfg->strSeletedDir);
    GetIntervalTime(&ptPageCfg->iIntervalSecond);
}


int PageInit(void)
{
    int iError;
    iError = MainPageInit();
    iError |= SettingPageInit();
    iError |=IntervalPageInit();
    iError |=BrowsePageInit();
    iError |= AutoPageInit();
	iError |= ManulPageInit();
    return iError;
}