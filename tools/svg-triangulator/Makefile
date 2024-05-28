LDFLAGS=`xml2-config --libs`
CXXFLAGS=`xml2-config --cflags`

OBJ=xml.o trigger.o svg.o

full: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -onewtrigger $(LDFLAGS)

default: full

clean:
	rm -f $(OBJ)
	rm -f newtrigger
