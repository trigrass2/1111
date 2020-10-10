/*------------------------------------------------------------------------
 Module:        fileext.h
 Author:        solar
 Description:   ANSI文件操作接口
------------------------------------------------------------------------*/

#ifndef _FILEEXT_H
#define _FILEEXT_H

#include <sys/stat.h>
#include <dirent.h>  
#include <fcntl.h> 

#if 0
#ifdef RT_USING_DFS
#include "dfs_posix.h"
#ifndef SEEK_SET
#define	SEEK_SET	0	/* set file offset to offset */
#endif

#ifndef SEEK_CUR
#define	SEEK_CUR	1	/* set file offset to current plus offset */
#endif

#ifndef SEEK_END
#define	SEEK_END	2	/* set file offset to EOF plus offset */
#endif

#endif

#ifdef RT_USING_YAFFS2
#include "yaffsfs.h"
#ifdef RT_USING_MTD_NOR
#include "yaffs_nor_flash.h"
#endif
#ifdef RT_USING_MTD_NAND
#include "yaffs_nand_flash.h"
#endif

struct	stat
{
    DWORD	st_mode;	/* file mode (see below) */
    DWORD	st_size;	/* size of file, in bytes */
    DWORD	st_mtime;	/* time of last modification */
};

typedef void DIR;

struct dirent
{
    char d_name[NAME_MAX+1];   /* The null-terminated file name */
};

int stat(const char * name, struct stat * pStat);
int mkdir(char *dirName, int flag);
DIR *opendir(char * dirName );
struct dirent *readdir(DIR * pDir);
int closedir(DIR * pDir);
#endif


FILE *	fopen (const char *, const char *);  
int	fclose (FILE *);
size_t	fread (void *, size_t, size_t, FILE *);
size_t	fwrite (const void *, size_t, size_t, FILE *);
int	fseek (FILE *, long, int);
int lfseek(FILE *stream, long offset, int origin );

#undef feof
int	feof (FILE *);

unsigned long freespace(const char *path);
unsigned long totalspace(const char *path);

int	cp  (const char *, const char *);
int format_disk(const char *path);
int erase_disk(const char *path);
#endif

#endif

