#!/bin/sh
set -eu
cd "$(dirname "$0")"
default_cc=gcc
default_cflags="-Og -g -Wall -Werror -Wextra -pedantic -std=gnu99"
default_lflags=""
case "$(uname)" in
Darwin) default_cc=clang ;;
DragonFly) ;;
FreeBSD) default_cc=clang ;;
Haiku) default_cflags="-g -Wall -Werror" ;;
Linux) default_lflags="-lrt" ;;
NetBSD) default_lflags="-lrt" ;;
OpenBSD) default_cc=clang ;;
SunOS) ;;
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
