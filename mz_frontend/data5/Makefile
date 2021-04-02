.DEFAULT_GOAL := default

packager:
	rm -f base3d/packer
	$(CXX) -std=c++14 -opacker packer.cpp

data:	packager
	rm -f ./base.pfs
	ls assets/*.*  | xargs ./packer
	mv ./data.pfs ../base5.pfs

images:
	python ./make_package.py

full: images data

default: full
