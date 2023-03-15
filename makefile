CC = gcc
CFLAGS = -Wall
LDFLAGS =

SRCS = main.c linuxCommands.c
OBJS = $(SRCS:.c=.o)

all: shell

shell: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c linuxCommands.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) shell