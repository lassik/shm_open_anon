// Copyright 2019 Lassi Kortela
// SPDX-License-Identifier: ISC

#include <sys/types.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shm_open_anon.h"

#define PROGNAME "test_shared"

static void
diesys(const char *msg)
{
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
	exit(1);
}

static int
fd_without_close_on_exec(int fd)
{
	int flags;

	if ((flags = fcntl(fd, F_GETFD)) == -1)
		diesys("get close-on-exec");
	flags &= ~FD_CLOEXEC;
	if (fcntl(fd, F_SETFD, flags) == -1)
		diesys("set close-on-exec");
	return fd;
}

static int
shm_open_anon_shared(void)
{
        int fd;

        if ((fd = shm_open_anon()) == -1)
                diesys("shm_open_anon()");
	return fd_without_close_on_exec(fd);
}

static void *
map_shared_memory_from_fd(int fd, size_t *out_size)
{
	struct stat st;
	void *buf;
	size_t size;

	if (fstat(fd, &st) == -1) {
		diesys("fstat");
	}
	*out_size = size = (size_t)st.st_size;
	buf = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED) {
		diesys("mmap");
	}
	return buf;
}

static void
parent(void)
{
	char *child_args[3] = { "./" PROGNAME, "child", 0 };
	char *buf;
	size_t bufsize;
	pid_t child;
	int fd, status;

	bufsize = 10 * 1024 * 1024;
	if ((fd = shm_open_anon_shared()) == -1)
		diesys("shm_open_anon_shared");
	if (ftruncate(fd, (off_t)bufsize) == -1)
		diesys("ftruncate");
	buf = map_shared_memory_from_fd(fd, &bufsize);
	memset(buf, 0, bufsize);
	snprintf(buf, bufsize, "hello from parent");
	if ((child = fork()) == -1)
		diesys("fork");
	if (!child) {
		if (dup2(fd, 3) == -1)
			diesys("dup2");
		execv(child_args[0], child_args);
		diesys("execv");
	}
	if (waitpid(child, &status, 0) == -1)
		diesys("cannot wait for child");
	printf("Parent: %s\n", buf);
}

static void
child(void)
{
	char *buf;
	size_t bufsize;

	buf = map_shared_memory_from_fd(3, &bufsize);
	printf("Child: %s\n", buf);
	snprintf(buf, bufsize, "hello from child");
}

int
main(int argc, char **argv)
{
	(void)argv;
	if (argc == 1) {
		parent();
	} else if ((argc == 2) && !strcmp(argv[1], "child")) {
		child();
	} else {
		fprintf(stderr, "usage\n");
		exit(1);
	}
	return 0;
}
