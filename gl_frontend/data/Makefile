.DEFAULT_GOAL := default

PACKAGER=packer

CRUNCHER=../../tools/wobj-compiler/target/wobj-compiler-1.0-SNAPSHOT-jar-with-dependencies.jar

$(CRUNCHER):
	mvn -q -f ../../tools/wobj-compiler/pom.xml clean compile package install

$(PACKAGER):
	rm -f $(PACKAGER)
	$(CXX) -std=c++14 -o$(PACKAGER) packer.cpp

models: $(CRUNCHER)
	ls src/*.obj  | xargs java -jar ../../tools/wobj-compiler/target/wobj-compiler-1.0-SNAPSHOT-jar-with-dependencies.jar

data:	$(PACKAGER)
	rm -f ./base.pfs
	ls assets/*.*  | xargs ./packer
	mv ./data.pfs ../base.pfs
	cp ../base.pfs ../filesystem

images:
	python3 ./make_package.py

maps:
	python3 ./compile_map.py

full: images maps models data

default: full
