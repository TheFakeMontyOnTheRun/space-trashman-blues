.DEFAULT_GOAL := default

cruncher: 
	mvn -q -f wobj-compiler/pom.xml clean compile package install
	cp wobj-compiler/target/wobj-compiler-1.0-SNAPSHOT-jar-with-dependencies.jar wobj-compiler/dist 

packager:
	rm -f base3d/packer
	$(CXX) -std=c++14 -opacker packer.cpp

data:	packager cruncher
	rm -f ./base.pfs
	ls src/*.obj  | xargs java -jar wobj-compiler/dist/wobj-compiler-1.0-SNAPSHOT-jar-with-dependencies.jar
	ls assets/*.*  | xargs ./packer
	mv ./data.pfs ../base.pfs

images:
	python ./make_package.py

maps:
	python ./compile_map.py

full: images maps data

default: full
