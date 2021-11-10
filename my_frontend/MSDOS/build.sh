#!/usr/bin/env bash
docker run -v $(PWD):/usr/src/app ia16-gpp /usr/bin/ia16-elf-gcc -DDONT_INCLUDE MSDOS/src/game.c -oderelict.com
