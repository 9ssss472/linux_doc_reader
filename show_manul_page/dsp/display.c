#include "display.h"
#include <string.h>
#include "config.h"
#include <stdlib.h>

static PT_dispOpr PT_dispOprHead = NULL;
static PT_dispOpr g_ptDefautDispOpr = NULL;
static PT_VideoMem g_ptVideoMemHead = NULL;
/*
*@brief:为链表添加表头或节点
*/
int registerDisOpr(PT_dispOpr dispOpr)
{
    if(PT_dispOprHead == NULL)
    {
        PT_dispOprHead = dispOpr;
        dispOpr ->pNext = NULL;
    }
    else
    {
        PT_dispOpr PT_Tmp = PT_dispOprHead;

        while(PT_Tmp ->pNext != NULL)
        {
            PT_Tmp = PT_Tmp ->pNext;
        }
        PT_Tmp ->pNext = dispOpr;
        dispOpr ->pNext = NULL;

    }
    
    return 0;
}

/*
*@brief:把链表上各个节点的名字打印出来
*/
int showDispOpr(void)
{
    int i = 1;

    PT_dispOpr PT_Tmp = PT_dispOprHead;

    while(PT_Tmp)
    {
        DBG_PRINTF("%d: %s\r\n",i++,PT_Tmp ->name);
        PT_Tmp = PT_Tmp ->pNext;
    }

    return 0;
}


/*
*@brief:获取name符合参数一的节点
*/
PT_dispOpr getDispOpr(char * name)
{
    PT_dispOpr PT_Tmp = PT_dispOprHead;

    while(PT_Tmp)
    {
        if(strcmp(name, PT_Tmp ->name) == 0)
        {
            return PT_Tmp;
        }

        PT_Tmp = PT_Tmp ->pNext;
    }
    return NULL;
}


int SelectAndInitDefautDisp(char * name)
{
    PT_dispOpr ptDispOpr;

    ptDispOpr = getDispOpr(name);

    if(ptDispOpr == NULL)
    {
        DBG_PRINTF("SelectAndInitDefautDisp failed\r\n");
        return -1;
    }

    ptDispOpr->dispInit();
    ptDispOpr->cleanScreen(0);

    g_ptDefautDispOpr = ptDispOpr;

    return 0;
}

PT_dispOpr GetDefautDispOpr(void)
{
    return g_ptDefautDispOpr;
}

int GetDispResolution(int *piXres, int *piYres, int *piBpp)
{
	if (g_ptDefautDispOpr)
	{
		*piXres = g_ptDefautDispOpr->xres;
		*piYres = g_ptDefautDispOpr->yres;
		*piBpp  = g_ptDefautDispOpr->bpp;
		return 0;
	}
	else
	{
		return -1;
	}
}

int AllocateVidieoMem(int iNum)
{
    int i;

    int iXres = 0, iYres = 0, iBpp = 0;
    int iVMSize, iLineBytes;
    PT_VideoMem ptVideoMem;
    PT_dispOpr ptDefautDispOpr;

    ptDefautDispOpr = GetDefautDispOpr();

    GetDispResolution(&iXres, &iYres, &iBpp);
    iVMSize =  iXres * iYres * iBpp /8;
    iLineBytes = iXres * iBpp /8;

    ptVideoMem = malloc(sizeof(T_VideoMem));

    if(ptVideoMem == NULL)
    {
        DBG_PRINTF("malloc ptVideoMem failed\r\n");
        return -1;
    }

    ptVideoMem ->bDevFrameBuffer = 1;
    ptVideoMem ->ePicState = PS_BLANK;
    ptVideoMem ->eVieoMemState = VMS_FREE;
    ptVideoMem ->ID = 0;
    ptVideoMem ->tPixelDatas.aucPixelDatas = ptDefautDispOpr ->pucDispMemBase;
    ptVideoMem ->tPixelDatas.iBpp = iBpp;
    ptVideoMem ->tPixelDatas.iHeight = iYres;
    ptVideoMem ->tPixelDatas.iWidth = iXres;
    ptVideoMem ->tPixelDatas.iLineBytes = iLineBytes;
    ptVideoMem ->tPixelDatas.iTotalBytes = iVMSize;

    if (iNum != 0)
    {
        ptVideoMem ->eVieoMemState = VMS_USED_FOR_CUR;
    }

    ptVideoMem ->ptNext = g_ptVideoMemHead;
    g_ptVideoMemHead = ptVideoMem;

    for (i = 0; i < iNum; i++)
    {
        ptVideoMem = malloc(sizeof(T_VideoMem) + iVMSize);
        
        if(ptVideoMem == NULL)
        {
            return -1;
        }

        ptVideoMem ->bDevFrameBuffer = 0;
        ptVideoMem ->ID = 0;
        ptVideoMem ->ePicState = PS_BLANK;
        ptVideoMem ->eVieoMemState = VMS_FREE;
        ptVideoMem ->tPixelDatas.iBpp = iBpp;
        ptVideoMem ->tPixelDatas.iHeight = iYres;
        ptVideoMem ->tPixelDatas.iWidth = iXres;
        ptVideoMem ->tPixelDatas.iLineBytes = iLineBytes;
        ptVideoMem ->tPixelDatas.iTotalBytes = iVMSize;
        ptVideoMem ->tPixelDatas.aucPixelDatas = (unsigned char*)(ptVideoMem + 1);

        ptVideoMem ->ptNext = g_ptVideoMemHead;
        g_ptVideoMemHead = ptVideoMem;

    }

    return 0;

}


