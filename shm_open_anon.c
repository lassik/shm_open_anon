// Copyright 2019 Lassi Kortela
// SPDX-License-Identifier: ISC

#define _GNU_SOURCE

#include <sys/types.h>

#include <sys/mman.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//

#undef IMPL_CLASSIC
#undef IMPL_DEV_SHM
#undef IMPL_SHM_ANON
#undef IMPL_SHM_MKSTEMP

#ifdef __APPLE__
#ifdef __MACH__
#define IMPL_CLASSIC
#endif
#endif

#ifdef __NetBSD__
#define IMPL_CLASSIC
#endif

#ifdef __DragonFly__
#define IMPL_CLASSIC
#endif

#ifdef __linux__
#define _GNU_SOURCE
#define IMPL_DEV_SHM
#endif

#ifdef __FreeBSD__
#define IMPL_SHM_ANON
#endif

#ifdef __OpenBSD__
#define IMPL_SHM_MKSTEMP
#endif

//

#ifdef IMPL_DEV_SHM
#ifndef O_TMPFILE
#define O_TMPFILE 0
#endif
#endif

//

static int save_errno_and_close(int fd)
{
	int save;

	save = errno;
	close(fd);
	errno = save;
	return -1;
}

//

#ifdef IMPL_CLASSIC
int shm_open_anon_private(void)
{
	char name[16];
	int fd;

	snprintf(name, sizeof(name), "/tmp/shmXXXXXXX");
	if (mktemp(name) == NULL)
		return -1;
	if ((fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600)) == -1)
		return -1;
	if (shm_unlink(name) == -1)
		return save_errno_and_close(fd);
	return fd;
}
#endif

//

#ifdef IMPL_DEV_SHM
int shm_open_anon_private(void)
{
	char name[16];
	int fd;

	snprintf(name, sizeof(name), "/dev/shm/XXXXXX");
	if ((fd = mkostemp(name, O_CLOEXEC | O_TMPFILE)) == -1)
		return -1;
	if (shm_unlink(name) == -1)
		return save_errno_and_close(fd);
	return fd;
}
#endif

//

#ifdef IMPL_SHM_MKSTEMP
int shm_open_anon_private(void)
{
	char name[16];
	int fd;

	snprintf(name, sizeof(name), "/tmp/shmXXXXXXX");
	if ((fd = shm_mkstemp(name)) == -1)
		return -1;
	if (shm_unlink(name) == -1)
		return save_errno_and_close(fd);
	return fd;
}
#endif

//

#ifdef IMPL_SHM_ANON
int shm_open_anon_private(void)
{
	return shm_open(SHM_ANON, 0, 0);
}
#endif
