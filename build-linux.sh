#!/bin/sh
set -eux
${CC:-gcc} -Wall -Wextra -pedantic -std=gnu99 -c shm_open_anon.c
${CC:-gcc} -Wall -Wextra -pedantic -std=gnu99 -c test_shared.c
${CC:-gcc} -Wall -Wextra -pedantic -std=gnu99 \
	-o test_shared shm_open_anon.c test_shared.c -lrt
