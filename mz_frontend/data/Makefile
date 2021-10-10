.DEFAULT_GOAL := default

PACKAGER = ./packer

packager: 
	$(CC) packer.c -opacker

data: $(PACKAGER)
	echo "const uint8_t data[] ={" > ../Engine3D/maps.h
	echo "" > ./datastrip
	$(PACKAGER) src/map1.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map2.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map3.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map4.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map5.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map6.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map7.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map8.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map9.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map10.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map11.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map12.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map13.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map14.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map15.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map16.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map17.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map18.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map19.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map20.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map21.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map22.txt >> ../Engine3D/maps.h 2>>./datastrip
	$(PACKAGER) src/map23.txt >> ../Engine3D/maps.h 2>>./datastrip
	echo " 0};\n" >> ../Engine3D/maps.h



#TODO: precalc absolute offsets
	echo "const long dataPositions[] ={ 0,\n" >> ../Engine3D/maps.h
	cat ./datastrip >> ../Engine3D/maps.h
	echo " 0\n};" >> ../Engine3D/maps.h




all: packager data

default: packager data

