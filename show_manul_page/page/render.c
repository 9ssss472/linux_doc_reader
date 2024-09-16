#include "pic_operation.h"
#include "file.h"
#include <stdio.h>
#include "config.h"
#include "display.h"
#include <stdlib.h>
#include "page_manager.h"
#include "fonts_manager.h"
#include "string.h"
#include "encoding_manager.h"
#include "picfmt_manager.h"


extern T_PicFileParser T_BMPPicFileParser;

void FreePixelDatasForIcon(PT_PixelDatas ptPixelDatas)
{
	T_BMPPicFileParser.FreePixelDatas(ptPixelDatas);
}


int GetFilePixel(char* pcFileName, PT_PixelDatas ptPixelDatas)
{
    PT_FileInfo ptFileInfo;
    extern T_PicFileParser T_BMPPicFileParser;
    int iPrintNum;
    int iError;
    int iXres, iYres, iBpp;

    ptFileInfo = malloc(sizeof(T_FileInfo));
    if(ptFileInfo == NULL)
    {
        DBG_PRINTF("malloc PT_FileInfo failed\r\n");
        return -1;
    }

    iPrintNum = snprintf(ptFileInfo ->strFileName, 128, "%s%s",ICON_PATH,pcFileName);

    ptFileInfo ->strFileName[127] = '\0';
    
    if(iPrintNum < 0)
    {
        DBG_PRINTF("snprintf failed\r\n");
        return -1;
    }

    iError = MapFile(ptFileInfo);
    if ( iError < 0)
    {
        DBG_PRINTF("MapFile failed\r\n");
        return -1;
    }

    iError = T_BMPPicFileParser.isSuport(ptFileInfo);
    if ( iError < 0)
    {
        DBG_PRINTF("picfile is not suported\r\n");
        return -1;
    }
    GetDispResolution(&iXres, &iYres, &iBpp);

    ptPixelDatas->iBpp = iBpp;

    iError = T_BMPPicFileParser.GetPicData(ptFileInfo, ptPixelDatas);
    if ( iError < 0)
    {
        DBG_PRINTF("picfile GetPicData failed\r\n");
        return -1;
    }

    return iError;
}

int ConvertPixelData(T_PicPos tPicPos)
{
    PT_dispOpr ptDispOpr;
    unsigned char * pucDispMem;
    int iYWidth, iXWidth;
    int iX, iY;

    ptDispOpr = GetDefautDispOpr();
    pucDispMem = ptDispOpr ->pucDispMemBase;

    pucDispMem = pucDispMem + tPicPos.iLeftTopY * ptDispOpr -> iLineBytes + \
                tPicPos.iLeftTopX * ptDispOpr -> bpp / 8;

    iYWidth = (tPicPos.iRightBotY - tPicPos.iLeftTopY);
    iXWidth = (tPicPos.iRightBotX - tPicPos.iLeftTopX ) * ptDispOpr -> bpp / 8 ;

    for (iY = 0; iY < iYWidth; iY++)
    {
        for (iX = 0; iX < iXWidth; iX++)
        {
            pucDispMem[iX] = ~pucDispMem[iX];
    
        }
        pucDispMem += ptDispOpr -> iLineBytes;
    }

    return 0;
}

void FlushPageMem2FB(PT_VideoMem ptPageVideoMem)
{
    if (!ptPageVideoMem ->bDevFrameBuffer)
    {
        GetDefautDispOpr() ->ShowPage(ptPageVideoMem);
    }
}

void FreePixelDatasFrmFile(PT_PixelDatas ptPixelDatas)
{
	//Parser("bmp")->FreePixelDatas(ptPixelDatas);
	free(ptPixelDatas->aucPixelDatas);
}

void FreePageMem(PT_VideoMem ptPageVideoMem)
{
    ptPageVideoMem ->eVieoMemState = VMS_FREE;
	if (ptPageVideoMem->ID == -1)
    {
        ptPageVideoMem->ePicState = PS_BLANK;
    }
}

