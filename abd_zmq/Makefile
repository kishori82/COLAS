CC=gcc
CFLAGS= -lm

TARGET = libabdzmq.a

all:$(TARGET)

SRCS=reader.c server.c

OBJS=$(SRCS:.c=.o)

%.o:%.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET): $(OBJS)
	ar rcs $@  $<

clean:
	rm -rf *.o $(TARGET)
