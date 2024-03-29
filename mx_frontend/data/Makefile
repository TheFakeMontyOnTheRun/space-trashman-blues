.DEFAULT_GOAL := default

PACKAGER=packer

CRUNCHER=../../tools/wobj-compiler/target/wobj-compiler-1.0-SNAPSHOT-jar-with-dependencies.jar

DISTILLER=distiller

TRIGGER=trigger

$(CRUNCHER):
	mvn -q -f ../../tools/wobj-compiler/pom.xml clean compile package install

$(PACKAGER):
	rm -f $(PACKAGER)
	$(CXX) -std=c++14 -o$(PACKAGER) packer.cpp

models: $(CRUNCHER)
	ls src/*.obj  | xargs java -jar ../../tools/wobj-compiler/target/wobj-compiler-1.0-SNAPSHOT-jar-with-dependencies.jar

data:	$(PACKAGER)
	#python3 ./check_textures.py
	rm -f ./base.pfs
	ls assets/*.*  | xargs ./packer
	mkdir -p output
	cp ./data.pfs output/base.pfs
	bin2c ../base.pfs ../base3d/include/basepfs.h basepfs
	mv ./data.pfs ../base.pfs

images:
	python3 ./make_package.py

maps:
	python3 ./compile_map.py

full: images maps models data

default: full
