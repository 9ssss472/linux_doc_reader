
#include "draw.h"


typedef struct PageDesc{
    unsigned char * pucLcdFirstPosAtFile;
    unsigned char * pucLcdNextFirstPosAtFile;
    int iPage;
    struct PageDesc * pNextPage;
    struct PageDesc * pPrePage;
}T_PageDesc, *PT_PageDesc;

PT_PageDesc g_ptPage = NULL;
PT_PageDesc g_ptCurPage = NULL;

static PT_EncodingOpr g_ptEncondingOpr;
static unsigned char * g_pucLcdFirstPosAtFile;
static unsigned char * g_pucLcdNextPosAtFile;

static unsigned char * g_pucTextMemEnd;
static PT_dispOpr g_ptDispOpr;
static unsigned int g_dwFontSize;

/*
*@brief:打开文本文件，内存映射，获取文本第一个字符的指针和文本末尾指针, 为该文本选择合适的解码方式。
*/
int OpenTextFile(char * pcFileName)
{
    int iFd;
    struct stat tStat;
    unsigned char * pucMemOfText;
    int iError;
   

    iFd = open(pcFileName, O_RDONLY);
    if(iFd < 0)
    {
        perror("open pcFileName");
        return -1;
    }
  

    iError = fstat(iFd, &tStat);
    if(iError < 0)
    {
        perror("fstat iFd");
        return -1;
    }
   

    pucMemOfText = mmap(NULL, tStat.st_size,PROT_READ, MAP_SHARED, iFd, 0);

    if(pucMemOfText == (void *)-1)
    {
        perror("mmap iFd");
        return -1;
    }
    DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

    g_ptEncondingOpr = selectOprForFile(pucMemOfText);

    

    if(g_ptEncondingOpr == NULL)
    {
        DBG_PRINTF("can't find enconding opr for this text type");
        return -1;
    }
    DBG_PRINTF("g_ptEncondingOpr ->name : %s\r\n",g_ptEncondingOpr ->name);

    g_pucLcdFirstPosAtFile = pucMemOfText + g_ptEncondingOpr ->headlen;
    g_pucTextMemEnd = pucMemOfText + tStat.st_size;

    return 0;
}


/*
*@breif:从注册的解码链表ptFontOprSupportedHead中找到不支持文本解码方式的节点,删除掉它;如果支持旧初始出解码方式
*arg:1.GBK的点阵文件 2.freetype的矢量字体文件 3.字体大小
*/
int SetTextDetail(char * pcHZKFile, char * pcFileFreetype, unsigned int dwFontSize)
{
    PT_FontOpr   ptFontOpr;
    PT_FontOpr   ptTmp;
    ptFontOpr = g_ptEncondingOpr->ptFontOprSupportHead;
    int iError = 0;
    int iRet = -1;

    g_dwFontSize = dwFontSize;

    while(ptFontOpr)
    {
        if(strcmp(ptFontOpr->name, "ascil")  == 0)
        {
            iError = ptFontOpr->FontInit(NULL, dwFontSize);
        }
        else if(strcmp(ptFontOpr->name, "gbk")  == 0)
        {
            iError = ptFontOpr->FontInit(pcHZKFile, dwFontSize);
        }
        else
        {
            iError = ptFontOpr->FontInit(pcFileFreetype, dwFontSize);
            
        }
        
        ptTmp = ptFontOpr -> pNext;
        if(iError == 0)
        {
            iRet = 0;
        }
        else
        {
           delFontOprForEncoding(g_ptEncondingOpr, ptFontOpr);
        }
        
        ptFontOpr = ptTmp;
    }

    return iRet;

}




/*
*@brief:初始化显示器设备
*/
int SelectDiapOpr(char * pcDispType)
{
    int iError = -1;

    DBG_PRINTF("pcDispType:%s\r\n",pcDispType );
    g_ptDispOpr = getDispOpr(pcDispType);
    if(g_ptDispOpr)
    {
        iError = g_ptDispOpr ->dispInit();

        DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return iError;
    }

    return iError;
}

