#include <string.h>
#include "pic_operation.h"
#include "config.h"

int PicMerge(int iX, int iY, PT_PixelDatas  SmallPixelDatas, PT_PixelDatas BigPixelDatas)
{
    unsigned  char * pucSrc, *pucDest;
    int i = 0;

    if(SmallPixelDatas ->iWidth > BigPixelDatas ->iWidth || 
        SmallPixelDatas ->iHeight > BigPixelDatas ->iHeight ||
        SmallPixelDatas ->iBpp != BigPixelDatas ->iBpp
    )
    {
        DBG_PRINTF("picMerge error \r\n");
        return -1;
    }

    pucSrc = SmallPixelDatas ->aucPixelDatas;
    pucDest = BigPixelDatas ->aucPixelDatas + BigPixelDatas ->iLineBytes * iY + iX * BigPixelDatas ->iBpp /8 ;

    for(i = 0; i<SmallPixelDatas ->iHeight; i++)
    {
        memcpy(pucDest, pucSrc, SmallPixelDatas ->iLineBytes);
        pucSrc += SmallPixelDatas ->iLineBytes;
        pucDest += BigPixelDatas ->iLineBytes;
    }


    return 0;
}