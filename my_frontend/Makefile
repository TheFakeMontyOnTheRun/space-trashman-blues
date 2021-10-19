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