/*
*@brief:x轴偏移一个像素
*/
int IncLcdX(int oldX)
{
    if(oldX + 1 < g_ptDispOpr->xres)
    {
        return oldX + 1;
    }

    return 0;
}

/*
*@brief:y轴偏移一个字体高度
*/
int IncLcdY(int oldY)
{
    if(oldY + g_dwFontSize < g_ptDispOpr ->yres)
    {
        return oldY + g_dwFontSize;
    }

    return 0;
}


/*
*@brief:判断是否满页和满行，如果满行了，计算下一行首个字符的起始坐标
*/
int RelocateFontPos(PT_FontBitMap FontBitMap)
{
    int xTmp, yTmp, iY;

    if(FontBitMap->iYMax > g_ptDispOpr ->yres)
    {
        return -1;
    }

    if(FontBitMap->iXMax > g_ptDispOpr ->xres)
    {
        iY = IncLcdY(FontBitMap ->iCurOriginY);

        if(iY == 0)
        {
            return -1;
        }
        else
        {
            xTmp = 0 - FontBitMap -> iCurOriginX;
            yTmp = iY - FontBitMap ->iCurOriginY;
           

            FontBitMap ->iCurOriginX += xTmp;
            FontBitMap ->iNextOriginX += xTmp;
            FontBitMap ->iCurOriginY += yTmp;
            FontBitMap ->iNextOriginY += yTmp;

            FontBitMap ->iXLeft += xTmp;
            FontBitMap ->iXMax += xTmp;
            FontBitMap ->iYMax += yTmp;
            FontBitMap ->iYTop += yTmp;
        }
        return 0;
    }

    return 0;
}

/*
*@brief:显示一个字体
*/
static int ShowOneFont(PT_FontBitMap ptFontBitMap)
{
    int x,y;
    int i;
    int bit;
    int ucByte;


    if(ptFontBitMap ->iBpp ==1)
    {
        for(y = ptFontBitMap ->iYTop; y < ptFontBitMap ->iYMax; y++)
        {
            i = (y - ptFontBitMap->iYTop) * ptFontBitMap ->iPitch;
            for(x = ptFontBitMap ->iXLeft, bit = 7; x < ptFontBitMap ->iXMax; x++)
            {
                if(bit == 7)
                {
                    ucByte = ptFontBitMap ->pucBuffer[i++];
                }

                if(ucByte & (1 << bit))
                {   
                    g_ptDispOpr ->drawOnePixel(x, y, COLOR_FOREGROUND);
                }
                else
                {
                    /* code */
                }

                bit --;
                if(bit == -1)
                {
                    bit = 7;
                }       
            }
        }
    }
    else
    {
        return -1;
    }
    
    return 0;

}

/*
*@breif:显示一页
*/
int ShowOnePage(unsigned char *pucTextFileMemCurPos)
{
    unsigned char *pucBufferStart = pucTextFileMemCurPos;
    unsigned int pdwCode;
    int iLen;
    int bHasGetCode = 0;
    PT_FontOpr ptFontOpr = g_ptEncondingOpr ->ptFontOprSupportHead;
    T_FontBitMap tFontBitMap; 

    tFontBitMap.iCurOriginX = 0;
    tFontBitMap.iCurOriginY = g_dwFontSize;
    int iError;

    int bHasNotClrSceen = 1;

    while(1)
    {
        iLen = g_ptEncondingOpr->getCodeFrmBuf(pucBufferStart,g_pucTextMemEnd, &pdwCode);
        
        if(iLen == 0)
        {
            if(bHasGetCode)
            {
                return 0;
            }
            else
            {
                DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                return -1;
            }      
        }
        bHasGetCode = 1;

       
        pucBufferStart += iLen;

        if(pdwCode == '\n')
        {
            tFontBitMap.iCurOriginX = 0;
            tFontBitMap.iCurOriginY = IncLcdY(tFontBitMap.iCurOriginY);

            if( 0 == tFontBitMap.iCurOriginY)
            {
                return 0;
            }
            else
            {
                continue;
            } 
        }
        else if (pdwCode == '\r')
        {
            continue;
        }
        else if( pdwCode == '\t')
        {
            pdwCode = ' ';
        }
        
        while(ptFontOpr)
        {
           
            iError =  ptFontOpr ->GetFontBitmap(pdwCode, &tFontBitMap);
            
            if(iError == 0)
            {
                if( RelocateFontPos(&tFontBitMap))
                {
                    return 0;
                }

                if(bHasNotClrSceen)
                {
                   
                    g_ptDispOpr ->cleanScreen(COLOR_BACKGROUND);
                    bHasNotClrSceen = 0;
                   
                }
                
                if(ShowOneFont(&tFontBitMap))
                {
                    return -1;
                }

                tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX;
                tFontBitMap.iCurOriginY = tFontBitMap.iNextOriginY;
                g_pucLcdNextPosAtFile = pucBufferStart;

                break;
            }
            ptFontOpr = ptFontOpr ->pNext;
        }
    }

    return 0;
}


