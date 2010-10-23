
/* Unix header for X11 Win32 implementation */

#ifndef __PWD_H__
#define __PWD_H__

struct passwd
{
	char* pw_name;
	int   pw_uid;
	int   pw_gid;
	char* pw_dir;
	char* pw_shell;
	char* pw_gecos;
};

struct passwd* getpwuid(int uid);
struct passwd* getpwnam(const char *name);

#endif
