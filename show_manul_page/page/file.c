#include "pic_operation.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "config.h"
#include <sys/mman.h>
#include "file.h"
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int MapFile(PT_FileInfo ptFileInfo)
{
    int iFd;
    struct stat tFileStat;
	FILE * tFp;

    tFp = fopen(ptFileInfo ->strFileName, "r+");
    if (tFp == NULL)
    {
        DBG_PRINTF("open %s failed\r\n",ptFileInfo ->strFileName);
        return -1;
    }

	ptFileInfo->tFp = tFp;

	iFd = fileno(tFp);

    fstat(iFd, &tFileStat);
    ptFileInfo->iFileSize = tFileStat.st_size;
    

    ptFileInfo -> pucFileMem = mmap(NULL, tFileStat.st_size, PROT_READ, MAP_SHARED, iFd, 0);
    if(ptFileInfo -> pucFileMem == (void *) -1)
    {
        DBG_PRINTF("mmap %s failed\r\n", ptFileInfo ->strFileName);
        return -1;
    }

    return 0;
}

void MunmapFile(PT_FileInfo ptFileInfo)
{
    munmap(ptFileInfo -> pucFileMem, ptFileInfo-> iFileSize);
    fclose(ptFileInfo ->tFp);
}

static int isDir(char *strFilePath, char *strFileName)
{
    char strTmp[256];
    struct stat tStat;

    snprintf(strTmp, 256, "%s/%s", strFilePath, strFileName);
    strTmp[255] = '\0';

    if ((stat(strTmp, &tStat) == 0) && S_ISDIR(tStat.st_mode))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static int isRegFile(char *strFilePath, char *strFileName)
{
    char strTmp[256];
    struct stat tStat;

    snprintf(strTmp, 256, "%s/%s", strFilePath, strFileName);
    strTmp[255] = '\0';

    if ((stat(strTmp, &tStat) == 0) && S_ISREG(tStat.st_mode))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}



/* 把某目录下所含的顶层子目录、顶层目录下的文件都记录下来 */
int GetDirContents(char *strDirName, PT_DirContent **pptDirContents, int *piNumber)	
{
    PT_DirContent *aptDirContents;
	struct dirent **aptNameList;
	int iNumber;
	int i;
	int j;


	/* 扫描目录,结果按名字排序,存在aptNameList[0],aptNameList[1],... */
	iNumber = scandir(strDirName, &aptNameList, 0, alphasort);
	if (iNumber < 0)
	{
		DBG_PRINTF("scandir error : %s!\n", strDirName);
		return -1;
	}

	/* 忽略".", ".."这两个目录 */
	aptDirContents = malloc(sizeof(PT_DirContent) * (iNumber - 2));
	if (NULL == aptDirContents)
	{
		DBG_PRINTF("malloc error!\n");
		return -1;
	}
    *pptDirContents = aptDirContents;

	for (i = 0; i < iNumber - 2; i++)
	{
		aptDirContents[i] = malloc(sizeof(T_DirContent));
		if (NULL == aptDirContents)
		{
			DBG_PRINTF("malloc error!\n");
			return -1;
		}
	}

	/* 先把目录挑出来存入aptDirContents */
	for (i = 0, j = 0; i < iNumber; i++)
	{
		/* 忽略".",".."这两个目录 */
		if ((0 == strcmp(aptNameList[i]->d_name, ".")) || (0 == strcmp(aptNameList[i]->d_name, "..")))
			continue;
        /* 并不是所有的文件系统都支持d_type, 所以不能直接判断d_type */
		/* if (aptNameList[i]->d_type == DT_DIR) */
        if (isDir(strDirName, aptNameList[i]->d_name))
		{
			strncpy(aptDirContents[j]->strName, aptNameList[i]->d_name, 256);
			aptDirContents[j]->strName[255] = '\0';
			aptDirContents[j]->eFileType    = FILETYPE_DIR;
			aptDirContents[j] ->isPicture = 0;
            free(aptNameList[i]);
            aptNameList[i] = NULL;
			j++;
		}
	}
	DBG_PRINTF("%s %s %d\r\n", __FILE__, __FUNCTION__, __LINE__);

	/* 再把常规文件挑出来存入aptDirContents */
	for (i = 0; i < iNumber; i++)
	{
        if (aptNameList[i] == NULL)
            continue;
        
		/* 忽略".",".."这两个目录 */
		if ((0 == strcmp(aptNameList[i]->d_name, ".")) || (0 == strcmp(aptNameList[i]->d_name, "..")))
			continue;
        /* 并不是所有的文件系统都支持d_type, 所以不能直接判断d_type */
		/* if (aptNameList[i]->d_type == DT_REG) */
        if (isRegFile(strDirName, aptNameList[i]->d_name))
		{
			strncpy(aptDirContents[j]->strName, aptNameList[i]->d_name, 256);
			aptDirContents[j]->strName[255] = '\0';

			if(strstr(aptDirContents[j]->strName, ".jpg") != NULL || strstr(aptDirContents[j]->strName, ".bmp") != NULL )		
				/*文件是图片*/
				aptDirContents[j] ->isPicture = 1;
			else
				aptDirContents[j] ->isPicture = 0;
			aptDirContents[j]->eFileType    = FILETYPE_FILE;

            free(aptNameList[i]);
            aptNameList[i] = NULL;
			j++;
		}
	}

	/* 释放aptDirContents中未使用的项 */
	for (i = j; i < iNumber - 2; i++)
	{
		free(aptDirContents[i]);
	}

	/* 释放scandir函数分配的内存 */
	for (i = 0; i < iNumber; i++)
	{
        if (aptNameList[i])
        {
    		free(aptNameList[i]);
        }
	}
	free(aptNameList);

	*piNumber = j;

	for(i = 0; i < j; i++)
	{
		DBG_PRINTF("%s\r\n",aptDirContents[i]->strName);
	}
	
	return 0;
}