/*
*@brief:记录新的一页到页链表上
*/
static int RecordPage(PT_PageDesc ptPageNew)
{
    if(g_ptPage == NULL)
    {
        g_ptPage = ptPageNew;
    }
    else
    {
        PT_PageDesc ptPageTmp = g_ptPage;

        while(ptPageTmp ->pNextPage != NULL)
        {
            ptPageTmp = ptPageTmp->pNextPage;
        }
        ptPageTmp ->pNextPage = ptPageNew;
        ptPageNew ->pNextPage = NULL;
        ptPageNew ->pPrePage = ptPageTmp;
    }
    
    return 0;
}

/*
*@brief:显示下一页
*/
int ShowNextPage(void)
{
    unsigned char * pucTextCurMemPos;
    int iError;
    PT_PageDesc ptPage;

    if(g_ptCurPage != NULL)
    {
        pucTextCurMemPos = g_ptCurPage ->pucLcdNextFirstPosAtFile;
    }
    else
    {
        pucTextCurMemPos = g_pucLcdFirstPosAtFile;
    }
    
    iError = ShowOnePage(pucTextCurMemPos);
    

    if(iError == 0)
    {

        if(g_ptCurPage && g_ptCurPage->pNextPage)
        {
            g_ptCurPage = g_ptCurPage ->pNextPage;
        }
        ptPage = malloc(sizeof(T_PageDesc));

        if(ptPage)
        {
            ptPage->pucLcdFirstPosAtFile  = pucTextCurMemPos;
            ptPage->pucLcdNextFirstPosAtFile = g_pucLcdNextPosAtFile;
            ptPage->pNextPage = NULL;
            ptPage->pPrePage = NULL;
            g_ptCurPage = ptPage;
            DBG_PRINTF("%s %d, pos = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int)ptPage->pucLcdFirstPosAtFile);
            RecordPage(ptPage);

            return 0;
        }
        else
        {
            return -1;
        }
    }


    return iError;
}

/*
*@breif:显示上一页
*/
int ShowPrePage(void)
{
    PT_PageDesc ptPage = g_ptCurPage ->pPrePage;
    int iError;

    if(!g_ptCurPage || !g_ptCurPage ->pPrePage)
    {
        return -1;
    }
    DBG_PRINTF("%s %d, pos = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int)g_ptCurPage->pPrePage->pucLcdFirstPosAtFile);
    iError = ShowOnePage(ptPage ->pucLcdFirstPosAtFile);
    if(iError == 0)
    {
        g_ptCurPage = ptPage;
    }

    return iError;
}

int getResolution(int *xRes, int *yRes)
{
	if(g_ptDispOpr)
	{
		*xRes = g_ptDispOpr->xres;
		*yRes = g_ptDispOpr->yres;
		return 0;
	}
	else
	{
		return -1;
	}
	
}