int PushButtom(PT_PicPos ptPicPos)
{
   return ConvertPixelData(*ptPicPos);
}

int ReleaseButtom(PT_PicPos ptPicPos)
{
    return ConvertPixelData(*ptPicPos);
}

/* 返回值: 设置了VideoMem中多少字节 */
static int SetColorForPixelInVideoMem(int iX, int iY, PT_VideoMem ptVideoMem, unsigned int dwColor)
{
	unsigned char *pucVideoMem;
	unsigned short *pwVideoMem16bpp;
	unsigned int *pdwVideoMem32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;

	pucVideoMem      = ptVideoMem->tPixelDatas.aucPixelDatas;
	pucVideoMem      += iY * ptVideoMem->tPixelDatas.iLineBytes + iX * ptVideoMem->tPixelDatas.iBpp / 8;
	pwVideoMem16bpp  = (unsigned short *)pucVideoMem;
	pdwVideoMem32bpp = (unsigned int *)pucVideoMem;

	//DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	//DBG_PRINTF("x = %d, y = %d\n", iX, iY);
	
	switch (ptVideoMem->tPixelDatas.iBpp)
	{
		case 8:
		{
			*pucVideoMem = (unsigned char)dwColor;
			return 1;
			break;
		}
		case 16:
		{
			iRed   = (dwColor >> (16+3)) & 0x1f;
			iGreen = (dwColor >> (8+2)) & 0x3f;
			iBlue  = (dwColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			*pwVideoMem16bpp	= wColor16bpp;
			return 2;
			break;
		}
		case 32:
		{
			*pdwVideoMem32bpp = dwColor;
			return 4;
			break;
		}
		default :
		{			
			return -1;
		}
	}

	return -1;
}



static int MergeOneFontToVideoMem(PT_FontBitMap ptFontBitMap, PT_VideoMem ptVideoMem)
{
	int i = 0;
	int x, y;
	int bit;
	int iNum;
	unsigned char ucByte;

	if (ptFontBitMap->iBpp == 1)
	{
        DBG_PRINTF("ibpp = %d\r\n",ptFontBitMap->iBpp );
		for (y = ptFontBitMap->iYTop; y < ptFontBitMap->iYMax; y++)
		{
			i = (y - ptFontBitMap->iYTop) * ptFontBitMap->iPitch;
			for (x = ptFontBitMap->iXLeft, bit = 7; x < ptFontBitMap->iXMax; x++)
			{
				if (bit == 7)
				{
					ucByte = ptFontBitMap->pucBuffer[i++];
				}
				
				if (ucByte & (1<<bit))
				{
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_FOREGROUND);
				}
				else
				{
					/* 使用背景色 */
					// g_ptDispOpr->ShowPixel(x, y, 0); /* 黑 */
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_BACKGROUND);
				}
				if (iNum == -1)
				{
					return -1;
				}
				bit--;
				if (bit == -1)
				{
					bit = 7;
				}
			}
		}
	}
	else if (ptFontBitMap->iBpp == 8)
	{
        DBG_PRINTF("ibpp = %d\r\n",ptFontBitMap->iBpp );
		for (y = ptFontBitMap->iYTop; y < ptFontBitMap->iYMax; y++)
			for (x = ptFontBitMap->iXLeft; x < ptFontBitMap->iXMax; x++)
			{
				//g_ptDispOpr->ShowPixel(x, y, ptFontBitMap->pucBuffer[i++]);
				if (ptFontBitMap->pucBuffer[i++])
				{
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_FOREGROUND);
				}
				else
				{
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_BACKGROUND);
				}
				
				if (iNum == -1)
				{
					return -1;
				}
			}
	}
	else
	{
		DBG_PRINTF("ShowOneFont error, can't support %d bpp\n", ptFontBitMap->iBpp);
		return -1;
	}
	return 0;
}


#if 0


#else
	
