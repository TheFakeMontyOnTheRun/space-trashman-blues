PACKAGER = ./packer

packager: 
	$(CC) packer.c -opacker

data: $(PACKAGER)
	echo "const uint8_t data[] ={" > ../src/maps.h
	echo "" > ./datastrip
	$(PACKAGER) src/map1.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map2.txt >> ../src/maps.h 2>>./datastrip
	$(PACKAGER) src/map3.txt >> ../src/maps.h 2>>./datastrip
	echo " 0};\n" >> ../src/maps.h


	echo "const long dataPositions[] ={\n" >> ../src/maps.h
	cat ./datastrip >> ../src/maps.h
	echo " 0\n};" >> ../src/maps.h




all: data

default: data

