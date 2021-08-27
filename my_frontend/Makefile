cpc:
	mkdir -p CPCVersion/src
	rm -f -R CPCVersion/obj/*
	cp -f src/map.h CPCVersion/src
	cp -f src/maps.h CPCVersion/src
	cp -f ../core/include/Core.h CPCVersion/src
	cp -f ../core/include/Derelict.h CPCVersion/src
	touch CPCVersion/src/game.c
	cat src/cpc.c > CPCVersion/src/game.c
	cat src/demo.c >> CPCVersion/src/game.c
	cat ../core/src/Core.c >> CPCVersion/src/game.c
	cat ../core/src/Derelict.c >> CPCVersion/src/game.c

sms:
	rm -f 8bitworkshop/*
	rmdir 8bitworkshop
	mkdir 8bitworkshop
	touch 8bitworkshop/game.c
	echo "#include <stdint.h>" > 8bitworkshop/game.c
	echo "#include <stdio.h>" >> 8bitworkshop/game.c
	echo "#include <stdlib.h>" >> 8bitworkshop/game.c
	echo "#include <string.h>" >> 8bitworkshop/game.c
	echo "#include <assert.h>" >> 8bitworkshop/game.c
	echo "#define INCLUDE_ROOM_DESCRIPTIONS" >> 8bitworkshop/game.c
	echo "#define DONT_INCLUDE" >> 8bitworkshop/game.c
	echo "#define CAN_PICK_OBJECT_AT_ANY_DISTANCE" >> 8bitworkshop/game.c
	echo "#define SMS" >> 8bitworkshop/game.c
	cat ../core/include/Core.h >> 8bitworkshop/game.c
	cat ../core/include/Derelict.h >> 8bitworkshop/game.c
	cat ../core/include/Parser.h >> 8bitworkshop/game.c
	cat src/map.h >> 8bitworkshop/game.c
	cat src/maps.h >> 8bitworkshop/game.c
	cat ../core/src/Core.c >> 8bitworkshop/game.c
	cat ../core/src/Derelict.c >> 8bitworkshop/game.c
	cat ../core/src/Parser.c >> 8bitworkshop/game.c
	cat src/demo.c >> 8bitworkshop/game.c
	cat src/sms.c >> 8bitworkshop/game.c

ia16:
	rm -f MSDOS/src/*
	cat src/msdos.c > MSDOS/src/game.c
	cat ../core/include/Core.h >> MSDOS/src/game.c
	cat ../core/include/Derelict.h >> MSDOS/src/game.c
	cat src/map.h >> MSDOS/src/game.c
	cat src/demo.c >> MSDOS/src/game.c
	cat ../core/src/Core.c >> MSDOS/src/game.c
	cat ../core/src/Derelict.c >> MSDOS/src/game.c
	./MSDOS/build.sh
