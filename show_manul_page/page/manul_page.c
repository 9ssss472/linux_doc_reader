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

#define DIR_FILE_ICON_WIDTH    40
#define DIR_FILE_ICON_HEIGHT   DIR_FILE_ICON_WIDTH
#define DIR_FILE_NAME_HEIGHT   20
#define DIR_FILE_NAME_WIDTH   (DIR_FILE_ICON_HEIGHT + DIR_FILE_NAME_HEIGHT)
#define DIR_FILE_ALL_WIDTH    DIR_FILE_NAME_WIDTH
#define DIR_FILE_ALL_HEIGHT   DIR_FILE_ALL_WIDTH

static char g_strCurDir[256] = DEFAULT_DIR;
static char g_strSelectedDir[256] = DEFAULT_DIR;

static char *g_strDirClosedIconName  = "fold_closed.bmp";
static char *g_strDirOpenedIconName  = "fold_opened.bmp";
static char *g_strFileIconName = "file.bmp";

static T_PicPos g_atMenuIconsLayout[] = {
//	{0, 0, 0, 0, "return.bmp"},
	{0, 0, 0, 0, "up.bmp"},
	{0, 0, 0, 0, "select.bmp"},
	{0, 0, 0, 0, "pre_page.bmp"},
	{0, 0, 0, 0, "next_page.bmp"},
	{0, 0, 0, 0, NULL},
};


static T_PageLayout g_tBrowsePageMenuIconsLayout = {
	.iMaxTotalBytes = 0,
	.atLayout       = g_atMenuIconsLayout,
};

static T_PageLayout g_tBrowsePageDirAndFileLayout = {
	.iMaxTotalBytes = 0,
	//.atLayout       = g_atDirAndFileLayout,
};

static T_PicPos *g_atDirAndFileLayout;
static int g_iDirFileNumPerCol, g_iDirFileNumPerRow;

/* 用来描述某目录里的内容 */
static PT_DirContent *g_aptDirContents;  /* 数组:存有目录下"顶层子目录","文件"的名字 */
static int g_iDirContentsNumber;         /* g_aptDirContents数组有多少项 */
static int g_iStartIndex = 0;            /* 在屏幕上显示的第1个"目录和文件"是g_aptDirContents数组里的哪一项 */

static T_PixelDatas g_tDirClosedIconPixelDatas;
static T_PixelDatas g_tDirOpenedIconPixelDatas;
static T_PixelDatas g_tFileIconPixelDatas;



/* 计算菜单中各图标坐标值 */
static void CalcBrowsePageMenusLayout(PT_PageLayout ptPageLayout)
{
    int iXres, iYres, iBpp;
    int iWidth;
	int iHeight;
    int i;
    int iTmpTotalBytes;

    GetDispResolution(&iXres, &iYres, &iBpp);

    PT_PicPos atLayout;

    atLayout = ptPageLayout->atLayout;

    ptPageLayout ->iBpp = iBpp;

    if (iXres < iYres)
	{			 
		/*	 iXres/4
		 *	  ----------------------------------
		 *	   up	select	pre_page  next_page
		 *
		 *
		 *
		 *
		 *
		 *
		 *	  ----------------------------------
		 */
		 
		iWidth  = iXres / 4;
		iHeight = iWidth;
		
		/* return图标 */
		atLayout[0].iLeftTopY  = 0;
		atLayout[0].iRightBotY = atLayout[0].iLeftTopY + iHeight - 1;
		atLayout[0].iLeftTopX  = 0;
		atLayout[0].iRightBotX = atLayout[0].iLeftTopX + iWidth - 1;

		/* up图标 */
		atLayout[1].iLeftTopY  = 0;
		atLayout[1].iRightBotY = atLayout[1].iLeftTopY + iHeight - 1;
		atLayout[1].iLeftTopX  = atLayout[0].iRightBotX + 1;
		atLayout[1].iRightBotX = atLayout[1].iLeftTopX + iWidth - 1;

		/* select图标 */
		atLayout[2].iLeftTopY  = 0;
		atLayout[2].iRightBotY = atLayout[2].iLeftTopY + iHeight - 1;
		atLayout[2].iLeftTopX  = atLayout[1].iRightBotX + 1;
		atLayout[2].iRightBotX = atLayout[2].iLeftTopX + iWidth - 1;

		/* pre_page图标 */
		atLayout[3].iLeftTopY  = 0;
		atLayout[3].iRightBotY = atLayout[3].iLeftTopY + iHeight - 1;
		atLayout[3].iLeftTopX  = atLayout[2].iRightBotX + 1;
		atLayout[3].iRightBotX = atLayout[3].iLeftTopX + iWidth - 1;
#if 0
		/* next_page图标 */
		atLayout[4].iLeftTopY  = 0;
		atLayout[4].iRightBotY = atLayout[4].iLeftTopY + iHeight - 1;
		atLayout[4].iLeftTopX  = atLayout[3].iRightBotX + 1;
		atLayout[4].iRightBotX = atLayout[4].iLeftTopX + iWidth - 1;
#endif
	}
	else
	{
		/*	 iYres/4
		 *	  ----------------------------------
		 *	   up		  
		 *
		 *    select
		 *
		 *    pre_page
		 *  
		 *   next_page
		 *
		 *	  ----------------------------------
		 */
		 
		iHeight  = iYres / 4;
		iWidth = iHeight;

		/* return图标 */
		atLayout[0].iLeftTopY  = 0;
		atLayout[0].iRightBotY = atLayout[0].iLeftTopY + iHeight - 1;
		atLayout[0].iLeftTopX  = 0;
		atLayout[0].iRightBotX = atLayout[0].iLeftTopX + iWidth - 1;
		
		/* up图标 */
		atLayout[1].iLeftTopY  = atLayout[0].iRightBotY+ 1;
		atLayout[1].iRightBotY = atLayout[1].iLeftTopY + iHeight - 1;
		atLayout[1].iLeftTopX  = 0;
		atLayout[1].iRightBotX = atLayout[1].iLeftTopX + iWidth - 1;
		
		/* select图标 */
		atLayout[2].iLeftTopY  = atLayout[1].iRightBotY + 1;
		atLayout[2].iRightBotY = atLayout[2].iLeftTopY + iHeight - 1;
		atLayout[2].iLeftTopX  = 0;
		atLayout[2].iRightBotX = atLayout[2].iLeftTopX + iWidth - 1;
		
		/* pre_page图标 */
		atLayout[3].iLeftTopY  = atLayout[2].iRightBotY + 1;
		atLayout[3].iRightBotY = atLayout[3].iLeftTopY + iHeight - 1;
		atLayout[3].iLeftTopX  = 0;
		atLayout[3].iRightBotX = atLayout[3].iLeftTopX + iWidth - 1;
#if 0		
		/* next_page图标 */
		atLayout[4].iLeftTopY  = atLayout[3].iRightBotY + 1;
		atLayout[4].iRightBotY = atLayout[4].iLeftTopY + iHeight - 1;
		atLayout[4].iLeftTopX  = 0;
		atLayout[4].iRightBotX = atLayout[4].iLeftTopX + iWidth - 1;
#endif		
	}

    i = 0;
	while (atLayout[i].PicName)
	{
		iTmpTotalBytes = (atLayout[i].iRightBotX - atLayout[i].iLeftTopX + 1) * (atLayout[i].iRightBotY - atLayout[i].iLeftTopY + 1) * iBpp / 8;
		if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
		{
			ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
		}
		i++;
	}

}



