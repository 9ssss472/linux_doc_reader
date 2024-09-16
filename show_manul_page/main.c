#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <config.h>
#include <draw.h>
#include <encoding_manager.h>
#include <fonts_manager.h>
#include <display.h>
#include <string.h>
#include "input.h"
#include "debug_manager.h"
#include "pic_operation.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "page_manager.h"
#include "picfmt_manager.h"


/*
*@Usage:./filename bmp
*/
int main(int argc, char ** argv)
{
    if(argc < 2)
    {
        printf("Usage: %s <freetype filename>\r\n", argv[0]);
        return -1;
    }

    DebugInit();
    DebugChanelInit();

    DisplayInit();
    SelectAndInitDefautDisp("fb");

    AllocateVidieoMem(6);

    PageInit();
    PicFmtsInit();

    InputInit();
    AllInputDevicesInit();

    FontInit();

    SetFontDetail("freetype", argv[1], 24);

    DebugInit();

    Page("main") ->Run();

    return 0;
}