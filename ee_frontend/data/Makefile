.DEFAULT_GOAL := default

packager:
	rm -f base3d/packer
	$(CXX) -std=c++14 -opacker packer.cpp

data:	packager
	python3 ./check_textures.py
	rm -f ./base.pfs
	ls src/*.obj  | xargs java -jar wobj-compiler/dist/wobj-compiler-1.0-SNAPSHOT-jar-with-dependencies.jar
	ls assets/*.*  | xargs ./packer
	mv ./data.pfs ../base.pfs
	cp ../base.pfs ../filesystem
	bin2c ../base.pfs ../base3d/include/basepfs.h basepfs

images:
	python3 ./make_package.py

maps:
	python3 ./compile_map.py

full: images maps data

default: full
