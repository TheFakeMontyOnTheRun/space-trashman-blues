cpc:
	rm -f CPCVersion/src/*
	rm -f -R CPCVersion/obj/*
	cp -f src/map.h CPCVersion/src
	cp -f ../core/include/Core.h CPCVersion/src
	cp -f ../core/include/Derelict.h CPCVersion/src
	cat src/cpc.c > CPCVersion/src/game.c
	cat src/demo.c >> CPCVersion/src/game.c
	cat ../core/src/Core.c >> CPCVersion/src/game.c
	cat ../core/src/Derelict.c >> CPCVersion/src/game.c

sms:
	rm -f 8bitworkshop/*
	cat src/sms.c > 8bitworkshop/game.c
	cat ../core/include/Core.h >> 8bitworkshop/game.c
	cat ../core/include/Derelict.h >> 8bitworkshop/game.c
	cat src/map.h >> 8bitworkshop/game.c
	cat src/demo.c >> 8bitworkshop/game.c
	cat ../core/src/Core.c >> 8bitworkshop/game.c
	cat ../core/src/Derelict.c >> 8bitworkshop/game.c