PT_VideoMem GetFreeVideoMem(int ID, int iBur)
{
    PT_VideoMem ptVideoMem = g_ptVideoMemHead;

    while(ptVideoMem)
    {
       
        /*优先选择ID相同的内存*/
        if((ptVideoMem->ID == ID) && (ptVideoMem ->eVieoMemState == VMS_FREE ))
        {
          
            ptVideoMem ->eVieoMemState = iBur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
            return ptVideoMem;
        }
        ptVideoMem = ptVideoMem ->ptNext;
    }

    ptVideoMem = g_ptVideoMemHead;
    
    while(ptVideoMem)
    {
        if(ptVideoMem ->eVieoMemState == VMS_FREE && ptVideoMem ->ID == 0)
        {
            ptVideoMem ->eVieoMemState = iBur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
            ptVideoMem ->ID = ID;
            ptVideoMem ->ePicState = PS_BLANK;
           
            return ptVideoMem;
        }
        ptVideoMem = ptVideoMem ->ptNext;
    }

    return NULL;
}


PT_VideoMem GetDevVideoMem(void)
{
	PT_VideoMem ptTmp = g_ptVideoMemHead;
	
	while (ptTmp)
	{
		if (ptTmp->bDevFrameBuffer)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}


/* 把显存全部清为某种颜色 */
void ClearVideoMem(PT_VideoMem ptVideoMem, unsigned int dwColor)
{
	unsigned char *pucVM;
	unsigned short *pwVM16bpp;
	unsigned int *pdwVM32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;
	int i = 0;

	pucVM	   = ptVideoMem->tPixelDatas.aucPixelDatas;
	pwVM16bpp  = (unsigned short *)pucVM;
	pdwVM32bpp = (unsigned int *)pucVM;

	switch (ptVideoMem->tPixelDatas.iBpp)
	{
		case 8:
		{
			memset(pucVM, dwColor, ptVideoMem->tPixelDatas.iTotalBytes);
			break;
		}
		case 16:
		{
			iRed   = (dwColor >> (16+3)) & 0x1f;
			iGreen = (dwColor >> (8+2)) & 0x3f;
			iBlue  = (dwColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			while (i < ptVideoMem->tPixelDatas.iTotalBytes)
			{
				*pwVM16bpp	= wColor16bpp;
				pwVM16bpp++;
				i += 2;
			}
			break;
		}
		case 32:
		{
			while (i < ptVideoMem->tPixelDatas.iTotalBytes)
			{
				*pdwVM32bpp = dwColor;
				pdwVM32bpp++;
				i += 4;
			}
			break;
		}
		default :
		{
			DBG_PRINTF("can't support %d bpp\n", ptVideoMem->tPixelDatas.iBpp);
			return;
		}
	}

}



int DisplayInit(void)
{
    int Error;

    Error = FbInit();

    return Error;
}