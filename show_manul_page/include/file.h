#ifndef FILE_H
#define FILE_H

#include <stdbool.h>

typedef struct  tFileInfo
{
    //int iFileFd;
    FILE * tFp;
    unsigned char* pucFileMem;
    int iFileSize;
    char strFileName[256];
}T_FileInfo, *PT_FileInfo;

typedef enum {
	FILETYPE_DIR = 0,
	FILETYPE_FILE,
}E_FileType;



typedef struct DirContent {
	char strName[256];
	E_FileType eFileType;
    bool isPicture;
}T_DirContent, *PT_DirContent;

int MapFile(PT_FileInfo ptFileInfo);
void MunmapFile(PT_FileInfo ptFileInfo);
int GetDirContents(char *strDirName, PT_DirContent **pptDirContents, int *piNumber)	;

#endif // ! FILE_H

