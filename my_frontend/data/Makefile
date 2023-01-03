.DEFAULT_GOAL := default

RLE_COMPRESSOR=./rlecompressor
CRUNCHER=wobj-compiler/dist/wobj-compiler-1.0-SNAPSHOT-jar-with-dependencies.jar
PACKAGER=packer

datastrip_data: $(RLE_COMPRESSOR)
	echo "const uint8_t data[] ={" > ../src/maps.h
	echo "" > ./datastrip
	$(RLE_COMPRESSOR) assets/01.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/02.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/03.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/04.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/05.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/06.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/07.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/08.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/09.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/10.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/11.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/12.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/13.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/14.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/15.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/16.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/17.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/18.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/19.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/20.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/21.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/22.txt >> ../src/maps.h 2>>./datastrip
	$(RLE_COMPRESSOR) assets/23.txt >> ../src/maps.h 2>>./datastrip
	echo " 0};\n" >> ../src/maps.h
#TODO: precalc absolute offsets
	echo "const long dataPositions[] ={ 0,\n" >> ../src/maps.h
	cat ./datastrip >> ../src/maps.h
	echo " 0\n};" >> ../src/maps.h
	rm ./datastrip
	rm $(RLE_COMPRESSOR)

$(CRUNCHER):
	mvn -q -f wobj-compiler/pom.xml clean compile package install
	mkdir -p wobj-compiler/dist
	cp wobj-compiler/target/wobj-compiler-1.0-SNAPSHOT-jar-with-dependencies.jar wobj-compiler/dist

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

