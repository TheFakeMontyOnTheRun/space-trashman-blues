.DEFAULT_GOAL := default

PACKAGER = ./packer

packager: 
	$(CC) packer.c -opacker

data: $(PACKAGER)
	echo "const uint8_t data[] ={" > ../src/maps.h
	echo "" > ./datastrip
	$(PACKAGER) src/map1.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map2.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map3.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map4.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map5.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map6.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map7.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map8.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map9.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map10.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map11.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map12.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map13.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map14.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map15.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map16.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map17.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map18.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map19.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map20.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map21.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map22.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map23.txt >> ../src/maps.h 2>>./datastrip
	echo " 0};\n" >> ../src/maps.h



#TODO: precalc absolute offsets
	echo "const long dataPositions[] ={ 0,\n" >> ../src/maps.h
	cat ./datastrip >> ../src/maps.h
	echo " 0\n};" >> ../src/maps.h




all: packager data

default: packager data

