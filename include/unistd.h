
/* Unix header for X11 Win32 implementation */

#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <stdio.h>

#define R_OK (0x04)

typedef unsigned long u_long;
typedef unsigned long uid_t;
typedef unsigned long fd_set;

long random(void);
int srandom(int seed);

uid_t getuid(void);
int gethostname(char *buf, int size);
int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);

void bzero(void *ptr, size_t len);

FILE* popen(const char* command, const char* mode);
int pclose(FILE *pf);

#endif
