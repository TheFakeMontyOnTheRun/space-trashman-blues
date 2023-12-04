CXXFLAGS=-Iinclude

OBJS=src/editor-core.o

MAIN=cli/cli-main.o

TARGET=cli-editor

$(TARGET): $(MAIN) $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(MAIN) -o$(TARGET)


all: $(TARGET)


clean:
	rm -f $(TARGET) $(MAIN) $(OBJS)