/* 计算目录和文件的显示区域 */
static int CalcBrowsePageDirAndFilesLayout(void)
{
	int iXres, iYres, iBpp;
	int iLeftTopX, iLeftTopY;
	int iLeftTopXBak;
	int iRightBotX, iRightBotY;
    int iIconWidth, iIconHeight;
    int iNumPerCol, iNumPerRow;
    int iDeltaX, iDeltaY;
    int i, j, k = 0;
	
	GetDispResolution(&iXres, &iYres, &iBpp);

	if (iXres < iYres)
	{
		/* --------------------------------------
		 *    up select pre_page next_page 图标
		 * --------------------------------------
		 *
		 *           目录和文件
		 *
		 *
		 * --------------------------------------
		 */
		iLeftTopX  = 0;
		iRightBotX = iXres - 1;
		iLeftTopY  = g_atMenuIconsLayout[0].iRightBotY + 1;
		iRightBotY = iYres - 1;
	}
	else
	{
		/*	 iYres/4
		 *	  ----------------------------------
		 *	   up      |
		 *             |
		 *    select   |
		 *             |     目录和文件
		 *    pre_page |
		 *             |
		 *   next_page |
		 *             |
		 *	  ----------------------------------
		 */
		iLeftTopX  = g_atMenuIconsLayout[0].iRightBotX + 1;
		iRightBotX = iXres - 1;
		iLeftTopY  = 0;
		iRightBotY = iYres - 1;
	}

    /* 确定一行显示多少个"目录或文件", 显示多少行 */
    iIconWidth  = DIR_FILE_NAME_WIDTH;
    iIconHeight = iIconWidth;

    /* 图标之间的间隔要大于10个象素 */
    iNumPerRow = (iRightBotX - iLeftTopX + 1) / iIconWidth;
    while (1)
    {
        iDeltaX  = (iRightBotX - iLeftTopX + 1) - iIconWidth * iNumPerRow;
        if ((iDeltaX / (iNumPerRow + 1)) < 10)
            iNumPerRow--;
        else
            break;
    }

    iNumPerCol = (iRightBotY - iLeftTopY + 1) / iIconHeight;
    while (1)
    {
        iDeltaY  = (iRightBotY - iLeftTopY + 1) - iIconHeight * iNumPerCol;
        if ((iDeltaY / (iNumPerCol + 1)) < 10)
            iNumPerCol--;
        else
            break;
    }

    /* 每个图标之间的间隔 */
    iDeltaX = iDeltaX / (iNumPerRow + 1);
    iDeltaY = iDeltaY / (iNumPerCol + 1);

    g_iDirFileNumPerRow = iNumPerRow;
    g_iDirFileNumPerCol = iNumPerCol;


    /* 可以显示 iNumPerRow * iNumPerCol个"目录或文件"
     * 分配"两倍+1"的T_PicPos结构体: 一个用来表示图标,另一个用来表示名字
     * 最后一个用来存NULL,借以判断结构体数组的末尾
     */
	g_atDirAndFileLayout = malloc(sizeof(T_PicPos) * (2 * iNumPerRow * iNumPerCol + 1));
    if (NULL == g_atDirAndFileLayout)
    {
        DBG_PRINTF("malloc error!\n");
        return -1;
    }

    /* "目录和文件"整体区域的左上角、右下角坐标 */
    g_tBrowsePageDirAndFileLayout.iLeftTopX      = iLeftTopX;
    g_tBrowsePageDirAndFileLayout.iRightBotX     = iRightBotX;
    g_tBrowsePageDirAndFileLayout.iLeftTopY      = iLeftTopY;
    g_tBrowsePageDirAndFileLayout.iRightBotY     = iRightBotY;
    g_tBrowsePageDirAndFileLayout.iBpp           = iBpp;
    g_tBrowsePageDirAndFileLayout.atLayout       = g_atDirAndFileLayout;
    g_tBrowsePageDirAndFileLayout.iMaxTotalBytes = DIR_FILE_ALL_WIDTH * DIR_FILE_ALL_HEIGHT * iBpp / 8;

    
    /* 确定图标和名字的位置 
     *
     * 图标是一个正方体, "图标+名字"也是一个正方体
     *   --------
     *   |  图  |
     *   |  标  |
     * ------------
     * |   名字   |
     * ------------
     */
    iLeftTopX += iDeltaX;
    iLeftTopY += iDeltaY;
    iLeftTopXBak = iLeftTopX;
    for (i = 0; i < iNumPerCol; i++)
    {        
        for (j = 0; j < iNumPerRow; j++)
        {
            /* 图标 */
            g_atDirAndFileLayout[k].iLeftTopX  = iLeftTopX + (DIR_FILE_NAME_WIDTH - DIR_FILE_ICON_WIDTH) / 2;
            g_atDirAndFileLayout[k].iRightBotX = g_atDirAndFileLayout[k].iLeftTopX + DIR_FILE_ICON_WIDTH - 1;
            g_atDirAndFileLayout[k].iLeftTopY  = iLeftTopY;
            g_atDirAndFileLayout[k].iRightBotY = iLeftTopY + DIR_FILE_ICON_HEIGHT - 1;

            /* 名字 */
            g_atDirAndFileLayout[k+1].iLeftTopX  = iLeftTopX;
            g_atDirAndFileLayout[k+1].iRightBotX = iLeftTopX + DIR_FILE_NAME_WIDTH - 1;
            g_atDirAndFileLayout[k+1].iLeftTopY  = g_atDirAndFileLayout[k].iRightBotY + 1;
            g_atDirAndFileLayout[k+1].iRightBotY = g_atDirAndFileLayout[k+1].iLeftTopY + DIR_FILE_NAME_HEIGHT - 1;

            iLeftTopX += DIR_FILE_ALL_WIDTH + iDeltaX;
            k += 2;
        }
        iLeftTopX = iLeftTopXBak;
        iLeftTopY += DIR_FILE_ALL_HEIGHT + iDeltaY;
    }

    /* 结尾 */
    g_atDirAndFileLayout[k].iLeftTopX   = 0;
    g_atDirAndFileLayout[k].iRightBotX  = 0;
    g_atDirAndFileLayout[k].iLeftTopY   = 0;
    g_atDirAndFileLayout[k].iRightBotY  = 0;
    g_atDirAndFileLayout[k].PicName = NULL;

    return 0;
}

