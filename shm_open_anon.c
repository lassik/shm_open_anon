// Copyright 2019 Lassi Kortela
// SPDX-License-Identifier: ISC

#include <sys/types.h>

#include <sys/mman.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
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
#define IMPL_DEV_SHM
#endif

#ifdef __FreeBSD__
#define IMPL_SHM_ANON
#endif

#ifdef __OpenBSD__
#define IMPL_SHM_MKSTEMP
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

static int fd_without_close_on_exec(int fd)
{
	int flags;

	if ((flags = fcntl(fd, F_GETFD)) == -1)
		return save_errno_and_close(fd);
	flags &= ~FD_CLOEXEC;
	if (fcntl(fd, F_SETFD, flags) == -1)
		return save_errno_and_close(fd);
	return fd;
}

//

#ifdef IMPL_CLASSIC
int shm_open_anon_private(void)
{
	char name[16];
	int fd;

	snprintf(name, sizeof(name), "/tmp/shmXXXXXXXX");
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

	snprintf(name, sizeof(name), "/dev/shm/XXXXXXXX");
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

	snprintf(name, sizeof(name), "/tmp/shmXXXXXXXX");
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

//

int shm_open_anon_shared(void)
{
	return fd_without_close_on_exec(shm_open_anon_private());
}
