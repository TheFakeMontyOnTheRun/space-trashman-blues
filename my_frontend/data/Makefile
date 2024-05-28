.DEFAULT_GOAL := default

RLE_COMPRESSOR=./rlecompressor
PACKAGER=packer

datastrip_data: $(RLE_COMPRESSOR)
	echo "const uint8_t data[] ={" > ../include/map-data.h
	echo "" > ./datastrip
	$(RLE_COMPRESSOR) assets/01.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/02.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/03.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/04.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/05.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/06.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/07.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/08.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/09.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/10.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/11.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/12.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/13.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/14.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/15.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/16.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/17.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/18.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/19.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/20.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/21.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/22.txt >> ../include/map-data.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/23.txt >> ../include/map-data.h 2>>./datastrip
	echo " 0};\n" >> ../include/map-data.h
#TODO: precalc absolute offsets
	echo "const uint16_t dataPositions[] ={ 0,\n" >> ../include/map-data.h
	cat ./datastrip >> ../include/map-data.h
	echo " 0\n};" >> ../include/map-data.h
	rm ./datastrip
	rm $(RLE_COMPRESSOR)

$(PACKAGER):
	$(CXX) -std=c++14 -o$(PACKAGER) packer.cpp

pfs_data:	$(PACKAGER) $(CRUNCHER)
	rm -f ./base.pfs
	ls -r src/*.obj  | xargs java -jar wobj-compiler/dist/wobj-compiler-1.0-SNAPSHOT-jar-with-dependencies.jar
	ls -r assets/*.*  | xargs ./packer
	mv ./data.pfs ../base.pfs
	rm -f $(PACKAGER)

images:
	python3 ./make_package.py

all: $(RLE_COMPRESSOR) datastrip_data images pfs_data

default: all

