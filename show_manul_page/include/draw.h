#ifndef _DRAW_H
#define _DRAW_H

#include "config.h"
#include "display.h"
#include "encoding_manager.h"
#include "fonts_manager.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

int OpenTextFile(char * pcFileName);
int SetTextDetail(char * pcHZKFile, char * pcFileFreetype, unsigned int dwFontSize);
int SelectDiapOpr(char * pcDispType);
int IncLcdX(int oldX);
int IncLcdY(int oldY);
int RelocateFontPos(PT_FontBitMap FontBitMap);
int ShowOnePage(unsigned char *pucTextFileMemCurPos);
int ShowNextPage(void);
int ShowPrePage(void);


#endif /* _DRAW_H */
