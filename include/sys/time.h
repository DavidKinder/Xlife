
/* BSD Unix header for X11 Win32 implementation */

#ifndef __SYS_TIME_H__
#define __SYS_TIME_H__

struct timeval
{
	long tv_sec;
	long tv_usec;
};

struct timezone
{
	int tz_minuteswest;
	int tz_dsttime;
};

int gettimeofday(struct timeval *tp, struct timezone *tzp);

#endif
