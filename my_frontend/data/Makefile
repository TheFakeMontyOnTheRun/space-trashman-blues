PACKAGER = ./packer

packager: 
	$(CC) packer.c -opacker

data: $(PACKAGER)
	echo "" > ../src/maps.h
	$(PACKAGER) src/map1.txt >> ../src/maps.h

all: data

default: data

