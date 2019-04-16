#!/bin/sh
set -eux
${CC:-clang} -Wall -Wextra -pedantic -std=c99 -c shm_open_anon.c
${CC:-clang} -Wall -Wextra -pedantic -std=c99 -c test_shared.c
${CC:-clang} -Wall -Wextra -pedantic -std=c99 \
             -o test_shared shm_open_anon.c test_shared.c
