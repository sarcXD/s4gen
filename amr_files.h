#ifndef AMR_FILES_H
#define AMR_FILES_H

#ifdef __unix__
#include <sys/types.h>
#include <dirent.h>

#define _platform_open_dir opendir
#define _platform_read_dir readdir
#define _platform_make_dir mkdir
#define _platform_file_open fopen
#define _platform_file_write fwrite
#define _platform_file_close fclose
#endif


#endif
