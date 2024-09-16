#include "pic_operation.h"
#include <string.h>
#include "config.h"
#include <stdlib.h>
#include "picfmt_manager.h"

#pragma pack(push) /* 将当前pack设置压栈保存 */
#pragma pack(1)    /* 必须在结构体定义之前使用 */

typedef struct tagBITMAPFILEHEADER { /* bmfh */
	unsigned short bfType; 
	unsigned long  bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned long  bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER { /* bmih */
	unsigned long  biSize;
	unsigned long  biWidth;
	unsigned long  biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned long  biCompression;
	unsigned long  biSizeImage;
	unsigned long  biXPelsPerMeter;
	unsigned long  biYPelsPerMeter;
	unsigned long  biClrUsed;
	unsigned long  biClrImportant;
} BITMAPINFOHEADER;

#pragma pack(pop) /* 恢复先前的pack设置 */

static int isBMPFormat(PT_FileInfo ptFileMap);
static int GetBMPData(PT_FileInfo ptFileMap, PT_PixelDatas ptPixelData);
static int FreeBMPPixelDatas(PT_PixelDatas ptPixelData);

T_PicFileParser T_BMPPicFileParser = {
    .name = "BMP",
    .isSuport = isBMPFormat,
    .GetPicData = GetBMPData,
    .FreePixelDatas = FreeBMPPixelDatas,
};


static int isBMPFormat(PT_FileInfo ptFileMap)
{
    unsigned char *aFileHead = ptFileMap->pucFileMem;

    if(aFileHead[0] == 0x42 && aFileHead[1] == 0x4d)
    {
        DBG_PRINTF("image is suported\r\n");
        return 1;
    }

    return 0;
}

/*
@brief:把图片的一行数据从24BPP转换成32或16BPP
@arg1:图片的一行宽度，像素为单位
*/
int convertOneLine(int iWidth, int iSrcBpp, int iDestBpp, unsigned char * pucSrcPixelDatas, unsigned char * pucDestPixeleDatas)
{
    unsigned int * PixelData32Bpp = (unsigned int * )pucDestPixeleDatas;
    unsigned short * PixelData16Bpp = (unsigned short *)pucDestPixeleDatas;
    int dwcolor = 0;
    int pos = 0;
    int i = 0;
    int iRed, iGreen, iBlue;

    if(iSrcBpp != 24)
    {
        DBG_PRINTF("iBpp != 24");
        return -1;
    }

    if(iDestBpp == 24)
    {
        memcpy(pucDestPixeleDatas,pucSrcPixelDatas, iWidth * 3);
    }

    for(i = 0; i<iWidth; i++)
    {
        iRed = pucSrcPixelDatas[pos++];
        iGreen = pucSrcPixelDatas[pos++];
        iBlue = pucSrcPixelDatas[pos++];

        if(iDestBpp == 32)
        {
            dwcolor = iRed << 16 | iGreen << 8 |  iBlue;
            *PixelData32Bpp = dwcolor;
            PixelData32Bpp ++;
        }
        else if(iDestBpp == 16)
        {
            iRed = iRed >> 3;
            iGreen = iGreen >> 2;
            iBlue = iBlue >> 2;
            dwcolor = iRed << 11 | iGreen << 5 | iBlue;
            *PixelData16Bpp = dwcolor;
            PixelData16Bpp++;
        }   
    }

    return 0;
}




static int GetBMPData(PT_FileInfo ptFileMap, PT_PixelDatas ptPixelData)
{

    int iWidth;
    int iHeight;
    int iBpp;
    int iLineByte;
    unsigned char * pucSrc, *pucDest;
    int i;
    int iLineWidthAlign;
    int iLineWidthReal;
    unsigned char *aFileHead = ptFileMap->pucFileMem;

    BITMAPFILEHEADER * ptBITMATFILEHEADER = (BITMAPFILEHEADER *) aFileHead;
    BITMAPINFOHEADER * ptBITMAPINFOHEADER = (BITMAPINFOHEADER *) (aFileHead + sizeof(BITMAPFILEHEADER));

    iBpp = ptBITMAPINFOHEADER ->biBitCount;

    DBG_PRINTF("iBpp = %d\r\n", iBpp);

    if(iBpp != 24)
    {
        DBG_PRINTF("iBpp != 24");
        return -1;
    }

    iWidth = ptBITMAPINFOHEADER ->biWidth;
    iHeight = ptBITMAPINFOHEADER ->biHeight;
    iLineByte = iWidth * iBpp / 8;

    ptPixelData ->iHeight = iHeight;
    ptPixelData ->iLineBytes = iLineByte;
    ptPixelData ->iWidth = iWidth;

#if 0
    DBG_PRINTF("iWidth = %d\r\n",iWidth);
    DBG_PRINTF("iHeight = %d\r\n",iHeight);
    DBG_PRINTF("iLineByte = %d\r\n",iLineByte);
    DBG_PRINTF("iBpp = %d\r\n",iBpp);

#endif
    ptPixelData ->aucPixelDatas = malloc(iWidth * iHeight * iBpp /8);

    if(ptPixelData ->aucPixelDatas == NULL)
    {
        DBG_PRINTF("malloc ptPixelData ->aucPixelDatas failed");
        return -1;
    }

   // pucSrc =  ptBITMAPINFOHEADER + (iHeight - 1) * iLineByte;
   iLineWidthReal = iWidth * iBpp / 8;
   iLineWidthAlign = (iLineWidthReal + 3) & ~0x03;

   pucSrc = aFileHead + ptBITMATFILEHEADER ->bfOffBits;
   pucSrc += (iHeight - 1) * iLineWidthAlign;

   pucDest = ptPixelData ->aucPixelDatas;

    for(i = 0; i < iHeight; i++)
    {
        convertOneLine(iWidth, iBpp,16, pucSrc, pucDest);
        pucSrc -= iLineWidthAlign;
        pucDest += ptPixelData ->iLineBytes;
    }

    
    return 0;
}



static int FreeBMPPixelDatas(PT_PixelDatas ptPixelData)
{
    free(ptPixelData -> aucPixelDatas);

    return 0;
}


int BMPParserInit(void)
{
	return RegisterPicFileParser(&T_BMPPicFileParser);
}