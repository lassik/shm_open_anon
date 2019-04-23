// Copyright 2019 Lassi Kortela
// SPDX-License-Identifier: ISC

#include <sys/types.h>

#include <sys/mman.h>

#ifdef __linux__
#include <linux/unistd.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#undef IMPL_MEMFD
#undef IMPL_POSIX
#undef IMPL_SHM_ANON
#undef IMPL_SHM_MKSTEMP
#undef IMPL_UNLINK_OR_CLOSE

#ifdef __APPLE__
#ifdef __MACH__
#define IMPL_POSIX "/tmp/shmXXXXXXX"
#endif
#endif

#ifdef __DragonFly__
#define IMPL_POSIX "/tmp/shmXXXXXXX"
#endif

#ifdef __NetBSD__
#define IMPL_POSIX "/shmXXXXXXX"
#endif

#ifdef __linux__
#ifdef __NR_memfd_create
#define IMPL_MEMFD
#endif
#endif

#ifdef __FreeBSD__
#define IMPL_SHM_ANON
#endif

#ifdef __OpenBSD__
#define IMPL_SHM_MKSTEMP
#endif

#ifdef IMPL_POSIX
#define IMPL_UNLINK_OR_CLOSE
#endif

#ifdef IMPL_SHM_MKSTEMP
#define IMPL_UNLINK_OR_CLOSE
#endif

#ifdef IMPL_UNLINK_OR_CLOSE
static int
shm_unlink_or_close(const char *name, int fd)
{
	int save;

	if (shm_unlink(name) == -1) {
		save = errno;
		close(fd);
		errno = save;
		return -1;
	}
	return fd;
}
#endif

#ifdef IMPL_POSIX
int
shm_open_anon(void)
{
	char name[16];
	int fd;

	snprintf(name, sizeof(name), "%s", IMPL_POSIX);
	if (mktemp(name) == NULL)
		return -1;
	fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
	if (fd == -1)
		return -1;
	return shm_unlink_or_close(name, fd);
}
#endif

#ifdef IMPL_SHM_MKSTEMP
int
shm_open_anon(void)
{
	char name[16];
	int fd;

	snprintf(name, sizeof(name), "/tmp/shmXXXXXXX");
	if ((fd = shm_mkstemp(name)) == -1)
		return -1;
	return shm_unlink_or_close(name, fd);
}
#endif

#ifdef IMPL_SHM_ANON
int
shm_open_anon(void)
{
	return shm_open(SHM_ANON, O_RDWR, 0);
}
#endif

#ifdef IMPL_MEMFD
int
shm_open_anon(void)
{
	return syscall(__NR_memfd_create, "shm_anon", (unsigned int)0);
}
#endif
