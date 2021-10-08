CFLAGS = -g -c -DDONT_INCLUDE

CC = ia16-elf-gcc

OBJS = src/game.c

TARGET = derelict.com

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
	rm *~
