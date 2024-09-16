#ifndef _ENCODING_MANAGER_H
#define _ENCODING_MANAGER_H

#include <string.h>
#include "fonts_manager.h"

typedef struct EncodingOpr {
    char * name;    /*ansi, utf8, utf16le, utf16be*/
    int headlen;
    PT_FontOpr ptFontOprSupportHead;
    int (*getCodeFrmBuf)(unsigned char * pcBufferStart, unsigned char *pucBufferEnd, unsigned int * pdwCode);
    int (*isSuport)(unsigned char * pcBufferHead);
    struct EncodingOpr *pNext;
}T_EncodingOpr, *PT_EncodingOpr;

int registerEncodingOpr( PT_EncodingOpr ptEncodingOpr);
int addFontOprForEncoding(PT_EncodingOpr encodingOpr, PT_FontOpr fontOpr);
PT_EncodingOpr selectOprForFile(unsigned char * pucBuffer);
int delFontOprForEncoding(PT_EncodingOpr encodingOpr, PT_FontOpr fontOpr);
int AscilEncodingInit(void);
int Utf8EncodingInit(void);
int Utf16beInit(void);
int Utf16leInit(void);
int EncodingInit(void);
void showEncodingOpr(void);
int GetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode);
#endif /* _ENCODING_MANAGER_H */