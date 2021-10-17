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
	rm -f MasterSystem/bank0.c
	touch MasterSystem/bank0.c
	echo "#include <stdint.h>" > MasterSystem/bank0.c
	echo "#include <stdio.h>" >> MasterSystem/bank0.c
	echo "#include <stdlib.h>" >> MasterSystem/bank0.c
	echo "#include <string.h>" >> MasterSystem/bank0.c
	echo "#include <assert.h>" >> MasterSystem/bank0.c
	echo "#define INCLUDE_ROOM_DESCRIPTIONS" >> MasterSystem/bank0.c
	echo "#define DONT_INCLUDE" >> MasterSystem/bank0.c
	echo "#define CAN_PICK_OBJECT_AT_ANY_DISTANCE" >> MasterSystem/bank0.c
	cat ../core/include/Core.h >> MasterSystem/bank0.c
	cat ../core/include/Derelict.h >> MasterSystem/bank0.c
	cat ../core/include/Parser.h >> MasterSystem/bank0.c
	cat src/map.h >> MasterSystem/bank0.c
	cat src/maps.h >> MasterSystem/bank0.c
	cat ../core/src/Core.c >> MasterSystem/bank0.c
	cat ../core/src/Derelict.c >> MasterSystem/bank0.c
	cat ../core/src/Parser.c >> MasterSystem/bank0.c
	cat src/demo.c >> MasterSystem/bank0.c
	cat src/sms.c >> MasterSystem/bank0.c

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
