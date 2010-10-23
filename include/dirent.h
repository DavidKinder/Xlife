
/* Unix header for X11 Win32 implementation */

#ifndef __DIRENT_H__
#define __DIRENT_H__

#include "unistd.h"

typedef unsigned long DIR;

struct dirent
{
  char d_namlen;
  char d_name[260];
};

int closedir(DIR* dir);
DIR* opendir(const char *dirname);
struct dirent* readdir(DIR *dir);

#endif
