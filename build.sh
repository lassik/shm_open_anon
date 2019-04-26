#!/bin/sh
set -eu
case "$(uname)" in
Darwin)
	default_cc=clang
	default_cflags="-Wall -Wextra -pedantic -std=c99"
	default_lflags=""
	;;
DragonFly)
	default_cc=gcc
	default_cflags="-Wall -Wextra -pedantic -std=c99"
	default_lflags=""
	;;
FreeBSD)
	default_cc=clang
	default_cflags="-Wall -Wextra -pedantic -std=c99"
	default_lflags=""
	;;
Haiku)
	default_cc=gcc
	default_cflags="-Wall"
	default_lflags=""
	;;
Linux)
	default_cc=gcc
	default_cflags="-Wall -Wextra -pedantic -std=gnu99"
	default_lflags="-lrt"
	;;
NetBSD)
	default_cc=gcc
	default_cflags="-Wall -Wextra -pedantic -std=c99"
	default_lflags="-lrt"
	;;
OpenBSD)
	default_cc=clang
	default_cflags="-Wall -Wextra -pedantic -std=c99"
	default_lflags=""
	;;
SunOS)
	default_cc=gcc
	default_cflags="-Wall -Wextra -pedantic -std=c99"
	default_lflags=""
	;;
*)
	echo "Operating system not supported: $(uname)" >&2
	exit 1
	;;
esac
CC="${CC:-$default_cc}"
CFLAGS="${CFLAGS:-$default_cflags}"
LFLAGS="${LFLAGS:-$default_lflags}"
echo "Entering directory '$PWD'"
set -x
$CC $CFLAGS -c shm_open_anon.c
$CC $CFLAGS -c test_shared.c
$CC $LFLAGS -o test_shared test_shared.o shm_open_anon.o
