#include "fonts_manager.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "config.h"

static int GBKFontInit(char * pcFontFile, unsigned int dwFontSize);
static int GBKGetFontBitmap(unsigned int dwcode, PT_FontBitMap ptFontBitMap);


static T_FontOpr GBKFontOpr = {
    .name = "gbk",
    .FontInit = GBKFontInit,
    .GetFontBitmap = GBKGetFontBitmap,
};

static int iGBKfd;
static unsigned char * pucMemStart;
static unsigned char * pucMemEnd;

/*
*@brief:打开GBK的点阵文件，映射虚拟内存
*@arg:1.点阵文件字符串。2.字体大小
*/
static int GBKFontInit(char * pcFontFile, unsigned int dwFontSize)
{
    struct stat buf;
    int iError;

    if(dwFontSize != 16)
    {
        DBG_PRINTF("不支持%d大小的字体\r\n",dwFontSize);
        return -1;
    }

    iGBKfd = open(pcFontFile, O_RDONLY);
    if(iGBKfd < 0)
    {
        perror("open iGBKfd");
        return -1;
    }

    iError = fstat( iGBKfd, &buf);
    if(iError == -1)
    {
        perror("fstat iGBKfd");
        return -1;
    }

    pucMemStart = mmap(NULL, buf.st_size,  PROT_READ, MAP_SHARED,iGBKfd, 0);
    if(pucMemStart == (unsigned char *) -1)
    {
        perror("mmap iGBKfd");
        return -1;
    }

    pucMemEnd = pucMemStart + buf.st_size;

    return 0;
    
}

/*
*@brief:跟据编码值（参数一）获取点阵地址存到参数二的成员中
*/
static int GBKGetFontBitmap(unsigned int dwcode, PT_FontBitMap ptFontBitMap)
{
    int iWhere, iArea;
    int iPenX = ptFontBitMap ->iCurOriginX;
    int iPenY = ptFontBitMap ->iNextOriginY;

    if(dwcode && 0xffff0000)
    {
        DBG_PRINTF("GBK编码错误\r\n");
        return -1;
    }

    iArea = (int)(dwcode & 0xff) - 0xA1;
    iWhere = (int)((dwcode >> 8) & 0xff) - 0xA1;
    
    if(iArea < 0 || iWhere < 0)
    {
        DBG_PRINTF("iArea or iWhere wrong\r\n");
        return -1;
    }

    ptFontBitMap ->iXLeft = iPenX;
    ptFontBitMap ->iYTop = iPenY - 16;
    ptFontBitMap ->iXMax = iPenX + 16;
    ptFontBitMap ->iYMax = iPenY;
    ptFontBitMap ->iPitch = 2;

    ptFontBitMap ->pucBuffer = pucMemStart +(iArea * 94 + iWhere) * 32;

    if(ptFontBitMap ->pucBuffer >= pucMemEnd)
    {
        DBG_PRINTF("ptFontBitMap ->pucBuffer >= pucMemEnd\r\n");
        DBG_PRINTF("ptFontBitMap ->pucBuffer = %p\r\n",ptFontBitMap ->pucBuffer);
        DBG_PRINTF("pucMemEnd = %p\r\n",pucMemEnd);
        return -1;
    }

    ptFontBitMap ->iNextOriginX = iPenX + 16;
    ptFontBitMap ->iNextOriginY = iPenY;

    return 0;
}


int GBKInit(void)
{

    return registerFontOpr(&GBKFontOpr);
}