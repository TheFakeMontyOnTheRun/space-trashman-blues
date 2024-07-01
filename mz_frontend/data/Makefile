.DEFAULT_GOAL := default

CRUNCHER=wobj-compiler/dist/wobj-compiler-1.0-SNAPSHOT-jar-with-dependencies.jar
PACKAGER=packer

$(PACKAGER):
	rm -f $(PACKAGER)
	$(CXX) -std=c++14 -o$(PACKAGER) packer.cpp

data:	$(PACKAGER)
	rm -f ./base.pfs
	ls -r assets/*.*  | xargs ./packer
	bin2c ../base.pfs ../include/basepfs.h basepfs
	mv ./data.pfs ../base.pfs

images:
	python3 ./make_package.py

maps:
	python3 ./compile_map.py

full: images maps data

default: full
