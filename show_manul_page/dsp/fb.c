#include "display.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "config.h"
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>


static int fbDispInit(void);
static int fbDrawOnePixel(int x, int y, int color);
static int fbCleanScreen(unsigned int color);
static int FBShowPage(PT_VideoMem ptVideoMem);

static T_dispOpr g_tDispOpr = {
    .name = "fb",
    .dispInit = fbDispInit,
    .drawOnePixel =  fbDrawOnePixel,
    .cleanScreen = fbCleanScreen,
	.ShowPage = FBShowPage,
}; 

int g_iFd;
struct fb_var_screeninfo g_tFb;
int g_screenSize;
unsigned char * p_fbBase = NULL;
static unsigned int g_dwLineWidth;
static unsigned int g_dwPixelWidth;

/*
*@brief：获取fb的可变参数，为fb类型的显示器映射虚拟内存
*/
static int fbDispInit(void)
{
    int iError;

    g_iFd = open(SCREEN_PATH, O_RDWR);
    if(g_iFd < 0)
    {
        perror("open g_iFd");
        return -1;
    }

    iError = ioctl(g_iFd, FBIOGET_VSCREENINFO, &g_tFb);
    if(iError < 0 )
    {
        perror("ioctl:");
        return -1;
    }

    g_tDispOpr.xres = g_tFb.xres;
    g_tDispOpr.yres = g_tFb.yres;
    g_tDispOpr.bpp = g_tFb.bits_per_pixel;
	
    g_screenSize = g_tDispOpr.xres * g_tDispOpr.yres * g_tDispOpr.bpp / 8;
    g_dwLineWidth  = g_tFb.xres * g_tFb.bits_per_pixel / 8;
	g_dwPixelWidth = g_tFb.bits_per_pixel / 8;
	
	g_tDispOpr.iLineBytes = g_dwLineWidth;

    p_fbBase = mmap(NULL, g_screenSize, PROT_WRITE, MAP_SHARED, g_iFd, 0);
    if(p_fbBase == (void *)-1)
    {
        perror("mmap fd:");
        return -1;
    }

	g_tDispOpr.pucDispMemBase = p_fbBase;

	return 0;
}


static int fbDrawOnePixel(int iX, int iY, int dwColor)
{
    unsigned char *pucFB;
	unsigned short *pwFB16bpp;
	unsigned int *pdwFB32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;

	if ((iX >= g_tFb.xres) || (iY >= g_tFb.yres))
	{
		DBG_PRINTF("out of region\n");
		return -1;
	}

	pucFB      = p_fbBase + g_dwLineWidth * iY + g_dwPixelWidth * iX;
	pwFB16bpp  = (unsigned short *)pucFB;
	pdwFB32bpp = (unsigned int *)pucFB;
	
	switch (g_tFb.bits_per_pixel)
	{
		case 8:
		{
			*pucFB = (unsigned char)dwColor;
			break;
		}
		case 16:
		{
			iRed   = (dwColor >> (16+3)) & 0x1f;
			iGreen = (dwColor >> (8+2)) & 0x3f;
			iBlue  = (dwColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			*pwFB16bpp	= wColor16bpp;
			break;
		}
		case 32:
		{
			*pdwFB32bpp = dwColor;
			break;
		}
		default :
		{
			DBG_PRINTF("can't support %d bpp\n", g_tFb.bits_per_pixel);
			return -1;
		}
	}

	return 0;

}


static int fbCleanScreen(unsigned int dwBackColor)
{
	unsigned char *pucFB;
	unsigned short *pwFB16bpp;
	unsigned int *pdwFB32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;
	int i = 0;

	pucFB      = p_fbBase;
	pwFB16bpp  = (unsigned short *)pucFB;
	pdwFB32bpp = (unsigned int *)pucFB;

	switch (g_tFb.bits_per_pixel)
	{
		case 8:
		{
			memset(p_fbBase, dwBackColor, g_screenSize);
			break;
		}
		case 16:
		{
			iRed   = (dwBackColor >> (16+3)) & 0x1f;
			iGreen = (dwBackColor >> (8+2)) & 0x3f;
			iBlue  = (dwBackColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			while (i < g_screenSize)
			{
				*pwFB16bpp	= wColor16bpp;
				pwFB16bpp++;
				i += 2;
			}
			break;
		}
		case 32:
		{
			while (i < g_screenSize)
			{
				*pdwFB32bpp	= dwBackColor;
				pdwFB32bpp++;
				i += 4;
			}
			break;
		}
		default :
		{
			DBG_PRINTF("can't support %d bpp\n", g_tFb.bits_per_pixel);
			return -1;
		}
	}

	return 0;
}


static int FBShowPage(PT_VideoMem ptVideoMem)
{
	memcpy(g_tDispOpr.pucDispMemBase, ptVideoMem ->tPixelDatas.aucPixelDatas, ptVideoMem ->tPixelDatas.iTotalBytes);
	return 0;
}


int FbInit(void)
{
    return registerDisOpr(&g_tDispOpr);
}