static int isFontInArea(int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY, PT_FontBitMap ptFontBitMap)
{
    if ((ptFontBitMap->iXLeft >= iTopLeftX) && (ptFontBitMap->iXMax <= iBotRightX) && \
         (ptFontBitMap->iYTop >= iTopLeftY) && (ptFontBitMap->iYMax <= iBotRightY))
         return 1;
    else
        return 0;
        
}

/*
 * 在videomem的指定矩形中间显示字符串
 * 参考: 03.freetype\02th_arm\06th_show_lines_center
 */
int MergerStringToCenterOfRectangleInVideoMem(int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY, unsigned char *pucTextString, PT_VideoMem ptVideoMem)
{
	int iLen;
	int iError;
	unsigned char *pucBufStart;
	unsigned char *pucBufEnd;
	unsigned int dwCode;
	T_FontBitMap tFontBitMap;
	
	int bHasGetCode = 0;

	int iMinX = 32000, iMaxX = -1;
	int iMinY = 32000, iMaxY = -1;

	int iStrTopLeftX , iStrTopLeftY;

	int iWidth, iHeight;

	tFontBitMap.iCurOriginX = 0;
	tFontBitMap.iCurOriginY = 0;
	pucBufStart = pucTextString;
	pucBufEnd   = pucTextString + strlen((char *)pucTextString);

	/* 0. 清除这个区域 */
	ClearRectangleInVideoMem(iTopLeftX, iTopLeftY, iBotRightX, iBotRightY, ptVideoMem, COLOR_BACKGROUND);
	
	/* 1.先计算字符串显示的总体宽度、高度 */
	while (1)
	{
		/* 从字符串中逐个取出字符 */
		iLen = GetCodeFrmBuf(pucBufStart, pucBufEnd, &dwCode);
		if (0 == iLen)
		{
			/* 字符串结束 */
			if (!bHasGetCode)
			{
				//DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
				return -1;
			}
			else
			{
				break;
			}
		}

		bHasGetCode = 1;
		pucBufStart += iLen;

		/* 获得字符的位图, 位图信息里含有字符显示时的左上角、右下角坐标 */
		iError = GetFontBitmap(dwCode, &tFontBitMap);
		if (0 == iError)
		{									
			if (iMinX > tFontBitMap.iXLeft)
			{
				iMinX = tFontBitMap.iXLeft;
			}
			if (iMaxX < tFontBitMap.iXMax)
			{
				iMaxX = tFontBitMap.iXMax;
			}

			if (iMinY > tFontBitMap.iYTop)
			{
				iMinY = tFontBitMap.iYTop;
			}
			if (iMaxY < tFontBitMap.iXMax)
			{
				iMaxY = tFontBitMap.iYMax;
			}
			
			tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX;
			tFontBitMap.iCurOriginY = tFontBitMap.iNextOriginY;
		}
		else
		{
			DBG_PRINTF("GetFontBitmap for calc width/height error!\n");
		}
	}	

	iWidth  = iMaxX - iMinX;
	iHeight = iMaxY - iMinY;

    /* 如果字符串过长 */
    if (iWidth > iBotRightX - iTopLeftX)
    {
        iWidth = iBotRightX - iTopLeftX;
    }

    /* 如果字符串过高 */
	if (iHeight > iBotRightY - iTopLeftY)
	{
		DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		//DBG_PRINTF("iHeight = %d, iBotRightY - iTopLeftX = %d - %d = %d\n", iHeight, iBotRightY, iTopLeftY, iBotRightY - iTopLeftY);
		return -1;
	}
	//DBG_PRINTF("iWidth = %d, iHeight = %d\n", iWidth, iHeight);

	/* 2.确定第1个字符的原点 
	 * 2.1 先计算左上角坐标
	 */
	iStrTopLeftX = iTopLeftX + (iBotRightX - iTopLeftX - iWidth) / 2;
	iStrTopLeftY = iTopLeftY + (iBotRightY - iTopLeftY - iHeight) / 2;
	//DBG_PRINTF("iNewFirstFontTopLeftX = %d, iNewFirstFontTopLeftY = %d\n", iNewFirstFontTopLeftX, iNewFirstFontTopLeftY);

	/*	 
	 * 2.2 再计算第1个字符原点坐标
	 * iMinX - 原来的iCurOriginX(0) = iStrTopLeftX - 新的iCurOriginX
	 * iMinY - 原来的iCurOriginY(0) = iStrTopLeftY - 新的iCurOriginY
	 */
	tFontBitMap.iCurOriginX = iStrTopLeftX - iMinX;
	tFontBitMap.iCurOriginY = iStrTopLeftY - iMinY;

	//DBG_PRINTF("iCurOriginX = %d, iCurOriginY = %d\n", tFontBitMap.iCurOriginX, tFontBitMap.iCurOriginY);
	
	pucBufStart = pucTextString;	
	bHasGetCode = 0;
	while (1)
	{
		/* 从字符串中逐个取出字符 */
		iLen = GetCodeFrmBuf(pucBufStart, pucBufEnd, &dwCode);
		if (0 == iLen)
		{
			/* 字符串结束 */
			if (!bHasGetCode)
			{
				DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
				return -1;
			}
			else
			{
				break;
			}
		}

		bHasGetCode = 1;
		pucBufStart += iLen;

		/* 获得字符的位图 */
		iError = GetFontBitmap(dwCode, &tFontBitMap);
		if (0 == iError)
		{
			//DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
			/* 显示一个字符 */
            if (isFontInArea(iTopLeftX, iTopLeftY, iBotRightX, iBotRightY, &tFontBitMap))
            {
    			if (MergeOneFontToVideoMem(&tFontBitMap, ptVideoMem))
    			{
    				DBG_PRINTF("MergeOneFontToVideoMem error for code 0x%x\n", dwCode);
    				return -1;
    			}
            }
            else
            {
                return 0;
            }
			//DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
			
			tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX;
			tFontBitMap.iCurOriginY = tFontBitMap.iNextOriginY;
		}
		else
		{
			DBG_PRINTF("GetFontBitmap for drawing error!\n");
		}
	}

	return 0;
}