int ManulPageGetInputEvent(PT_PageLayout ptPageLayout, PT_EventOpr ptInputEvent)
{
	return GenericGetInputEvent(ptPageLayout, ptInputEvent);
}

static int GetInputPositionInPageLayout(PT_PageLayout ptPageLayout, PT_EventOpr ptInputEvent)
{
    int i = 0;
    PT_PicPos atLayout = ptPageLayout->atLayout;
        
    /* 处理数据 */
    /* 确定触点位于哪一个按钮上 */
    while (atLayout[i].iRightBotY)
    {
        if ((ptInputEvent->iX >= atLayout[i].iLeftTopX) && (ptInputEvent->iX <= atLayout[i].iRightBotX) && \
             (ptInputEvent->iY >= atLayout[i].iLeftTopY) && (ptInputEvent->iY <= atLayout[i].iRightBotY))
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


/* aptDirContents数组中有iDirContentNumber项
 * 从第iStartIndex项开始显示, 显示满一页
 */
static int GenerateBrowsePageDirAndFile(int iStartIndex, int iDirContentsNumber, PT_DirContent *aptDirContents, PT_VideoMem ptVideoMem)
{
    int iError = 0;
    int i, j, k = 0;
    int iDirContentIndex = iStartIndex;
    PT_PageLayout ptPageLayout = &g_tBrowsePageDirAndFileLayout;
	PT_PicPos atLayout = ptPageLayout->atLayout;

    ClearRectangleInVideoMem(ptPageLayout->iLeftTopX, ptPageLayout->iLeftTopY, ptPageLayout->iRightBotX, ptPageLayout->iRightBotY, ptVideoMem, COLOR_BACKGROUND);

    SetFontSize(atLayout[1].iRightBotY - atLayout[1].iLeftTopY - 5);
    
    for (i = 0; i < g_iDirFileNumPerCol; i++)
    {
        for (j = 0; j < g_iDirFileNumPerRow; j++)
        {
            if (iDirContentIndex < iDirContentsNumber)
            {
                /* 显示目录或文件的图标 */
                if (aptDirContents[iDirContentIndex]->eFileType == FILETYPE_DIR)
                {
                    PicMerge(atLayout[k].iLeftTopX, atLayout[k].iLeftTopY, &g_tDirClosedIconPixelDatas, &ptVideoMem->tPixelDatas);
                }
                else
                {
                    PicMerge(atLayout[k].iLeftTopX, atLayout[k].iLeftTopY, &g_tFileIconPixelDatas, &ptVideoMem->tPixelDatas);
                }

                k++;
                /* 显示目录或文件的名字 */
                //DBG_PRINTF("MergerStringToCenterOfRectangleInVideoMem: %s\n", aptDirContents[iDirContentIndex]->strName);
                iError = MergerStringToCenterOfRectangleInVideoMem(atLayout[k].iLeftTopX, atLayout[k].iLeftTopY, atLayout[k].iRightBotX, atLayout[k].iRightBotY, (unsigned char *)aptDirContents[iDirContentIndex]->strName, ptVideoMem);
                //ClearRectangleInVideoMem(atLayout[k].iLeftTopX, atLayout[k].iLeftTopY, atLayout[k].iRightBotX, atLayout[k].iRightBotY, ptVideoMem, 0xff0000);
                k++;

                iDirContentIndex++;
            }
            else
            {
                break;
            }
        }
        if (iDirContentIndex >= iDirContentsNumber)
        {
            break;
        }
    }
	return iError;
}



static int GenerateDirAndFileIcons(PT_PageLayout ptPageLayout)
{
	T_PixelDatas tOriginIconPixelDatas;
    int iError;
	int iXres, iYres, iBpp;
    PT_PicPos atLayout = ptPageLayout->atLayout;

	GetDispResolution(&iXres, &iYres, &iBpp);

    /* 给目录图标、文件图标分配内存 */
    g_tDirClosedIconPixelDatas.iBpp          = iBpp;
    g_tDirClosedIconPixelDatas.aucPixelDatas = malloc(ptPageLayout->iMaxTotalBytes);
    if (g_tDirClosedIconPixelDatas.aucPixelDatas == NULL)
    {
        return -1;
    }

    g_tDirOpenedIconPixelDatas.iBpp          = iBpp;
    g_tDirOpenedIconPixelDatas.aucPixelDatas = malloc(ptPageLayout->iMaxTotalBytes);
    if (g_tDirOpenedIconPixelDatas.aucPixelDatas == NULL)
    {
        return -1;
    }

    g_tFileIconPixelDatas.iBpp          = iBpp;
    g_tFileIconPixelDatas.aucPixelDatas = malloc(ptPageLayout->iMaxTotalBytes);
    if (g_tFileIconPixelDatas.aucPixelDatas == NULL)
    {
        return -1;
    }

    /* 从BMP文件里提取图像数据 */
    /* 1. 提取"fold_closed图标" */
    iError = GetFilePixel(g_strDirClosedIconName, &tOriginIconPixelDatas);
    if (iError)
    {
        DBG_PRINTF("GetFilePixel %s error!\n", g_strDirClosedIconName);
        return -1;
    }
    g_tDirClosedIconPixelDatas.iHeight = atLayout[0].iRightBotY - atLayout[0].iLeftTopY + 1;
    g_tDirClosedIconPixelDatas.iWidth  = atLayout[0].iRightBotX - atLayout[0].iLeftTopX + 1;
    g_tDirClosedIconPixelDatas.iLineBytes  = g_tDirClosedIconPixelDatas.iWidth * g_tDirClosedIconPixelDatas.iBpp / 8;
    g_tDirClosedIconPixelDatas.iTotalBytes = g_tDirClosedIconPixelDatas.iLineBytes * g_tDirClosedIconPixelDatas.iHeight;
    PicZoom(&tOriginIconPixelDatas, &g_tDirClosedIconPixelDatas);
    FreePixelDatasForIcon(&tOriginIconPixelDatas);

    /* 2. 提取"fold_opened图标" */
    iError = GetFilePixel(g_strDirOpenedIconName, &tOriginIconPixelDatas);
    if (iError)
    {
        DBG_PRINTF("GetFilePixel %s error!\n", g_strDirOpenedIconName);
        return -1;
    }
    g_tDirOpenedIconPixelDatas.iHeight = atLayout[0].iRightBotY - atLayout[0].iLeftTopY + 1;
    g_tDirOpenedIconPixelDatas.iWidth  = atLayout[0].iRightBotX - atLayout[0].iLeftTopX + 1;
    g_tDirOpenedIconPixelDatas.iLineBytes  = g_tDirOpenedIconPixelDatas.iWidth * g_tDirOpenedIconPixelDatas.iBpp / 8;
    g_tDirOpenedIconPixelDatas.iTotalBytes = g_tDirOpenedIconPixelDatas.iLineBytes * g_tDirOpenedIconPixelDatas.iHeight;
    PicZoom(&tOriginIconPixelDatas, &g_tDirOpenedIconPixelDatas);
    FreePixelDatasForIcon(&tOriginIconPixelDatas);

    /* 3. 提取"file图标" */
    iError = GetFilePixel(g_strFileIconName, &tOriginIconPixelDatas);
    if (iError)
    {
        DBG_PRINTF("GetFilePixel %s error!\n", g_strFileIconName);
        return -1;
    }
    g_tFileIconPixelDatas.iHeight = atLayout[0].iRightBotY - atLayout[0].iLeftTopY + 1;
    g_tFileIconPixelDatas.iWidth  = atLayout[0].iRightBotX - atLayout[0].iLeftTopX+ 1;
    g_tFileIconPixelDatas.iLineBytes  = g_tDirClosedIconPixelDatas.iWidth * g_tDirClosedIconPixelDatas.iBpp / 8;
    g_tFileIconPixelDatas.iTotalBytes = g_tFileIconPixelDatas.iLineBytes * g_tFileIconPixelDatas.iHeight;
    PicZoom(&tOriginIconPixelDatas, &g_tFileIconPixelDatas);
    FreePixelDatasForIcon(&tOriginIconPixelDatas);

    return 0;
}



static void ShowBrowsePage(PT_PageLayout ptPageLayout)
{
	PT_VideoMem ptVideoMem;
	int iError = 0;

	PT_PicPos atLayout = ptPageLayout->atLayout;
		
	/* 1. 获得显存 */
	ptVideoMem = GetFreeVideoMem(ID("manul"), 1);
	if (ptVideoMem == NULL)
	{
		DBG_PRINTF("can't get video mem for browse page!\n");
		return;
	}

	/* 2. 描画数据 */

	/* 如果还没有计算过各图标的坐标 */
	if (atLayout[0].iLeftTopX == 0)
	{
		CalcBrowsePageMenusLayout(ptPageLayout);
        CalcBrowsePageDirAndFilesLayout();
	}

    /* 如果还没有生成"目录和文件"的图标 */
    if (!g_tDirClosedIconPixelDatas.aucPixelDatas)
    {
        GenerateDirAndFileIcons(&g_tBrowsePageDirAndFileLayout);
    }

	iError = GeneratePage(ptPageLayout, ptVideoMem);
    if(iError)
    {
        DBG_PRINTF("GeneratePage failed\r\n");
    }

    iError = GenerateBrowsePageDirAndFile(g_iStartIndex, g_iDirContentsNumber, g_aptDirContents, ptVideoMem);

	/* 3. 刷到设备上去 */
	FlushPageMem2FB(ptVideoMem);

	/* 4. 解放显存 */
	FreePageMem(ptVideoMem);
}

/* 对于目录图标, 把它改为"file_opened图标"
 * 对于文件图标, 只是反选
 */
static void SelectDirFileIcon(int iDirFileIndex)
{
    int iDirFileContentIndex;
    PT_VideoMem ptDevVideoMem;

    ptDevVideoMem = GetDevVideoMem();

    iDirFileIndex = iDirFileIndex & ~1;    
    iDirFileContentIndex = g_iStartIndex + iDirFileIndex/2;
    
    if (g_aptDirContents[iDirFileContentIndex]->eFileType == FILETYPE_DIR)
    {
        PicMerge(g_atDirAndFileLayout[iDirFileIndex].iLeftTopX, g_atDirAndFileLayout[iDirFileIndex].iLeftTopY, &g_tDirOpenedIconPixelDatas, &ptDevVideoMem->tPixelDatas);
    }
    else
    {
        PushButtom(&g_atDirAndFileLayout[iDirFileIndex]);
        PushButtom(&g_atDirAndFileLayout[iDirFileIndex + 1]);
    }
}

/* 对于目录图标, 把它改为"file_closeed图标"
 * 对于文件图标, 只是反选
 */
static void DeSelectDirFileIcon(int iDirFileIndex)
{
    int iDirFileContentIndex;
    PT_VideoMem ptDevVideoMem;
    
    ptDevVideoMem = GetDevVideoMem();

    iDirFileIndex = iDirFileIndex & ~1;    
    iDirFileContentIndex = g_iStartIndex + iDirFileIndex/2;
    
    if (g_aptDirContents[iDirFileContentIndex]->eFileType == FILETYPE_DIR)
    {
        PicMerge(g_atDirAndFileLayout[iDirFileIndex].iLeftTopX, g_atDirAndFileLayout[iDirFileIndex].iLeftTopY, &g_tDirClosedIconPixelDatas, &ptDevVideoMem->tPixelDatas);
    }
    else
    {
        ReleaseButtom(&g_atDirAndFileLayout[iDirFileIndex]);
        ReleaseButtom(&g_atDirAndFileLayout[iDirFileIndex + 1]);
    }
}




/* bCur = 1 表示必须获得videomem, 因为这是马上就要在LCD上显示出来的
 * bCur = 0 表示这是做准备用的, 有可能无法获得videomem
 */
static PT_VideoMem PrepareNextPicture(int bCur, char * strFileName)
{
	T_PixelDatas tOriginIconPixelDatas;
	T_PixelDatas tPicPixelDatas;
    PT_VideoMem ptVideoMem;
	int iError;
	int iXres, iYres, iBpp;
    int iTopLeftX, iTopLeftY;
    float k;
    
    
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

/*
*返回值1表示松开时X坐标与按下时X坐标超过正100个像素
*返回值-1表示松开时X坐标与按下时X坐标超过负100个像素
*返回值0表示松开时X坐标与按下时X坐标不超过100个像素
*/
static int Beyond100Pixels(PT_EventOpr ptCurEvent, PT_EventOpr ptPreEvent)
{
    int iSub;
    iSub = ptCurEvent ->iX - ptPreEvent->iX;

    /*显示下一副图片*/
    if(iSub > 100)
    {
        return 1;
    }
    else if(iSub < -100)/*显示上一副图片*/
    {
        return -1;
    }
    else    /*退出显示*/
    {
        return 0;
    }
     
}

/*
*Index:g_aptDirContents数组的第几个成员
*TotalNumbers：g_aptDirContents数组共有几个成员
*Diretion:1显示上一张图片，-1显示下一张图片
*/
static void ShowOnePicture(int *Index, int TotalNumbers, int Diretion )
{
    int * ptiIndex = Index;
    char strTmp[256];
    PT_VideoMem ptVideoMem;

    do
    {
        if(Diretion == 1)
            (*ptiIndex) --;
        else if(Diretion == -1)
            (*ptiIndex) ++;
        
        if( *ptiIndex > TotalNumbers -1)
        {
            *ptiIndex = 0;
        }
        else if(*ptiIndex < 0)
        {
            *ptiIndex = TotalNumbers -1;
        }

    }while(g_aptDirContents[*ptiIndex] ->isPicture == 0);

    snprintf(strTmp, 256, "%s/%s", g_strCurDir, g_aptDirContents[*ptiIndex]->strName);
    DBG_PRINTF("iDirFileContentIndex = %d\r\n",*ptiIndex);
    ptVideoMem = PrepareNextPicture(1, strTmp);
    /* 刷到设备上去 */
    FlushPageMem2FB(ptVideoMem);

    /* 解放显存 */
    FreePageMem(ptVideoMem); 
}
/*
 * browse页面有两个区域: 菜单图标, 目录和文件图标
 * 为统一处理, "菜单图标"的序号为0,1,2,3,..., "目录和文件图标"的序号为1000,1001,1002,....
 *
 */
#define DIRFILE_ICON_INDEX_BASE 1000
int ManulPageRun(void)
{
	int iIndex;
	T_EventOpr tInputEvent;
	T_EventOpr tInputEventPrePress;
    PT_VideoMem ptVideoMem;

    int iRet;
    int bDisplayPressed = 0;        /*屏幕是否被按下*/
	int bIconPressed = 0;          /* 界面上是否有图标显示为"被按下" */

    /* "连续播放图片"时, 是从哪一个目录读取文件呢? 这是由"选择"按钮来选择该目录的
     * 点击目录图标将进入下一级目录, 哪怎样选择目录呢?
     * 1. 先点击"选择"按钮
     * 2. 再点击某个目录图标
     */
    int bHaveClickSelectIcon = 0;
    
	int iIndexPressed = -1;    /* 被按下的图标 */
    int iDirFileContentIndex;
    
    int iError;
    PT_VideoMem ptDevVideoMem;

    char strTmp[256];
    char *pcTmp;

	/* 这两句只是为了避免编译警告 */
	tInputEventPrePress.tTime.tv_sec = 0;
	tInputEventPrePress.tTime.tv_usec = 0;

    DBG_PRINTF("%s %s %d\r\n", __FILE__, __FUNCTION__, __LINE__);
    ptDevVideoMem = GetDevVideoMem();

    /* 0. 获得要显示的目录的内容 */
    iError = GetDirContents(g_strCurDir, &g_aptDirContents, &g_iDirContentsNumber);
    if (iError)
    {
        DBG_PRINTF("GetDirContents error!\n");
        return 0;
    }
    DBG_PRINTF("%s %s %d\r\n", __FILE__, __FUNCTION__, __LINE__);
    	
	/* 1. 显示页面 */
	ShowBrowsePage(&g_tBrowsePageMenuIconsLayout);

	/* 2. 创建Prepare线程 */

	/* 3. 调用GetInputEvent获得输入事件，进而处理 */
	while (1)
	{
        /* 先确定是否触摸了菜单图标 */
		iIndex = ManulPageGetInputEvent(&g_tBrowsePageMenuIconsLayout, &tInputEvent);

        /* 如果触点不在菜单图标上, 则判断是在哪一个"目录和文件"上 */
        if (iIndex == -1)
        {
            iIndex = GetInputPositionInPageLayout(&g_tBrowsePageDirAndFileLayout, &tInputEvent);
            if (iIndex != -1)
            {                
                if (g_iStartIndex + iIndex / 2 < g_iDirContentsNumber)  /* 判断这个触点上是否有图标 */
                    iIndex += DIRFILE_ICON_INDEX_BASE; /* 这是"目录和文件图标" */
                else
                    iIndex = -1;
            }
        }
        
        /* 如果是松开 */
		if (tInputEvent.iPresure == 0)
		{
            /* 如果当前有图标被按下 */
			if (bIconPressed)
			{
                 if (iIndexPressed < DIRFILE_ICON_INDEX_BASE)  /* 菜单图标 */
                 {
                    /* 释放图标 */
                    if (iIndexPressed != 1) /* "选择"图标单独处理 */
                    {
        				ReleaseButtom(&g_atMenuIconsLayout[iIndexPressed]);
                    }
                    bIconPressed    = 0;

				    if (iIndexPressed == iIndex) /* 按下和松开都是同一个按钮 */
    				{
    					switch (iIndex)
    					{
    						case 0: /* "向上"按钮 */
    						{
                                if (0 == strcmp(g_strCurDir, "/"))  /* 已经是顶层目录 */
                                {
                                    return 0;
                                }

                                pcTmp = strrchr(g_strCurDir, '/'); /* 找到最后一个'/', 把它去掉 */
                                *pcTmp = '\0';
                                
                                iError = GetDirContents(g_strCurDir, &g_aptDirContents, &g_iDirContentsNumber);
                                if (iError)
                                {
                                    DBG_PRINTF("GetDirContents error!\n");
                                    return 0;
                                }
                                g_iStartIndex = 0;
                                iError = GenerateBrowsePageDirAndFile(g_iStartIndex, g_iDirContentsNumber, g_aptDirContents, ptDevVideoMem);
                                
    							break;
    						}
                            case 1: /* "选择" */
                            {
                                if (!bHaveClickSelectIcon)  /* 第1次点击"选择"按钮 */
                                {
                                    bHaveClickSelectIcon = 1;
                                }
                                else
                                {
                                    ReleaseButtom(&g_atMenuIconsLayout[iIndexPressed]);
                                    bIconPressed    = 0;
                                    bHaveClickSelectIcon = 0;
                                }
                                break;
                            }
                            case 2: /* "上一页" */
                            {
                                g_iStartIndex -= g_iDirFileNumPerCol * g_iDirFileNumPerRow;
                                if (g_iStartIndex >= 0)
                                {
                                    iError = GenerateBrowsePageDirAndFile(g_iStartIndex, g_iDirContentsNumber, g_aptDirContents, ptDevVideoMem);
                                }
                                else
                                {
                                    g_iStartIndex += g_iDirFileNumPerCol * g_iDirFileNumPerRow;
                                }
                                break;
                            }
                            case 3: /* "下一页" */
                            {
                                g_iStartIndex += g_iDirFileNumPerCol * g_iDirFileNumPerRow;
                                if (g_iStartIndex < g_iDirContentsNumber)
                                {
                                    iError = GenerateBrowsePageDirAndFile(g_iStartIndex, g_iDirContentsNumber, g_aptDirContents, ptDevVideoMem);
                                }
                                else
                                {
                                    g_iStartIndex -= g_iDirFileNumPerCol * g_iDirFileNumPerRow;
                                }
                                break;
                            }
    						default:
    						{
    							break;
    						}
    					}
    				}
                }                
                else /* "目录和文件图标" */
                {

                    /*
                     * 如果按下和松开时, 触点不处于同一个图标上, 则释放图标
                     */
                    if (iIndexPressed != iIndex)
                    {
                        DeSelectDirFileIcon(iIndexPressed - DIRFILE_ICON_INDEX_BASE);
                        bIconPressed      = 0;
                    }
                    else if (bHaveClickSelectIcon) /* 按下和松开都是同一个按钮, 并且"选择"按钮是按下状态 */
                    {
                        DeSelectDirFileIcon(iIndexPressed - DIRFILE_ICON_INDEX_BASE);
                        bIconPressed    = 0;
                        
                        /* 如果是目录, 记录这个目录 */
                        iDirFileContentIndex = g_iStartIndex + (iIndexPressed - DIRFILE_ICON_INDEX_BASE)/2;
                        if (g_aptDirContents[iDirFileContentIndex]->eFileType == FILETYPE_DIR)
                        {
                            ReleaseButtom(&g_atMenuIconsLayout[1]);  /* 同时松开"选择按钮" */
                            bHaveClickSelectIcon = 0;

                            /* 记录目录名 */
                            snprintf(strTmp, 256, "%s/%s", g_strCurDir, g_aptDirContents[iDirFileContentIndex]->strName);
                            strTmp[255] = '\0';
                            strcpy(g_strSelectedDir, strTmp);
                            printf("strTmp = %s\r\n",strTmp);
                        }
                    }
                    else  /* "选择"按钮不被按下时, 单击目录则进入 */
                    {
                        DeSelectDirFileIcon(iIndexPressed - DIRFILE_ICON_INDEX_BASE);
                        bIconPressed    = 0;
                        
                        /* 如果是目录, 进入这个目录 */
                        iDirFileContentIndex = g_iStartIndex + (iIndexPressed - DIRFILE_ICON_INDEX_BASE)/2;
                        if (g_aptDirContents[iDirFileContentIndex]->eFileType == FILETYPE_DIR)
                        {
                            snprintf(strTmp, 256, "%s/%s", g_strCurDir, g_aptDirContents[iDirFileContentIndex]->strName);
                            strTmp[255] = '\0';
                            strcpy(g_strCurDir, strTmp);
                            iError = GetDirContents(g_strCurDir, &g_aptDirContents, &g_iDirContentsNumber);
                            if (iError)
                            {
                                DBG_PRINTF("GetDirContents error!\n");
                                return -1;
                            }
                            g_iStartIndex = 0;
                            iError = GenerateBrowsePageDirAndFile(g_iStartIndex, g_iDirContentsNumber, g_aptDirContents, ptDevVideoMem);
                        }
                        else 
                        {
                            if(g_aptDirContents[iDirFileContentIndex] ->isPicture == 1)
                            {
                                /* 如果是图片则显示它*/
                                snprintf(strTmp, 256, "%s/%s", g_strCurDir, g_aptDirContents[iDirFileContentIndex]->strName);
                                DBG_PRINTF("iDirFileContentIndex = %d\r\n",iDirFileContentIndex);
                                ptVideoMem = PrepareNextPicture(1, strTmp);
                                    /* 刷到设备上去 */
                                FlushPageMem2FB(ptVideoMem);

                                /* 解放显存 */
                                FreePageMem(ptVideoMem); 

                                while (1)
                                {
                                    GetInputEvent(&tInputEvent);
                                    
                                    if(tInputEvent.iPresure == 0)/*如果是松开屏幕*/
                                    {
                                        bDisplayPressed = 0;
                                        iRet = Beyond100Pixels(&tInputEvent, &tInputEventPrePress);

                                        if(iRet == 0)/*退出显示*/
                                        {
                                            break;
                                        }
                                        else if(iRet == 1)/*显示上一张图片*/
                                        {
                                            ShowOnePicture(&iDirFileContentIndex, g_iDirContentsNumber, iRet);                                          
                                        }
                                        else/*显示下一张图片*/
                                        {
                                            ShowOnePicture(&iDirFileContentIndex, g_iDirContentsNumber, iRet);
                                            
                                        }       
                                    }
                                    else if(bDisplayPressed == 1)/*如果屏幕已经被按下*/
                                    {
                                        continue;
                                    }
                                    else/*第一次按下屏幕*/
                                    {
                                        bDisplayPressed = 1;
                                        tInputEventPrePress = tInputEvent;
                                    }
                                }
                                iError = GeneratePage(&g_tBrowsePageMenuIconsLayout, ptDevVideoMem);
                                iError = GenerateBrowsePageDirAndFile(g_iStartIndex, g_iDirContentsNumber, g_aptDirContents, ptDevVideoMem);

                            }
                        }

                    }                    
                }
            }                                
		}        
		else /* 按下状态 */
		{			
			if (iIndex != -1)
			{
                if (!bIconPressed)  /* 之前未曾有按钮被按下 */
                {
    				bIconPressed = 1;
    				iIndexPressed = iIndex;                    
					tInputEventPrePress = tInputEvent;  /* 记录下来 */
                    if (iIndex < DIRFILE_ICON_INDEX_BASE)  /* 菜单图标 */
                    {
                        if (!(bHaveClickSelectIcon && (iIndexPressed == 1)))  /* 如果已经按下"选择"按钮, 自然不用再次反转该图标 */
        					PushButtom(&g_atMenuIconsLayout[iIndex]);
                    }
                    else   /* 目录和文件图标 */
                    {
                        SelectDirFileIcon(iIndex - DIRFILE_ICON_INDEX_BASE);
                    }
                }

                /* 长按"向上"按钮, 返回 */
				if (iIndexPressed == 0)
				{
					if (TimeMSBetween(tInputEventPrePress.tTime, tInputEvent.tTime) > 2000)
					{
                        return 0;
					}
				}

			}
		}
	}
    return 0;
}


static T_PageOpr g_tManulPageAction = {
	.name          = "manul",
	.Run           = ManulPageRun,
	.GetInputEvent = ManulPageGetInputEvent,
	//.Prepare       = BrowsePagePrepare;
};

int ManulPageInit(void)
{
	return RegisterPageOpr(&g_tManulPageAction);
}
