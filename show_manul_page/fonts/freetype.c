#include "fonts_manager.h"
#include "config.h"
#include <ft2build.h> 

static int FreeTypeFontInit(char * pcFontFile, unsigned int dwFontSize);
static int FreeTypeGetFontBitmap(unsigned int dwcode, PT_FontBitMap ptFontBitMap);
static void FreeTypeSetFontSize(unsigned int dwFontSize);

static T_FontOpr GBKFontOpr = {
    .name = "freetype",
    .FontInit = FreeTypeFontInit,
    .GetFontBitmap = FreeTypeGetFontBitmap,
    .SetFontSize = FreeTypeSetFontSize,
};

#include FT_FREETYPE_H
#include FT_GLYPH_H


FT_Library g_tLibrary;
FT_Face g_tFace;
FT_GlyphSlot g_tSlot;

/*
*@brief:初始化FreeType库
*/
static int FreeTypeFontInit(char * pcFontFile, unsigned int dwFontSize)
{
    FT_Error FTError;

    FTError = FT_Init_FreeType(&g_tLibrary);
    if(FTError != 0)
    {
        DBG_PRINTF("FT_Init_FreeType failed");
        return -1;
    }

    FTError = FT_New_Face(g_tLibrary, pcFontFile, 0 ,&g_tFace);
    if(FTError != 0)
    {
        DBG_PRINTF("FT_New_Face failed");
        return -1;
    }
    g_tSlot = g_tFace ->glyph;

    FTError = FT_Set_Pixel_Sizes(g_tFace, dwFontSize, dwFontSize);
    if(FTError != 0)
    {
        DBG_PRINTF("FT_Set_Pixel_Sizes failed");
        return -1;
    }

    return 0;
}


static int FreeTypeGetFontBitmap(unsigned int dwcode, PT_FontBitMap ptFontBitMap)
{
    int iPenX = ptFontBitMap->iCurOriginX;
    int iPenY = ptFontBitMap->iCurOriginY;
    int FTError;

    FTError = FT_Load_Char(g_tFace, dwcode, FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
    if(FTError != 0)
    {
        DBG_PRINTF("FT_Load_Char failed\r\n");
        return -1;
    }

    ptFontBitMap->iXLeft = iPenX + g_tSlot->bitmap_left;
    ptFontBitMap->iYTop  = iPenY - g_tSlot -> bitmap_top;

    ptFontBitMap->iXMax = ptFontBitMap->iXLeft + g_tSlot ->bitmap.width;
    ptFontBitMap->iYMax = ptFontBitMap->iYTop + g_tSlot ->bitmap.rows;
    ptFontBitMap->iPitch = g_tSlot->bitmap.pitch;
    ptFontBitMap->iNextOriginX = iPenX + g_tSlot->advance.x / 64;
    ptFontBitMap->iNextOriginY = iPenY;
    ptFontBitMap->pucBuffer = g_tSlot->bitmap.buffer;
    ptFontBitMap->iBpp = 1;


    return 0;
}

static void FreeTypeSetFontSize(unsigned int dwFontSize)
{
	FT_Set_Pixel_Sizes(g_tFace, dwFontSize, 0);
}

int FreeTypeInit(void)
{
    return registerFontOpr(&GBKFontOpr);
}