#ifndef __PIC_OPRATION_H
#define __PIC_OPRATION_H

#include <stdio.h>
#include "file.h"

typedef struct PixelDatas {
    int iWidth;
    int iHeight;
    int iBpp;
    int iLineBytes;
    int iTotalBytes;
    unsigned char *aucPixelDatas;
}T_PixelDatas, *PT_PixelDatas;

typedef struct PicFileParser{
    char * name;
    int (*isSuport)(PT_FileInfo ptFileMap);
    int (*GetPicData)(PT_FileInfo ptFileMap, PT_PixelDatas ptPixelData);
    int (*FreePixelDatas)(PT_PixelDatas ptPixelData);
    struct PicFileParser *ptNext;
}T_PicFileParser, *PT_PicFileParser;


int PicMerge(int iX, int iY, PT_PixelDatas  SmallPixelDatas, PT_PixelDatas BigPixelDatas);
int PicZoom(PT_PixelDatas ptOriginPic, PT_PixelDatas ptZoomPic);
int GetFilePixel(char* pcFileName, PT_PixelDatas ptPixelDatas);
PT_PicFileParser Parser(char *pcName);
PT_PicFileParser GetParser(PT_FileInfo ptFileMap);
#endif