/* ipc.hpp
 * Copyright by beyondy, 2008-2010
 * All rights reserved.
**/

#ifndef __BEYONDY__IPC_HPP
#define __BEYONDY__IPC_HPP

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <limits>

#ifndef IPC_PERM
#define IPC_PERM (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#endif /* IPC_PEERM */

static key_t n2key(const char *name)
{
	char *endptr;
	key_t retval = strtoul(name, &endptr, 0);

	if (!retval || endptr == name || *endptr != '\0') {
		if ((retval = ftok(name, 'b')) == (key_t)-1) {
			retval = 0;	// TODO: crc32(name);
		}
	}

	return retval;
}

static void msec2timespec(long msec, struct timespec& tspec)
{
	if (msec == -1) { // for ever
		tspec.tv_sec = std::numeric_limits<typeof tspec.tv_sec>::max();
		tspec.tv_nsec = 0;
	}
	else {
#if 1
		struct timeval tv;

		gettimeofday(&tv, NULL);
		suseconds_t usec = tv.tv_usec + (msec % 1000) * 1000;

		tspec.tv_sec = tv.tv_sec + (msec / 1000) + (usec / 1000000);
		tspec.tv_nsec = (usec % 1000000) * 1000;
#else
		clock_gettime(CLOCK_REALTIME, &tspec);	
		uint32_t sec = 0;

		sec = msec / 1000;
		msec = msec - sec * 1000;
		tspec.tv_sec += sec;
		tspec.tv_nsec += (msec * 1000 * 1000);
#endif
	}

	return;
}

#endif /*! __BEYONDY__IPC_HPP */

