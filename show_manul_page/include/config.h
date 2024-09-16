#ifndef _CONFIG_H
#define _CONFIG_H

#include "debug_manager.h"

#define SCREEN_PATH "/dev/fb0"



#define COLOR_BACKGROUND   0xE7DBB5  /* 泛黄的纸 */
#define COLOR_FOREGROUND   0x514438  /* 褐色字体 */

//#define DBG_PRINTF(...)  
#define DBG_PRINTF DebugPrint

#define DEFAULT_DIR "/"

#define ICON_PATH  "/etc/digitpic/icons/"

#endif /* _CONFIG_H */