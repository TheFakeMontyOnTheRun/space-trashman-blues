.DEFAULT_GOAL := default

CRUNCHER=wobj-compiler/dist/wobj-compiler-1.0-SNAPSHOT-jar-with-dependencies.jar
PACKAGER=packer

#$(CRUNCHER):
#	mvn -q -f wobj-compiler/pom.xml clean compile package install
#	mkdir -p wobj-compiler/dist
#	cp wobj-compiler/target/wobj-compiler-1.0-SNAPSHOT-jar-with-dependencies.jar wobj-compiler/dist

$(PACKAGER):
	rm -f $(PACKAGER)
	$(CXX) -std=c++14 -o$(PACKAGER) packer.cpp

#data:	$(PACKAGER) $(CRUNCHER)
data:	$(PACKAGER)
	rm -f ./base.pfs
#	ls -r src/*.obj  | xargs java -jar wobj-compiler/dist/wobj-compiler-1.0-SNAPSHOT-jar-with-dependencies.jar
	ls -r assets/*.*  | xargs ./packer
	bin2c ../base.pfs ../Engine3D/basepfs.h basepfs
	mv ./data.pfs ../base.pfs

images:
	python3 ./make_package.py

maps:
	python3 ./compile_map.py

full: images maps data

default: full
