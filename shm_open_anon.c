// Copyright 2019 Lassi Kortela
// SPDX-License-Identifier: ISC

#ifdef __linux__
#define _GNU_SOURCE
#include <linux/unistd.h>
#endif

#include <sys/types.h>

#include <sys/mman.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#undef IMPL_MEMFD
#undef IMPL_POSIX
#undef IMPL_SHM_ANON
#undef IMPL_SHM_MKSTEMP
#undef IMPL_UNLINK_OR_CLOSE

#ifdef __linux__
#ifdef __NR_memfd_create
#define IMPL_MEMFD
#endif
#endif

#ifdef __FreeBSD__
#define IMPL_SHM_ANON
#endif

#ifdef __HAIKU__
#define IMPL_POSIX "/shm-"
#endif

#ifdef __NetBSD__
#define IMPL_POSIX "/shm-"
#endif

#ifdef __APPLE__
#ifdef __MACH__
#define IMPL_POSIX "/shm-"
#endif
#endif

#ifdef __sun
#define IMPL_POSIX "/shm-"
#endif

#ifdef __DragonFly__
#define IMPL_POSIX "/tmp/shm-"
#endif

#ifdef __OpenBSD__
#define IMPL_SHM_MKSTEMP "/shm-"
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
	const char alphabet[] = "0123456789"
	                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	                        "abcdefghijklmnopqrstuvwxyz";
	char name[16] = IMPL_POSIX;
	char *p;
	size_t n;
	int fd;

	if ((fd = open("/dev/random", O_RDONLY)) == -1) {
		return -1;
	}
	errno = 0;
	p = strchr(name, 0);
	n = sizeof(name) - 1 - (p - name);
	if (read(fd, p, n) != (ssize_t)n) {
		return -1;
	}
	if (close(fd) == -1) {
		return -1;
	}
	while (p < name + sizeof(name) - 1) {
		*p = alphabet[*p % strlen(alphabet)];
		p++;
	}
	*p = 0;
	fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
	if (fd == -1)
		return -1;
	return shm_unlink_or_close(name, fd);
}
#endif

#ifdef IMPL_SHM_MKSTEMP
int
pshm_open_anon(void)
{
	char name[16] = IMPL_SHM_MKSTEMP "XXXXXXXXXX";
	int fd;

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
	return syscall(
	  __NR_memfd_create, "shm_anon", (unsigned int)(MFD_CLOEXEC));
}
#endif