#endif


int GetPixelDatasFrmFile(char *strFileName, PT_PixelDatas ptPixelDatas)
{
	T_FileInfo tFileMap;
	int iError;
	int iXres, iYres, iBpp;
    PT_PicFileParser ptParser;

	strncpy(tFileMap.strFileName, strFileName, 256);
	tFileMap.strFileName[255] = '\0';
	
	iError = MapFile(&tFileMap);
	if (iError)
	{
		DBG_PRINTF("MapFile %s error!\n", strFileName);
		return -1;
	}
	DBG_PRINTF("%s %s %d\r\n", __FILE__,__FUNCTION__, __LINE__);
    ptParser = GetParser(&tFileMap);
	if (ptParser == NULL)
	{
		DBG_PRINTF("%s is not bmp file\n", strFileName);
        MunmapFile(&tFileMap);
		return -1;
	}
	DBG_PRINTF("%s %s %d\r\n", __FILE__,__FUNCTION__, __LINE__);

	GetDispResolution(&iXres, &iYres, &iBpp);
	ptPixelDatas->iBpp = iBpp;
	iError = ptParser->GetPicData(&tFileMap, ptPixelDatas);
	if (iError)
	{
		DBG_PRINTF("GetPicData for %s error!\n", strFileName);
        MunmapFile(&tFileMap);
		return -1;
	}
	DBG_PRINTF("%s %s %d\r\n", __FILE__,__FUNCTION__, __LINE__);

    MunmapFile(&tFileMap);
	return 0;
}

void ClearRectangleInVideoMem(int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY, PT_VideoMem ptVideoMem, unsigned int dwColor)
{
	int x, y;
	for (y = iTopLeftY; y <= iBotRightY; y++)
		for (x = iTopLeftX; x <= iBotRightX; x++)
			SetColorForPixelInVideoMem(x, y, ptVideoMem, dwColor);
}


